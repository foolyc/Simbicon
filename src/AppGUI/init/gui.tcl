lappend auto_path "tcllibs"
package require BWidget




proc editTrajectory { } {
	trajectoryToEdit [.dialog.stateSelect getvalue] [.dialog.trajectorySelect getvalue]
}
  
proc populateTrajectories { } {
	.dialog.trajectorySelect configure -values [getTrajectoryNames [.dialog.stateSelect getvalue] ]
	.dialog.trajectorySelect setvalue first
	editTrajectory
}
  
  
#A procedure to make a toplevel window
proc makeControlDialog { } {
	toplevel .dialog ;#Make the window
	#Put things in it
	frame .dialog.editionbuttons -padx 15 -pady 5
	image create photo undo -file [file join ../data/buttons/undo.gif]
	image create photo redo -file [file join ../data/buttons/redo.gif]
	image create photo recordState -file [file join ../data/buttons/recS.gif]
	checkbutton .dialog.recordState -image recordState -anchor center -width 21 -height 21 -selectcolor SeaGreen1 -indicatoron 0 -variable toggleControlshots
	button .dialog.undo -image undo -anchor center -width 23 -height 23 -command {controllerUndo}
	button .dialog.redo -image redo -anchor center -width 23 -height 23 -command {controllerRedo}
	pack .dialog.undo .dialog.redo .dialog.recordState -side left -in .dialog.editionbuttons -anchor w
	frame .dialog.shotinfo -padx 15 -pady 5
	label .dialog.nextshotlabel -text "Current controller :" 
	label .dialog.nextshot -textvariable currControlShot
	pack .dialog.nextshotlabel .dialog.nextshot -side left -in .dialog.shotinfo -anchor w
	ComboBox .dialog.stateSelect -values [getStateNames] -editable false -width 40 -height 0 -modifycmd populateTrajectories
	ComboBox .dialog.trajectorySelect -values { {Not specified} } -editable false -width 40 -height 0 -modifycmd editTrajectory
	#Pack everything
	pack .dialog.editionbuttons .dialog.shotinfo .dialog.stateSelect .dialog.trajectorySelect -in .dialog

	.dialog.stateSelect setvalue first
	populateTrajectories
	

}

proc finalizeUI {} {

	# Check if the commands to populate the curve edition combo boxes are available
	if {[info commands getStateNames] == "getStateNames"} {
		makeControlDialog
	}

}

proc reloadAnimation {} {

	anim reload
	if {[info commands getStateNames] == "getStateNames"} {
		.dialog.stateSelect configure -values [getStateNames]
		.dialog.stateSelect setvalue first
		populateTrajectories
	}
	
}

proc tryToUpdatePose {val} {
	if {[info commands updateTargetPose] == "updateTargetPose"} {
		updateTargetPose
	}
}



#### the main frame. All other frames should be packed into this one
frame .mainframe
pack .mainframe -side top -fill x

#### simulation speed
frame .simsettings
label .labelSpeed -text "Simulation Speed:" -relief raised
radiobutton .r0 -text x2 -variable slowdown -value 2 -anchor w 
radiobutton .r1 -text x1 -variable slowdown -value 1 -anchor w 
radiobutton .r2 -text x1/2 -variable slowdown -value 0.5 -anchor w 
radiobutton .r4 -text x1/4 -variable slowdown -value 0.25 -anchor w 
radiobutton .r8 -text x1/16 -variable slowdown -value 0.0625 -anchor w 
.r1 select
pack .labelSpeed -side top -fill x -in .simsettings -anchor w
pack .r0 .r1 .r2 .r4 .r8 -side left -fill x -in .simsettings

#### visualization options
frame .viewoptions
label .labelView -text "Visualization Options:" -relief raised
checkbutton .colprimitives -text "Show Collision Primitives" -variable drawCDPs -anchor w
checkbutton .axes -text "Show Global Axes" -variable drawGlobalAxes -anchor w
checkbutton .forces -text "Show Contact Forces" -variable drawContactForces -anchor w
checkbutton .target -text "Show Target Pose" -variable drawDesiredPose -anchor w
checkbutton .fps -text "Show FPS" -variable drawFPS -anchor w
checkbutton .shadow -text "Show Shadows" -variable drawShadows -anchor w
checkbutton .ground -text "Show Ground Plane" -variable drawGroundPlane -anchor w
.fps select
.shadow select
.axes select
#.target select
.ground select
pack .labelView -side top -fill x -in .viewoptions -anchor w
pack .axes .colprimitives .forces .target .fps .shadow .ground -side top -fill y -in .viewoptions -anchor w



