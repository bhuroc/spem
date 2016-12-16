/**
 *@file EyelinkCollector.cc
 *@brief 
 */
#include <iostream>
#include <eyelink/eyelink_headers.h>
#include "EyelinkCollector.h"

using namespace std;

EyelinkCollector::EyelinkCollector()
{
    sampling_rate = 250;
    frame_no = 0;
}

EyelinkCollector::EyelinkCollector(int rate)
{
    sampling_rate = rate;
}

EyelinkCollector::~EyelinkCollector()
{
}

bool EyelinkCollector::start_recording()
{
    int err;
    if(err = ::start_recording(0, 0, 1, 0)) {
        cerr << "Eyelink failed starting recording " << err << ".\n";
        return false;
    }
    frame_no = 0;
    return true;

}

void EyelinkCollector::stop_recording()
{
    ::stop_recording();
}

int EyelinkCollector::update_frame()
{
    if(eyelink_current_mode() & IN_RECORD_MODE) {

        if(eyelink_newest_float_sample(NULL) == 1 ) {
            ++ frame_no;
            eyelink_newest_float_sample(&data);
        }
    }

    return frame_no;
}

int EyelinkCollector::get_data(EyelinkDataFrame &d)
{
    d.time_stamp = data.time;
#if EYELINK_COLLECT_RAW
    d.left_pupil[0] = data.px[0];
    d.left_pupil[1] = data.py[0];
    d.right_pupil[0] = data.px[1];
    d.right_pupil[1] = data.py[1];
    d.left_head_ref[0] = data.hx[0];
    d.left_head_ref[1] = data.hy[0];
    d.right_head_ref[0] = data.hx[1];
    d.right_head_ref[1] = data.hy[1];
#endif
    d.left_pupil_size = data.pa[0];
    d.right_pupil_size = data.pa[1];
    d.left_screen_gaze[0] = data.gx[0];
    d.left_screen_gaze[1] = data.gy[0];
    d.right_screen_gaze[0] = data.gx[1];
    d.right_screen_gaze[1] = data.gy[1];
    d.resolution_x = data.rx;
    d.resolution_y = data.ry;

    return frame_no;
}
   


