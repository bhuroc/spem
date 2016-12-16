/**
 *@file Eyelink.cc
 *@brief 
 */
#include <iostream>
#include <process.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoAsciiText.h>
#include <Inventor/nodes/SoLightModel.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoFont.h>
#include <VML/Graphics/inventor.h>
#include <VML/Graphics/utils.h>
#include <eyelink/eyelink_headers.h>
#include "VML/System/Slog.h"
#include "Eyelink.h"

using namespace std;
using namespace VML;

/**
 * The calibration (HV9) targets are hardwired in Calibr.ini.  It assumes a
 * 640x480 screen and the 9 points are on a regular grid that's 40 pixels
 * away from the edges.  That translates 0.88 and 0.83 of the screen.  Ideally
 * we should override the pixel positions using our own display resolutions.
 * For now, I'll stick with the out-of-factory values and do the scaling. The
 * scaling affect eyelink_driftcorr_start(screen_x, screen_y) most.
 */

Eyelink *Eyelink::instance = 0;

Eyelink::Eyelink()
{
    // Half of a typical 22" screen size
    horiz_dist = 240/2;
    vert_dist = 180/2;

    // Donut size in mm
    inner_radius = 1;
    outer_radius = 5;

    // Size of the screen
    screen_width = -1;
    screen_height = -1;

    // Timeout doing drift correction
    dc_timeout = 2500;

    // A rough measure of pixel size, for exact values, using set_screen_size()
    pixel_size = 0.3;

    // Default drift correct target position
    dc_pixel_x = 320;
    dc_pixel_y = 240;

    SoSeparator *root_calibration = 0;
    SoSeparator *root_drift_correct = 0;
}


void Eyelink::set_donut_size(double inner, double outer)
{
    inner_radius = inner;
    outer_radius = outer;
}

void Eyelink::set_screen_size(double w, double h)
{
    screen_width = w;
    screen_height = h;
}

void Eyelink::set_drift_correct_timeout(double t)
{
    dc_timeout = t;
}

