set status_lib [file join $env(VMLROOT) "lib" "statuslib.tcl"]

proc enable_run_button {} {
    global subject_found 
    if { $subject_found } {
        .f.bok configure -state active
    } else {
        .f.bok configure -state disabled 
    }
}

proc check_subject {subject_input} {
    global subject_found 
    puts [pwd]
    if { [file exists "calib_$subject_input.txt" ] } {
        puts "subject found"
        set subject_found 1
    } else {
        puts "subject not found"
        set subject_found 0
    }

    enable_run_button
    return 1
}


set subject_found 0
# 1: dynamic noise
# 2: static noise
# 3: hand visible
# 4: dark
set subject_block_order_latin {\
{4	3	1	2	3	4	2	1}
{1	4	3	2	1	3	2	4}
{2	3	1	4	2	4	3	1}
{1	2	3	4	1	2	3	4}
{4	1	2	3	3	1	4	2}
{4	2	3	1	2	1	4	3}
{1	3	4	2	4	3	1	2}
{3	2	4	1	3	4	2	1}
{1	4	3	2	2	1	4	3}
{3	4	2	1	4	3	1	2}
{2	1	4	3	2	1	4	3}
{3	2	4	1	4	2	1	3}
{3	1	2	4	3	4	2	1}
{2	3	1	4	4	3	1	2}
{4	1	2	3	1	2	3	4}
{2	4	1	3	1	2	3	4}
}

proc append_rating_blocks { latin numbering} {
    if {[expr $numbering % 2] == 0} {
        for { set i 0 } {$i < 4} {incr i 1} {
            lappend latin [expr [lindex $latin $i] + 4]
        }
        for { set i 0 } {$i < 4} {incr i 1} {
            lappend latin [expr [lindex $latin $i] + 8]
        }
    } else {
        for { set i 0 } {$i < 4} {incr i 1} {
            lappend latin [expr [lindex $latin $i] + 8]
        }
        for { set i 0 } {$i < 4} {incr i 1} {
            lappend latin [expr [lindex $latin $i] + 4]
    }
}

    puts "append $latin"
    return $latin
}

proc build_full_subject_block_orders { } {
    global subject_block_order_latin
    global subject_block_order

    set i 0 
    foreach b $subject_block_order_latin {
        lappend subject_block_order [append_rating_blocks $b $i]
        incr i
    }

    puts "the full block orders"
    puts $subject_block_order
}

build_full_subject_block_orders

set subject_number 1
set block_schedule [lindex $subject_block_order 0]

frame .f -borderwidth 2 -relief ridge -padx 10 -pady 10 
ttk::label .f.lsubject -text "Subject name" -anchor w 
entry .f.esubject -textvariable subject_name -validate key -vcmd {check_subject %P}
ttk::button .f.bok -text Run -command run -state disabled
ttk::button .f.bquit -text Quit -command quit

ttk::label .f.subject_number_label -text "Subject number" -anchor w
spinbox .f.subject_number -from 1 -to [llength $subject_block_order] -width 10 -textvariable subject_number -command { set block_schedule [lindex $subject_block_order [expr $subject_number - 1]]}

frame .w -padx 10 -pady 10
text .w.text -width 40 -height 15 -wrap none
scrollbar .w.yscr -orient vertical -command [list .w.text yview]
scrollbar .w.xscr -orient horizontal -command [list .w.text xview]

pack .f -pady 5 -fill x -padx 10 -ipadx 10 
grid .f.lsubject .f.esubject -sticky news -pady 5 -padx 5
grid .f.subject_number_label .f.subject_number -sticky news -pady 5 -padx 5
grid .f.bok .f.bquit -pady 5 

pack .w -side bottom -fill both
grid .w.text .w.yscr -sticky news
grid .w.xscr -sticky news

catch {console show}

proc sync_output {fid} {
    #puts [gets $fid]
    set c [gets $fid line]
    if {$c>=0} {
        .w.text insert end "$line\n"
    }
    if {[eof $fid]} {
        puts "program closed."
        close $fid
    }
}

proc check_status_file {} {
    global block_schedule
    global status_lib subject_name
    global current_block_mode
    global status

    set status_name [file join $subject_name "status.txt"]

    if {[file exists $status_name]} {
        source $status_lib
        source $status_name

        get_status
        set block_schedule [get_entry "block_schedule"]
        set finished_blocks $status(number_of_finished_blocks)

        puts -nonewline "$subject_name has finished $finished_blocks blocks "
        # in run(), I've grouped the block schedule using :, so I need
        # to split them to list again here
        set current_block_mode [lindex [split $block_schedule :] $finished_blocks]
        puts "and current block is $current_block_mode of $block_schedule."

    } else {
        # this is the subject's first block
        # 
        set current_block_mode [lindex $block_schedule 0]
        puts "This is $subject_name's first block and it's a $current_block_mode block."
    }

}

proc run {} {
    # clear the text widget
    .w.text delete 1.0 end
    global kid subject_name block_schedule 
    global current_block_mode
    global subject_number

    puts "===================="
    puts $subject_name
    puts $subject_number
    puts $block_schedule
    puts "===================="

    check_status_file

    puts "|spem $subject_name $current_block_mode [join $block_schedule :] $subject_number"

    set messages {
        "Today's your lucky day."
        "Dynamic noise. Insert the backing and turn off the light."
        "Static noise. Insert the backing and turn off the light."
        "Hand visible.  Pull out the backing, turn off the monitor and turn on the light."
        "Hand invisible.  Pull out the backing and turn off the monitor and the light."
        "Dynamic noise pursuit rating. Insert the backing and turn off the light."
        "Static noise pursuit rating. Insert the backing and turn off the light."
        "Hand visible pursuit rating.  Pull out the backing, turn off the monitor and turn on the light."
        "Hand invisible pursuit rating.  Pull out the backing and turn off the monitor and the light."
        "Dynamic noise fixating rating. Insert the backing and turn off the light."
        "Static noise fixating rating. Insert the backing and turn off the light."
        "Hand visible fixating rating.  Pull out the backing, turn off the monitor and turn on the light."
        "Hand invisible fixating rating.  Pull out the backing and turn off the monitor and the light."
    }

    set msg [lindex $messages $current_block_mode]
    puts $msg
    tk_messageBox -default ok -message $msg -title Start -type ok
    set kid [open "|spem $subject_name $current_block_mode [join $block_schedule :] $subject_number 2>@1 " "r"]

    fconfigure $kid -blocking 0
    fileevent $kid readable "sync_output $kid"
}

proc quit {} {
    exit
}
