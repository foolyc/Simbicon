/*
	Simbicon 1.5 Controller Editor Framework, 
	Copyright 2009 Stelian Coros, Philippe Beaudoin and Michiel van de Panne.
	All rights reserved. Web: www.cs.ubc.ca/~van/simbicon_cef

	This file is part of the Simbicon 1.5 Controller Editor Framework.

	Simbicon 1.5 Controller Editor Framework is free software: you can 
	redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Simbicon 1.5 Controller Editor Framework is distributed in the hope 
	that it will be useful, but WITHOUT ANY WARRANTY; without even the 
	implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Simbicon 1.5 Controller Editor Framework. 
	If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * tcltk.cpp: Sets up tcl/tk
 */
#include "UI.h"
#include "tcltk.h"
#include <stdio.h>
#include <string.h>
#include <Core/SimGlobals.h>

/**
 * The console command implements a Tcl interface to the 
 * various console options.
 */
int ConsoleCmd (ClientData clientData, Tcl_Interp *interp, int argc, CONST84 char **argv){
    ConsoleInfo *info = (ConsoleInfo *) clientData;
    Tcl_Interp *consoleInterp = info->consoleInterp;
    int length;
    char c;

    // Check for validity
    if (argc < 2) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0], " option ?arg arg ...?\"", (char *) NULL);
	    return TCL_ERROR;
    }
    
    c = argv[1][0];
    length = (int) strlen(argv[1]);
    
    // Evaluate the command
    Tcl_Preserve((ClientData) consoleInterp);
    if ((c == 't') && (strncmp(argv[1], "title", length)) == 0) {
        Tcl_DString dString;
        Tcl_DStringInit(&dString);
        Tcl_DStringAppend(&dString, "wm title . ", -1);
        if (argc == 3) {
            Tcl_DStringAppendElement(&dString, argv[2]);
        }
        Tcl_Eval(consoleInterp, Tcl_DStringValue(&dString));
        Tcl_DStringFree(&dString);
    } else if ((c == 'h') && (strncmp(argv[1], "hide", length)) == 0) {
        Tcl_Eval(info->consoleInterp, "wm withdraw .");
    } else if ((c == 's') && (strncmp(argv[1], "show", length)) == 0) {
        Tcl_Eval(info->consoleInterp, "wm deiconify .");
    } else if ((c == 'e') && (strncmp(argv[1], "eval", length)) == 0) {
        if (argc == 3) {
            Tcl_Eval(info->consoleInterp, argv[2]);
        } else {
            Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
                             " eval command\"", (char *) NULL);
            return TCL_ERROR;
        }
    } else {
        Tcl_AppendResult(interp, "bad option \"", argv[1],
                         "\": should be hide, show, or title",
                         (char *) NULL);
		Tcl_Release((ClientData) consoleInterp);
        return TCL_ERROR;
    }
    
    Tcl_Release((ClientData) consoleInterp);
    return TCL_OK;
}


/**
 * This command allows the console interp to communicate 
 * with the main interpreter.
 */
static int InterpreterCmd (ClientData clientData, Tcl_Interp *interp, int argc, CONST84 char **argv){
    ConsoleInfo *info = (ConsoleInfo *) clientData;
    Tcl_Interp *otherInterp = info->interp;
    int result = TCL_OK;
    int length;
    char c;

    // Check for validity
    if (argc < 2) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0], " option ?arg arg ...?\"", (char *) NULL);
	    return TCL_ERROR;
    }
    
    c = argv[1][0];
    length = (int) strlen(argv[1]);

    // Evaluate the command
    Tcl_Preserve((ClientData) otherInterp);
    if ((c == 'e') && (strncmp(argv[1], "eval", length)) == 0) {
        result = Tcl_GlobalEval(otherInterp, argv[2]);
        Tcl_AppendResult(interp, otherInterp->result, (char *) NULL);
    } else if ((c == 'r') && (strncmp(argv[1], "record", length)) == 0) {
        Tcl_RecordAndEval(otherInterp, argv[2], TCL_EVAL_GLOBAL);
        Tcl_AppendResult(interp, otherInterp->result, (char *) NULL);
    } else {
        Tcl_AppendResult(interp, "bad option \"", argv[1],
                         "\": should be eval or record",
                         (char *) NULL);
        result = TCL_ERROR;
    }

    Tcl_Release((ClientData) otherInterp);
    return result;
}

