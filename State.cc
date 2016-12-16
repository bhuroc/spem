/**
 *@file State.cc
 *@brief 
 */
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <BVL/math/linalg/linalg.h>
#include <VML/Graphics/ViewingTransforms.h>
#include <VML/Graphics/utils.h>
#include <VML/Graphics/inventor.h>
#include <VML/System/Slog.h>
#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/SbViewportRegion.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoMatrixTransform.h>
#include <Inventor/nodes/SoTexture2Transform.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoFont.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoText3.h>
#include <Inventor/nodes/SoAsciiText.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoNormal.h>
#include <Inventor/nodes/SoFaceSet.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoCallback.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoGetMatrixAction.h>
#include <stdio.h>
#include <acml.h>
#include "ExperimentManager.h"
#include "State.h"
#include "App.h"
#include "Eyelink.h"

#define IVFILE(f) "lib/"##f

using namespace std;
using namespace BVL;
using namespace VML;

extern ViewingTransforms viewer;
extern ExperimentManager mgr;

VML::ElapsedTimer State::timer;

NEWSTATE_SOURCE(Init);
NEWSTATE_SOURCE(PreTrial);
NEWSTATE_SOURCE(PostTrial);
NEWSTATE_SOURCE(Trial);
NEWSTATE_SOURCE(Done);
NEWSTATE_SOURCE(Speeding);
NEWSTATE_SOURCE(EyelinkCalibration);
NEWSTATE_SOURCE(EyelinkDriftCorrect);
NEWSTATE_SOURCE(Interlude);

// process_aperture and some utils, de-clutter this file.
#include "StateMisc.cc"


///////////////////////////////////////////////////////////////
// 
// Init state: display a circle to line up and card board and
// the handle so we know the init handle coordinate frame
// 
///////////////////////////////////////////////////////////////
StateInit::StateInit(App *app) : State(app)
{
    // construct my scene: the handle and the big circle
    root = new SoSeparator;
    root->ref();

    root->addChild(app->get_light_model());

    //                       root
    //   +--------------------------------+-----------+
    // global displacement           cardboard   handle           

    ADD_NODE0(root, global_disp, SoTranslation);
    double gdx, gdy, gdz;
    mgr.get_access_offset(gdx,gdy,gdz);
    cerr << "access " << gdx << " " << gdy << " " << gdz << "\n";
    global_disp->translation.setValue(gdx, gdy, gdz);

    // Need a circle, whose size is determined by view angle
    // and distance to the table
    ADD_NODE0(root,cardboard,SoSeparator);

    // Draw the circle on the table
    cardboard->addChild(app->get_table2scr());

    ADD_NODE(cardboard,h_color,SoMaterial,diffuseColor,SbVec3f(0.35,0.1,0.1));

    // Push it down a little.
    ADD_NODE(cardboard,push_card,SoTranslation,translation,SbVec3f(0,0,-0.1));

    const double handle_margin=50*2; // 50mm each side for subjects to move the handle
    double hole_size = handle_margin;
    // You want the diameter to be the hole size, yet unit_disk is a circle
    // with radius 1, so the scale factor should be half of calculated above
    hole_size /= 2;
    ADD_NODE(cardboard,hole_scale,SoScale,scaleFactor,SbVec3f(hole_size,hole_size,1.));

    SoInput in;
    in.openFile(IVFILE("unit_disk.iv"));
    cardboard->addChild(SoDB::readAll(&in));
    in.closeFile();

    // The handle.  At the start of the experiment, user lines up 
    // the real handle with the wirefire to register the initial 
    // coordiante frame.
    in.openFile(mgr.get_handle_iv_file().c_str());
    SoSeparator *handle = SoDB::readAll(&in);
    in.closeFile();

    // Draw a handle on the table
    ADD_NODE0(root,handle_static,SoSeparator);
    handle_static->addChild(app->get_table2scr());
    handle_static->addChild(handle);

    ADD_NODE0(root,handle_dynamic,SoSeparator);
    // Add the transformation to track the another handle
    //handle_dynamic->addChild(app->get_rbinscr());
    //handle_dynamic->addChild(app->get_handle2rb());

    ADD_NODE(handle_dynamic,red,SoMaterial,diffuseColor,SbVec3f(1,0,0));
    handle_dynamic->addChild(handle);

    handle_path = new SoPath;
    handle_path->ref();
    handle_path->append(root);
    handle_path->append(handle_dynamic);
}

