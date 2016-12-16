#ifndef _EYELINK_H_
#define _EYELINK_H_

/**
 *@file Eyelink.h
 *@brief 
 */
#include <VML/System/ElapsedTimer.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTranslation.h>

class Eyelink 
{
    public:
        enum DriftCorrectResult{
            DC_SUCCEED=NO_REPLY+1,
            DC_FAIL,
            DC_ABORT,
            DC_MODE,
            DC_TIMEOUT,
            DC_REPEAT
        };

        static Eyelink *get_instance() {
            if(instance == 0) {
                instance = new Eyelink;
            }
            return instance;
        }

        void set_donut_size(double inner, double outer);
        void set_screen_size(double w, double h);
        void set_drift_correct_timeout(double t);
        void initialize();
        void shutdown();
        void set_dc_target_pixel_position(double x, double y);

        void calibrate();
        DriftCorrectResult drift_correct();

        SoSeparator *get_calibration_scene() {
            return root_calibration;
        }

        SoSeparator *get_drift_correct_scene() {
            return root_drift_correct;
        }

        void get_info();

        void set_calibration_done() {
            calibration_done = true;
        }

        bool is_calibration_done() const {
            return calibration_done;
        }

    private:
        Eyelink();
        static Eyelink *instance;

        SoSeparator *root_calibration;
        SoSeparator *root_drift_correct;
        double vert_dist, horiz_dist;
        double inner_radius,outer_radius;
        //screen size in mm, used to remap a screen point to 640x480 space
        double screen_width, screen_height;
        double pixel_size;
        VML::ElapsedTimer timer;
        double dc_timeout;//drift correct timeout
        int dc_pixel_x, dc_pixel_y;
        bool calibration_done;
};
#endif/*_EYELINK_H_*/

