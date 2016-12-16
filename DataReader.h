#ifndef _DATAREADER_H_
#define _DATAREADER_H_

/**
 *@file DataReader.h
 *@brief 
 */
#include <iostream>
#include <fstream>
#include <sstream>
#include <deque>
#include <assert.h>
#include <algorithm>
#include <iterator>
#include <VML/System/debug.h>
#include "Trial.h"
#include "Datum.h"
#include "ExperimentManager.h"

#undef max

class DataReader
{
    public:
        DataReader(const char *filename) {
            std::cerr << "creating reader from file " << filename << " ";
            is = new std::ifstream(filename);
            assert(is);
            started = false;
            // read in the sampling rate of the optotrak and eyelink
            double opto_rate, eyelink_rate;
            std::string line;
            getline(*is, line);

            std::istringstream ss(line);
            ss >> opto_rate >> eyelink_rate;
            if(is->bad() || ss.bad()) {
                vml_error("Can't read sampling rates from recording.");
            }
            std::cerr << "opto rate " << opto_rate << " eyelink rate " << eyelink_rate << "\n";

            // time gap between two recorded frames
            double faster_rate = std::max( eyelink_rate, opto_rate);
            mean_gap = 1000./faster_rate;
        }

        ~DataReader() {
            delete is;
        }

        // Copy the motion data to m, without simulating the recording
        // process, as does in read_data.
        void get_motion(std::vector<Datum> &m) {
            std::copy(motion.begin(), motion.end(), std::back_inserter<std::vector<Datum>>(m));
        }

        // Return true if there are more data to read
        bool read_data(Datum &d) {
            // XXX this is checked in the writer thread, but
            // setting the started value is in the reader thread
                
            if(motion.empty()) {
                return false;
            }else if(!started) {
                d = motion.front();
                // don't do epoch=d.opto_frame_no, because d.frame_no
                // is set to 0 right after.
                epoch_frame_no = motion.front().opto_frame_no;
                d.opto_frame_no = 0;
                prev_d = d;
                return false;
            }else {
                d = motion.front();
                d.opto_frame_no -= epoch_frame_no;
                // 4 if optotrak samples at 250 hz
                // 2 if because eyelink samples at 500hz
                // because eyelink is at least as fast as optotrak
                // use its time_stamp to compute the time between
                // two frames.
                double wait_time = (d.eyelink_time_stamp-prev_d.eyelink_time_stamp);
                if(wait_time <= 0)
                    wait_time = mean_gap;
                if(timer.get_elapsed_millisec() >= wait_time){
                    motion.pop_front();
                    timer.reset();
                }
                prev_d = d;
                return true;
            }
        }

        void start() { started = true; timer.reset();}
        void stop() { started = false;}

        // Read a whole trial, return false when no more data's in
        // the recorded file
        bool read_one_trial(Trial &t) {
            // Read one line
            std::string line;
            getline(*is, line);
            if(is->eof()) {
                std::cerr << "EOF\n";
                return false;
            }

            std::istringstream ss(line);
            motion.clear();

            // Read trial no and the motion
            int trial_no;
            ss >> trial_no;
            while(!ss.eof()) {
                Datum x;
                ss >> x;
                if(ss.good()) {
                    motion.push_back(x);
                }
            }

            if(ss.bad()) {
                std::cerr << "WARNING: after reading motion, state is bad.\n";
            }

            // Read the trial
            getline(*is, line);
            if(is->eof()) {
                std::cerr << "EOF\n";
                return false;
            }

            ss.clear();
            ss.str(line);
            ss >> t;
            if(ss.bad()) {
                std::cerr << "FATAL: Bad recording.\n";
                return false;
            }
            std::cerr << "Read trial " << trial_no << ": " << t << " length " << motion.size() << "\n";
            return true;
        }

    private:
        std::istream *is;
        bool started;
        std::deque<Datum> motion;
        VML::ElapsedTimer timer;
        Datum prev_d;
        double mean_gap;
        int epoch_frame_no;
};

#endif/*_DATAREADER_H_*/

