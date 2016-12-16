#ifndef _TRAJECTORY_H_
#define _TRAJECTORY_H_

/**
 *@file Trajectory.h
 *@brief A helper to RobotActor.
 */
#include <vector>
#include <BVL/motion/pose/Pose.h>
#include <VML/Robot/RobotInterface.h>
#include "TrajectorySampler.h"

class Trajectory {
    public:
        Trajectory(const VML::RobotTransforms *r);
        ~Trajectory() {}

        void set_trajectory_sampler(TrajectorySampler *s) {
            trajectory_sampler = s;
        }

        double set_raw_trajectory(const std::vector<VML::RobotTrajectoryNode> &raw);
        std::vector<VML::RobotTrajectoryNode> &get_trajectory() {
            return trajectory;
        }

        void search_for_next_segment(std::vector<VML::RobotTrajectoryNode> &next, double t0);
        BVL::Vector<double> compute_velocity(std::vector<VML::RobotTrajectoryNode>::const_iterator &s);
        BVL::Pose get_starting_point() const;

        BVL::Vector<double> safe_guard(const BVL::Vector<double> &p);

    private:
        const VML::RobotTransforms *robot_xform;
        TrajectorySampler *trajectory_sampler;
        std::vector<VML::RobotTrajectoryNode> trajectory;
        double sampling_duration;
        BVL::Pose screen_center;
};

#endif/*_TRAJECTORY_H_*/

