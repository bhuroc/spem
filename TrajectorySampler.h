#ifndef _TRAJECTORYSAMPLER_H_
#define _TRAJECTORYSAMPLER_H_

/**
 *@file TrajectorySampler.h
 *@brief 
 */
#include <vector>
#include <VML/Robot/RobotInterface.h>

class TrajectorySampler
{
    public:
        virtual ~TrajectorySampler() {}

        virtual void search_for_next_segment(std::vector<VML::RobotTrajectoryNode> &next, double t0) = 0;
        virtual void reset() = 0;
};

#endif/*_TRAJECTORYSAMPLER_H_*/