void Eyelink::initialize()
{
    int w, h;
    if(screen_width < 0 || screen_height < 0)  {
        VML::get_screen_pixel_size(w, h);
        screen_width = w * pixel_size;
        screen_height = h * pixel_size;
    }

    // I can use arbitrary values for the dists, but then mapping
    // from screen coord to pixels is more awakard
    horiz_dist = 0.88 * screen_width / 2;
    vert_dist = 0.83 * screen_height / 2;
    slog << "Eyelink calibration grid spacing " << horiz_dist << " " << vert_dist << "\n";

    double text_offset_x=outer_radius, text_offset_y=outer_radius;// mm

    root_calibration = new SoSeparator;
    root_calibration->ref();

    ADD_NODE(root_calibration,c,SoLightModel,model,SoLightModel::BASE_COLOR);
    ADD_NODE(root_calibration,m,SoMaterial,diffuseColor,SbVec3f(1,0,0));
    ADD_NODE2(root_calibration,font,SoFont,name,"Times New Roman",size, 10);

    SoSeparator *ring = make_ring(inner_radius, outer_radius, 2, 30);

    // On virtual screen, the points look like:
    //    6           2           7
    //    4           1           5
    //    8           3           9

    SoTranslation *text_offset = new SoTranslation;
    text_offset->translation.setValue(text_offset_x, text_offset_y, 0.);

#define PLACE_DONUT(label, position) \
    SoSeparator *p##label=new SoSeparator;\
    SoTranslation *t##label = new SoTranslation;\
    t##label->translation.setValue(position);\
    SoAsciiText *l##label = new SoAsciiText;\
    l##label->string.setValue(#label);\
    p##label->addChild(t##label);\
    p##label->addChild(ring);\
    p##label->addChild(text_offset);\
    p##label->addChild(l##label);\
    root_calibration->addChild(p##label)

    PLACE_DONUT(1, SbVec3f(0, 0, 0));
    PLACE_DONUT(2, SbVec3f(0, vert_dist, 0));
    PLACE_DONUT(3, SbVec3f(0, -vert_dist, 0));
    PLACE_DONUT(4, SbVec3f(-horiz_dist, 0, 0));
    PLACE_DONUT(5, SbVec3f(horiz_dist, 0, 0));
    PLACE_DONUT(6, SbVec3f(-horiz_dist, vert_dist, 0));
    PLACE_DONUT(7, SbVec3f(horiz_dist, vert_dist, 0));
    PLACE_DONUT(8, SbVec3f(-horiz_dist, -vert_dist, 0));
    PLACE_DONUT(9, SbVec3f(horiz_dist, -vert_dist, 0));

    /////////////////////////////////////////////////////////
    // 
    // Scene for drift correct, a single donut at the origin.
    // It's up to the user to move the target to its desired 
    // position p and the user should also compute the right 
    // pixel values correspondent to the position p.
    // In other words, this class doesn't know anything about
    // the viewer/screen transformation.
    //
    root_drift_correct = new SoSeparator;
    root_drift_correct->ref();
    root_drift_correct->addChild(c);
    root_drift_correct->addChild(m);
    root_drift_correct->addChild(ring);

    if(open_eyelink_connection(0)) {
        slog << "Can't connect to eyelink.\n";
        throw std::runtime_error("Can't connect to eyelink.");
    }
    //::eyecmd_printf("screen_pixel_coords = %ld %ld %ld %ld", 0, 0, w, h);

    calibration_done = true;
}

void Eyelink::shutdown()
{
    //root_calibration->unref();
    //root_drift_correct->unref();
    close_eyelink_connection();
    delete instance; instance = 0;
}

void Eyelink::get_info()
{
    char buf[100];

    if(!eyelink_is_connected()) {
        return;
    }
    eyelink_read_request("screen_pixel_coords");
    Sleep(500);
    if(eyelink_read_reply(buf) == OK_RESULT) { 
        slog << "screen pixel coords " << string(buf) << "\n"; 
    } 
}

static unsigned _stdcall run_calibration(void *arg)
{
    Eyelink *eyelink = (Eyelink*)arg;
    do_tracker_setup();
    cerr << "-- do_tracker_setup returned.\n";
    eyelink_wait_for_mode_ready(500);
    eyelink->set_calibration_done();

    return 0;
}

void Eyelink::calibrate()
{
    unsigned tid;
    calibration_done = false;
    _beginthreadex(NULL, 0, run_calibration, this, 0, &tid);
}

void Eyelink::set_dc_target_pixel_position(double x, double y)
{
    dc_pixel_x = x ;
    dc_pixel_y = y ;
}

Eyelink::DriftCorrectResult Eyelink::drift_correct()
{
    int result=NO_REPLY; 

    if(!eyelink_is_connected()) {
        return DC_MODE; // Not connected
    }

#ifdef EYELINK_COMPUTER
    result = do_drift_correct(dc_pixel_x, dc_pixel_y, 0, 0);
    slog << "do_drift_correct returns " << result << "\n";
    if(result == 27) {
        slog << "Drift correction aborted.\n";
        return DC_ABORT;
    }
    return DC_SUCCEED;
#else

    // If not in drift correct mode, start the correction computation
    if(!(eyelink_current_mode() & IN_DRIFTCORR_MODE)) {
        eyelink_driftcorr_start(dc_pixel_x,dc_pixel_y);
        if(eyelink_wait_for_mode_ready(1000)) {
            slog << "Eyelink timed out while switching to drift correction mode.\n";
        }
    }

    timer.reset();

    if(eyelink_current_mode() & IN_DRIFTCORR_MODE) {
        while(timer.get_elapsed_millisec() < dc_timeout) {
            eyelink_accept_trigger();

            do {
                result = eyelink_cal_result();
            }while (result==NO_REPLY && timer.get_elapsed_millisec() < dc_timeout);

            if(result == OK_RESULT ) {
                if(eyelink_apply_driftcorr()) {
                    slog << "Failed when applying drift correct.\n";
                    return DC_FAIL;
                }else {
                    slog << "Drift corrected.\n";
                    set_offline_mode();
                    if(eyelink_wait_for_mode_ready(1000)) {
                        slog << "Time out when switching to offline mode after drift correct's applied.\n";
                    }
                    return DC_SUCCEED;
                }
            }else if(result == 27) {
                slog << "Drift correct aborted.\n";
                return DC_ABORT;
            }else if(result == -1) {
                slog << "Drift correct failed:" << result <<"\n";
                return DC_FAIL;
            }else if(result == 1) {
                slog << "Repeat drift correct.\n";
                return DC_REPEAT;
            }
            slog << "still trying: " << result << "\n";
        }
    }else {
        Beep(440,120);
        slog << "Can't go into drift correct mode.\n";
        return DC_MODE;
    }
#endif
    return DC_TIMEOUT;
}