/**
 * If the console command is deleted we destroy the console 
 * window and all associated data structures.
 */
void ConsoleDeleteProc(ClientData clientData) {
    ConsoleInfo *info = (ConsoleInfo *) clientData;
    Tcl_DeleteInterp(info->consoleInterp);
    info->consoleInterp = NULL;
}

/**
 * This event procedure is registered on the main window of 
 * the slave interpreter.  If the user or a running script 
 * causes the main window to be destroyed, then we need to 
 * inform the console interpreter by invoking 
 * "tkConsoleExit".
*/
static void ConsoleEventProc(ClientData clientData, XEvent *eventPtr){
    ConsoleInfo *info = (ConsoleInfo *) clientData;
    Tcl_Interp *consoleInterp = info->consoleInterp;
    
    if (eventPtr->type == DestroyNotify) {
        // It is possible that the console interpreter itself has
        // already been deleted. In that case the consoleInterp
        // field will be set to NULL. If the interpreter is already
        // gone, we do not have to do any work here.
        
        if (consoleInterp != NULL) {
            Tcl_Preserve((ClientData) consoleInterp);
            Tcl_Eval(consoleInterp, "tkConsoleExit");
            Tcl_Release((ClientData) consoleInterp);
        }
    }
}

/**
 * Initialize the console.  This code actually creates a 
 * new application and associated interpreter.  This 
 * effectivly hides the implementation from the main 
 * application.
 */
int TkConsoleInit(Tcl_Interp *interp) {
    Tcl_Interp *consoleInterp;
    ConsoleInfo *info;
    Tk_Window mainWindow = Tk_MainWindow(interp);

    static char initCmd[] = "source $tk_library/console.tcl";
    
    // Create interp
    consoleInterp = Tcl_CreateInterp();
    if (consoleInterp == NULL) {
	    return TCL_ERROR;
    }
	Globals::tclInterpreter = consoleInterp;
    
    // Initialize Tcl and Tk.
    if (Tcl_Init(consoleInterp) != TCL_OK
            || Tk_Init(consoleInterp) != TCL_OK) {
        Tcl_Release((ClientData) consoleInterp);
	    return TCL_ERROR;
    }
    
	// Add console commands to the interp 
    info = (ConsoleInfo *) ckalloc(sizeof(ConsoleInfo));
    info->interp = interp;
    info->consoleInterp = consoleInterp;
    Tcl_CreateCommand(interp, "console", ConsoleCmd, (ClientData) info,
	                  (Tcl_CmdDeleteProc *) ConsoleDeleteProc);
    Tcl_CreateCommand(consoleInterp, "consoleinterp", InterpreterCmd,
	                  (ClientData) info, (Tcl_CmdDeleteProc *) NULL);
    Tk_CreateEventHandler(mainWindow, StructureNotifyMask, 
                          ConsoleEventProc, (ClientData) info);
    
    // Create the console window
    Tcl_Preserve((ClientData) consoleInterp);
    if (Tcl_Eval(consoleInterp, initCmd) == TCL_ERROR) {
	    printf("Eval error: %s", consoleInterp->result);
    }
    Tcl_Release((ClientData) consoleInterp);

    return TCL_OK;
}

/**
 * Processes TCL/TK events. 
 */
void CheckTCLTKEvents(){
	// Process TCL/TK events 
	while(Tk_DoOneEvent(TK_DONT_WAIT) != 0);
}

/**
 * Performs initialization of Tcl and Tk
 * and registers new commands with the Tcl interpreter
 */
