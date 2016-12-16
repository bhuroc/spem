/**
 *@file StateMachine.cc
 *@brief 
 */
#include <vector>
#include <deque>
#include <algorithm>
#include <iterator>
#include <stdio.h>
#include <BVL/motion/rigid_body/rigid_body.h>
#include <VML/Graphics/ViewingTransforms.h>
#include <VML/Graphics/utils.h>
#include <VML/Graphics/inventor.h>
#include <VML/GUI/RenderManager.h>
#include <VML/System/ElapsedTimer.h>
#include <VML/System/Slog.h>
#include "ExperimentManager.h"
#include "Trial.h"
#include "TrackingDataSource.h"
#include "DataRecorder.h"
#include "State.h"
#include "App.h"
#include "Datum.h"
#include "Eyelink.h"

using namespace std;
using namespace BVL;
using namespace VML;

// Globals
ExperimentManager mgr;
ViewingTransforms viewer;

App::App()
{
    current_state = 0;
    time_for_drift_correct = false;
    time_for_eyelink_calibration = false;
    current_trial_idx = -1; // wait for the first start_new_trial() call
    tracing_eye = false;
}

App::~App()
{
    delete_states();
    if(mgr.is_doing_eye_tracking()) {
        Eyelink::get_instance()->shutdown();
    }
    // XXX unref the nodes created here
}


void App::update_pose()
{
    // Update the engine
    Datum d = producer->read_dump();
    saved_d = d;
    if(eye_tracer && tracing_eye) {
        eye_tracer->update_gaze(d.left_screen_gaze[0],
                d.left_screen_gaze[1],
                d.right_screen_gaze[0],
                d.right_screen_gaze[1]);
    }

    // see if we are missing markers
    if(fabs(d.x[0]) > 1e5) {
        Beep(500, 500);
        Beep(600, 500);
    }

}

bool App::idle(int) 
{
    update_pose();

    WindowEvent e;
    if(!event_queue.empty()) {
        e = event_queue.front();
        event_queue.clear();
    }
    e.time_stamp = timer.get_elapsed_millisec();
    current_state = current_state->transition(e);

#if DEBUG > 3
    cerr << "current state " << current_state->get_name()<< " "; 
    cerr << timer.get_elapsed_millisec() << "\n";
#endif

    SoSeparator *s = current_state->get_scene();
    if(tracing_eye) {
        s->addChild(eye_tracer->get_scene());
    }
    renderer->set_scene(s);
    renderer->draw();

    return true;
}

int App::handle_event(const WindowEvent &e) 
{
    if(e.type == WE_KEY) {
        if(e.key == 27) {
            producer->stop();
            // and other clearance
            return 0;
        }else if(e.key == 'd' && mgr.is_doing_eye_tracking()) {
            time_for_drift_correct = !time_for_drift_correct;
            // don't return 1 here, put it in the event queue
        }else if((e.key == 'e'||e.key == 'g') && mgr.is_doing_eye_tracking()) {
            if(current_state != StateEyelinkCalibration::get_instance()) {
                time_for_eyelink_calibration = true;
            }
        }
        event_queue.push_back(e);
        // the return 0 decision should be upon each 'else if', I'm just
        // lazy here to get the default 'c' action.
        return 0;
    }else if (e.type == WE_MOUSE_3D_BUTTON_DOWN 
            ||e.type == WE_MOUSE_DOWN
            ||e.type == WE_MOUSE_WHEEL
            ) {
        event_queue.push_back(e);
        return 1;
    }

    return 0;
}

bool App::start_new_trial(bool restart) 
{
    if(!restart)
        ++ current_trial_idx ;

    if(current_trial_idx == trials.size()) // no more trials to run
        return false;

    update_states();

    // start the recorder
    cerr << "Writing header for trial " << current_trial_idx << "\n";
    recorder->grab_write_lock();
    recorder->write_header(current_trial_idx, trials[current_trial_idx]);
    recorder->release_write_lock();

    timer.reset();
    return true;
}

void App::finish_one_trial(bool bad)
{
    if(bad)
        trials[current_trial_idx].status = BAD_TRIAL;
    else
        trials[current_trial_idx].status = GOOD_TRIAL;

    cerr << "Writing tail for trial " << current_trial_idx << "\n";

    recorder->grab_write_lock();
    recorder->write_tail(trials[current_trial_idx]);
    recorder->release_write_lock();

    if(mgr.is_doing_eye_tracking() && 
            current_trial_idx % mgr.get_drift_correct_frequency() == 0 && 
            current_trial_idx > 0)
        time_for_drift_correct = true;
}