State *StateInit::transition(const WindowEvent &e)
{
    if(e.type == WE_MOUSE_3D_BUTTON_DOWN || e.type == WE_MOUSE_DOWN) {

        // Search for the node that right before the translation node in
        // handle*_iv_description
        SoSearchAction sa;
        sa.setName(mgr.get_tool_name().c_str());
        sa.apply(handle_path);
        SoPath *root_handle = sa.getPath();
        assert(root_handle);

        SbViewportRegion rgn;
        SoGetMatrixAction ma(rgn);
        ma.apply(root_handle);

        app->start_new_trial();
        if(mgr.is_doing_eye_tracking()) {
            return change_state(this,StateEyelinkCalibration::get_instance());
        }else
            return change_state(this,StatePreTrial::get_instance());
    }
    return StateInit::get_instance();
}


///////////////////////////////////////////////////////////////
// 
// Pre-trial state: dispaly the quadrant and the tip of the tool.
// When the tip is close to the origin, transition to trial state.
//
///////////////////////////////////////////////////////////////
StatePreTrial::StatePreTrial(App *app):State(app)
{
    // load my scene: the qudrant and the tool tip
    root = new SoSeparator;
    root->ref();
}

void StatePreTrial::update_scene()
{
}

void StatePreTrial::pre_action()
{
    PlaySound("ding.wav",NULL, SND_ASYNC);
}


State *StatePreTrial::transition(const WindowEvent &e)
{
    wait_for_lining_up = false;
    if(!wait_for_lining_up && timer.get_elapsed_millisec()>1500) {
        app->start_recording();
        return StateTrial::get_instance();
    }
    return StatePreTrial::get_instance();
}

StatePostTrial::StatePostTrial(App *app):State(app)
{
    // load my scene: the qudrant and the tool tip
    root = new SoSeparator;
    root->ref();
}

State *StatePostTrial::transition(const WindowEvent &e)
{
    PlaySound("chord.wav",NULL, SND_SYNC);
    return change_state(this,StatePreTrial::get_instance());
}

///////////////////////////////////////////////////////////////
// 
// Trial state: the subject moves the pattern for what-ever seconds
// 
///////////////////////////////////////////////////////////////
void StateTrial::read_noise()
{
    get_screen_pixel_size(noise_width, noise_height);

    int np = noise_width*noise_height;
    int number_images = mgr.get_number_of_noise_images();

    int seed=13, lseed = 1;
    int info;
    int lstate = 16;
    int state[16];
    srandinitialize(1, 1, &seed, &lseed, state, &lstate, &info);
    if(info != 0) {
        std::cerr << "Can't init RNG: " << info << "\n";
    }
    float *noise_buf = new float[np];
    for(int i=0; i<number_images; ++i) {
        unsigned char *noise = new unsigned char [np];
        float low=mgr.get_noise_contrast_low();
        float high=mgr.get_noise_contrast_high();

        sranduniform(np, low, high, state, noise_buf, &info);
        for(int j=0;j<np;++j) {
            noise[j] = 255*noise_buf[j];
        }
        
        if(info != 0) {
            std:: cerr << "Can't generate noise " << info << "\n";
        }else
            cerr << "generated noise" << i << "\n";
        noise_data.push_back(noise);
    }
    delete[] noise_buf;

}

PFNGLWINDOWPOS2IPROC glWindowPos2i;

void draw_noise_mask(void *state, SoAction *)
{
    StateTrial *trial_state= (StateTrial*)state;

    glClear(GL_COLOR_BUFFER_BIT); 
    glWindowPos2i(0, 0);
    glDrawPixels(trial_state->get_noise_width(),
            trial_state->get_noise_height(),
            GL_LUMINANCE,
            GL_UNSIGNED_BYTE,
            trial_state->get_noise_data()
            );
}

