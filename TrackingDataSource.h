#ifndef _DATAPRODUCER_H_
#define _DATAPRODUCER_H_

/**
 *@file TrackingDataSource.h
 *@brief Real time (through threading) Opto/Eye-tracking data; either from 
 * real device or recorded data.
 */
#include <windows.h>
#include <deque>
#include <VML/Optotrak/Optotrak.h>
#include <VML/Optotrak/OptoCollector.h>
#include <VML/System/ElapsedTimer.h>
#include <VML/System/BoundedBuffer.h>
#include <VML/Graphics/ViewingTransforms.h>
#include "EyelinkCollector.h"
#include "DataRecorder.h"
#include "DataReader.h"
#include "Datum.h"

class TrackingDataSource
{
    public:
        TrackingDataSource(double screen_refresh_rate, double opto_rate, double eyelink_rate, DataRecorder<Datum> *s, const VML::ViewingTransforms *v, int num_tool_markers=3);
        TrackingDataSource(double screen_refresh_rate, double eyelink_rate, DataReader *r, DataRecorder<Datum> *s, const VML::ViewingTransforms *v);
        ~TrackingDataSource();

        /**
         * Start and stop the producer's thread.
         */
        void start();
        void stop();

        /**
         * The producer's thread, writing to the bounded buffer.
         */
        void write();

        /**
         * The consumer.
         */
        Datum read();
        Datum read_dump();

        /**
         * The writer sometimes will be stopped because the buffer
         * is full.  We need this to manually restart the writer to
         * get the latest positional data.*/
        void clear_buffer() {
            buf->clear();
        }

        /**
         * Pause and restart dumping to the sink.
         */
        void pause_recording();
        void start_recording();

    private:
        void init(double screen_refresh_rate, double producing_freq, DataRecorder<Datum> *s);

        int num_markers;
        HANDLE thread_id;
        VML::PoseCollector *collector;
        EyelinkCollector eye_tracker;
        VML::BoundedBuffer<Datum, DataRecorder<Datum> > *buf;
        DataRecorder<Datum> *sink;
        volatile bool done;
        VML::ElapsedTimer timer;
        bool paused;
        int missing_frame;
        VML::AffineTransform xform_opto2screen;
        bool is_doing_eye_tracking;
        bool has_lock;
};

#endif/*_DATAPRODUCER_H_*/

