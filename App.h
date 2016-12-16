#ifndef _APP_H_
#define _APP_H_

/**
 *@file App.h
 *@brief 
 */
#include <vector>
#include <deque>
#include <memory>
#include <BVL/math/fit/LineFit2D.h>
#include <VML/GUI/VMLApp.h>
#include <VML/GUI/GLWindow.h>
#include <VML/GUI/RenderManager.h>
#include <VML/System/ElapsedTimer.h>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoMatrixTransform.h>
#include <Inventor/nodes/SoRotationXYZ.h>
#include <Inventor/nodes/SoLightModel.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/engines/SoComposeMatrix.h>
#include <Inventor/engines/SoCalculator.h>
#include "EyePositionTracer.h"
#include "Trial.h"
#include "TrackingDataSource.h"
#include "DataRecorder.h"
#include "DataReader.h"

class Datum;
class State;

class App : public VML::VMLApp, public VML::EventHandler
{
    public:
        App();
        ~App();

        int init(int argc, char **argv);

        const Trial* get_current_trial() {
            return &(trials[current_trial_idx]);
        }

        SoLightModel *get_light_model() const {
            return lm;
        }

        SoDirectionalLight *get_light_source() const {
            return light_source;
        }

        SoMatrixTransform *get_table2scr() {
            return table2scr;
        }

        SoMatrixTransform *get_net_transform() {
            return net_transform;
        }

        /** 
         * Returns true if there are more trials, false otherwise.
         *
         * If a trial turns bad, call this function with true to rerun
         * the trial.
         */
        bool start_new_trial(bool restart = false);

        void update_pose();

        // Force a real read_dump() to clear the buffer
        void force_updating_pose() {
            producer->clear_buffer();
        }

        // Record the reported angle.  If bad is true, this
        // trial is bad (too fast or too slow hand movement,e.g.)
        void finish_one_trial(bool bad=false);

        bool do_drift_correct() const {
            return time_for_drift_correct;
        }

        void done_drift_correct() {
            time_for_drift_correct = false;
        }

        bool do_eyelink_calibration() const {
            return time_for_eyelink_calibration;
        }

        void done_eyelink_calibration() {
            time_for_eyelink_calibration = false;
        }


        void stop_recording() {
            if(reader) {
                reader->stop();
            }
            producer->pause_recording();
            recorder->flush();
        }

        void start_recording() {
            if(reader) {
                reader->start();
            }
            producer->start_recording();
        }

        // for debugging
        // makrer 4 is the one attached to the finger
        void get_handle_position(double &x, double &y, double &z) const {
            x = saved_d.x[3];
            y = saved_d.y[3];
            z = saved_d.z[3];
        }

        void get_handle_velocity(double &vx, double &vy, double &vz) const {
            /*
            vx = saved_d.vx;
            vy = saved_d.vy;
            vz = saved_d.vz;
            */
        }

        bool idle(int); 
        int handle_event(const VML::WindowEvent &e); 

    private:
        void create_kits();
        void create_states();
        void delete_states();
        void update_states();
        void load_trials();
        void capture_screen();

        std::tr1::shared_ptr<VML::GLWindow> win;
        std::tr1::shared_ptr<VML::RenderManager> renderer;
        std::vector<Trial> trials;
        int current_trial_idx;
        std::deque<VML::WindowEvent> event_queue;
        VML::ElapsedTimer timer;
        std::tr1::shared_ptr<TrackingDataSource> producer;
        std::tr1::shared_ptr<DataRecorder<Datum>> recorder;
        std::tr1::shared_ptr<DataReader> reader;
        State *current_state;

        SoLightModel *lm;
        SoDirectionalLight *light_source;

        SoMatrixTransform *table2scr;
        SoMatrixTransform *net_transform;
        
        bool time_for_drift_correct;
        bool time_for_eyelink_calibration;

        Datum saved_d;

        bool tracing_eye;
        std::tr1::shared_ptr<EyePositionTracer> eye_tracer;

};

#endif/*_APP_H_*/