StateTrial::StateTrial(App *app) : State(app)
{
    glWindowPos2i = (PFNGLWINDOWPOS2IPROC)
                wglGetProcAddress("glWindowPos2i");

    if(mgr.is_limiting_speed()) {
        mgr.get_speed_limits(low_speed_limit, high_speed_limit);
        slog << " *** getting speed limits " << low_speed_limit << " " << high_speed_limit << "\n";
    }

    read_noise();

    // load my scene: the front cover and the stripes
    root = new SoSeparator;
    root->ref();

    display_switch = new SoSwitch;
    root->addChild(display_switch);

    ///////////////////////////////////////////////////////////
    // The white noise mask 
    // It's drawn right on the screen.
    ADD_NODE0(display_switch, draw_screen, SoCallback);
    draw_screen->setCallback(draw_noise_mask, this);

}

void StateTrial::update_scene()
{
    if(app->get_current_trial()->visual_pattern == BLANK_SCREEN) {
        display_switch->whichChild = SO_SWITCH_NONE;
    }else {
        display_switch->whichChild = 0;
    }

    // reset which frame the noise loops
    noise_frame = rand()%(noise_data.size());

    average_speed = 0.;
    speed_count = 0;
    first_transition = true;
    timer.reset();

    // Note: app will start the recording and I'm not responsible
}

const unsigned char *StateTrial::get_noise_data() {
    if(app->get_current_trial()->visual_pattern == DYNAMIC_NOISE) {
        noise_frame = (noise_frame+1)%noise_data.size();
    }
    return noise_data[noise_frame];
}

State* StateTrial::go_next()
{
    app->finish_one_trial(false);
    if(app->do_drift_correct()) {
        return StateEyelinkDriftCorrect::get_instance();
    }else if(app->do_eyelink_calibration()) {
        return StateEyelinkCalibration::get_instance();
    }else if(app->start_new_trial() == false) {
        return change_state(this, StateDone::get_instance());
    }else {
        return change_state(this,StatePostTrial::get_instance());
    }
}

State *StateTrial::transition(const WindowEvent &e)
{
    // Should really process the event (a timer event in this case), but
    // for performance consideration (we don't have to put the timer event 
    // in the event queue), use app's timer directly.
    if(mgr.is_limiting_speed()) {
        double vx, vy, vz;
        app->get_handle_velocity(vx,vy,vz);
        double v=sqrt(vx*vx+vy*vy+vz*vz);
        average_speed += v;
        speed_count ++;
    }

    if(first_transition) {
        first_transition = false;
        //cerr << "entering StateTrial at " << timer.get_elapsed_millisec() << "\n";
        timer.reset();
    }

    if(timer.get_elapsed_millisec()>mgr.get_trial_duration()) {
        //cerr << "leaving StateTrial at " << timer.get_elapsed_millisec() << "\n";
        app->stop_recording();

        if(mgr.is_limiting_speed()) {
            // XXX magic number here
            // v is mm/(1/sampling_rate s) = sampling_rate mm/s
            // 250/(mgr.get_trial_duration()/1000) -- 250Hz
            // 
            average_speed /= speed_count;
            average_speed *= 250;
            slog << "-- Average speed: " << average_speed << " count " << speed_count <<"\n";
            StateSpeeding *st = (StateSpeeding*)StateSpeeding::get_instance();
            if(average_speed < low_speed_limit) {
                st->set_speed_violation(0);
                return st;
            }else if(average_speed > high_speed_limit) {
                st->set_speed_violation(1);
                return st;
            }
            return go_next();
        }else {
            return go_next();
        }
    }else
        return StateTrial::get_instance();
}


///////////////////////////////////////////////////////////////
// 
// Done state: nothing much to do, wait for ESC
// 
///////////////////////////////////////////////////////////////
StateDone::StateDone(App *app) : State(app)
{
    // load my scene: simple text or fancy animation
    root = new SoSeparator;
    root->ref();

    root->addChild(app->get_light_source());
    
    ADD_NODE(root,move_up,SoTranslation,translation,SbVec3f(0,20,0));
    ADD_NODE2(root,font,SoFont,name,"Times New Roman",size,20);
    ADD_NODE2(root,text,SoText3,string,"Well Done",justification,SoText3::CENTER);

    play_finish_sound = false;
}

State *StateDone::transition(const WindowEvent &)
{
    if(play_finish_sound) {
        PlaySound("tada.wav",NULL, SND_ASYNC);
        play_finish_sound = false;
    }
    return StateDone::get_instance();
}


