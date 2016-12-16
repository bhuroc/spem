#ifndef _TRIAL_H_
#define _TRIAL_H_

/**************************************************
*
* $Id$
*
**************************************************/
/**
 *@file Trial.h
 *@brief 
 */
#include <iostream>
#include <string>

#define GOOD_TRIAL 0
#define BAD_TRIAL 1

#define DYNAMIC_NOISE 0
#define STATIC_NOISE 2
#define BLANK_SCREEN 4

struct Trial
{

    Trial()
        :visual_pattern(-1)
    {
        status = GOOD_TRIAL;
    }

    Trial(int n, double p, int d, double r)
        :visual_pattern(n)
    {
        status = GOOD_TRIAL;
    }


    Trial(const std::string &s);

    void dump(std::ostream &os);
    int visual_pattern ;// dynamic noise, static noise or bland screen
    int status;//various info: if this is a bad trial,e.g.
};

std::ostream &operator<<(std::ostream &os, const Trial &t);
std::istream &operator>>(std::istream &os, Trial &t);
#endif/*_TRIAL_H_*/

