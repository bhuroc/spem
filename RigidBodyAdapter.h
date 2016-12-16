#ifndef _RIGIDBODYADAPTER_H_
#define _RIGIDBODYADAPTER_H_

/**
 *@file RigidBodyAdapter.h
 *@brief 
 */
#include <vector>
#include <BVL/math/linalg/linalg.h>
#include <BVL/motion/pose/Pose.h>
#include <BVL/motion/kalman/KalmanFilter.h>

class RigidBodyAdapter {
    public:
        RigidBodyAdapter();
        ~RigidBodyAdapter();

        int get_num_markers() const {
            return num_markers;
        }
        int load(const char *filename);
        void setup_filters(double pos_noise, double vel_noise, double accel_noise);
        void init_filters(const BVL::Vector<double> &x0, const BVL::Matrix<double> &P0);

        void compute_pose();
        void set_marker_position(double x, double y, double z, int n);
        int get_pose(BVL::Pose &d) const {
            d.set_rotation(rot);
            d.set_translation(translation);
            return 0;
        }

        int get_filtered_pose(BVL::Pose &d) const {
            d.set_rotation(rot_filtered);
            d.set_translation(translation_filtered);
            return 0;
        }

        void get_filtered_velocity(double &vx, double &vy, double &vz) const;

    private:
        int num_markers;
        int future_steps;
        std::vector<BVL::Vector<double> > x_rigidbody;
        std::vector<BVL::Vector<double> > x_opto, x_filtered;
        std::vector<BVL::KalmanFilter*> filters;
        BVL::Matrix<double> rot, rot_filtered;
        BVL::Vector<double> translation, translation_filtered;
        double velo_x, velo_y, velo_z;
};


#endif/*_RIGIDBODYADAPTER_H_*/