///////////////////////////////////////////////////////////////
// 
// Speeding state
// 
///////////////////////////////////////////////////////////////
StateSpeeding::StateSpeeding(App *app) : State(app)
{
    // load my scene: simple text or fancy animation
    root = new SoSeparator;
    root->ref();

    root->addChild(app->get_light_model());
    ADD_NODE(root,red,SoMaterial,diffuseColor,SbVec3f(1,0,0));
    
    ADD_NODE2(root,font,SoFont,name,"Times New Roman",size,24);
    speeding_text = new SoSwitch;
    root->addChild(speeding_text);
    ADD_NODE2(speeding_text,text1,SoText3,string,"Faster",justification,SoText3::CENTER);
    ADD_NODE2(speeding_text,text2,SoText3,string,"Slower",justification,SoText3::CENTER);

}

void StateSpeeding::set_speed_violation(int violation) 
{
    speeding_text->whichChild = violation;
    timer.reset();
}

State *StateSpeeding::transition(const WindowEvent &e)
{
    if(timer.get_elapsed_millisec()>1000 || e.type == WE_MOUSE_3D_BUTTON_DOWN || e.type == WE_MOUSE_DOWN) {
        // second arg, true, indicates this is a bad trial
        app->finish_one_trial(true);
        app->start_new_trial(true);//don't have to worry about start_new_trial returning false: we are re-running this trial.
        return change_state(this, StatePreTrial::get_instance());
    }
    return this;
}

///////////////////////////////////////////////////////////////
// 
// EyelinkCalibration state
// 
///////////////////////////////////////////////////////////////
StateEyelinkCalibration::StateEyelinkCalibration(App *app) : State(app)
{
    eyelink = Eyelink::get_instance();
    root = eyelink->get_calibration_scene();
    wait_for_eyelink = false;
}

State *StateEyelinkCalibration::transition(const WindowEvent &e)
{
    // launch the calibration from this computer
    if(e.type == WE_KEY && (e.key == 'E'||e.key == 'e')) {

        eyelink->calibrate();
        wait_for_eyelink = true;

        // The other fix is to purge the buffer by calling 
        // force_updating_pose, without creating a new thread
        // for eyelink->calibrate().  The multi-threaded solution
        // is better in that the interface isn't hung up.

        //app->force_updating_pose();
        return this;
    }

    if((e.type == WE_KEY &&  (e.key == 'g'|| e.key =='G')) 
            ||
            (wait_for_eyelink && eyelink->is_calibration_done())) {

        wait_for_eyelink = false;
        app->done_eyelink_calibration();
        if(app->start_new_trial() == false) {
            return StateDone::get_instance();
        }else {
            //return change_state(this,StatePreTrial::get_instance());
            return change_state(this,StateInterlude::get_instance());
        }
    }

    return this;
}

///////////////////////////////////////////////////////////////
// 
// EyelinkDriftCorrect state
// 
///////////////////////////////////////////////////////////////
StateEyelinkDriftCorrect::StateEyelinkDriftCorrect(App *app) : State(app)
{
    eyelink = Eyelink::get_instance();
    root = eyelink->get_drift_correct_scene();
}

State *StateEyelinkDriftCorrect::transition(const WindowEvent &e)
{
    if((e.type == WE_KEY && e.key=='d') || eyelink->drift_correct()==Eyelink::DC_SUCCEED) {
        app->done_drift_correct();
        if(app->start_new_trial() == false) {
            return StateDone::get_instance();
        }else {
            return change_state(this,StatePreTrial::get_instance());
        }
    }else {
        // Drift correct not done, keep trying
        return this;
    }
}

///////////////////////////////////////////////////////////////
// 
// Interlude
// 
///////////////////////////////////////////////////////////////
StateInterlude::StateInterlude(App *app):State(app)
{
    // load my scene: the qudrant and the tool tip
    root = new SoSeparator;
    root->ref();
}

void StateInterlude::pre_action() 
{
    timer.reset();
    PlaySound("lib/interlude.wav",NULL, SND_ASYNC);
}

State *StateInterlude::transition(const WindowEvent &e)
{
    double t=timer.get_elapsed_millisec();
    if( t>=30000 || (t > 10000 &&
        (e.type == WE_KEY &&  e.key == ' ')) ) {
            return change_state(this,StatePreTrial::get_instance());
    }

    return this;
}

