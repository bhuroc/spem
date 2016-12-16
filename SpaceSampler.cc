/**
 *@file SpaceSampler.cc
 *@brief 
 */
#include <iostream>
#include <BVL/math/linalg/linalg.h>
#include "SpaceSampler.h"

using namespace  std;
using namespace  BVL;
using namespace  VML;

Vector<double> SpaceSampler::compute_velocity(vector<RobotTrajectoryNode>::iterator s)
{
    if(trajectory.end()-s <= 1)
        return Vector<double>(3, 0.);
    Vector<double> x0=s->pose.get_translation();
    Vector<double> x1=(s+1)->pose.get_translation();
    double dt=(s+1)->time_stamp-s->time_stamp;
    return (x1-x0)/dt*1000.;
}

Vector<double> SpaceSampler::compute_velocity_direction(vector<RobotTrajectoryNode>::iterator s)
{
    if(trajectory.end()-s <= 2)
        return Vector<double>(3, 0.);

    Vector<double> x0=s->pose.get_translation();
    vector<RobotTrajectoryNode>::iterator s1=s;
    Vector<double> x1=s1->pose.get_translation();
    double d;
    do {
        ++s1;
        if(s1==trajectory.end()) {
            cerr << "Unlikely, happened anyway.\n";
            break;
        }
        x1 = s1->pose.get_translation();
        d=norm(x1-x0);
    } while((d < 1.) && (s1!=trajectory.end()));

    if(s1!=trajectory.end())
        return x1-x0;
    else
        return Vector<double>(3,0.);
}

double SpaceSampler::compute_distance(vector<RobotTrajectoryNode>::iterator i1, vector<RobotTrajectoryNode>::iterator i2)
{
    Vector<double> x0=i1->pose.get_translation();
    Vector<double> x1=i2->pose.get_translation();
    return norm(x1-x0);
}

void SpaceSampler::search_for_next_segment(vector<RobotTrajectoryNode> &segment, double t0)
{
    segment.clear();
    if(turning_point == trajectory.end())
        return;

    double d_max = 1., d;

    // compute the velocity of the turning point
    BVL::Vector<double> v0 = compute_velocity_direction(turning_point);
    cerr <<"starting from "<<turning_point->time_stamp<< " v0 " << v0 << "\n";
    cerr << "end is " << (trajectory.end()-1)->time_stamp << "\n";

    // find the farthest point whose velocity is on the
    // same side of v0
    double t_start=-1, t_end=-1;
    vector<RobotTrajectoryNode>::iterator h=turning_point+1;
    while(h!=trajectory.end()) {
        BVL::Vector<double> v=compute_velocity_direction(h);
        BVL::Vector<double> vreal=compute_velocity(h);
        d = compute_distance(turning_point, h);
        cerr << "searching..."<<h->time_stamp << " v " << v << " " << d << " (d max "<<d_max << ")\n";
        
        double spd=norm(vreal);
        h->speed = spd;

        if(dot_prod(v, v0) < 0. && d >= d_max) {
            //cerr << "jumping out\n";
            break;
        }

        if(d>d_max)
            d_max = d;
        ++h;
    }

    vector<RobotTrajectoryNode>::iterator last;
    // h is either the end or the reflection point
    if(h != trajectory.end()) {
        last = h;
    }else
        last = h-1;

    // find the first and last node whose speed is significantly
    // greater than 0
    vector<RobotTrajectoryNode>::iterator t_move=turning_point;
    while(t_move != last) {
        if (t_move->speed >= 3) {
            t_start = t_move->time_stamp;
            break;
        }
        ++t_move;
    }
    t_move = last;
    while(t_move != turning_point) {
        if (t_move->speed >= 3) {
            t_end = t_move->time_stamp;
            break;
        }
        --t_move;
    }

    // can't use last here: inc might be 0
    int inc=(h-turning_point)/segment_length;
    //if inc is 0, it means the number of point in this segment
    // is less than segment_length, in which case, it should be deemed
    // to be too short a segment.  If h is approaching the end
    // We'll ignore it.
    if(inc == 0) {
        if( trajectory.end()-h <= 10) {
            cerr << "segment too short, not filling segment and return.\n";
            turning_point = h;
            return;
        }
        else
            inc = 1;
    }

    double dt=last->time_stamp - turning_point->time_stamp;
    double spd = d/dt*1000.;
    double real_spd = d/(t_end-t_start)*1000.;
    const double max_speed = 300.; // 600mm/s
#define DEBUG
#ifdef DEBUG
    cerr << "dist " << d << " in " << dt << " ms. net time is " << t_end-t_start << "\n";
    cerr << "speed " << spd << " " << spd/max_speed*200. << " net speed "<<real_spd << " " << real_spd /max_speed * 200.<< "\n";
#endif
    // copy "segment_length" of points in between 
    while(turning_point < h && turning_point <trajectory.end()-inc) {
        //cerr << "copy normal "<<turning_point->time_stamp << "\n";
        // *2 because external speed is set at 50%
        double robot_spd = real_spd/max_speed * 100.*2.;
        if(robot_spd > 70)
            robot_spd = 70.;
        if(robot_spd <= 0)
            robot_spd = 5.;
        turning_point->speed = robot_spd;
        //cerr << "robot moves to " << turning_point->pose << "\n";
        segment.push_back(*turning_point);
        turning_point += inc;
    }
    cerr << "copy last "<<last->time_stamp << "\n";
    segment.push_back(*last);

    turning_point = h;
}

