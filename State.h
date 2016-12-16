#ifndef _STATE_H_
#define _STATE_H_

/**
 *@file State.h
 *@brief 
 */
#include <VML/GUI/WindowEvent.h>
#include <VML/System/ElapsedTimer.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/SoPath.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoRotationXYZ.h>
#include <Inventor/nodes/SoTexture2Transform.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoAsciiText.h>

class App;
class Eyelink;

class State
{
    protected:
        State(App *a) : app(a) {}

    public:
        virtual ~State() {
            root->unref();
        }

        virtual void update_scene() {}
        virtual void pre_action() {}
        virtual void post_action() {}
        virtual State *transition(const VML::WindowEvent &e) = 0;
        virtual State *change_state(State *from, State *to) {
            from->post_action();
            to->pre_action();
            return to;
        }

        virtual SoSeparator *get_scene() const {
            return root;
        }
        virtual const char* get_name() const = 0;
    protected:
        App *app;
        SoSeparator *root;
        static VML::ElapsedTimer timer;
};

#define NEWSTATE(state_name)  \
    protected:\
        State##state_name(App *);\
    public:\
        static void create_instance(App *app) {\
           instance = new State##state_name(app);\
        }\
        static State *get_instance() {\
            return instance;\
        }\
        State *transition(const VML::WindowEvent &e);\
        const char *get_name() const {\
            return #state_name;\
        }\
    private:\
        static State *instance

#define NEWSTATE_SOURCE(state_name) State *State##state_name::instance = 0


class StateInit: public State
{
    NEWSTATE(Init);
    public:
    ~StateInit() { handle_path->unref(); }
    private:
    SoPath *handle_path;
};

class StatePreTrial: public State
{
    NEWSTATE(PreTrial);

    public:
        void update_scene();
        void pre_action();

    private:
        SoAsciiText *prompt_text;
        bool wait_for_lining_up;
};

class StatePostTrial: public State
{
    NEWSTATE(PostTrial);
};

class StateTrial: public State
{
    NEWSTATE(Trial);

    public:
        void update_scene();
        int get_noise_width() {
            return noise_width;
        }

        int get_noise_height() {
            return noise_height;
        }

        const unsigned char *get_noise_data();

        State *go_next();
    private:
        void read_noise();

        SoSwitch *display_switch;
        double average_speed;
        int speed_count;
        double low_speed_limit, high_speed_limit;

        bool first_transition;

        int noise_width, noise_height;
        std::vector<unsigned char *> noise_data;
        SoTexture2 *square_dots;
        double beep_time, beep_time0;
        int beep;
        int noise_frame;
};

class StateDecision: public State
{
    NEWSTATE(Decision);
    
    public:
        void update_scene();

    private:
        SoRotationXYZ *compass_orient;
};

class StateDone: public State
{
    NEWSTATE(Done);
        void pre_action() {
            play_finish_sound = true;
        }
    private:
    bool play_finish_sound;
};

class StateSpeeding: public State
{
    NEWSTATE(Speeding);
    public:
        void set_speed_violation(int);

    private:
        SoSwitch *speeding_text;
};

class StateEyelinkCalibration: public State
{
    NEWSTATE(EyelinkCalibration);
    private:
        Eyelink *eyelink;
        bool wait_for_eyelink;
};

class StateEyelinkDriftCorrect: public State
{
    NEWSTATE(EyelinkDriftCorrect);
    private:
        Eyelink *eyelink;
};


class StateInterlude: public State
{
    NEWSTATE(Interlude);

    public:
        void pre_action();

};

#endif/*_STATE_H_*/

