/**
 *@file RigidBodyAdapter.cc
 *@brief 
 */
#include <iostream>
#include <fstream>
#include <string>
#include <BVL/motion/pose/absolute_orient.h>
#include "RigidBodyAdapter.h"

using namespace std;
//using namespace VML;
using namespace BVL;

RigidBodyAdapter::RigidBodyAdapter()
{
    future_steps = 2;
}

RigidBodyAdapter::~RigidBodyAdapter()
{
    for(int i=0; i<num_markers; ++i)
        delete filters[i];
}

int RigidBodyAdapter::load(const char *filename)
{
    char *ndi_path=getenv("ND_DIR");
    string fullname=ndi_path;
    fullname +="/rigid/";
    fullname += filename;
    fullname +=".rig";

    ifstream ifs(fullname.c_str());
    if(!ifs) {
        cerr << "Can't open " << fullname << "\n";
        return 0;
    }

    string line;
    // the first line is file tile
    getline(ifs, line);// cerr << "1 " << line << "\n";
    // the secend line is empty;
    getline(ifs, line);// cerr << "2 " << line << "\n";
    // the 3rd line is "Real 3D"
    getline(ifs, line);// cerr << "3 " << line << "\n";
    // now it's the number of markers
    ifs >> num_markers;
    // rest of the line
    getline(ifs, line);// cerr << "4 " << num_markers << line << "\n";
    int num_views;
    ifs >> num_views;
    getline(ifs, line);// cerr << "5 " << num_views << line << "\n";
    // FRONT
    getline(ifs, line);// cerr << "6 " << line << "\n";
    // Markers
    getline(ifs, line);// cerr << "7 " << line << "\n";
    for(int i=0; i<num_markers; ++i) {
        int n;
        double x[3];
        int v;
        ifs >> n>> x[0] >> x[1] >> x[2] >> v;
        Vector<double> p(3, x);
        cerr << "markers "<<n << " " << p << " " << v << "\n";
        x_rigidbody.push_back(p);
        x_opto.push_back(p);//just a placement, not really p itself
        x_filtered.push_back(p);//just a placement, not really p itself
    }

    // ignore the rest of the file
    return num_markers;
}

void RigidBodyAdapter::setup_filters(double pos_noise, double vel_noise, double accel_noise)
{
    /* The plant is
     *          ( 1 1 1/2)
     * x(n+1) = ( 0 1 1  ) x(n) + Q
     *          ( 0 0 1  )
     * 
     * and the measurement is
     *
     * y(n) = ( 1 0 0 ) x(n) + R
     *
     */
    BVL::Matrix<double> F(9, 9, 
            "1 1 0.5 0 0 0   0 0 0  "
            "0 1 1   0 0 0   0 0 0  "
            "0 0 1   0 0 0   0 0 0  "
            "0 0 0   1 1 0.5 0 0 0  "
            "0 0 0   0 1 1   0 0 0  "
            "0 0 0   0 0 1   0 0 0  "
            "0 0 0   0 0 0   1 1 0.5 "
            "0 0 0   0 0 0   0 1 1  " 
            "0 0 0   0 0 0   0 0 1  "
            );
    BVL::Matrix<double> H(3, 9, 
            "1 0 0 0 0 0 0 0 0 "
            "0 0 0 1 0 0 0 0 0 "
            "0 0 0 0 0 0 1 0 0 "
            );
    BVL::Matrix<double> Q(9,9, 0.);
    Q(1,1) = pos_noise;
    Q(2,2) = vel_noise;
    Q(3,3) = accel_noise;
    Q(4,4) = pos_noise;
    Q(5,5) = vel_noise;
    Q(6,6) = accel_noise;
    Q(7,7) = pos_noise;
    Q(8,8) = vel_noise;
    Q(9,9) = accel_noise;

    BVL::Matrix<double> R(3,3, 0.);
    R(1,1) = 0.1; //measurement_error
    R(2,2) = 0.1;
    R(3,3) = 0.1;
    for(int i=0; i<num_markers; ++i ) {

        BVL::KalmanFilter *kf=new BVL::KalmanFilter(F, Q, H, R);
        filters.push_back(kf);
    }
}
void RigidBodyAdapter::init_filters(const BVL::Vector<double> &x0, const BVL::Matrix<double> &P0)
{
    for(int i=0; i<filters.size(); ++i)
        filters[i]->init(x0, P0);
}

// pass the raw marker position
void RigidBodyAdapter::set_marker_position(double x, double y, double z, int n)
{

    if(fabs(x) > 1e10)
        return;
    
    x_opto[n](1) = x;
    x_opto[n](2) = y;
    x_opto[n](3) = z;

    // run the filter on this marker;
    filters[n]->run(x_opto[n]);
    Vector<double> future_state = filters[n]->get_future_state(future_steps);
    x_filtered[n](1)= future_state(1);
    x_filtered[n](2)= future_state(4);
    x_filtered[n](3)= future_state(7);

    velo_x = future_state(2);
    velo_y = future_state(5);
    velo_z = future_state(8);
}

void RigidBodyAdapter::get_filtered_velocity(double &vx,double &vy,double &vz) const
{
    vx = velo_x;
    vy = velo_y;
    vz = velo_z;
}

void RigidBodyAdapter::compute_pose()
{
    absolute_orientation(x_rigidbody, x_opto, rot, translation);
//    cerr << "rot " << rot << "\n";
    absolute_orientation(x_rigidbody, x_filtered, rot_filtered, translation_filtered);
 //   cerr << "rot filetered " << rot_filtered << "\n";
}

