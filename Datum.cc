/**
 *@file Datum.cc
 *@brief 
 */
#include <iostream>
#include "Datum.h"

std::ostream &operator<<(std::ostream &os, const Datum &d)
{
    os << d.opto_frame_no << " " <<d.time_stamp;
    for(int i=0; i<NUM_MARKERS; ++i) {
        os << " " << d.x[i] << " " << d.y[i] << " " << d.z [i]
        << " " << d.scr_x[i] << " " << d.scr_y[i] << " " << d.scr_z[i] ;
    }
    os  << " " << d.eyelink_frame_no << " " << d.eyelink_time_stamp
        << " " << d.left_pupil_size << " " << d.right_pupil_size 
        << " " << d.left_screen_gaze[0] << " " << d.left_screen_gaze[1] 
        << " " << d.right_screen_gaze[0] << " " << d.right_screen_gaze[1] 
        << " " << d.resolution_x << " " << d.resolution_y
        << " ";

    return os;
}

std::istream &operator>>(std::istream &is, Datum &d)
{
    is >> d.opto_frame_no >> d.time_stamp;
    for(int i=0; i<NUM_MARKERS; ++i) {
        is >> d.x[i] >> d.y[i] >> d.z [i]
        >> d.scr_x[i] >> d.scr_y[i] >> d.scr_z[i] ;
    }
    is >> d.eyelink_frame_no >> d.eyelink_time_stamp
        >> d.left_pupil_size >> d.right_pupil_size
        >> d.left_screen_gaze[0] >> d.left_screen_gaze[1]
        >> d.right_screen_gaze[0] >> d.right_screen_gaze[1]
        >> d.resolution_x >> d.resolution_y;
    return is;
}

