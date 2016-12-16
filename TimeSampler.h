#ifndef _TIMESAMPLER_H_
#define _TIMESAMPLER_H_

/**
 *@file TimeSampler.h
 *@brief 
 */
#include "TrajectorySampler.h"

class TimeSampler : public TrajectorySampler
{
    public:
        TimeSampler(std::vector<VML::RobotTrajectoryNode> &t) :trajectory(t) {
            pickup_point = t.begin();
            // how many points to pass to the robot at once
            // the segment should be should enough to be considered
            // constant speed movement and long enough for the robot
            // to response
            sampling_duration = 100; // ms
            // every time send 400/100 frames to the robot
            segment_duration = 400; // has to longer than sampling duration
            segment_length = ceil(segment_duration/sampling_duration);
        }
        ~TimeSampler() {}

        void set_segment_length(double l) {
            segment_duration = l;
        }

        void set_sub_sampling_duration(double s) {
            sampling_duration = s;
        }
        
        void reset() {
            pickup_point = trajectory.begin();
        }

        void search_for_next_segment(std::vector<VML::RobotTrajectoryNode> &next, double t0);
    private:
        std::vector<VML::RobotTrajectoryNode> &trajectory;
        std::vector<VML::RobotTrajectoryNode>::iterator pickup_point;
        double sampling_duration;
        double segment_duration;
        int segment_length;
};

#endif/*_TIMESAMPLER_H_*/