int Tcl_AppInit(Tcl_Interp *interp){
    // Initialize Tcl and Tk
    if (Tcl_Init(interp) == TCL_ERROR || Tk_Init(interp) == TCL_ERROR)
        return TCL_ERROR;

	//keep a reference to the interpreter so that we can use it later
    Globals::tclInterpreter = interp;
	Globals::changeCurrControlShotStr(-1);

    Tcl_StaticPackage(interp, "Tk", Tk_Init, Tk_SafeInit);

	//get the console window
	if( Globals::useConsole ) {
		int tkwin = Tk_CreateConsoleWindow(interp);
		if (tkwin == TCL_ERROR)
			return TCL_ERROR;
	}

	//get a list of callback functions
    Tcl_CreateCommand(interp, "cls",			cls,			(ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp, "<",				pipeIn,			(ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "quit",			quit,			(ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(interp, "launch",			launch,			(ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp, "anim",			animation,		(ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp, "camera",			camera,			(ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
	Tcl_CreateCommand(interp, "instantChar",	instantChar,	(ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);


	//and link some variables as well
	Tcl_LinkVar(interp, "slowdown",					(char *) &Globals::animationTimeToRealTimeRatio, TCL_LINK_DOUBLE);
	Tcl_LinkVar(interp, "drawFPS",					(char *) &Globals::drawFPS, TCL_LINK_INT);
	Tcl_LinkVar(interp, "drawCubeMap",				(char *) &Globals::drawCubeMap, TCL_LINK_INT);
	Tcl_LinkVar(interp, "drawGlobalAxes",			(char *) &Globals::drawGlobalAxes, TCL_LINK_INT);
	Tcl_LinkVar(interp, "drawShadows",				(char *) &Globals::drawShadows, TCL_LINK_INT);
	Tcl_LinkVar(interp, "drawCollisionPrimitives",	(char *) &Globals::drawCollisionPrimitives, TCL_LINK_INT);
	Tcl_LinkVar(interp, "followCharacter",			(char *) &Globals::followCharacter, TCL_LINK_INT);
	Tcl_LinkVar(interp, "drawCDPs",					(char *) &Globals::drawCollisionPrimitives, TCL_LINK_INT);
	Tcl_LinkVar(interp, "drawJoints",				(char *) &Globals::drawJoints, TCL_LINK_INT);
	Tcl_LinkVar(interp, "drawContactForces",		(char *) &Globals::drawContactForces, TCL_LINK_INT);
	Tcl_LinkVar(interp, "drawDesiredPose",			(char *) &Globals::drawDesiredPose, TCL_LINK_INT);
	Tcl_LinkVar(interp, "showPushInterface",		(char *) &Globals::drawPushInterface, TCL_LINK_INT);
	Tcl_LinkVar(interp, "showCurveEditor",			(char *) &Globals::drawCurveEditor, TCL_LINK_INT);
	Tcl_LinkVar(interp, "showCanvas",				(char *) &Globals::drawCanvas, TCL_LINK_INT);
	Tcl_LinkVar(interp, "toggleScreenshots",		(char *) &Globals::drawScreenShots, TCL_LINK_INT);
	Tcl_LinkVar(interp, "toggleControlshots",		(char *) &Globals::drawControlShots, TCL_LINK_INT);
	Tcl_LinkVar(interp, "toggleUpdateDVTraj",		(char *) &Globals::updateDVTraj, TCL_LINK_INT);
	Tcl_LinkVar(interp, "toggleWorldshots",			(char *) &Globals::drawWorldShots, TCL_LINK_INT);
	Tcl_LinkVar(interp, "forceHeadingControl",		(char *) &SimGlobals::forceHeadingControl, TCL_LINK_INT);
	Tcl_LinkVar(interp, "drawGroundPlane",			(char *) &Globals::drawGroundPlane, TCL_LINK_INT);
	Tcl_LinkVar(interp, "conInterpValue",			(char *) &SimGlobals::conInterpolationValue, TCL_LINK_DOUBLE);
	Tcl_LinkVar(interp, "currControlShot",          (char *) &Globals::currControlShotStr, TCL_LINK_STRING);
	Tcl_LinkVar(interp, "targetPosePhase",			(char *) &Globals::targetPosePhase, TCL_LINK_DOUBLE);
//	Tcl_LinkVar(interp, "conInterpValue",			(char *) &SimGlobals::conInterpolationValue, TCL_LINK_DOUBLE);
	Tcl_LinkVar(interp, "bipVel",					(char *) &SimGlobals::bipDesiredVelocity, TCL_LINK_DOUBLE);
	
//	Tcl_LinkVar(interp, "part1dPos",			(char *) &DynamicWorld1D::pos, TCL_LINK_DOUBLE);
//	Tcl_LinkVar(interp, "part1dVel",			(char *) &DynamicWorld1D::vel, TCL_LINK_DOUBLE);
	Tcl_LinkVar(interp, "part1dTarget",			(char *) &SimGlobals::targetPos, TCL_LINK_DOUBLE);
	Tcl_LinkVar(interp, "softness",			(char *) &SimGlobals::constraintSoftness, TCL_LINK_INT);
	Tcl_LinkVar(interp, "cgitn",			(char *) &SimGlobals::CGIterCount, TCL_LINK_INT);
	Tcl_LinkVar(interp, "lincnt",			(char *) &SimGlobals::linearizationCount, TCL_LINK_INT);


	Tcl_LinkVar(interp, "rootSagittal",			(char *) &SimGlobals::rootSagittal, TCL_LINK_DOUBLE);
	Tcl_LinkVar(interp, "rootLateral",			(char *) &SimGlobals::rootLateral, TCL_LINK_DOUBLE);
	Tcl_LinkVar(interp, "swingHipSagittal",			(char *) &SimGlobals::swingHipSagittal, TCL_LINK_DOUBLE);
	Tcl_LinkVar(interp, "swingHipLateral",			(char *) &SimGlobals::swingHipLateral, TCL_LINK_DOUBLE);
	Tcl_LinkVar(interp, "stanceAngleSagittal",			(char *) &SimGlobals::stanceAngleSagittal, TCL_LINK_DOUBLE);
	Tcl_LinkVar(interp, "stanceAngleLateral",			(char *) &SimGlobals::stanceAngleLateral, TCL_LINK_DOUBLE);
	Tcl_LinkVar(interp, "stanceKnee",			(char *) &SimGlobals::stanceKnee, TCL_LINK_DOUBLE);


	Tcl_LinkVar(interp, "comX",			(char *) &SimGlobals::COMOffsetX, TCL_LINK_DOUBLE);
	Tcl_LinkVar(interp, "comZ",			(char *) &SimGlobals::COMOffsetZ, TCL_LINK_DOUBLE);



	Tcl_EvalFile(interp, "init/setup.tcl");


	//and start the main loop...
	glutMainLoop();

	return TCL_OK;
}

/**
 *	PROC:	tprintf()
 *	DOES:	substitute for printf
 *          In the windows version of Tcl, stdout doesn't get bound
 *          to the TclConsole. tprintf() is a work-around which prints
 *          things using the Tcl 'puts' command
 */
int tprintf(const char *format, ...) {
	int n;
	va_list ap;
	STR str,argstr,newstuff;
	static STR line;
	static int lineptr = 0;
	newstuff[0] = 0;

	// print into 'newstuff'
	va_start(ap,format);
	n = vsprintf(argstr,format,ap);
	strcat(newstuff,argstr);
	va_end(ap);

	// look for line-feeds, print out using Tcl 'puts' as necessary
	int len = strlen(newstuff);
	for (int c=0; c<len; c++) {
		if (newstuff[c]=='\n') {
				line[lineptr++] = 0;
				sprintf(str,"puts \"%s\"",line);
				//Tcl_Eval(Interp,str);
				Tcl_Eval(Globals::tclInterpreter,str);
				lineptr = 0;
		} else {
				line[lineptr++] = newstuff[c];
		}
	}
	return n;
}

/**
 * Clear the console
 */
int cls (ClientData clientData, Tcl_Interp *interp, int argc, CONST84 char **argv){
    Tcl_Eval(interp,"console eval {.console delete 1.0 end}");
	return TCL_OK;
}


/**
 * This method is used to quickly load other files in a pipe in sort of way
 */
int pipeIn (ClientData clientData, Tcl_Interp *interp, int argc, CONST84 char **argv){
	if (argc<2) 
	{
	   Tcl_AppendResult(interp, "Usage: %s <file>");
	   return TCL_OK;
	 }
	Tcl_EvalFile(interp,argv[1]);	/* read in file & interpret */
	return TCL_OK;
}

