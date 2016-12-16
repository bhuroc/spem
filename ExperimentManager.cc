/**
 *@file ExperimentManager.cc
 *@brief 
 */
#include <iostream>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <string>
#include "ExperimentManager.h"

#define CONFIG_FILENAME "config.tcl"

using namespace std;
using namespace VML;

ExperimentManager::ExperimentManager()
{
    config.open(CONFIG_FILENAME);

    forced_no_eye_tracking = false;
}

void ExperimentManager::manage_subject(const std::string &n) 
{
    user_status.open(n.c_str());
    user_status.set_doing_recording(true);

    extern char version[];
    user_status.add_status_item("program_version", version);

    // save the params to the status file
    vector<string> params;
    config.get_all_params(params);
    string formatted_params;
    vector<string>::iterator p=params.begin();
    while(p != params.end()) {
        formatted_params += '{';
        formatted_params += *p++;
        formatted_params += "}\t";

        // see if *p has space in it
        string value=*p++;
        if(value.find_first_of(" \n\r\t") != string::npos) {
            formatted_params +='{';
            formatted_params += value;
            formatted_params +='}';
        }else {
            formatted_params += value;
        }
        formatted_params += "\n";
    }
    user_status.add_status_item("params", formatted_params);
}


ExperimentManager::~ExperimentManager()
{
}

