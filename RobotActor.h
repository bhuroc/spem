#ifndef _ROBOTACTOR_H_
#define _ROBOTACTOR_H_

/**
 *@file RobotActor.h
 *@brief 
 */
#include <VML/Robot/RobotController.h>
#include <VML/Graphics/ViewingTransforms.h>
#include <VML/Robot/RobotTransforms.h>
#include <VML/System/ElapsedTimer.h>
#include <vector>
#include <memory>
#include "Trajectory.h"
#include "TrajectorySampler.h"

class RobotActor : public VML::RobotController {
    public:
        RobotActor(const VML::RobotTransforms *r);
        ~RobotActor();

        void set_raw_trajectory(const std::vector<VML::RobotTrajectoryNode> &t) ;
        void do_command();

        void start_move();

        bool stopped() {
            return !is_moving;
        }

        // may be redundant, call traj::search_for_next directly
        void move_to_next_segment();

        // put the robot to the center of the screen.
        void put_to_center();
    private:
        HANDLE mut_traj;
        VML::ElapsedTimer timer;
        Trajectory raw_traj;
        std::vector<VML::RobotTrajectoryNode> cooked_traj;
        std::tr1::shared_ptr<TrajectorySampler> sampler;
        bool is_moving;
        int command;// 0 to put to screen center;1 to move trajectory
};

#endif/*_ROBOTACTOR_H_*/

