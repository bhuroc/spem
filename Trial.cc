/**
 *@file Trial.cc
 *@brief 
 */
#include <sstream>
#include "Trial.h"

Trial::Trial(const std::string &s) 
{
    std::istringstream is(s);
    is >> visual_pattern;
    status = GOOD_TRIAL;
}

void Trial::dump(std::ostream &os)
{
    os <<"use visual pattern :"<<visual_pattern<< "\n";

    if(status == GOOD_TRIAL)
        os << "good\n";
    else
        os << "BAD\n";
}

std::ostream &operator<<(std::ostream &os, const Trial &t)
{
    os << t.visual_pattern<< " " 
        << t.status;

    return os;
}

std::istream &operator>>(std::istream &is, Trial &t)
{
    is >> t.visual_pattern;

    return is;
}

