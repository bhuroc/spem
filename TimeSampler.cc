/**
 *@file TimeSampler.cc
 *@brief 
 */
#include <iostream>
#include "TimeSampler.h"
#include <VML/Robot/RobotInterface.h>

using namespace  std;
using namespace  VML;

class NextNode {
    public:
        NextNode(double t):t0(t) {}
        bool operator()(const RobotTrajectoryNode &i) const {
            return i.time_stamp > t0;
        }
    private:
        double t0;
};

void TimeSampler::search_for_next_segment(vector<RobotTrajectoryNode> &segment, double t0)
{
    segment.clear();

    NextNode nn(t0);

    //cerr << "search for next segment at time "<< t0 << "\n";

    vector<RobotTrajectoryNode>::iterator h;
    h = find_if(pickup_point, trajectory.end(), nn);
    if(h != trajectory.end()) {
        //cerr << "found at "<<trajectory.end()-h << " "<<h->time_stamp << "\n";
        if(trajectory.end()-h <= segment_length) {
            copy(h, trajectory.end(), back_inserter(segment));
            pickup_point = trajectory.end();
        } else {
            copy(h, h+segment_length, back_inserter(segment));
            pickup_point = h+segment_length;
        }
    }
}

