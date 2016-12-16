#ifndef _EXPERIMENTMANAGER_H_
#define _EXPERIMENTMANAGER_H_

/**
 *@file ExperimentManager.h
 *@brief 
 */
#include <string>
#include <sstream>
#include <BVL/math/linalg/linalg.h>
#include <VML/System/StringTokenizer.h>
#include <VML/System/TclConfigure.h>
#include <VML/System/UserStatus.h>

class ExperimentManager
{
    public:
        ExperimentManager();
        ~ExperimentManager();

        void manage_subject(const std::string &n);

        const char *get_calib_name() const {
            return user_status.get_calib_name();
        }

        const char *get_output_name() const {
            return user_status.get_output_name();
        }

        void add_status_item(const std::string &key, const std::string &val) {
            user_status.add_status_item(key, val);
        }

        template <typename T>
        void add_status_item(const std::string &key, const T &val) {
            std::ostringstream os;
            os<<val;
            user_status.add_status_item(key, os.str());
        }

        int get_current_block_number() const {
            return user_status.get_current_block_number();
        }

        std::string get_tool_name() const {
           return config.get_string("tool name");
        }

        void get_access_offset(double &x, double &y, double &z) {
            VML::StringTokenizer<double> s(config.get_string("access offset"));
            x = s.get_next_token();
            y = s.get_next_token();
            z = s.get_next_token();
        }

        BVL::Vector<double> get_robot_tool_offset() const {
            VML::StringTokenizer<double> s(config.get_string("tool offset"));
            BVL::Vector<double> t(6, 0.);
            t(1) =  s.get_token(0);
            t(2) =  s.get_token(1);
            t(3) =  s.get_token(2);
            t(4) =  s.get_token(3);
            t(5) =  s.get_token(4);
            t(6) =  s.get_token(5);

            return t;
        }

        bool is_doing_eye_tracking() const {
            if(forced_no_eye_tracking)
                return false;
            else
                return config.get_string("do eye tracking") == "true";
        }

        // for demo,which will ignore the eyetracking entry in the cfg file.
        void force_no_eye_tracking() {
            forced_no_eye_tracking = true;
        }

        bool is_using_3d_mouse() const {
            return config.get_string("use 3d mouse")=="true";
        }

        bool is_doing_rotation() const {
            return config.get_string("do rotation") == "true";
        }

        int get_num_trials() const {
            config.get_int("number of trials");
        }

        // Every so many trials to do a drift correction
        int get_drift_correct_frequency() const {
            return config.get_int("drift correct frequency");
        }

        // How far the tip is away from the screen center but still
        // considered to be at the center. Default 3mm.
        double get_tool_tip_tolerance() const {
            return config.get_double("tool tip tolerance");
        }

        // How long a trial lasts, or how long the subject will
        // move the cuby back and forth. Default : 3000 ms.
        double get_trial_duration() const {
            return config.get_double("trial duration");
        }

        // Default: true
        bool is_doing_stereo() const {
            return config.get_string("stereo") == "true";
        }

        const char *get_recording_name() const {
            return user_status.get_recording_name();
        }
        
        // How fast to run the Optotrak and Eyelink
        double get_opto_sampling_rate() const {
            return config.get_double("opto sampling rate");
        }

        void set_recorded_eyelink_rate(double r) {
            recorded_eyelink_rate = r;
        }

        void set_recorded_opto_rate(double r) {
            recorded_opto_rate = r;
        }

        double get_eyelink_sampling_rate() const {
            return config.get_double("eyelink sampling rate");
        }

        // the eyelink rate of the last recorded block
        double get_recorded_eyelink_rate() const {
            return recorded_eyelink_rate;
        }

        // the opto rate of the last recorded block
        double get_recorded_opto_rate() const {
            return recorded_opto_rate;
        }

        double get_eyelink_screen_width() const {
            return config.get_double("eyelink screen width");
        }

        double get_eyelink_screen_height() const {
            return config.get_double("eyelink screen height");
        }

        bool is_limiting_speed() const {
            return config.get_string("speed limits").size()!=0;
        }

        void get_speed_limits(double &low, double &high) const {
            VML::StringTokenizer<double> s(config.get_string("speed limits"));
            low = s.get_token(0);
            high = s.get_token(1);
        }

        void get_trials(std::vector<std::string> &trials) {
            config.run_command("create_trials");
            config.get_list("trials", trials);
        }

        bool is_doing_logging() const {
            return config.get_string("logging")=="true";
        }

        void set_current_block_mode(int m) {
            current_block_mode = m;
        }

        char get_current_block_mode() const {
            return current_block_mode;
        }

        // Returns the transfrom from the handle's local coords
        // to the opto rigid body coords. Can be obtained using
        // RigidLocalAligner
        BVL::Matrix<double> get_handle_transform() {
            return BVL::Matrix<double>(4,4,config.get_string("handle2rb transform").c_str());
        }

        std::string get_handle_iv_file() {
            return config.get_string("handle iv file");
        }

        int get_number_of_noise_images() {
            return config.get_int("number of noise images");
        }


        double get_beep_interval() {
            return config.get_double("beep interval");
        }

        double get_noise_contrast_low() const {
            return config.get_double("noise contrast low");
        }

        double get_noise_contrast_high() const {
            return config.get_double("noise contrast high");
        }

    private:
        VML::UserStatus user_status;
        VML::TclConfigure config;
        int current_block_mode;
        bool forced_no_eye_tracking;
        double recorded_opto_rate, recorded_eyelink_rate;
};

#endif/*_EXPERIMENTMANAGER_H_*/

