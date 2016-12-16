#ifndef _EYELINKCOLLECTOR_H_
#define _EYELINKCOLLECTOR_H_

#include <Inventor/nodes/SoSeparator.h>
#include <Eyelink/eyelink_headers.h>

/**
 *@file EyelinkCollector.h
 *@brief 
 */

// A subsect of FSAMPLE
struct EyelinkDataFrame {
    EyelinkDataFrame():time_stamp(0),left_pupil_size(0),right_pupil_size(0),resolution_x(0),resolution_y(0), status(0) {
        left_screen_gaze[0] = 0.;
        left_screen_gaze[1] = 0.;
        right_screen_gaze[0] = 0.;
        right_screen_gaze[1] = 0.;
    }

    unsigned int time_stamp;
#ifdef EYELINK_COLLECT_RAW
    double left_pupil[2], right_pupil[2];
    double left_head_ref[2], right_head_ref[2];
#endif
    double left_pupil_size,right_pupil_size;
    double left_screen_gaze[2], right_screen_gaze[2];
    double resolution_x, resolution_y;
    int status;
};

class EyelinkCollector
{
    public:
        EyelinkCollector();
        EyelinkCollector(int sampling_rate);
        ~EyelinkCollector();

        bool start_recording();
        void stop_recording();

        // set parameters
        int init(unsigned int settings);

        // retrieve data
        int update_frame();

        int get_data(EyelinkDataFrame &d);

    private:
        int sampling_rate;
        int frame_no;
        FSAMPLE data;
};

#endif/*_EYELINKCOLLECTOR_H_*/

