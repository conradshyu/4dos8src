/*
 *************************************************************************
 ** DEFAULT.C
 *************************************************************************
 ** Window default functions
 **
 ** Copyright (c) 1996  JP Software, All rights reserved
 *************************************************************************
 */

#include "winlib.h"
#include "general.h"
#include "scrdef.h"


/*
 *************************************************************************
 ** SetScreenDefaults
 *************************************************************************
 ** Set the window color and screen defaults
 *************************************************************************
 ** Requires: nPalette = number of the color palette to use
 ** Returns : none
 *************************************************************************
 */
void SetScreenDefaults(int nPalette)
{
	// Disable blinking
   d_setblink(DISABLE);

	// Don't save cursor position when a field is exited
   d_change(FSAVECURSOR, DISABLE);

	// Don't exit field when filled (when disabled, was incorrectly wrapping
	// back to first char in field)
   d_change(FRETURN, ENABLE);

	// Set so that bottom line help will be faster 
	d_change(SYSFAST, ENABLE);

	// Don't save initial screen contents
	d_change(SYSENV, DISABLE);

	// Clear screen before displaying a window
	d_change(SYSBGCOLOR, SC_WIN);
	d_change(SYSCLEAR, ENABLE);

	// Draw with non-intuitive borders (was messing up w/dropdown menus)
	d_change(SYSBORDER, DISABLE);

	//
	// Colors
	//

	// Make window border same color as border message
	d_change(SYSBORDCOLOR, ENABLE);

	// General
	d_change(GRAYCOLOR, SC_GRAY);

	// Window
	d_change(WSHADCOLOR, SC_SHADOW);
   d_change(WCOLOR, SC_WIN);
   d_change(WBRDCOLOR, SC_BORDER);
   d_change(WUMCOLOR, SC_BORDER);

	// Controls
   d_change(CDATACOLOR, SC_DATA);
	// Greyed to look like combo boxes
   //d_change(CDATAFOCUS, SC_DATA_FOCUS);
   //d_change(CDATASELECT, SC_DATA_FOCUS);
   d_change(CDATAFOCUS, SC_DATA);
   d_change(CDATASELECT, SC_DATA);

   d_change(CPROMPTCOLOR, SC_NO_FOCUS);
   d_change(CPROMPTFOCUS, SC_FOCUS);
   d_change(CPROMPTSELECT, SC_FOCUS);

	// Buttons
   d_change(BUTTONCOLOR, SC_NO_FOCUS_BUTTON);
   d_change(BFOCUSCOLOR, SC_FOCUS_BUTTON);
   d_change(BHIGHLTCOLOR, SC_HILITE_BUTTON);
   d_change(BSHADCOLOR, SC_SHADOW_BUTTON);

	// Checkboxes and radio buttons
   d_change(BXMARKFCOLOR, SC_NO_FOCUS);
   d_change(BXMARKCOLOR, SC_NO_FOCUS);
   d_change(BXFOCUSCOLOR, SC_FOCUS);
   d_change(BXHIGHLTCOLOR, SC_HILITE);
   d_change(CHECKBOXCOLOR, SC_NO_FOCUS);

	// Combo boxes
   d_change(CBDROPCOLOR, SC_NO_FOCUS);

	// Menus
   d_change(MITEMCOLOR, SC_MENU);
   d_change(MCOLOR, SC_MENU);
   d_change(MBRDCOLOR, SC_MENU);
   d_change(MBARCOLOR, SC_MENU_SELECT);
   d_change(MKEYCOLOR, SC_MENU_HILITE);
   d_change(MHELPCOLOR, SC_WIN_LINE);
   d_change(MSHADCOLOR, SC_SHADOW);

} // End SetScreenDefaults