#### camera options
frame .camerasettings
frame .followchar
button .v1 -text Front -anchor w -width 5 -command {camera c_front}
button .v2 -text Side -anchor w -width 5 -command {camera c_side}
button .v3 -text "  45" -anchor w -width 5 -command {camera c_45}
button .v4 -text " -45" -anchor w -width 5 -command {camera c_n45}
button .v8 -text Back  -anchor w -width 5 -command {camera c_back}
label .labelcamera -text "Camera:" -relief raised
checkbutton .camerafollowup -text "Follow Character" -variable followCharacter -anchor s

.camerafollowup select

pack .labelcamera -side top -fill x -in .camerasettings -anchor w
pack .v1 .v3 .v2 .v4 .v8 -side left -pady 3 -padx 4 -fill x -in .camerasettings -anchor e
pack .camerafollowup -side left -in .followchar -anchor n



#### simulator commands
frame .simulatorlabel
frame .simulatorframe -padx 15 -pady 5
label .labelsimulator -text "Animation:" -relief raised
image create photo reload -file [file join ../data/buttons/reload.gif]
image create photo restart -file [file join ../data/buttons/restart.gif]
image create photo start -file [file join ../data/buttons/start.gif]
image create photo pause -file [file join ../data/buttons/pause.gif]
image create photo step -file [file join ../data/buttons/step.gif]
image create photo record -file [file join ../data/buttons/rec.gif]
image create photo recordWorld -file [file join ../data/buttons/recW.gif]
button .simulstart -image start -anchor center -width 23 -height 23 -command {anim start}
button .simulpause -image pause -anchor center -width 23 -height 23 -command {anim pause}
button .simulstep -image step -anchor center -width 23 -height 23 -command {anim step}
button .simulrestart -image restart -anchor center -width 23 -height 23 -command {anim restart}
button .simulreload -image reload -anchor center -width 23 -height 23 -command reloadAnimation
checkbutton .record -image record -anchor center -width 21 -height 21 -selectcolor SeaGreen1 -indicatoron 0 -variable toggleScreenshots
checkbutton .recordWorld -image recordWorld -anchor center -width 21 -height 21 -selectcolor SeaGreen1 -indicatoron 0 -variable toggleWorldshots

scale .currPos -label "Target pose phase" -orient h -digit 4 -from 0.0 -to 1.0 -variable targetPosePhase -resolution 0.001 -length 6.5cm -sliderlength 0.2cm -command tryToUpdatePose

pack .labelsimulator -side top -fill x -in .simulatorlabel -anchor w
pack .simulrestart .simulstart .simulpause .simulstep .simulreload .record .recordWorld -side left -in .simulatorframe -anchor w



#### character options
frame .characterframe
label .labelcharacter -text "Extras:" -relief raised
checkbutton .push -text "Push Interface" -variable showPushInterface -anchor w
checkbutton .curve -text "CurveEditor" -variable showCurveEditor -anchor w
checkbutton .controlHeading -text "Heading Control" -variable forceHeadingControl -anchor w
.controlHeading select

pack .labelcharacter -side top -fill x -in .characterframe -anchor w
pack .push .curve .controlHeading -side top -fill y -in .characterframe -anchor w

#### other commands
frame .buttonframe -pady 5
label .otherlabel -text "" -relief raised
button .exit -text Quit -anchor center -width 15 -command {quit}
pack .otherlabel -side top -fill x -in .buttonframe -anchor w
pack .exit -in .buttonframe -pady 5 -side top


wm resizable . 0 0

pack .simsettings -in .mainframe -side top -fill x
pack .viewoptions -in .mainframe -side top -fill x
pack .camerasettings .followchar -in .mainframe -side top -fill x
pack .simulatorlabel .simulatorframe .currPos -in .mainframe -side top -fill x
pack .characterframe -in .mainframe -side top -fill x
pack .buttonframe -in .mainframe -side top -fill x
