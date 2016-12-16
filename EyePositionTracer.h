#ifndef _EYEPOSITIONTRACER_H_
#define _EYEPOSITIONTRACER_H_

/**
 *@file EyePositionTracer.h
 *@brief Used to see where the eyes are looking at.
 */
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoSwitch.h>

class EyePositionTracer
{
    public:
        EyePositionTracer();
        ~EyePositionTracer() {root->unref();}

        void set_screen_info(double screen_width, double screen_height, double calibration_screen_width, double calibration_screen_height, int pixel_width=640,int pixel_height = 480);

        void toggle_showing_eye_overlay() {
            showing_eye_overlay = !showing_eye_overlay;
            if(showing_eye_overlay) {
                show_eye->whichChild = SO_SWITCH_ALL;
            }
        }

        SoSeparator *get_scene() {
            return root;
        }

        void update_gaze(double lx, double ly, double rx, double ry);

    private:
        SoSeparator *root;
        SoTranslation *left_x, *left_y, *left_p, *right_x, *right_y, *right_p;
        double x_scale, y_scale, x_edge, y_edge, x_center, y_center;
        SoSwitch *show_eye;
        bool showing_eye_overlay;
        double margin;
};


#endif/*_EYEPOSITIONTRACER_H_*/

