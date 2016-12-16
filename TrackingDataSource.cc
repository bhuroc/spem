/**
 *@file TrackingDataSource.cc
 *@brief 
 */
#include <windows.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <process.h>
#include "TrackingDataSource.h"
#include "DataRecorder.h"
#include "Datum.h"
#include "ExperimentManager.h"
#include "FileCollector.h"

using namespace std;
using namespace BVL;
using namespace VML;
using std::max;

extern ExperimentManager mgr;

TrackingDataSource::TrackingDataSource(double screen_refresh_rate, double opto_rate, double eyelink_rate, DataRecorder<Datum> *s, const VML::ViewingTransforms *viewer, int num_tool_markers)
{
    OptoCollector *opto = new OptoCollector;
    opto->set_frame_frequency(opto_rate);
    num_markers = num_tool_markers;
    opto->add_markers(num_tool_markers);
    opto->setup_collection();
    opto->activate();

    xform_opto2screen = viewer->get_world_to_screen_transform();

    collector = opto;
    double producing_rate = max(opto_rate, eyelink_rate);
    init(screen_refresh_rate,producing_rate,s);

}

TrackingDataSource::TrackingDataSource(double screen_refresh_rate, double eyelink_rate, DataReader *r, DataRecorder<Datum> *s, const ViewingTransforms *viewer)
{
    collector = new FileCollector(r);

    xform_opto2screen = viewer->get_world_to_screen_transform();
    double opto_rate = mgr.get_recorded_opto_rate();
    double producing_rate = max(opto_rate, eyelink_rate);
    init(screen_refresh_rate,producing_rate,s);

}


void TrackingDataSource::init(double screen_refresh_rate, double producing_freq, DataRecorder<Datum> *s)
{
    // If sampling_interval is 16.7ms: 60Hz both eyes
    // producing_freq is 250Hz, or every 4ms a frame
    // if we want to store data for a whole sampling_interval
    // the size is sampling_interval*producing_freq/1000
    int size = ceil(producing_freq/screen_refresh_rate)*4;
    sink = s;
    buf = new BoundedBuffer<Datum, DataRecorder<Datum> >(size, s);
    done = false;
    thread_id = 0;
    paused = true;

    // Have to store the parameters, which are managed by tcl in the main
    // thread, so the working thread won't need to access that interp.
    is_doing_eye_tracking = mgr.is_doing_eye_tracking();
    has_lock = false;
}

TrackingDataSource::~TrackingDataSource()
{
    OptoCollector *c=dynamic_cast<OptoCollector*>(collector);
    if(c)
        c->deactivate();
    delete collector;
    delete buf;
}

static unsigned __stdcall run_producer(void *arg)
{
    TrackingDataSource *producer = (TrackingDataSource*)arg;
    cerr << "starting the write thread.\n";
    producer->write();

    return 0;
}

void TrackingDataSource::start()
{
    unsigned tid;
    missing_frame = 0;
    thread_id=(HANDLE)_beginthreadex(NULL, 0, run_producer, this, 0, &tid);
}

void TrackingDataSource::write()
{
    cerr << "enter the write thread."<<endl;
    while(!done) {
        Datum d;

        // optotrak part
        d.opto_frame_no = collector->update_frame();
        d.time_stamp = timer.get_elapsed_millisec();
        if(d.opto_frame_no == -1) {
            d.opto_frame_no = -- missing_frame ;
            d.fill_invalid_data();
        }else{ 
            for(int i=0; i<num_markers; ++i) {
                double rd[3];
                collector->get_position(rd, i);

                Vector<double> marker_opto(3, rd), marker_scr;
                marker_scr = xform_opto2screen.transform(marker_opto);

                d.fill_data(marker_opto, marker_scr, i);
            }
        }

        if(is_doing_eye_tracking) {

            // eyelink part
            d.eyelink_frame_no = eye_tracker.update_frame();
            EyelinkDataFrame e;
            eye_tracker.get_data(e);

            d.eyelink_time_stamp = e.time_stamp;
            d.left_pupil_size = e.left_pupil_size;
            d.right_pupil_size = e.right_pupil_size;
            d.left_screen_gaze[0] = e.left_screen_gaze[0];
            d.left_screen_gaze[1] = e.left_screen_gaze[1];
            d.right_screen_gaze[0] = e.right_screen_gaze[0];
            d.right_screen_gaze[1] = e.right_screen_gaze[1];
            d.resolution_x = e.resolution_x;
            d.resolution_y = e.resolution_y;
        }

        /*
        if(paused) {
            sink->release_write_lock();
            has_lock = false;
        }else if(!has_lock) {
            has_lock = true;
            sink->grab_write_lock();
        }
        */
        buf->write(d);
    }
    cerr << "quit the write thread."<<endl;
}

Datum TrackingDataSource::read()
{
    return buf->read();
}

Datum TrackingDataSource::read_dump()
{
    Datum d;
    if(paused) {
        d = buf->read();
    }else {
        d = buf->read_dump();
    }

    return d;
}

void TrackingDataSource::pause_recording() 
{
    paused = true;
    clear_buffer();
    if(is_doing_eye_tracking) {
        eye_tracker.stop_recording();
    }
}

void TrackingDataSource::start_recording() 
{
    if(is_doing_eye_tracking) {
        eye_tracker.start_recording();
    }
    paused = false;
}

void TrackingDataSource::stop()
{
    done = true;
    cerr << "wait for the write thread."<<endl;
    buf->release();
    WaitForSingleObject(thread_id, INFINITE);

    cerr << "write thread terminated."<<endl;
}


