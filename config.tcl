set run_from_shell 0
if {[info exists argv0]} {
    set run_from_shell 1
}

#######################################################################
#
# Creating trials
#
#######################################################################
proc create_trials_real { } {
    global trials

    # pattern 0 dynamic 2 static 4 blank
    lappend trials {2 }
    lappend trials {2 }
    lappend trials {2 }
    lappend trials {2 }

    shuffle trials 0 end
    param "number of trials" [llength trials]
}

proc create_trials_test { } {
    global trials

    # pattern 0 dynamic 2 static 4 blank
    lappend trials {0 }
    lappend trials {0 }
    lappend trials {0 }
    lappend trials {0 }
    lappend trials {2 }
    lappend trials {4 }

    param "number of trials" [llength trials]
}

# having one extra level of abstraction, so ExperimentManager
# can set subject's name, info from last block and so on, before
# calling this function, without worrying the creator type
proc create_trials { } {
    global params

    switch -exact -- $params(trial\ creator\ type) {
        real { create_trials_real }
        test { create_trials_test }
    }
}

#######################################################################
#
# Loading parameters
#
#######################################################################
if {$run_from_shell == 0 } {
    source params.txt
}

#######################################################################
#
# Off-line test
#
#######################################################################
if { $run_from_shell == 1 } {
    if { [info exist env(VMLROOT)] } {
        set paramlib [file join $env(VMLROOT) lib paramlib.tcl]
    } else {
        puts "VMLROOT not defined."
    }
    source $paramlib
    source params.txt
    create_trials
    output_list $trials
}

