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

/*
 * tcltk.h: Defines the functions related to TCL/TK .
 */
#ifndef TCLTK_H
#define TCLTK_H

/**
 * This data structure holds information that is needed
 * for a tcl console. 
*/
typedef struct tmpConsoleInfo {
    Tcl_Interp *consoleInterp;	// Interpreter for the console. 
    Tcl_Interp *interp;		    // Interpreter to send console commands. 
} ConsoleInfo;

// Performs initialization of Tcl and Tk
int Tcl_AppInit(Tcl_Interp *interp);


// Clear console
int cls (ClientData clientData, Tcl_Interp *interp, int argc, CONST84 char **argv);

// Used to source input from files
int pipeIn _ANSI_ARGS_((ClientData clientData, Tcl_Interp *interp, int argc, CONST84 char **argv));


#endif

