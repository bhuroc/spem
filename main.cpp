/**
 *@file main.cpp
 *@brief 
 */
#include <iostream>
#include <stdexcept>
#include <VML/System/debug.h>
#include "App.h"

extern void print_version();

int main(int argc, char **argv)
{
    if(argc == 2 && argv[1][1] == 'v') { // both /v and -v will work.
        print_version();
        return 0;
    }

    VML::check_debug_env();
    App app;

    try {
        if(app.init(argc, argv)) {
            std::cerr << "Can't init app.\n";
            return 1;
        }
    } catch (std::exception &e) {
        std::cerr << "init threw an exception " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "init threw an unknown exception \n";
    }

    app.main_loop();

    return 0;
}



