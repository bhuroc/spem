/**
 *@file Trajectory.cc
 *@brief 
 */
#include <vector>
#include <algorithm>
#include <memory>
#include <VML/Robot/RobotTransforms.h>
#include <BVL/math/linalg/linalg.h>
#include "Trajectory.h"

using namespace std;
using namespace VML;
using namespace BVL;

// The programmer's manual Fig 4-4.  In Control set 0 and load
// 5kg, the speed is about (300-150)/(0.6-0.4)=750 mm/s
// XXX These 2 number will be from the mgr
const double max_speed = 750.;
const double tool_safe_margin = 50.;

Vector<double> Trajectory::safe_guard(const Vector<double> &p)
{
    const double min_x = -290;
    const double max_x = 290;
    const double min_y = 600-300-10;
    const double max_y = 600+300-10;
    const double min_z = 400-450+10;
    const double max_z = 400+450+10;

    Vector<double> safe_p(p);
    safe_p(3) -= tool_safe_margin;
    if(safe_p(1) < min_x)
        safe_p(1) = min_x;
    if(safe_p(1) > max_x)
        safe_p(1) = max_x;

    if(safe_p(2) < min_y)
        safe_p(2) = min_y;
    if(safe_p(2) > max_y)
        safe_p(2) = max_y;

    if(safe_p(3) < min_z)
        safe_p(3) = min_z;
    if(safe_p(3) > max_z)
        safe_p(3) = max_z;

    return safe_p;
}


Trajectory::Trajectory(const RobotTransforms *r):robot_xform(r)
{
    sampling_duration = 100; // ms

}

Vector<double> Trajectory::compute_velocity(vector<RobotTrajectoryNode>::const_iterator &s)
{
    Matrix<double> tm(5,3,1.);
    Vector<double> x(5,0.), y(5, 0.), z(5, 0.);
    // get the 2-neighbors
    // fits a quadratic to position-time
    // x = a t^2 + b t + c
    // so 
    // v = 2a t
    Vector<double> trans;
    for(int i=1; i<=5; ++i) {
        trans = (s-3+i)->pose.get_translation();
        tm(i,2) = (s-3+i)->time_stamp;
        tm(i,1) = tm(i,2) * tm(i,2); 

        x(i) = trans(1);
        y(i) = trans(2);
        z(i) = trans(3);
    }

    // fit a quadratic to x, y and z individually
    Vector<double> xq = lapack_lls_qr_linear_solve(tm, x);
    Vector<double> yq = lapack_lls_qr_linear_solve(tm, y);
    Vector<double> zq = lapack_lls_qr_linear_solve(tm, z);

    // compute the velocity
    Vector<double> vel(3);
    vel(1) = 2 * xq(1) * s->time_stamp+xq(2) * 1000.;
    vel(2) = 2 * yq(1) * s->time_stamp+yq(2) * 1000.;
    vel(3) = 2 * zq(1) * s->time_stamp+zq(2) * 1000.;

    // the speed (mm/s)
    return vel;
}

class FutureNode {
    public:
        FutureNode(vector<RobotTrajectoryNode>::const_iterator i,double t):it(i), future(t) {}
        bool operator() (const RobotTrajectoryNode &n) const {
            return (n.time_stamp - it->time_stamp) > future;
        }
    private:
        vector<RobotTrajectoryNode>::const_iterator it;
        double future;
};

double Trajectory::set_raw_trajectory(const vector<RobotTrajectoryNode> &t)
{

    trajectory.clear();
    double epoch=t.begin()->time_stamp;

    //cerr << "setting raw trajectory (length "<<t.size() << ")\n";

    vector<RobotTrajectoryNode>::const_iterator it=t.begin();
    double avg_speed = 0.;
    // cook the raw trajectory
    while (it!=t.end()) {

        // subsampling at every 
        vector<RobotTrajectoryNode>::const_iterator s;
        FutureNode fn(it, sampling_duration);
        s=find_if(it, t.end(),fn); 

        // there's also a (remote) possiblity that 
        // s is very close to the beginning
        if(s<t.begin()+2) {
            cerr << __FUNCTION__<< " s is too close to the beginning.\n";
            abort();
        }

        // because we need at 3 point down the vector to computer velocity
        if(s >= t.end()-3) 
            break;

        //cerr << "found a node "<< sampling_duration <<"ms away at " << s-t.begin() << " relative "<<s-it << " time_stamp " << s->time_stamp << " ";

        // the pose in s is in screen coord, transform to robot's coord
        Pose p_rob;

#ifdef TEST
        // for testing only, so we can compare the sub-sampled trajectory
        // with the original one
        p_rob = s->pose;
#else
        robot_xform->screen_to_robot(s->pose, p_rob);
#endif

        // compute the speed (velocity depends on the transformation, but
        // not speed, so we can just compute speed in the given frame.)
        double v = norm(compute_velocity(s));
        avg_speed += v;
        //cerr << "and speed is " << v << "\n";

        //Vector<double> bot_p = p_rob.get_translation();
        //bot_p(3) -= tool_safe_margin;
#ifdef TEST 
        Vector<double> bot_p = p_rob.get_translation();
#else
        Vector<double> bot_p = safe_guard(p_rob.get_translation());
#endif
        p_rob.set_translation(bot_p);
        RobotTrajectoryNode r;
        r.pose = p_rob;
#ifdef TEST
        r.speed = v;
#else
        // can't exceed 50% of the speed
        r.speed = std::min(v/max_speed * 100.,50.);
        if(r.speed < 5) {
            //cerr << "too slow, set it to 5%\n";
            r.speed = 5;
        }
#endif
        r.figure = 3; // default figure
        r.time_stamp = s->time_stamp-epoch;
        trajectory.push_back(r);

        // also compute optimal seg_length???
        // the average speed
        it = s;
    }
    avg_speed /= trajectory.size();
    //cerr << "avg speed is " << avg_speed << "\n";

    // point the "screen center" to the first point
    screen_center = trajectory.begin()->pose;
    //cerr << "setting starting point " << screen_center << "\n";

    // upon receiving a new trajectory, reset the sampler
    trajectory_sampler->reset();

    return avg_speed/max_speed * 100.;
}

void Trajectory::search_for_next_segment(vector<RobotTrajectoryNode> &segment, double t0)
{
    trajectory_sampler->search_for_next_segment(segment, t0);
}

Pose Trajectory::get_starting_point() const
{
    return screen_center;
}

