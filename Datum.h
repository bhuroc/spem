#ifndef _DATUM_H_
#define _DATUM_H_

/**
 *@file Datum.h
 *@brief 
 */
#include <iostream>
#include <BVL/math/linalg/linalg.h>

#define NUM_MARKERS 4

struct Datum
{
    // optotrak part
    int opto_frame_no;
    double x[NUM_MARKERS], y[NUM_MARKERS], z[NUM_MARKERS]; // the raw values (in optotrack coords)
    double scr_x[NUM_MARKERS], scr_y[NUM_MARKERS], scr_z[NUM_MARKERS]; // in screen coord

    // eyelink part
    int eyelink_frame_no;
    double left_screen_gaze[2], right_screen_gaze[2];
    double left_pupil_size, right_pupil_size;
    double resolution_x, resolution_y;

    // time stamp
    double time_stamp;
    unsigned int eyelink_time_stamp;

    Datum():opto_frame_no(0), eyelink_frame_no(0),left_pupil_size(0),right_pupil_size(0),resolution_x(0),resolution_y(0), time_stamp(0),eyelink_time_stamp(0) {
        fill_invalid_data();
        left_screen_gaze[0] = left_screen_gaze[1] = 0;
        right_screen_gaze[0] = right_screen_gaze[1] = 0;
    }

    // If two datum items are the same, they won't be put into
    // the buffer.
    bool operator==(const Datum &d) const {
        return (opto_frame_no == d.opto_frame_no && 
                eyelink_frame_no == d.eyelink_frame_no);
    }

    void fill_invalid_data() {
        for(int i=0; i<NUM_MARKERS; ++i) {
            x[i] = 0;
            y[i] = 0;
            z[i] = 0;
            scr_x[i] = 0;
            scr_y[i] = 0;
            scr_z[i] = 0;
        }
    }

    void fill_data(const BVL::Vector<double> &opto, const BVL::Vector<double> inscr, int i) {
        x[i] = opto(1);
        y[i] = opto(2);
        z[i] = opto(3);
        scr_x[i] = inscr(1);
        scr_y[i] = inscr(2);
        scr_z[i] = inscr(3);
    }
};

std::ostream &operator<<(std::ostream &os, const Datum &d);
std::istream &operator>>(std::istream &is, Datum &d);

#endif/*_DATUM_H_*/

