/**
 *@file RobotActor.cc
 *@brief 
 */
#include <windows.h>
#include <process.h>
#include <iostream>
#include <functional>
#include <algorithm>
#include "RobotActor.h"
#include "TimeSampler.h"
#include "SpaceSampler.h"

using namespace std;
using namespace BVL;
using namespace VML;


RobotActor::RobotActor(const RobotTransforms *r) :RobotController(r),raw_traj(r)
{
    mut_traj = CreateMutex(NULL, FALSE, NULL);

    sampler = tr1::shared_ptr<TrajectorySampler>(new SpaceSampler(raw_traj.get_trajectory()));
    raw_traj.set_trajectory_sampler(sampler.get());
}

RobotActor::~RobotActor()
{
    CloseHandle(mut_traj);
}

void RobotActor::set_raw_trajectory(const vector<RobotTrajectoryNode> &t)
{
    // We don't really have to protect trajectory, because
    // when this function is called, the robot thread is idle
    WaitForSingleObject(mut_traj, INFINITE);
    double spd = raw_traj.set_raw_trajectory(t);
    //set_internal_speed(spd);
    ReleaseMutex(mut_traj);
}

void RobotActor::start_move()
{
    WaitForSingleObject(mutex, INFINITE);
    is_command_pending = true;
    is_moving = true;
    command = 1;
    timer.reset();
    ReleaseMutex(mutex);
}

void RobotActor::do_command() 
{
    if(command == 0) {
        Pose starting_point = raw_traj.get_starting_point();
        set_internal_speed(30.);
        move_euler(starting_point, 3);
        WaitForSingleObject(mutex, INFINITE);
        is_command_pending = false;
        is_moving = false;
        ReleaseMutex(mutex);
    }else {
        move_to_next_segment();
        if(cooked_traj.empty()) {
            cerr << "cooked traj's empty.\n";
            WaitForSingleObject(mutex, INFINITE);
            is_command_pending = false;
            is_moving = false;
            ReleaseMutex(mutex);
        }else {
            cerr << "moving "<<cooked_traj.size()<<" poses ";
            cerr << "at speed " << cooked_traj[0].speed << "\n";
            move_euler_trajectory(cooked_traj);
            //move_euler_path(cooked_traj);
        }
    }
}

void RobotActor::move_to_next_segment()
{
    //XXX protect with mutex
    // raw_traj's accessed by the producer thread and
    // cooked_traj by the consumer thread
    raw_traj.search_for_next_segment(cooked_traj, timer.get_elapsed_millisec());
}

// put_to_center is called from the main thread, if we don't protect 
// cooked_traj, the do_command will see cooked_traj not empty and
// gets preempted, then cooked_traj is cleared by put_to_center and right
// after the clearance, it's being preempted by the do_command again
// and boom
void RobotActor::put_to_center()
{
    WaitForSingleObject(mutex, INFINITE);
    is_command_pending = true;
    is_moving = true;
    command = 0;
    ReleaseMutex(mutex);
}

