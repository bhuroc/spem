#ifndef _FILECOLLECTOR_H_
#define _FILECOLLECTOR_H_

/**
 *@file FileCollector.h
 *@brief 
 */
#include <iostream>
#include <assert.h>
#include <VML/Optotrak/PoseCollector.h>
#include "DataReader.h"
#include "Datum.h"
#include "Trial.h"

class FileCollector : public VML::PoseCollector
{
    public:
        FileCollector(DataReader *r) :reader(r) {}
        ~FileCollector() {}


        int update_frame() {
            bool more = reader->read_data(d);
            if(more)
                return d.opto_frame_no;
            return -1;
        }

        int get_position(double *const x, int n=0) const  {
            x[0] = d.x[n];
            x[1] = d.y[n];
            x[2] = d.z[n];
            return d.opto_frame_no;
        }

        int get_position(double &x, double &y, double &z, int n=0) const {
            x = d.x[n];
            y = d.y[n];
            z = d.z[n];
            return d.opto_frame_no;
        }

        int get_filtered_position(double &x, double &y, double &z, int n=0) const {
            x = d.x[n];
            y = d.y[n];
            z = d.z[n];
            return d.opto_frame_no;
        }

        int get_filtered_velocity(double &vx, double &vy, double &vz, int n=0) const
        {
            //assert(0 && "Not implemented");
            return -1;
        }

        int get_pose(BVL::Pose &data, int n=0) const {
            data.set_translation(d.x[n],d.y[n],d.z[n]);
            data.set_rotation(0, 0, 0);
            return d.opto_frame_no;
        }

    private:
        Datum d;
        DataReader *reader;
};

#endif/*_FILECOLLECTOR_H_*/

