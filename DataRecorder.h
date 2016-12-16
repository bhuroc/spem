#ifndef _DATARECORDER_H_
#define _DATARECORDER_H_

/**
 *@file DataRecorder.h
 *@brief 
 */
#include <windows.h>
#include <iostream>
#include <fstream>
#include "Trial.h"

template <typename T>
class DataRecorder 
{
    public:
        DataRecorder(const char *filename, double opto_rate, double eyelink_rate) {
            os = new std::ofstream(filename);
            *os << opto_rate << " " << eyelink_rate << "\n";
            os_lock = CreateMutex(NULL, FALSE, NULL);
        }


        ~DataRecorder() {
            delete os;
            CloseHandle(os_lock);
        }

        void grab_write_lock() {
            WaitForSingleObject(os_lock, INFINITE);
        }

        void release_write_lock() {
            ReleaseMutex(os_lock);
        }
        
        void write_delimiter() {
//#define DEBUG
#ifdef DEBUG
            *os << ":";
#endif
//#undef DEBUG
        }

        void write_data(const T &d) {
            *os << d << " ";
        }

        void write_header(int no, const Trial &t) {
            *os << no <<"==" ;
        }

        void write_tail(const Trial &t) {
            flush();// to flush data frames
            *os << "\n" << t << "\n";
            flush();// prepare for next trial
        }

        void flush() {
            os->flush();
        }

    private:
        std::ostream *os;
        HANDLE os_lock;

};

#endif/*_DATARECORDER_H_*/

