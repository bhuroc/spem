#ifndef _SPACESAMPLER_H_
#define _SPACESAMPLER_H_

/**
 *@file SpaceSampler.h
 *@brief 
 */
#include <vector>
#include <BVL/math/linalg/linalg.h>
#include "TrajectorySampler.h"

class SpaceSampler : public TrajectorySampler
{
    public:
        SpaceSampler(std::vector<VML::RobotTrajectoryNode> &t) :trajectory(t) {
            turning_point = t.begin();

            // for each segment (between two 0 velocity points), 
            // how many points to sample
            segment_length = 6;
        }
        ~SpaceSampler() {}

        void set_segment_length(double l) {
            segment_length = l;
        }
        
        void reset() {
            turning_point = trajectory.begin();
        }
        double compute_distance(std::vector<VML::RobotTrajectoryNode>::iterator i1, std::vector<VML::RobotTrajectoryNode>::iterator i2);
        BVL::Vector<double> compute_velocity_direction(std::vector<VML::RobotTrajectoryNode>::iterator i);
        BVL::Vector<double> compute_velocity(std::vector<VML::RobotTrajectoryNode>::iterator i);

        void search_for_next_segment(std::vector<VML::RobotTrajectoryNode> &next, double t0);
    private:
        std::vector<VML::RobotTrajectoryNode> &trajectory;
        std::vector<VML::RobotTrajectoryNode>::iterator turning_point;
        int segment_length;
};

#endif/*_SPACESAMPLER_H_*/