int App::init(int argc, char **argv)
{
    if(argc != 5) {
        ::MessageBox(NULL, "Use the script or run it with \"spem subject current_block_mode block_schedule subject_number\"", "Wrong number of arguments", MB_OK);
        return 1;
    }

    // Init the Inventor database
    SoDB::init();

    // Get the subject's name
    string sub_name = argv[1];

    // The experiment manager takes care of maintaining user
    // profiles and experiment parameters.

    mgr.manage_subject(sub_name);
    mgr.set_current_block_mode(atoi(argv[2]));
    mgr.add_status_item("block_schedule", argv[3]);
    mgr.add_status_item("subject_number", argv[4]);

    string recorded_motion_name;

    // Write to log file?
    if(mgr.is_doing_logging()) {
        string fn = mgr.get_recording_name();
        fn+=".log";
        slog.open(fn.c_str());
    }

    slog << "loading viewer\n";
    viewer.load(mgr.get_calib_name());

    // Init the optotrak, both active and "passive" need it. 
    Optotrak::initialize();

    slog << "creating window.\n";
    // Create the window.
    unsigned int ws = GLWindow::default_settings|GLW_FULLSCREEN;
    if(mgr.is_doing_stereo()) {
        ws |= GLW_STEREO;
    }
    if(mgr.is_using_3d_mouse()) {
        ws |= GLW_MOUSE_3D;
    }

    win.reset(new GLWindow(ws));
    win->add_event_handler(this);

    // May need grid size and donut size from mgr.
    if(mgr.is_doing_eye_tracking()) {
        double w = mgr.get_eyelink_screen_width();
        double h = mgr.get_eyelink_screen_height();
        Eyelink::get_instance()->set_screen_size(w, h);
        ostringstream ns;
        ns << w << " " << h ;
        mgr.add_status_item("eyelink calib grid", ns.str());
        Eyelink::get_instance()->initialize();

        eye_tracer.reset(new EyePositionTracer);
        double p=viewer.get_pixel_size();
        double sx = p * win->get_width();
        double sy = p * win->get_height();
        eye_tracer->set_screen_info(sx, sy, w, h);
    }

    // Create the renderer.
    renderer.reset( new RenderManager(&viewer));
    renderer->set_smoothing(true);
    //renderer->set_smoothing(false);
    //renderer->set_blending(SoGLRenderAction::NONE);
    if(mgr.is_doing_stereo()) {
        renderer->set_doing_stereo(true);
    }
    win->add_painter(renderer.get());

    load_trials();

    slog << "creating kits\n";
    // Kits to drive the scenes of different states.
    create_kits();

    slog << "creating states\n";
    // Add new states here.
    create_states();
    
    mgr.add_status_item("recording|eyelink rate", mgr.get_eyelink_sampling_rate());
    mgr.add_status_item("recording|opto rate", mgr.get_opto_sampling_rate());

    recorder.reset(new DataRecorder<Datum>(mgr.get_recording_name(), mgr.get_opto_sampling_rate(), mgr.get_eyelink_sampling_rate()));
    producer.reset(new TrackingDataSource(
            120., // the screen refresh rate
            mgr.get_opto_sampling_rate(), // opto and eyelink sampling rate
            mgr.get_eyelink_sampling_rate(), // opto and eyelink sampling rate
            recorder.get(),
            &viewer,4));

    // Start the writer thread, but not dump to the recorder yet
    producer->start();

    slog << "ready to go.\n";

    // Let's start...
    if(mgr.is_doing_eye_tracking()) {
        current_state = StateEyelinkCalibration::get_instance();
    }else {
        start_new_trial();
        StatePreTrial::get_instance()->pre_action();
        current_state = StatePreTrial::get_instance();
    }

    return 0;
}

void App::create_kits()
{
    // The net transformation (the current transformation minus the
    // initial transformation
    net_transform = new SoMatrixTransform;
    net_transform->ref();

    // Lighting settings states can use.
    lm = new SoLightModel;
    lm->model = SoLightModel::BASE_COLOR;

    light_source = new SoDirectionalLight;

    table2scr=new SoMatrixTransform;
    table2scr->matrix=fill_matrix(viewer.get_tabletop_to_screen_matrix());
    table2scr->ref();
}

void App::create_states()
{
    StatePreTrial::create_instance(this);
    StatePostTrial::create_instance(this);
    StateTrial::create_instance(this);
    StateDone::create_instance(this);
    StateSpeeding::create_instance(this);
    StateInterlude::create_instance(this);
    if(mgr.is_doing_eye_tracking()) {
        StateEyelinkCalibration::create_instance(this);
        StateEyelinkDriftCorrect::create_instance(this);
    }
}

void App::delete_states()
{
    delete StatePreTrial::get_instance();
    delete StatePostTrial::get_instance();
    delete StateTrial::get_instance();
    delete StateDone::get_instance();
    delete StateSpeeding::get_instance();
    delete StateInterlude::get_instance();
    if(mgr.is_doing_eye_tracking()) {
        delete StateEyelinkCalibration::get_instance();
        delete StateEyelinkDriftCorrect::get_instance();
    }
}

void App::load_trials()
{
    string all_trial_text;//to save the trials in status file
    vector<string> trial_text;
    mgr.get_trials(trial_text);
    vector<string>::iterator b=trial_text.begin();
    while(b!=trial_text.end()) {

        // Put together the trial text and save to the
        // status file
        all_trial_text+='{';
        all_trial_text+=*b;
        all_trial_text+="}\n";

        Trial t(*b);
        cerr << "getting trial ";
        t.dump(cerr);
        cerr << " from input "<<*b <<"\n";

        switch(mgr.get_current_block_mode()) {
            case 1:// dynamic noise
            case 5:
            case 9:
                t.visual_pattern = DYNAMIC_NOISE;
                break;
            case 2: // static noise
            case 6:
            case 10:
                t.visual_pattern = STATIC_NOISE;
                break;
            default:
                t.visual_pattern = BLANK_SCREEN;
                break;
        }

        // Here's the meat.
        trials.push_back(t);
        ++b;
        if(mgr.get_current_block_mode() > 4) {
            // we only do one trial for the rating blocks
            break;
        }
    }

    mgr.add_status_item("trials", all_trial_text);
}

void App::update_states()
{
    // 
    // Not all states need update their scene according to current
    // trial.
    StatePreTrial::get_instance()->update_scene();
    StateTrial::get_instance()->update_scene();
}

