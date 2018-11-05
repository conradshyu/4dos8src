/*
 *************************************************************************
 ** DIALOG.C
 *************************************************************************
 ** Text-based dialog routines using the MIX C/Windows Toolchest
 **
 ** Copyright 1997 JP Software Inc., All rights reserved
 *************************************************************************
 ** Compiler defines:
 **   DEBUG       Adds debugging printf's and control test routine
 **                DlgIndexTest()
 *************************************************************************
 */

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <i86.h>

#ifdef DEBUG
#include <errno.h>
#endif

// 4DOS
#include "build.h"
#include "general.h"
#include "typedefs.h"
#include "product.h" 
#include "inifile.h"
#include "resource.h"  // IDI_ values

// MIX TUI libraries
#include "defaults.h"
#include "field.h"
#include "keyboard.h"
#include "menu.h"
#include "mouse.h"
#include "controls.h"
#include "window.h"
#include "winvars.h"
#ifdef DEBUG
#include "help.h"
#endif  // DEBUG

// DIALOG
#include "dialog.h"
#include "scrdef.h"
#include "iniutil.h"

// Field sizes
#define MAX_PATH_LEN  260  // Path field width
#define MAX_COMBO_LEN  15  // Combo field width
#define MAX_CAT_LEN    14  // Category label length

// Main window location
#define WIN_COL       1
#define WIN_ROW       2
// Main window dimensions
#define WIN_HEIGHT    21
#define WIN_WIDTH     78
// Dialog window location (relative to Main window)
#define DLG_COL       0
#define DLG_ROW       0
// Single line help window location and size
#define HELP_COL      0
#define HELP_ROW      24
#define HELP_WIDTH    80
// Top line menu location and size
#define MENU_COL      0
#define MENU_ROW      0
#define MENU_WIDTH    80


/*
 *************************************************************************
 ** Structures
 *************************************************************************
 */
// Dialog control information 
typedef struct {
	// Control info 
	unsigned int uType;         // Control type
	unsigned int uID;           // IDI_ value
	Control mCtrl;              // Filled when control is created

	// Common to all controls
	unsigned int uCol;          // Location column
	unsigned int uRow;          // Location row
	char *pszLabel;             // Label
	char *pszHelp;              // Single line help
	char *pszDirective;         // Directive name
	char *pszHelpDirective;     // Optional name to use when calling help

	union {
		unsigned int uWidth;     // Integer, text, combo box, and box
		unsigned int uGroup;     // Radio
		unsigned int uButtonID;  // Button and tab
	} Var1;

	union {
		unsigned int fSpin;       // Integer
		unsigned int uBufferLen;  // Text
		unsigned int uHeight;     // Combo box and box
	} Var2;
} INI_CONTROL;


// Map element
typedef struct {
	INI_CONTROL *pElement;
} DLG_INDEX;


// Category element
typedef struct {
	Window mDlg;                 // Dialog window
	Control mFocusCtrl;          // Current control (has focus)
	unsigned int uTabIndex;      // Index of TAB control
	unsigned int uFirstIndex;    // Index of first control
	int fBuilt;                  // Window has been created?
} CAT_INDEX;


/*
 *************************************************************************
 ** Internal function prototypes
 *************************************************************************
 */
static void    _F1Help(void);
static void    _Call4Help(char *pszHelpID);
static void    _LineHelp(char *pszHelp, char *pszDirective);
static int     _FocusFilter(Window mWin, Control mCtrl, int nEvent);
static int     _FixCtrlHilight(Control mCtrl, unsigned int uType, unsigned int nEvent);
static int     _KeyFilter(int nKey);
// static int     _MouseFilter(Mouse_State *pMouseStatus);
// static void    _InitControlElement(INI_CONTROL *pCtrlData);
static void    _InitCategoryElement(CAT_INDEX *pCatData);
static int     _CreateControl(Window mWin, INI_CONTROL *pCtrlData);
static int     _AddMenu(void);
static void    _MenuItem(void);
static unsigned int _ProcessMainMenu(unsigned int uCurrentCategory);

static int     _INITab(Window mWin, INI_CONTROL *pCtrlData);
static int     _INIBox(Window mWin, INI_CONTROL *pCtrlData);
static int     _INIStatic(Window mWin, INI_CONTROL *pCtrlData);
static Control _INIField(Window mWin, INI_CONTROL *pCtrlData);
static Control _INICheck(Window mWin, INI_CONTROL *pCtrlData);
static Control _INIInteger(Window mWin, INI_CONTROL *pCtrlData, int fSigned);
static Control _INICombo(Window mWin, INI_CONTROL *pCtrlData);
static Control _INIRadio(Window mWin, INI_CONTROL *pCtrlData);

static int     _PopUpExitBox(void);
static int     _PopUpKeyFilter(int nKey);
static int     _PopUpFocusFilter(Window mWin, Control mCtrl, int nEvent);

#ifdef DEBUG
static void    _PopUpBox(char *pszString1, char *pszString2);
#endif  // DEBUG

// static void    _SetCursorSize(int nStart, int nEnd, int fShow);
static void    _SetCursorStatus(int nStat);
static void    _MemoryError(char *pszMessage);


/*
 *************************************************************************
 ** Globals
 *************************************************************************
 */
Window gmHelpWin;                      // Single-line help window
Control gmCurrentCtrl;                 // Control that has the focus
DLG_INDEX *gpINIIndex;                 // INI control index
CAT_INDEX gaCategoryList[CATEGORY_MAX];// Individual category information
unsigned int guCategoryCnt = 0;        // Number of categories
unsigned int guCurrentCat = 0;         // Current category
unsigned int guLastKey;                // Last key pressed on menu
unsigned int guMenuWidth;              // Width of top-line menu
unsigned int guHelpWidth;              // Width of bottom-line help
unsigned int guHelpRow;                // Row of bottom-line help
int gfDlgOpen = TRUE;                  // Has DlgOpen run successfully?
int gfUseMouse = TRUE;                 // Enable mouse?
int gfInDialogWin = FALSE;             // Currently in dialog window?
int gfHelpAvail = FALSE;               // Help program available?
char gszHelpExe[MAX_PATH_LEN + 1] = "/0";  // Path and name of help program


// Menus
Window gmMenuWin;                      // Window for main menu
Menu gmMain;
Menu gmExit;
Menu gmConfigure;
Menu gmHelp;

// The menu select and exit keys must be given permanent storage
unsigned int guHorizMenuSelectKeys[] =
	{ _ENTER, _DNARROW, MOUSE_LEFTCLK, MOUSE_LEFTREL, 0 };
unsigned int guVertMenuSelectKeys[] =
	{ 32, _ENTER, MOUSE_LEFTCLK, MOUSE_LEFTREL, 0 };
unsigned int guVertMenuTermKeys[] = { _ESC, _F10, _LARROW, _RARROW, 0 };

// Shared help text for exit menu and exit dialog
char *gpszSaveLineHelp = "Change settings in current session, save to .INI file, and exit";
char *gpszUseLineHelp = "Change settings in current session and exit";
char *gpszCancelLineHelp = "Exit without changing settings";

// INI_CONTROL data
#include "inirc.h"


/*
 *************************************************************************
 ** DlgOpen
 *************************************************************************
 ** Initialize TUI functions
 ** Build index to control data
 *************************************************************************
 ** Requires: uINIControlCount = size of index needed (max # of IDI_ vals)
 ** Returns : 0  success
 *************************************************************************
 */
int DlgOpen(unsigned int uINIControlCount) {
	Window mTempWin;
	int i = 0;
	int j = 0;
	int nLastGroup = -1;
	int nLastType = INI_CTL_NULL;
	int nOffsetValue = 0;
	unsigned int uIndexSize;
	unsigned int uControlListLen;
	char szDialogTitle[25];


	// Build index that corresponds to the map array INIControlData
	//  and initialize
	uIndexSize = uINIControlCount * sizeof(DLG_INDEX);
	gpINIIndex = (DLG_INDEX *)malloc(uIndexSize);
	if (gpINIIndex == NULL)
		_MemoryError("INIIndex");
	memset(gpINIIndex, (char)0xFF, uIndexSize);

	uControlListLen = sizeof(gaControlList) / sizeof(INI_CONTROL);

	// Loop once for each category
	while ((j < uControlListLen) && (i < CATEGORY_MAX)) {

		_InitCategoryElement(&gaCategoryList[i]);
		gaCategoryList[i].uFirstIndex = j;
		guCategoryCnt++;

		// NULL entry ends category
		while ((gaControlList[j].uType != INI_CTL_NULL) && (j < uControlListLen)) {

#ifdef DEBUG
			printf("Initializing control %d (%s), category %d, ID %d, type %d\n", j, gaControlList[j].pszLabel, i, gaControlList[j].uID, gaControlList[j].uType);
#endif  // DEBUG

	   	if ((nOffsetValue = (gaControlList[j].uID - IDI_BASE)) > 0) {

				if (gaControlList[j].uType == INI_CTL_RADIO) {
					// Only want to assign first radio button
					if (gaControlList[j].Var1.uGroup != nLastGroup) {
	   				gpINIIndex[nOffsetValue].pElement = &gaControlList[j];
						nLastGroup = gaControlList[j].Var1.uGroup;
					}
					nLastType = INI_CTL_RADIO;
				}
				else if (nLastType == INI_CTL_COLOR) {
					// Only want to assign first control of color pair
					nLastType = INI_CTL_NULL;
				}
				else {
					// Assign control to index
	   			gpINIIndex[nOffsetValue].pElement = &gaControlList[j];
					nLastType = gaControlList[j].uType;
				}
#ifdef DEBUG
				printf("Pointer at %d is %p\n", nOffsetValue, gpINIIndex[nOffsetValue].pElement);
#endif  // DEBUG
			}
			else {
				// If control is a TAB, need to store the category
				if (gaControlList[j].uType == INI_CTL_TAB) {
					gaControlList[j].Var1.uButtonID = guCategoryCnt - 1;
					gaCategoryList[guCategoryCnt - 1].uTabIndex = j;
				}
			}

			j++;
		} // End while control type not NULL

		j++;
		i++;
	}  // End while j, i

	// Get defaults
	SetScreenDefaults(0);

	// Do not display windows while they are being created
  	d_change(WDISPLAY, DISABLE);

	// Create temp window to get screen sizes
	mTempWin = w_open(1, 1, 1, 1);
	w_close(mTempWin);

#ifdef DEBUG
	printf("Screen cols=%d, rows=%d\n", ex_crt_columns, ex_crt_rows);
	getchar();
#endif  // DEBUG
 
	// Do not enable mouse if more than 80 columns wide because it behaves
	//  irratically
	if (ex_crt_columns > 80)
		gfUseMouse = FALSE;

	// Enable mouse and keep from moving to bottom row where help text is
	if (gfUseMouse) {
		s_open();


/* EWL Took this out because it was not working with windowed sessions

		s_limit(0, (ex_crt_columns - 1) * ex_mouse.percol,
		        0, 23 * ex_mouse.perrow);
  		// Set function that will filter all mouse events
		s_filter(_MouseFilter);
*/
	}

  	// Set function that will filter all keystrokes
	k_filter(_KeyFilter);

	// Create menus
	if (_AddMenu() != 0)
		return(-1);
	gmMenuWin = m_display(gmMain);
	w_pencolor(gmMenuWin, SC_MENU_TITLE);

#ifdef DEBUG
	sprintf(szDialogTitle, " OPTION W/DEBUGGING (Build #%d) ", VER_BUILD);
#else
	sprintf(szDialogTitle, " OPTION ");
#endif  // DEBUG

	w_putsat(gmMenuWin, ex_crt_columns - strlen(szDialogTitle), 0,
	         szDialogTitle);
	w_on(gmMenuWin);

	// If we got this far, we were successful
	gfDlgOpen = TRUE;

	return(0);
}  // End DlgOpen


/*
 *************************************************************************
 ** DlgBuildWindow
 *************************************************************************
 ** Build one dialog using TUI library calls
 *************************************************************************
 ** Requires: uCategory = category of window to build
 ** Returns :  0 success or this category was already built
 **           <0 error (see code for individual reasons)
 *************************************************************************
 */
int DlgBuildWindow(unsigned int uCategory) {
	INI_CONTROL *pControl;
	CAT_INDEX *pHoldCat;
	char szBuffer[MAX_CAT_LEN + 3];
	int nLabelLen = 0;
	int i, j;

	// Check range of category and that the dialog is open
	if ((uCategory >= guCategoryCnt) || !gfDlgOpen)
		return(-1);

	// If the screen for this category already exists, don't build it again
	if ((gaCategoryList[uCategory].fBuilt == TRUE) &&
		 (gaCategoryList[uCategory].mDlg != NULL))
		return(0);

	pHoldCat = &gaCategoryList[uCategory];

	// Open window for this category
	pHoldCat->mDlg = w_open(WIN_COL, WIN_ROW + DLG_ROW, ex_crt_columns - 2,
	                        ex_crt_rows - 4);
	if (pHoldCat->mDlg == NULL)
		return(-2);

	// Use category label as window title
	pControl = &gaControlList[pHoldCat->uTabIndex];
	if (pControl != NULL) {
		nLabelLen = MIN(strlen(pControl->pszLabel), MAX_CAT_LEN);

		szBuffer[0] = ' ';
		// Copy string by character, excluding '~' and '\0'
		for (i = 0, j = 1; i <= nLabelLen; i++) {
			if ((pControl->pszLabel[i] != '\0') &&
			    (pControl->pszLabel[i] != '~')) {
				szBuffer[j] = toupper(pControl->pszLabel[i]);
				j++;
			}
		}  // End for i
		szBuffer[j] = ' ';
		szBuffer[j + 1] = '\0';

		w_umessage(pHoldCat->mDlg, szBuffer);
	}

	pControl = &gaControlList[pHoldCat->uFirstIndex];

	// NULL entry ends category
	while (pControl->uType != INI_CTL_NULL) {

		// Create dialog controls
		if (_CreateControl(pHoldCat->mDlg, pControl) != 0) {
#ifdef DEBUG
			printf("DEBUG: Error #%d creating control IDI-%d\n", ex_werrno,
			       pControl->uID);
			getchar();
#endif
			return(-5);
		}

		// Set the initial focus on the first control 
		if ((pHoldCat->mFocusCtrl == NULL) &&
		    (pControl->uType != INI_CTL_BOX) &&
		    (pControl->uType != INI_CTL_TAB) &&
		    (pControl->uType != INI_CTL_STATIC))
			pHoldCat->mFocusCtrl = pControl->mCtrl;

		pControl++;
	}  // End while

	if (pHoldCat->mFocusCtrl == NULL)
		return(-6);
	else {
		pHoldCat->fBuilt = TRUE;
		return(0);
	}
}  // End DlgBuildWindow


/*
 *************************************************************************
 ** DlgShowWindow
 *************************************************************************
 ** Display new category dialog
 *************************************************************************
 ** Requires: uCategory = category of window to show
 ** Returns :  0 success
 **           <0 error (see code for individual reasons)
 *************************************************************************
 */
int DlgShowWindow(unsigned int uCategory) {
	CAT_INDEX *pHoldCat;
	INI_CONTROL *pHoldCtrl;
	unsigned int uDlgRC = 0;
	unsigned int uReturnCat = 0;

	// Keys that will terminate the dialog
	// See _KeyFilter() for the keys that are translated into ALT_8 and ALT_9
	unsigned int aTermKeys[] = { ALT_8, ALT_9, ALT_X, ALT_C, ALT_H, 0 };

	static int fFirst = TRUE;


	// Check range of category and that the dialog is open
	if ((uCategory >= guCategoryCnt) || !gfDlgOpen)
		return(-1);

	// Make sure the screen has been built
	if (gaCategoryList[uCategory].fBuilt == FALSE)
		return(-2);

	pHoldCat = &gaCategoryList[uCategory];
	guCurrentCat = uCategory;

	// Allow windows to be displayed as soon as they are created
	// This is for the help line and message windows
  	d_change(WDISPLAY, ENABLE);

	// Show the new window
	w_on(pHoldCat->mDlg);

	uReturnCat = uCategory;
	gmCurrentCtrl = pHoldCat->mFocusCtrl;

	m_start(gmConfigure, m_getitem(gmConfigure, uCategory));

	// Do only on first call
	if (fFirst) {
		fFirst = FALSE;

		// Clear help line
		_LineHelp(" ", NULL);

		// Pull down category menu
		m_start(gmMain, m_getitem(gmMain, 2));
		k_addkey(_ENTER);

		uReturnCat = _ProcessMainMenu(uCategory);
	}

	// Repeat until a new category or exit is selected
   while (uReturnCat == uCategory) {
		// Open data entry window and process
		gfInDialogWin = TRUE;

		// This loop is a kludge to fix the infinite looping which occurred
		//  when the mouse was clicked on the single-line help window
		while (gfInDialogWin == TRUE) {
			uDlgRC = c_dialog2(pHoldCat->mDlg, gmCurrentCtrl, aTermKeys,
			                   _FocusFilter, C_DLG_CLICKEXIT);

			gfInDialogWin = FALSE;
			if (ISMOUSE(uDlgRC)) {
				int nClickRow;
				int nClickCol;
				Mouse_State mMState;

				if (s_getwindow(&nClickCol, &nClickRow) != gmMenuWin) {
					gfInDialogWin = TRUE;

					// Mouse event (click) was not being taken off the queue and
					//  the c_dialog2 routine was exiting as soon as it entered,
					//  causing another infinite looping problem
					// The kludge below adds a keystroke to the queue and eats all
					//  of the mouse events up to that point
					k_addkey(_ESC);
					while (ISMOUSE(s_getclick(&mMState)))
						;
				}
			}
		}

		// Turn off focus on current control
		c_change(gmCurrentCtrl, C_FOCUS, DISABLE, ENABLE);
		pHoldCtrl = (INI_CONTROL *)c_user_pointer(gmCurrentCtrl, NULL);
		_FixCtrlHilight(gmCurrentCtrl, pHoldCtrl->uType, CTL_UNFOCUS);

		// Clear help line
		_LineHelp(" ", NULL);

		// Prepare to process main menu
		m_start(gmMain, m_getitem(gmMain, 1));

		// If a menu hot-key (or exit key) was hit, drop appropriate menu
		switch (uDlgRC) {
		case ALT_8:  // Certain keys used to exit the dialog are mapped to ALT_8
         // Pop up box for exit
			uReturnCat = _PopUpExitBox();
			if (uReturnCat == B_RESUME)
				uReturnCat = uCategory;
			break;
		case ALT_9:  // Certain keys used to exit the dialog are mapped to ALT_9
		case ALT_X:  
			// Drop Exit menu
			k_addkey(_ENTER);
			uReturnCat = _ProcessMainMenu(uCategory);
			break;
		case ALT_C:
			// Drop Configure menu
			m_start(gmMain, m_getitem(gmMain, 2));
			k_addkey(_ENTER);
			uReturnCat = _ProcessMainMenu(uCategory);
			break;
		case ALT_H:
			// Drop Help menu
			m_start(gmMain, m_getitem(gmMain, 3));
			k_addkey(_ENTER);
			uReturnCat = _ProcessMainMenu(uCategory);
			break;
		default:  // For mouse events and other exit keys not listed above
			uReturnCat = _ProcessMainMenu(uCategory);
		}  // End switch
	}  // End while

	// Keep windows from being displayed as soon as they are created
  	d_change(WDISPLAY, DISABLE);

	return(uReturnCat);
}  // End DlgShowWindow


/*
 *************************************************************************
 ** DlgCloseWindow
 *************************************************************************
 ** Destroy window
 *************************************************************************
 ** Requires: uCategory = category of window to close
 ** Returns :  0 success
 **           <0 error (see code for individual reasons)
 *************************************************************************
 */
int DlgCloseWindow(unsigned int uCategory) {
	int i = 0;
	unsigned int uControlListLen;


	// Check range of category and that the dialog is open
	if ((uCategory >= guCategoryCnt) || !gfDlgOpen)
		return(-1);

	// Make sure the screen has been built
	if (gaCategoryList[uCategory].fBuilt == FALSE)
		return(-2);

	uControlListLen = sizeof(gaControlList) / sizeof(INI_CONTROL);
	i = gaCategoryList[uCategory].uFirstIndex;

	while ((gaControlList[i].uType != INI_CTL_NULL) && (i < uControlListLen)) {
		// Free memory allocated for directives
		if (gaControlList[i].pszDirective)
			free(gaControlList[i].pszDirective);

		// Delete combo boxes (special kludge for these controls)
		if ((gaControlList[i].uType == INI_CTL_COLOR) ||
		    (gaControlList[i].uType == INI_CTL_COMBO))
			c_delete(gaControlList[i].mCtrl);

		i++;
	}  // End while

	// Close window for line help
	_LineHelp(NULL, NULL);

	// Close the window
	w_close(gaCategoryList[uCategory].mDlg);

	gaCategoryList[uCategory].mDlg = NULL;
	gaCategoryList[uCategory].fBuilt = FALSE;

	return(0);
}  // End DlgCloseWindow


/*
 *************************************************************************
 ** DlgClose
 *************************************************************************
 ** Final cleanup
 *************************************************************************
 ** Requires:  nothing
 ** Returns :  0 success
 **           -1 dialog not open
 *************************************************************************
 */
int DlgClose(void) {

	// Check that the dialog is open
	if (!gfDlgOpen)
		return(-1);

	if (gpINIIndex)
		free(gpINIIndex);

	// Close menus
	m_free(gmExit);
	m_free(gmConfigure);
	m_free(gmHelp);
	m_free(gmMain);

	if (gfUseMouse) {
		// Close mouse
   	s_close();
	}

	// Close all windows
	w_closeall();

	gfDlgOpen = FALSE;

	return(0);
}  // End DlgClose


/*
 *************************************************************************
 ** DlgSetHelpExe
 *************************************************************************
 ** Set path to 4HELP
 *************************************************************************
 ** Requires:  pszExeName = fully qualified name for 4HELP
 ** Returns :  0 success
 **           -1 failure
 *************************************************************************
 */
int DlgSetHelpExe(char *pszExeName) {
	int nNameLen = strlen(pszExeName);


	if ((nNameLen > 0) && (nNameLen < sizeof(gszHelpExe))) {
		strcpy(gszHelpExe, pszExeName);
		gfHelpAvail = TRUE;
		return(0);
	}

	return(-1);
}  // End DlgSetHelpExe


/*
 *************************************************************************
 ** DlgSetFieldDirective
 *************************************************************************
 ** Assign the true directive name to the control
 *************************************************************************
 ** Requires:  uIndex = index of control to modify
 **            pszName = directive name
 ** Returns :  0 success
 *************************************************************************
 */
int DlgSetFieldDirective(unsigned int uIndex, char *pszName) {
	int i = 0;
	char *pszBuffer;
	INI_CONTROL *pCtrlData;


	pCtrlData = gpINIIndex[uIndex].pElement;

	pszBuffer = (char *)malloc(strlen(pszName) + 1);
	if (pszBuffer == NULL)
		_MemoryError("DlgSetFieldDirective:Buffer");
	strcpy(pszBuffer, pszName);
	pCtrlData->pszDirective = pszBuffer;

	switch (pCtrlData->uType) {
	case INI_CTL_COLOR:
		// Set directive for both BG color fields as well
		pCtrlData[1].pszDirective = pszBuffer;
		break;

	case INI_CTL_RADIO:
		// Set directive for rest of radio buttons
		for (i = 1; pCtrlData[i].Var1.uGroup == pCtrlData[0].Var1.uGroup; i++)
			pCtrlData[i].pszDirective = pszBuffer;
		break;
	}  // End switch

	return(0);
}  // End DlgSetFieldDirective


/*
 *************************************************************************
 ** DlgSetFieldVal
 *************************************************************************
 ** Set value of int, uint, checkbox or radio button control
 *************************************************************************
 ** Requires:  uIndex = index of control to modify
 **            uValue = new value for control
 ** Returns :  0 success
 **           -1 error
 *************************************************************************
 */
int DlgSetFieldVal(unsigned int uIndex, unsigned int uValue) {
	INI_CONTROL *pCtrlData;
	int nValue;
	int nRC = 0;


	// Reference the control data by using index
	pCtrlData = gpINIIndex[uIndex].pElement;
	nValue = (int)uValue;

	switch (pCtrlData->uType) {
	case INI_CTL_INT:  // Integer field
	case INI_CTL_UINT:  // Unsigned integer field
		nRC = c_change(pCtrlData->mCtrl, C_STATE, nValue, ENABLE);
		break;

	case INI_CTL_CHECK:  // Checkbox control; TRUE value checks box
		nRC = c_change(pCtrlData->mCtrl, C_STATE, nValue, ENABLE);
		break;

	case INI_CTL_RADIO:  // Radio button control
		// Check uValue'th button in group
		nRC = c_change(pCtrlData[uValue].mCtrl, C_STATE, TRUE, ENABLE);
		break;

	}  // End switch

	return(nRC);
}  // End DlgSetFieldVal


/*
 *************************************************************************
 ** DlgGetFieldVal
 *************************************************************************
 ** Get the current value of an int, uint or check box control
 *************************************************************************
 ** Requires:  uIndex = index of control
 **            puValue = pointer to storage area for field data
 ** Returns :  0 success
 *************************************************************************
 */
int DlgGetFieldVal(unsigned int uIndex, unsigned int *puValue) {
	INI_CONTROL *pCtrlData;
	int nRC = 0;


	// Reference the control data by using index
	pCtrlData = gpINIIndex[uIndex].pElement;

	switch (pCtrlData->uType) {

	// May have a problem w/ c_read() returning -1 if error
	// Don't know how else you're supposed to get the value of an int

	case INI_CTL_INT:  // Integer field
	case INI_CTL_UINT:  // Unsigned integer field
	case INI_CTL_CHECK:  // Checkbox control; TRUE value checks box
		*puValue = c_read(pCtrlData->mCtrl, C_STATE);
		break;

	}  // End switch

	return(nRC);
}  // End DlgGetFieldVal


/*
 *************************************************************************
 ** DlgGetComboVal
 *************************************************************************
 ** Get the current value of a combo box control
 *************************************************************************
 ** Requires:  uIndex = index of control
 **            puValue = pointer to storage area for field data
 ** Returns :  0 success
 **           -1 error
 *************************************************************************
 */
int DlgGetComboVal(unsigned int uIndex, unsigned int *puValue) {
	INI_CONTROL *pCtrlData;
	Field mField;
	char szBuffer[MAX_COMBO_LEN];
	int nPosInList = -1;
	int nRC = -1;


	// Reference the control data by using index
	pCtrlData = gpINIIndex[uIndex].pElement;

	// Get the value
	*puValue = 0;
	mField = c_get_field(pCtrlData->mCtrl);
	if (f_getstring(mField, szBuffer) >= 0) {
		nPosInList = c_find_item(pCtrlData->mCtrl, szBuffer);
		if (nPosInList >= 0) {
			*puValue = (unsigned int)nPosInList;
			nRC = 0;
		}
	}

	return(nRC);
}  // End DlgGetComboVal


/*
 *************************************************************************
 ** DlgGetColorVal
 *************************************************************************
 ** Get the current values of foreground and background color combo boxes
 *************************************************************************
 ** Requires:  uIndex = index of control
 **            puFGIndex = pointer to storage area for foreground color
 **            puBGIndex = pointer to storage area for background color
 ** Returns :  0 success
 **           -1 error
 *************************************************************************
 */
int DlgGetColorVal(unsigned int uIndex, unsigned int *puFGIndex,
                   unsigned int *puBGIndex) {
	INI_CONTROL *pCtrlData;
	Field mField;
	char szBuffer[MAX_COMBO_LEN];
	int nFGPosInList = -1;
	int nBGPosInList = -1;
	int nRC = -1;


	// Reference the control data by using index
	pCtrlData = gpINIIndex[uIndex].pElement;

	// Get foreground color
	mField = c_get_field(pCtrlData[0].mCtrl);
	if (f_getstring(mField, szBuffer) >= 0) {
		nFGPosInList = c_find_item(pCtrlData[0].mCtrl, szBuffer);
	}

	// Get background color
	mField = c_get_field(pCtrlData[1].mCtrl);
	if (f_getstring(mField, szBuffer) >= 0) {
		nBGPosInList = c_find_item(pCtrlData[1].mCtrl, szBuffer);
	}

	if ((nFGPosInList >= 0) && (nBGPosInList >= 0)) {
		// If we got here, we have valid values
		*puFGIndex = nFGPosInList;
		*puBGIndex = nBGPosInList;
		nRC = 0;
	}

	return(nRC);
}  // End DlgGetColorVal


/*
 *************************************************************************
 ** DlgGetRadioVal
 *************************************************************************
 ** Get the current value of a radio button control
 *************************************************************************
 ** Requires:  uIndex = index of control
 **            puValue = pointer to storage area for field data
 ** Returns :  0 success
 **           -1 error
 *************************************************************************
 */
int DlgGetRadioVal(unsigned int uIndex, unsigned int *puValue) {
	INI_CONTROL *pCtrlData;
	int i;
	int nRC = 0;


	// Reference the control data by using index
	pCtrlData = gpINIIndex[uIndex].pElement;

	// Check each button and return the index within the group (starting from
	//  zero)
	// Assumes that radio buttons that are grouped are in sequence in the
	//  element list
	i = 0;
	while ((pCtrlData[i].uType == INI_CTL_RADIO) &&
	       (pCtrlData[i].Var1.uGroup == pCtrlData[0].Var1.uGroup) &&
	       (c_read(pCtrlData[i].mCtrl, C_STATE) == FALSE))
		i++;

	if ((pCtrlData[i].uType == INI_CTL_RADIO) &&
	    (pCtrlData[i].Var1.uGroup == pCtrlData[0].Var1.uGroup)) {
		// Found the radio button that is on
		*puValue = i;
		nRC = 0;
	}
	else {
		nRC = -1;
	}

	return(nRC);
}  // End DlgGetRadioVal


/*
 *************************************************************************
 ** DlgSetFieldValRange
 *************************************************************************
 ** Set the range and increment values for an integer control
 *************************************************************************
 ** Requires:  uIndex = index of control to modify
 **            nMin = minimum field value
 **            nMax = maximum field value
 **            uIncrement = increment value
 ** Returns :  0 success
 **           -1 error
 *************************************************************************
 */
int DlgSetFieldValRange(unsigned int uIndex, int nMin, int nMax,
                        unsigned int uIncrement) {
	INI_CONTROL *pCtrlData;
	int nRC = 0;


	// Reference the control data using the index
	pCtrlData = gpINIIndex[uIndex].pElement;


	nRC = c_change(pCtrlData->mCtrl, C_INT_MIN, nMin, DISABLE);

	if (nRC == -1)
		return(-1);

	nRC = c_change(pCtrlData->mCtrl, C_INT_MAX, nMax, DISABLE);

	if (nRC == -1)
		return(-1);

	nRC = c_change(pCtrlData->mCtrl, C_INT_INCREMENT, (int)uIncrement, ENABLE);

	if (nRC == -1)
		return(-1);
	else
		return(0);
}  // End DlgSetFieldValRange


/*
 *************************************************************************
 ** DlgSetFieldString
 *************************************************************************
 ** Set the value of a text field, or add a value to the list for a combo
 **    box or color combo box
 *************************************************************************
 ** Requires:  uIndex = index of control to modify
 **            pszString = string data to write to field
 ** Returns :  0 success
 **           -1 error
 *************************************************************************
 */
int DlgSetFieldString(unsigned int uIndex, char *pszString) {
	INI_CONTROL *pCtrlData;
	int nRC = 0;


	// Reference the control data using the index
	pCtrlData = gpINIIndex[uIndex].pElement;

	switch (pCtrlData->uType) {
	case INI_CTL_TEXT:  // Text field
		if (f_setbuffer(c_get_field(pCtrlData->mCtrl), pszString) < 0)
			 nRC = -1;
		else
			c_draw(pCtrlData->mCtrl);
		break;

	case INI_CTL_COMBO:  // Combo box
		if (c_add_item(pCtrlData->mCtrl, pszString, -1, ENABLE) < 0)
			nRC = -1;
		break;

	case INI_CTL_COLOR:  // Color list control
		if (c_add_item(pCtrlData[0].mCtrl, pszString, -1, ENABLE) < 0)
			nRC = -1;
		else if (c_add_item(pCtrlData[1].mCtrl, pszString, -1, ENABLE) < 0)
			nRC = -1;
		break;

	}  // End switch

	return(nRC);
}  // End DlgSetFieldString


/*
 *************************************************************************
 ** DlgSetComboDefault
 *************************************************************************
 ** Set the default value of a combo box
 *************************************************************************
 ** Requires:  uIndex = index of control to modify
 **            uListIndex = index into list of combo box values referring
 **                            to the default value
 ** Returns :  0 success
 **           -1 error: bad index
 **           -2 error: could not set default value
 *************************************************************************
 */
int DlgSetComboDefault(unsigned int uIndex, unsigned int uListIndex) {
	INI_CONTROL *pCtrlData;
	char szBuffer[MAX_COMBO_LEN];
	int nRC = 0;


	// Reference the control data using the index
	pCtrlData = gpINIIndex[uIndex].pElement;

	if (c_get_item(pCtrlData->mCtrl, uListIndex, szBuffer, MAX_COMBO_LEN) !=
	    -1) {
		nRC = f_setbuffer(c_get_field(pCtrlData->mCtrl), szBuffer);

		c_change(pCtrlData->mCtrl, C_STATE, uListIndex, ENABLE);
		c_draw(pCtrlData->mCtrl);
	}
	else
		nRC = -2;

	return(nRC);
}  // End DlgSetComboDefault


/*
 *************************************************************************
 ** DlgSetColorDefault
 *************************************************************************
 ** Set the default value of a color combo box pair
 *************************************************************************
 ** Requires:  uIndex = index of control to modify
 **            uFGIndex = index into list of combo box values referring
 **                            to the default foreground color
 **            uBGIndex = index into list of combo box values referring
 **                            to the default background color
 ** Returns :  0 success
 **           -1 error: bad FG index
 **           -2 error: could not set FG default
 **           -3 error: bad BG index
 **           -4 error: could not set BG default
 *************************************************************************
 */
int DlgSetColorDefault(unsigned int uIndex, unsigned int uFGIndex,
                       unsigned int uBGIndex) {
	INI_CONTROL *pCtrlData;
	char szBuffer[MAX_COMBO_LEN];


	// Reference the control data using the index
	pCtrlData = gpINIIndex[uIndex].pElement;

	// Set foreground default
	if (c_get_item(pCtrlData[0].mCtrl, (int)uFGIndex, szBuffer,
	               MAX_COMBO_LEN) == -1)
		return(-1);

	if (f_setbuffer(c_get_field(pCtrlData[0].mCtrl), szBuffer) < 0)
		return(-2);

	c_change(pCtrlData[0].mCtrl, C_STATE, uFGIndex, ENABLE);

	c_draw(pCtrlData[0].mCtrl);

	// Set background default
	if (c_get_item(pCtrlData[1].mCtrl, uBGIndex, szBuffer, MAX_COMBO_LEN) == -1)
		return(-3);

	if (f_setbuffer(c_get_field(pCtrlData[1].mCtrl), szBuffer) < 0)
		return(-4);

	c_change(pCtrlData[1].mCtrl, C_STATE, uBGIndex, ENABLE);

	c_draw(pCtrlData[1].mCtrl);

	return(0);
}  // End DlgSetColorDefault


/*
 *************************************************************************
 ** DlgGetFieldString
 *************************************************************************
 ** Get the string value of a text field, combo box, or color combo box
 *************************************************************************
 ** Requires:  uIndex = index of control
 **            pszString = pointer to storage area for field value
 **            nStringMax = maximum length of string to return
 ** Returns :  0 success
 **           -1 error
 *************************************************************************
 */
int DlgGetFieldString(unsigned int uIndex, char *pszString, int nStringMax) {
	INI_CONTROL *pCtrlData;
	Field mField;
	int nFieldLen;
	char *pData;
	int nRC = -1;


	// Reference the control data using the index
	pCtrlData = gpINIIndex[uIndex].pElement;

	switch (pCtrlData->uType) {
	case INI_CTL_TEXT:  // Text field
	case INI_CTL_COLOR:  // Color list control
	case INI_CTL_COMBO:  // Combo box
		mField = c_get_field(pCtrlData->mCtrl);
		nFieldLen = f_buffersize(mField);

		// If field data is too big for buffer, cut it down
		if (nFieldLen >= nStringMax) {
			pData = (char *)malloc(nFieldLen + 1);
			if (pData == NULL)
				_MemoryError("Data");

			nRC = f_getbuffer(mField, pData);
			*(pData + nStringMax - 1) = '\0';
			strcpy(pszString, pData);

			free(pData);
		}
		else
			nRC = f_getbuffer(mField, pszString);
		break;
	}  // End switch

	if (nRC > 0) {
		// Strip trailing spaces
		strip_trailing(pszString, " ");

		// Strip leading spaces
		strip_leading(pszString, " ");

		// Want to return zero if good, not number of chars read
		nRC = 0;
	}

	return(nRC);
}  // End DlgGetFieldString


#ifdef DEBUG
/*
 *************************************************************************
 ** DlgIndexTest
 *************************************************************************
 ** Test individual control to see if it exists and is of correct type
 ** Only used when debugging
 *************************************************************************
 ** Requires:  uINIControlIndex = index of control to test
 **            uCtrlType = type control should be
 ** Returns :  0 success
 **           -1 control does not exist
 **           -2 control is not of correct type
 *************************************************************************
 */
int DlgIndexTest(unsigned int uINIControlIndex, unsigned int uCtrlType) {

	// Compare element to xFFFF
	if ((unsigned int)gpINIIndex[uINIControlIndex].pElement ==
	    (unsigned int)-1)
		return(-1);

	if (gpINIIndex[uINIControlIndex].pElement->uType != uCtrlType)
{
printf("\nIndex = %d, pointer = %p, TUI type %d != INIFILE type %d",
uINIControlIndex, gpINIIndex[uINIControlIndex].pElement, gpINIIndex[uINIControlIndex].pElement->uType, uCtrlType);
		return(-2);
}

	return(0);
}  // End DlgIndexTest
#endif  // DEBUG


/*
 *************************************************************************
 ** _FocusFilter
 *************************************************************************
 ** Function called automatically every time control focus is changed, or
 **    a control is selected
 ** Used to clear and set control specific single-line help and a kludge
 **    to change the color of the hilighted hot-key character to the same
 **    color as the focus text
 *************************************************************************
 ** Requires:  mWin = current window
 **            mCtrl = control about to gain or lose focus, or be selected
 **            nEvent = CTL_FOCUS, CTL_UNFOCUS, CTL_SELECT
 ** Returns :  0 success
 *************************************************************************
 */
static int _FocusFilter(Window mWin, Control mCtrl, int nEvent) {
	INI_CONTROL *pHoldCtrl;


	pHoldCtrl = (INI_CONTROL *)c_user_pointer(mCtrl, NULL);

#ifdef DEBUG
	// Set cursor to a size we can see in all controls
	//_SetCursorSize(6, 9, TRUE);
#endif

	// Highlighted character was not changing to focus color, so we have
	//  to force the color change
	_FixCtrlHilight(mCtrl, pHoldCtrl->uType, nEvent);

	switch (nEvent) {
	case CTL_FOCUS:  // The control is about to receive focus
		// Set global
		gmCurrentCtrl = mCtrl;

		// Write help line
		if (pHoldCtrl != NULL)
			_LineHelp(pHoldCtrl->pszHelp, pHoldCtrl->pszDirective);
		break;

	case CTL_UNFOCUS:  // The control is about to lose focus
		// Clear help line
		_LineHelp(NULL, NULL);

		// Unset global
		gmCurrentCtrl = NULL;
		break;

	case CTL_SELECT:  // The control is about to be selected
		break;

	}  // End switch

	return(0);
}  // End _FocusFilter


/*
 *************************************************************************
 ** _FixCtrlHilight
 *************************************************************************
 ** Kludge to change the color of the hilighted hot-key character to the
 **    same color as the focus text
 *************************************************************************
 ** Requires:  mCtrl = control about to gain or lose focus, or be selected
 **            uType = INI_ type of control
 **            nEvent = CTL_FOCUS, CTL_UNFOCUS, CTL_SELECT
 ** Returns :  0 success
 *************************************************************************
 */
static int _FixCtrlHilight(Control mCtrl, unsigned int uType, unsigned int nEvent) {

	switch (nEvent) {
	case CTL_FOCUS:  // The control is about to receive focus

		// Highlighted character was not changing to focus color, so we have
		//  to force the color change
		switch (uType) {
		case INI_CTL_INT:
		case INI_CTL_UINT:
			c_change(mCtrl, C_INT_HIGHLIGHT, SC_FOCUS, ENABLE);
			break;

		case INI_CTL_CHECK:
		case INI_CTL_RADIO:
			c_change(mCtrl, BOX_HIGHLIGHT, SC_FOCUS, ENABLE);
			break;

		case INI_CTL_BUTTON:
			c_change(mCtrl, BTN_HIGHLIGHT, SC_FOCUS_BUTTON, ENABLE);
			break;

		case INI_CTL_TAB:
			c_change(mCtrl, BTN_HIGHLIGHT, SC_FOCUS, ENABLE);
			break;

		case INI_CTL_COLOR:
		case INI_CTL_COMBO:
			c_change(mCtrl, C_LIST_HIGHLIGHT, SC_FOCUS, ENABLE);
			break;

		case INI_CTL_TEXT:
			c_change(mCtrl, C_FLD_HIGHLIGHT, SC_FOCUS, ENABLE);
			break;
		}  // End switch on uType

		break;

	case CTL_UNFOCUS:  // The control is about to lose focus
		// Change hilighted char back to hilight color
		switch (uType) {
		case INI_CTL_INT:
		case INI_CTL_UINT:
			c_change(mCtrl, C_INT_HIGHLIGHT, SC_HILITE, ENABLE);
			break;

		case INI_CTL_CHECK:
		case INI_CTL_RADIO:
			c_change(mCtrl, BOX_HIGHLIGHT, SC_HILITE, ENABLE);
			break;

		case INI_CTL_BUTTON:
			c_change(mCtrl, BTN_HIGHLIGHT, SC_HILITE_BUTTON, ENABLE);
			break;

		case INI_CTL_TAB:
			c_change(mCtrl, BTN_HIGHLIGHT, SC_HILITE, ENABLE);
			break;

		case INI_CTL_COLOR:
		case INI_CTL_COMBO:
			c_change(mCtrl, C_LIST_HIGHLIGHT, SC_HILITE, ENABLE);
			break;

		case INI_CTL_TEXT:
			c_change(mCtrl, C_FLD_HIGHLIGHT, SC_HILITE, ENABLE);
			break;
		}  // End switch on uType

	case CTL_SELECT:  // The control is about to be selected
		break;

	}  // End switch on nEvent

	return(0);
}  // End _FixCtrlHilight


/*
 *************************************************************************
 ** KeyFilter
 *************************************************************************
 ** Function called automatically every time a key is pressed
 ** Used to call help when F1 is pressed and for various key kludges
 *************************************************************************
 ** Requires:  nKey = value of key just pressed
 ** Returns :  translated key value
 *************************************************************************
 */
static int _KeyFilter(int nKey) {
	int nRC = nKey;  // Default is to pass key on to normal mWindow handler


	switch (nKey) {
	case _F1:
		_F1Help();
	
		nRC = -1;  // Causes key to be ignored
		break;

	case CTRL_C:
	case ALT_F4:
		// If we're in the dialog window, translate to something that can be
		//  used as a termination key for any control
		// This needs to be unique from the key for F10, because we want
		//  to handle it differently when the dialog is terminated
		//
		// If we are in the menu, exit menu
		if (gfInDialogWin)
			nRC = ALT_8;
		else {
			nRC = _ESC;
		}
		break;

	case _F10:
		// If we're in the dialog window, translate to something that can be
		//  used as a termination key for any control
		if (gfInDialogWin)
			nRC = (int)ALT_9;
		break;

	case ALT_8:
	case ALT_9:
		// Ignore these keys if we're in the dialog window
		if (gfInDialogWin)
			nRC = -1;

	// If we're in the menu, translate tabs to arrows to move between sub menus
	case _TAB:
		if (!gfInDialogWin)
			nRC = _RARROW;
		break;
	case SHIFT_TAB:
		if (!gfInDialogWin)
			nRC = _LARROW;
		break;

	}  // End switch 

	return(nRC);
}  // End _KeyFilter


/*
 *************************************************************************
 ** MouseFilter
 *************************************************************************
 ** Function called automatically every time a mouse event occurs
 ** Used to prevent mouse clicks on bottom line help from exiting dialog
 *************************************************************************
 ** Requires:  pMouseStatus = pointer to the mouse state structure
 ** Returns :  1 to add the event to the queue
 **            0 to ignore the event
 *************************************************************************

static int _MouseFilter(Mouse_State *pMouseStatus) {
	int nRC = 1;  // Default is to add the event to the queue


#ifdef QQQ
	// Ignore mouse action in bottom-line help window except for mouse movement
	if ((pMouseStatus->window == gmHelpWin) &&
	    (pMouseStatus->event_code != MOUSE_MOVE))
		nRC = 0;
#endif

	// Queue dialog exit key if clicked on menu
	if (gfInDialogWin &&
	    (pMouseStatus->window == gmMenuWin) &&
	    (pMouseStatus->event_code != MOUSE_MOVE))
			k_addkey(ALT_9);


	return(nRC);
}  // End _MouseFilter
*/


/*
 *************************************************************************
 ** F1Help
 *************************************************************************
 ** Decide which directive information to send to help function
 *************************************************************************
 ** Requires:  none
 ** Returns :  none
 *************************************************************************
 */
static void _F1Help(void) {
	INI_CONTROL *pControl;


	// Show help on current control
	pControl = (INI_CONTROL *)c_user_pointer(gmCurrentCtrl, NULL);
	if (pControl != NULL) {
		// If there is an alternate directive, use it
		// This is to get appropriate help for directives that cannot be
		//  looked up under the directive name (ie. share the same name as
		//  a command)
		if (pControl->pszHelpDirective != NULL)
			_Call4Help(pControl->pszHelpDirective);
		else
			_Call4Help(pControl->pszDirective);
	}
	else {
		// No directive name
		_Call4Help(NULL);
	}

}  // End _F1Help


/*
 *************************************************************************
 ** Call4Help
 *************************************************************************
 ** Set up call to 4HELP
 *************************************************************************
 ** Requires:  pszHelpID = text to look up in 4HELP
 ** Returns :  none
 *************************************************************************
 */
static void _Call4Help(char *pszHelpID) {
	Window mCurrentWin;
	static char *pszOptions1 = "/NX";     // Disable Alt-X
	static char *pszOptions2 = "/NX /X";  // Disable Alt-X, no mouse support
	char *pszHoldCmd;
	char *pszOpts = pszOptions1;
	//unsigned short _far *pnCursorStatus = MAKEP(0, 0x460);  // Ptr to cursor data
	unsigned int _far *pnCursorStatus = MAKEP(0, 0x460);  // Ptr to cursor data
	unsigned int uHoldCursor = 0;

	if (!gfHelpAvail) {
#ifdef DEBUG
		_PopUpBox("DEBUG", "4HELP.EXE location was not set");
#endif
		return;
	}

	if (pszHelpID == NULL)
		return;

	if (!gfUseMouse) {
		pszOpts = pszOptions2;
	}

	// Build command to call help
	pszHoldCmd = (char *)malloc(strlen(gszHelpExe) + strlen(pszOpts) +
	                            strlen(pszHelpID) + 3);

	if (pszHoldCmd == NULL)
		_MemoryError("HoldCmd");

	sprintf(pszHoldCmd, "%s %s %s", gszHelpExe, pszOpts, pszHelpID);

	// Store current cursor status
	uHoldCursor = *pnCursorStatus;

	// Close mouse (call to 4Help kills mouse functionality)
	if (gfUseMouse) {
		s_close();
		//s_off();
	}

	mCurrentWin = gaCategoryList[guCurrentCat].mDlg;

	// Hide windows
	w_off(mCurrentWin);
	w_off(gmMenuWin);
	w_off(gmHelpWin);

#ifdef DEBUG
	{
		int rc = 0;
		char szHold[75];

		errno = 0;
		if ((rc = system(pszHoldCmd)) != 0) {
			sprintf(szHold, "DEBUG:  Help call failed %d, errno=%d", rc, errno);
			_PopUpBox(szHold, getenv("COMSPEC"));
		}
	}
#else
	// Call help program
	system(pszHoldCmd);
#endif

	free(pszHoldCmd);

	// Restore windows
	w_on(gmHelpWin);
	w_on(gmMenuWin);
	w_on(mCurrentWin);
	if (gfInDialogWin) {
		w_front(mCurrentWin);
	}
	else {
		w_front(gmMenuWin);

		// Back out of menus because there is no way to turn the sub-menus
		//  on and off and we may have been viewing a sub-menu when help was
		//  called
		k_addkey(_ESC);
	}

	if (gfUseMouse) {
		// Open mouse
		s_open();
/* EWL
		s_filter(_MouseFilter);
		s_limit(0, (ex_crt_columns - 1) * ex_mouse.percol,
		        0, 23 * ex_mouse.perrow);
*/
		//s_softreset();
		//s_on();
	}

	// Restore cursor status
	_SetCursorStatus(uHoldCursor);

	return;
}  // End _Call4Help


/*
 *************************************************************************
 ** LineHelp
 *************************************************************************
 ** Display single line help
 *************************************************************************
 ** Requires:  pszHelp = help text to display
 **            pszDirective = directive name to display
 ** Returns :  none
 *************************************************************************
 */
static void _LineHelp(char *pszHelp, char *pszDirective) {
	static int fWindowOpen = FALSE;
	unsigned int uHelpLen;
	int nHoldColor;
	int nHoldBorder;
	char szMore[] = "<F1 for more>";


	if (fWindowOpen) {
		w_close(gmHelpWin);
		fWindowOpen = FALSE;
	}

	if (pszHelp != NULL) {
		// Display help message
		nHoldColor = d_change(WCOLOR, SC_WIN_LINE);
		nHoldBorder = d_change(WBRDTYPE, (int)BRD_NONE);

		gmHelpWin = w_open(HELP_COL, ex_crt_rows - 1, ex_crt_columns, 1);

		fWindowOpen = TRUE;
		w_putsat(gmHelpWin, 0, 0, pszHelp);

		if (pszDirective != NULL) {
			// Display directive
			uHelpLen = strlen(pszHelp);
			w_putsat(gmHelpWin, uHelpLen + 2, 0, "[");
			w_putsat(gmHelpWin, uHelpLen + 3, 0, pszDirective);
			w_putsat(gmHelpWin, uHelpLen + 3 + strlen(pszDirective), 0, "]");

			// Display 'more' message if the help exe name was set
			if (gfHelpAvail)
				w_putsat(gmHelpWin, ex_crt_columns - strlen(szMore), 0, szMore);
		}

		// Return defaults to previous values
		d_change(WCOLOR, nHoldColor);
		d_change(WBRDTYPE, nHoldBorder);
	}

	return;
}  // End _LineHelp


/*
 *************************************************************************
 ** CreateControl
 *************************************************************************
 ** Determine control type and call appropriate routine to create it
 *************************************************************************
 ** Requires:  mWin = current window
 **            pCtrlData = control to create
 ** Returns :  0 success
 **           -1 control could not be created
 *************************************************************************
 */
static int _CreateControl(Window mWin, INI_CONTROL *pCtrlData) {
	int nRC = 0;

	switch (pCtrlData->uType) {
	case INI_CTL_TEXT:  // Text field
		if (_INIField(mWin, pCtrlData) == NULL)
			nRC = -1;
		break;

	case INI_CTL_INT:  // Integer field
		if (_INIInteger(mWin, pCtrlData, TRUE) == NULL)
			nRC = -1;
		break;

	case INI_CTL_UINT:  // Unsigned integer field
		if (_INIInteger(mWin, pCtrlData, FALSE) == NULL)
			nRC = -1;
		break;

	case INI_CTL_CHECK:  // Checkbox control
		if (_INICheck(mWin, pCtrlData) == NULL) 
			nRC = -1;
		break;

	case INI_CTL_RADIO:  // Radio button control
		if (_INIRadio(mWin, pCtrlData) == NULL)
			nRC = -1;
		break;

	case INI_CTL_COLOR:  // Color list control
	case INI_CTL_COMBO:  // Combo box control
		if (_INICombo(mWin, pCtrlData) == NULL) 
			nRC = -1;
		break;

	case INI_CTL_BOX:  // Draw box
		nRC = _INIBox(mWin, pCtrlData);
		break;

	case INI_CTL_TAB:  // Create tab
		nRC = _INITab(mWin, pCtrlData);
		break;

	case INI_CTL_STATIC:  // Add static text
		nRC = _INIStatic(mWin, pCtrlData);
		break;
	}  // End switch

	return(nRC);
}  // End _CreateControl


/*
 *************************************************************************
 ** InitControlElement
 *************************************************************************
 ** Initialize INI_CONTROL structure
 *************************************************************************
 ** Requires:  pCtrlData = control to initializes
 ** Returns :  none
 *************************************************************************

static void _InitControlElement(INI_CONTROL *pCtrlData) {
	pCtrlData->uType = INI_CTL_NULL;
	pCtrlData->uID = 0;
	pCtrlData->mCtrl = NULL;
	pCtrlData->uCol = 0;
	pCtrlData->uRow = 0;
	pCtrlData->pszLabel = NULL;
	pCtrlData->pszHelp = NULL;
	pCtrlData->pszDirective = NULL;
	pCtrlData->Var1.uWidth = 0;
	pCtrlData->Var2.uHeight = 0;
}  // End _InitControlElement
*/


/*
 *************************************************************************
 ** InitCategoryElement
 *************************************************************************
 ** Initialize CAT_INDEX structure
 *************************************************************************
 ** Requires:  pCtrlData = control to initialize
 ** Returns :  none
 *************************************************************************
 */
static void _InitCategoryElement(CAT_INDEX *pCatData) {
		pCatData->uFirstIndex = 0;
		pCatData->mFocusCtrl = NULL;
		pCatData->mDlg = NULL;
		pCatData->fBuilt = FALSE;
}  // End _InitCategoryElement


/*
 *************************************************************************
 ** INIBox
 *************************************************************************
 ** Create box
 *************************************************************************
 ** Requires:  mWin = current window
 **            pCtrlData = control to create
 ** Returns :  0 if successful
 **           -1 if control could not be created
 *************************************************************************
 */
static int _INIBox(Window mWin, INI_CONTROL *pCtrlData) {
	int nRC;
	int nHoldColor;


	nHoldColor = w_pencolor(mWin, SC_WIN_BOX);

	// Display box
	nRC = w_box(mWin, LINE_SGL, pCtrlData->uCol + DLG_COL,
	            pCtrlData->uRow + DLG_ROW,
	            pCtrlData->uCol + DLG_COL + pCtrlData->Var1.uWidth - 1,
	            pCtrlData->uRow + DLG_ROW + pCtrlData->Var2.uHeight - 1);

	if (pCtrlData->pszLabel != NULL)
		w_putsat(mWin, pCtrlData->uCol + DLG_COL + 1, pCtrlData->uRow + DLG_ROW,
		         pCtrlData->pszLabel);

	w_pencolor(mWin, nHoldColor);

	return(nRC);
}  // End _INIBox


/*
 *************************************************************************
 ** INIStatic
 *************************************************************************
 ** Write static text to screen
 *************************************************************************
 ** Requires:  mWin = current window
 **            pCtrlData = control to create
 ** Returns :  0 if successful
 **           -1 if control could not be created
 *************************************************************************
 */
static int _INIStatic(Window mWin, INI_CONTROL *pCtrlData) {
	int nRC = 0;


	// Put static text on screen
	if (pCtrlData->pszLabel != NULL)
		nRC = w_putsat(mWin, pCtrlData->uCol + DLG_COL,
		               pCtrlData->uRow + DLG_ROW,
		               pCtrlData->pszLabel);

	return(nRC);
}  // End _INIStatic


/*
 *************************************************************************
 ** INITab
 *************************************************************************
 ** Create tab control
 *************************************************************************
 ** Requires:  mWin = current window
 **            pCtrlData = control to create
 ** Returns :  0 success
 *************************************************************************
 */
static int _INITab(Window mWin, INI_CONTROL *pCtrlData) {
	int nRC = 0;


	// Set the control's user pointer to our data struct
	c_user_pointer(pCtrlData->mCtrl, pCtrlData);

	return(nRC);
}  // End _INITab


/*
 *************************************************************************
 ** INIField
 *************************************************************************
 ** Create data entry field control
 *************************************************************************
 ** Requires:  mWin = current window
 **            pCtrlData = control to create
 ** Returns :  pointer to control if successful
 **            NULL if control could not be created
 *************************************************************************
 */
static Control _INIField(Window mWin, INI_CONTROL *pCtrlData) {
	int i, j;
	int nHLLoc = -1;
	int nAltKey = 0;
	int nLabelLen;
	char *pszHoldLabel = NULL;
	char szSpacer[] = "  ";
	Field mField;

	// Index tables for ALT-key combinations
	const unsigned int ALT_ALPHA[26] = {
   	ALT_A,ALT_B,ALT_C,ALT_D,ALT_E,ALT_F,ALT_G,ALT_H,ALT_I,ALT_J,
   	ALT_K,ALT_L,ALT_M,ALT_N,ALT_O,ALT_P,ALT_Q,ALT_R,ALT_S,ALT_T,
   	ALT_U,ALT_V,ALT_W,ALT_X,ALT_Y,ALT_Z
	};
	const unsigned int ALT_NUM[10] = {
   	ALT_0,ALT_1,ALT_2,ALT_3,ALT_4,ALT_5,ALT_6,ALT_7,ALT_8,ALT_9
	};


	if (pCtrlData->pszLabel != NULL) {
		// If there is a tilde in the label, remove it and record it's location
		//  so that we can hilight it
		nLabelLen = strlen(pCtrlData->pszLabel);
		pszHoldLabel = (char *)malloc(nLabelLen + strlen(szSpacer) + 1);
		if (pszHoldLabel == NULL)
			_MemoryError("INIField;HoldLabel");

		// Copy string by characters, excluding '~'
		for (i = 0, j = 0; i <= nLabelLen; i++) {
			// Record location of tilde if it preceeds an alphanumeric char
			if ((pCtrlData->pszLabel[i] == '~') &&
	 	    	isalnum(pCtrlData->pszLabel[i+1]))
				nHLLoc = i;
			else {
				pszHoldLabel[j] = pCtrlData->pszLabel[i];
				j++;
			}
		}  // End for i

		pszHoldLabel[j] = '\0';

		strcat(pszHoldLabel, szSpacer);
	}  // End if label not NULL

	// Create field
   mField = f_create(pszHoldLabel, "_");
   f_expand(mField, pCtrlData->Var2.uBufferLen, pCtrlData->Var1.uWidth);

	// Make field into a control and change settings 
   pCtrlData->mCtrl = c_add_field(mWin, mField, pCtrlData->uCol + DLG_COL,
	                               pCtrlData->uRow + DLG_ROW, NULL);

	// Set highlighted character and hot-key
	if (nHLLoc >= 0) {
		c_change(pCtrlData->mCtrl, C_FLD_HIGHLIGHT, SC_HILITE, DISABLE);
		c_change(pCtrlData->mCtrl, C_FLD_FLABELCOLOR, SC_FOCUS, DISABLE);
		c_change(pCtrlData->mCtrl, C_FLD_HLSTART, nHLLoc, DISABLE);
		c_change(pCtrlData->mCtrl, C_FLD_HLEND, nHLLoc + 1, DISABLE);

		// Index char value into appropriate alt key table
		if (isalpha(*(pszHoldLabel + nHLLoc)))
			nAltKey = ALT_ALPHA[tolower(*(pszHoldLabel + nHLLoc)) - 'a'];
		else if (isdigit(*(pszHoldLabel + nHLLoc)))
			nAltKey = ALT_NUM[*(pszHoldLabel + nHLLoc) - '0'];

		c_change(pCtrlData->mCtrl, C_HOTKEY, nAltKey, ENABLE);
	}

	// Set the control's user pointer to our data struct
	c_user_pointer(pCtrlData->mCtrl, pCtrlData);

	if (pszHoldLabel)
		free(pszHoldLabel);

	return(pCtrlData->mCtrl);
}  // End _INIField


/*
 *************************************************************************
 ** INICheck
 *************************************************************************
 ** Create check box control
 *************************************************************************
 ** Requires:  mWin = current window
 **            pCtrlData = control to create
 ** Returns :  pointer to control if successful
 **            NULL if control could not be created
 *************************************************************************
 */
static Control _INICheck(Window mWin, INI_CONTROL *pCtrlData) {
	char *pszHoldLabel = NULL;
	char spacer[] = "  ";


	if (pCtrlData->pszLabel != NULL) {
		// Add a space in front of label
		pszHoldLabel = (char *)malloc(strlen(pCtrlData->pszLabel) +
	                              	strlen(spacer) + 1);
		if (pszHoldLabel == NULL)
			_MemoryError("INICheck;HoldLabel");

		strcpy(pszHoldLabel, spacer);
		strcat(pszHoldLabel, pCtrlData->pszLabel);
	}  // End if label not NULL

	// Display check box
   pCtrlData->mCtrl = c_add_checkbox(mWin, pszHoldLabel,
	                                  pCtrlData->uCol + DLG_COL,
	                                  pCtrlData->uRow + DLG_ROW,
	                                  (int)USER_EVENT, CHECKBOX_LEFT, NULL);

	c_user_pointer(pCtrlData->mCtrl, pCtrlData);

	if (pszHoldLabel)
		free(pszHoldLabel);

	return(pCtrlData->mCtrl);
}  // End _INICheck


/*
 *************************************************************************
 ** INIRadio
 *************************************************************************
 ** Create radio button control
 *************************************************************************
 ** Requires:  mWin = current window
 **            pCtrlData = control to create
 ** Returns :  pointer to control if successful
 **            NULL if control could not be created
 *************************************************************************
 */
static Control _INIRadio(Window mWin, INI_CONTROL *pCtrlData) {
	char *pszHoldLabel = NULL;
	char spacer[] = " ";


	if (pCtrlData->pszLabel != NULL) {
		// Add a space in front of label
		pszHoldLabel = (char *)malloc(strlen(pCtrlData->pszLabel) +
												strlen(spacer) + 1);
		if (pszHoldLabel == NULL)
			_MemoryError("INIRadio;HoldLabel");

		strcpy(pszHoldLabel, spacer);
		strcat(pszHoldLabel, pCtrlData->pszLabel);
	}  // End if label not NULL

	// Display radio button
   pCtrlData->mCtrl = c_add_radio(mWin, pszHoldLabel,
	                               pCtrlData->uCol + DLG_COL,
	                               pCtrlData->uRow + DLG_ROW,
											 (int)USER_EVENT, pCtrlData->Var1.uGroup,
	                               CHECKBOX_LEFT, NULL);

	c_user_pointer(pCtrlData->mCtrl, pCtrlData);

	if (pszHoldLabel)
		free(pszHoldLabel);

	return(pCtrlData->mCtrl);
}  // End _INIRadio


/*
 *************************************************************************
 ** INICombo
 *************************************************************************
 ** Create combo box control
 *************************************************************************
 ** Requires:  mWin = current window
 **            pCtrlData = control to create
 ** Returns :  pointer to control if successful
 **            NULL if control could not be created
 *************************************************************************
 */
static Control _INICombo(Window mWin, INI_CONTROL *pCtrlData) {
	char *pszHoldLabel = NULL;
	char spacer[] = "  ";


	// Error if field width of 1 due to problem with MIX code
   if (pCtrlData->Var1.uWidth <= 1)
		return(NULL);

	if (pCtrlData->pszLabel != NULL) {
		// Add a spacer to end of label
		pszHoldLabel = (char *)malloc(strlen(pCtrlData->pszLabel) +
												strlen(spacer) + 1);
		if (pszHoldLabel == NULL)
			_MemoryError("INICombo;HoldLabel");

		strcpy(pszHoldLabel, pCtrlData->pszLabel);
		strcat(pszHoldLabel, spacer);
	}  // End if label not NULL

	// Display combo box
	pCtrlData->mCtrl = c_add_combobox(mWin, pszHoldLabel,
	                                  pCtrlData->uCol + DLG_COL,
	                                  pCtrlData->uRow + DLG_ROW,
	                                  pCtrlData->Var1.uWidth + 1 +
                                        strlen(pszHoldLabel),
	                                  pCtrlData->Var2.uHeight,
	                                  CB_NOEDIT | CB_DROPDOWN, 0);

	// Set defaults to get the colors correct
	c_change(pCtrlData->mCtrl, C_COMBO_FDATACOLOR, SC_COMBO_SELECT, DISABLE);
	c_change(pCtrlData->mCtrl, C_COMBO_SDATACOLOR, SC_COMBO_SELECT, DISABLE);
	c_change(pCtrlData->mCtrl, C_LIST_BORDTYPE, (int)BRD_NONE, DISABLE);
	c_change(pCtrlData->mCtrl, C_LIST_FBORDTYPE, (int)BRD_NONE, DISABLE);
	c_change(pCtrlData->mCtrl, C_LIST_SBORDTYPE, (int)BRD_NONE, DISABLE);
	c_change(pCtrlData->mCtrl, C_LIST_FBORDCOLOR, SC_FOCUS, DISABLE);
	c_change(pCtrlData->mCtrl, C_LIST_SBORDCOLOR, SC_FOCUS, DISABLE);
	c_change(pCtrlData->mCtrl, C_LIST_SBTEXTCOLOR, SC_COMBO_SELECT, DISABLE);
	c_change(pCtrlData->mCtrl, C_LIST_STEXTCOLOR, SC_COMBO_SELECT, ENABLE);
	c_change(pCtrlData->mCtrl, C_LIST_BARCOLOR, SC_COMBO_SELECT, ENABLE);
	c_change(pCtrlData->mCtrl, C_LIST_HIGHLIGHT, SC_HILITE, ENABLE);

	c_user_pointer(pCtrlData->mCtrl, pCtrlData);

	if (pszHoldLabel)
		free(pszHoldLabel);

	return(pCtrlData->mCtrl);
}  // End _INICombo


/*
 *************************************************************************
 ** INIInteger
 *************************************************************************
 ** Create integer control
 *************************************************************************
 ** Requires:  mWin = current window
 **            pCtrlData = control to create
 **            fSigned = TRUE if signed, FALSE if unsigned
 ** Returns :  pointer to control if successful
 **            NULL if control could not be created
 *************************************************************************
 */
static Control _INIInteger(Window mWin, INI_CONTROL *pCtrlData, int fSigned) {
	char *pszHoldLabel = NULL;
	char spacer[] = "  ";
	int nFlags = 0;


	if (pCtrlData->pszLabel != NULL) {
		// Add a spacer in front of label
		pszHoldLabel = (char *)malloc(strlen(pCtrlData->pszLabel) +
												strlen(spacer) + 1);
		if (pszHoldLabel == NULL)
			_MemoryError("INIInteger;HoldLabel");

		strcpy(pszHoldLabel, pCtrlData->pszLabel);
		strcat(pszHoldLabel, spacer);
	}  // End if label not NULL

	if (pCtrlData->Var2.fSpin)
		nFlags = C_INT_ARROWS;

	// Display integer field
	if (fSigned) {
   	pCtrlData->mCtrl = c_add_integer(mWin, pszHoldLabel,
		                                 pCtrlData->uCol + DLG_COL,
		                                 pCtrlData->uRow + DLG_ROW,
		                                 pCtrlData->Var1.uWidth,
		                                 nFlags, NULL);
	}
	else {
   	pCtrlData->mCtrl = c_add_unsigned(mWin, pszHoldLabel,
		                                  pCtrlData->uCol + DLG_COL,
		                                  pCtrlData->uRow + DLG_ROW,
		                                  pCtrlData->Var1.uWidth,
		                                  nFlags, NULL);
	}

	c_change(pCtrlData->mCtrl, C_INT_HIGHLIGHT, SC_HILITE, ENABLE);

	c_user_pointer(pCtrlData->mCtrl, pCtrlData);

	if (pszHoldLabel)
		free(pszHoldLabel);

	return(pCtrlData->mCtrl);
}  // End _INIInteger


/*
 *************************************************************************
 ** AddMenu
 *************************************************************************
 ** Create menu bar
 *************************************************************************
 ** Requires:  none
 ** Returns :  none
 *************************************************************************
 */
static int _AddMenu(void) {
	int nRC = 0;
	int i;
	unsigned int uBorder = BRD_SDDD;
	//unsigned int uShadow = SHADOW_RIGHT | SHADOW_BOTTOM;
	unsigned int uShadow = DISABLE;
	char szCatName[MAX_CAT_LEN + 2];
	Menu_Item mItem;
	INI_CONTROL *pmTabCntl;


	// Create menus
	gmMain = m_create(NULL, MENU_COL, MENU_ROW, ex_crt_columns, 1);
	gmExit = m_create(NULL, MENU_COL + 9, MENU_ROW + 2, 6, 3);
	gmConfigure = m_create(NULL, MENU_COL + 18, MENU_ROW + 2, MAX_CAT_LEN,
	                       CATEGORY_MAX);
	gmHelp = m_create(NULL, MENU_COL + 32, MENU_ROW + 2, 19, 5);


	// Main menu
	m_border(gmMain, BRD_NONE, DISABLE, JUST_CENTER);
	m_selectkeys(gmMain, guHorizMenuSelectKeys);
	m_helppos(gmMain, HELP_COL, ex_crt_rows - 1);

	mItem = m_additem(gmMain, "F10:  ");
	m_itemprotect(mItem, ENABLE);
	//m_itemcolor(mItem, SC_MENU_DISABLE, SC_MENU_DISABLE, SC_MENU_DISABLE);

	mItem = m_additem(gmMain, "  E~xit  ");
	m_itemsub(mItem, gmExit);
	m_itemhelp(mItem, "Exit OPTION");

	mItem = m_additem(gmMain, "  ~Configure  ");
	m_itemsub(mItem, gmConfigure);
	m_itemhelp(mItem, "Select category of settings to change");

	mItem = m_additem(gmMain, "  ~Help  ");
	m_itemsub(mItem, gmHelp);
	m_itemhelp(mItem, "View help on the current field, OPTION, or 4DOS.INI");

	// Disable help if help exe name not set
	if (!gfHelpAvail) {
		m_itemprotect(mItem, ENABLE);
		m_itemcolor(mItem, SC_MENU_DISABLE, SC_MENU_DISABLE, SC_MENU_DISABLE);
	}


	// Exit submenu
	m_border(gmExit, uBorder, uShadow, JUST_CENTER);
	m_termkeys(gmExit, guVertMenuTermKeys);
	m_selectkeys(gmExit, guVertMenuSelectKeys);
	m_helppos(gmExit, HELP_COL, ex_crt_rows - 1);

	mItem = m_additem(gmExit, "~Save  ");
	m_itemcode(mItem, USER_EVENT + B_SAVE);
	m_itemcall(mItem, _MenuItem);
	m_itemhelp(mItem, gpszSaveLineHelp);

	mItem = m_additem(gmExit, "~Use   ");
	m_itemcode(mItem, USER_EVENT + B_USE);
	m_itemcall(mItem, _MenuItem);
	m_itemhelp(mItem, gpszUseLineHelp);

	mItem = m_additem(gmExit, "~Cancel");
	m_itemcall(mItem, _MenuItem);
	m_itemcode(mItem, USER_EVENT + B_CANCEL);
	m_itemhelp(mItem, gpszCancelLineHelp);


	// Help submenu
	m_border(gmHelp, uBorder, uShadow, JUST_CENTER);
	m_termkeys(gmHelp, guVertMenuTermKeys);
	m_selectkeys(gmHelp, guVertMenuSelectKeys);
	m_helppos(gmHelp, HELP_COL, ex_crt_rows - 1);

	mItem = m_additem(gmHelp, "~Field          <F1>");
	m_itemcode(mItem, USER_EVENT + B_HELP_FIELD);
	m_itemcall(mItem, _MenuItem);
	m_itemhelp(mItem, "View help on the current field");

	mItem = m_additem(gmHelp, "~Keys               ");
	m_itemcode(mItem, USER_EVENT + B_HELP_KEYS);
	m_itemcall(mItem, _MenuItem);
	m_itemhelp(mItem, "View help on using the keyboard in OPTION");

	mItem = m_additem(gmHelp, "~OPTION             ");
	m_itemcode(mItem, USER_EVENT + B_HELP_OPTION);
	m_itemcall(mItem, _MenuItem);
	m_itemhelp(mItem, "View help on the OPTION configuration utility");
	// Disable Option Help Temporarily
	//m_itemprotect(mItem, ENABLE);
	//m_itemcolor(mItem, SC_MENU_DISABLE, SC_MENU_DISABLE, SC_MENU_DISABLE);

	mItem = m_additem(gmHelp, "~4DOS.INI           ");
	m_itemcode(mItem, USER_EVENT + B_HELP_INI);
	m_itemcall(mItem, _MenuItem);
	m_itemhelp(mItem, "View help on the initialization file 4DOS.INI");

	mItem = m_additem(gmHelp, "Help ~Contents      ");
	m_itemcode(mItem, USER_EVENT + B_HELP_TOC);
	m_itemcall(mItem, _MenuItem);
	m_itemhelp(mItem, "View the table of contents for the 4DOS help system");


	// Category submenu
	m_border(gmConfigure, uBorder, uShadow, JUST_CENTER);
	m_termkeys(gmConfigure, guVertMenuTermKeys);
	m_selectkeys(gmConfigure, guVertMenuSelectKeys);
	m_helppos(gmConfigure, HELP_COL, ex_crt_rows - 1);

	szCatName[MAX_CAT_LEN + 1] = '\0';
	for (i = 0; i < CATEGORY_MAX; i++) {
		// Get TAB control for this category
		pmTabCntl = &gaControlList[gaCategoryList[i].uTabIndex];

		if (pmTabCntl != NULL) {
			// Create label, spaced out to max len to allow mouse click on
			//  any point on line, not just on characters
			memset(szCatName, ' ', MAX_CAT_LEN + 1);
			memcpy(szCatName, pmTabCntl->pszLabel,
			       MIN(strlen(pmTabCntl->pszLabel), MAX_CAT_LEN + 1));

			mItem = m_additem(gmConfigure, szCatName);
			m_itemcode(mItem, USER_EVENT + i);
			m_itemcall(mItem, _MenuItem);
			m_itemhelp(mItem, pmTabCntl->pszHelp);
		}  // End if pmTabCntl != NULL
	}  // End for i

	return(nRC);
}  // End _AddMenu


/*
 *************************************************************************
 ** ProcessMainMenu
 *************************************************************************
 ** Process menu and handle selection
 *************************************************************************
 ** Requires:  uCurrentCategory = the current category (what else?)
 ** Returns :  new category
 *************************************************************************
 */
static unsigned int _ProcessMainMenu(unsigned int uCurrentCategory) {
	unsigned int uMenuRC;
	unsigned int uReturnCategory = uCurrentCategory;
	int fLoop = TRUE;


	guLastKey = 0;
	w_front(gmMenuWin);

	while (fLoop) {
		uMenuRC = m_process(gmMain);
		fLoop = FALSE;

		//if ((guLastKey == 0) && (ISMOUSE(uMenuRC))) {
		if (ISMOUSE(uMenuRC) && !ISUSER(guLastKey)) {
			int nClickRow;
			int nClickCol;

			if (s_getwindow(&nClickCol, &nClickRow) == gmMenuWin)
				fLoop = TRUE;
		}
	}  // End while

	if (ISUSER(guLastKey))
		uReturnCategory = guLastKey - USER_EVENT;

	switch (uReturnCategory) {
	case B_HELP_FIELD:
		// Show help on current control
		_F1Help();

		// Cancel exit
		uReturnCategory = uCurrentCategory;
		break;
	case B_HELP_KEYS:
		// Call to OPTION keyboard help
      _Call4Help("OPTKEYS");

		// Cancel exit
		uReturnCategory = uCurrentCategory;
		break;
	case B_HELP_OPTION:
		// Call to OPTION help
      _Call4Help("OPTHELP");

		// Cancel exit
		uReturnCategory = uCurrentCategory;
		break;
	case B_HELP_INI:
		// Call to INI file help
      _Call4Help("4DOS.INI");

		// Cancel exit
		uReturnCategory = uCurrentCategory;
		break;
	case B_HELP_TOC:
		// Call to help Table of Contents
      _Call4Help("");

		// Cancel exit
		uReturnCategory = uCurrentCategory;
		break;
	}  // End switch

	return(uReturnCategory);
}  // _ProcessMainMenu


/*
 *************************************************************************
 ** MenuItem
 *************************************************************************
 ** User function called when a menu item is selected
 *************************************************************************
 ** Requires:  none
 ** Returns :  none
 *************************************************************************
 */
static void _MenuItem(void) {
	// Record the last event code or last key processed by menu
	guLastKey = m_lastkey(m_lastmenu());
}  // End _MenuItem


/*
 *************************************************************************
 ** PopUpExitBox
 *************************************************************************
 ** Display popup box showing exit options
 *************************************************************************
 ** Requires:  none
 ** Returns :  B_SAVE, B_USE, B_CANCEL, or B_RESUME
 *************************************************************************
 */
static int _PopUpExitBox(void) {
	Window mPopUpWin;
	Control mFirstButton;
	Control mHoldCtrl;
	unsigned int uStyle = BUTTON_BORDER;
	unsigned int uDlgRC = 0;
	int nWinHeight = 5;
	int nWinWidth = 43;
	int nHoldColor = 0;
	int nHoldBrdColor = 0;
	char *pszResumeLineHelp = "Return to OPTION entry screen";

	// Keys that will terminate the dialog
	unsigned int aTermKeys[] = { USER_EVENT + B_SAVE, USER_EVENT + B_USE,
	                             USER_EVENT + B_CANCEL, USER_EVENT + B_RESUME,
	                             _ESC, CTRL_C, ALT_F4, 0 };


	// Use different key filter
	k_filter(_PopUpKeyFilter);

	// Change window colors
   nHoldColor = d_change(WCOLOR, SC_POPUP);
   nHoldBrdColor = d_change(WBRDCOLOR, SC_POPUP);

	// Open popup, centered in window
	mPopUpWin = w_open((ex_crt_columns - nWinWidth) / 2,
	                   (ex_crt_rows - nWinHeight) / 2, nWinWidth, nWinHeight);

	// Create buttons
	mHoldCtrl = c_add_button(mPopUpWin, " ~Save ", 2, 2,
	                         (int)USER_EVENT + B_SAVE, uStyle);
	c_user_pointer(mHoldCtrl, gpszSaveLineHelp);
	mFirstButton = mHoldCtrl;
	mHoldCtrl = c_add_button(mPopUpWin, " ~Use  ", 13, 2,
	                         (int)USER_EVENT + B_USE, uStyle);
	c_user_pointer(mHoldCtrl, gpszUseLineHelp);
	mHoldCtrl = c_add_button(mPopUpWin, "~Cancel", 24, 2,
	                         (int)USER_EVENT + B_CANCEL, uStyle);
	c_user_pointer(mHoldCtrl, gpszCancelLineHelp);
	mHoldCtrl = c_add_button(mPopUpWin, "~Resume", 35, 2,
	                         (int)USER_EVENT + B_RESUME, uStyle);
	c_user_pointer(mHoldCtrl, pszResumeLineHelp);

	// Dialog box handler
	uDlgRC = c_dialog2(mPopUpWin, mFirstButton, aTermKeys, _PopUpFocusFilter,
	                   C_DLG_CLICKEXIT);

	// Close popup dialog and restore colors
	w_close(mPopUpWin);
   d_change(WCOLOR, nHoldColor);
   d_change(WBRDCOLOR, nHoldBrdColor);

	// Restore key filtering to previous settings
	k_filter(_KeyFilter);

	// Default to RESUME
	if ((uDlgRC != USER_EVENT + B_SAVE) &&
	    (uDlgRC != USER_EVENT + B_USE) &&
	    (uDlgRC != USER_EVENT + B_CANCEL) &&
	    (uDlgRC != USER_EVENT + B_RESUME)) {
		uDlgRC = USER_EVENT + B_RESUME;
	}

	return(uDlgRC - USER_EVENT);
}  // End _PopUpExitBox


/*
 *************************************************************************
 ** PopUpKeyFilter
 *************************************************************************
 ** Function called automatically every time a key is pressed in the
 **  popup dialog
 *************************************************************************
 ** Requires:  nKey = value of key just pressed
 ** Returns :  translated key value
 *************************************************************************
 */
static int _PopUpKeyFilter(int nKey) {
	int nRC = nKey;  // Default is to pass key on to normal mWindow handler


	switch (nKey) {
	// Translate non-ALT versions of the hot keys into ALT keys
	// Convert this to a function if this routine is used by more than
	//  one popup box
	case (int)'s':
	case (int)'S':
		nRC = ALT_S;
		break;
	case (int)'u':
	case (int)'U':
		nRC = ALT_U;
		break;
	case (int)'c':
	case (int)'C':
		nRC = ALT_C;
		break;
	case (int)'r':
	case (int)'R':
		nRC = ALT_R;
		break;

	// Select on ENTER
	case _ENTER:
		nRC = (int)' ';
		break;
	}  // End switch 

	return(nRC);
}  // End _PopUpKeyFilter


/*
 *************************************************************************
 ** _PopUpFocusFilter
 *************************************************************************
 ** Function called automatically every time control focus is changed, or
 **    a control is selected in the popup box
 ** Used to clear and set control specific single-line help 
 ** This handles only button controls
 *************************************************************************
 ** Requires:  mWin = current window
 **            mCtrl = control about to gain or lose focus, or be selected
 **            nEvent = CTL_FOCUS, CTL_UNFOCUS, CTL_SELECT
 ** Returns :  0 success
 *************************************************************************
 */
static int _PopUpFocusFilter(Window mWin, Control mCtrl, int nEvent) {
	char *pszHoldText;


	// Highlighted character was not changing to focus color, so we have
	//  to force the color change
	_FixCtrlHilight(mCtrl, INI_CTL_BUTTON, nEvent);

	switch (nEvent) {
	case CTL_FOCUS:  // The control is about to receive focus
		pszHoldText = (char *)c_user_pointer(mCtrl, NULL);

		// Write help line
		if (pszHoldText != NULL)
			_LineHelp(pszHoldText, NULL);
		break;

	case CTL_UNFOCUS:  // The control is about to lose focus
		// Clear help line
		_LineHelp(NULL, NULL);
		break;

	case CTL_SELECT:  // The control is about to be selected
		break;

	}  // End switch

	return(0);
}  // End _FocusFilter


/*
 *************************************************************************
 ** _SetCursorSize
 *************************************************************************
 ** Set size of hardware cursor
 *************************************************************************
 ** Requires:  nStart =
 **            nEnd =
 **            fShow = if TRUE, display cursor, else hide cursor
 ** Returns :  none
 *************************************************************************

static void _SetCursorSize(int nStart, int nEnd, int fShow)
{
   union REGS      reg;


	// Clear registers
	memset((void *)&reg, 0, sizeof(reg));

	if (!fShow) {
   	nStart = nStart | 0x0020;
   	nEnd = nEnd & 0x00ff;
	}

  	reg.h.ch=(char)nStart;
   reg.h.cl=(char)nEnd;
   reg.h.ah=0x01;
   int86(0x10, &reg, &reg);

	return;
}  // End _SetCursorSize
*/


/*
 *************************************************************************
 ** _SetCursorStatus
 *************************************************************************
 ** Set size and status of hardware cursor
 *************************************************************************
 ** Requires:  nStat = status value
 ** Returns :  none
 *************************************************************************
 */
static void _SetCursorStatus(int nStat)
{
   union REGS      reg;


	// Clear registers
	memset((void *)&reg, 0, sizeof(reg));

  	reg.w.cx=(unsigned short)nStat;
   reg.h.ah=0x01;
   int86(0x10, &reg, &reg);

	return;
}  // End _SetCursorStatus


/*
 *************************************************************************
 ** MemoryError
 *************************************************************************
 ** Handle memory allocation error
 *************************************************************************
 ** Requires:  pszMessage = name of variable to display (shown on DEBUG only)
 ** Returns :  none
 *************************************************************************
 */
static void _MemoryError(char *pszMessage) {

#ifdef DEBUG
	printf("\n\n DEBUG: Could not allocate memory for %s in dialog.c\n",
	       pszMessage);
#endif

	FatalError(1, "Could not allocate necessary memory.");
}  // End _MemoryError


#ifdef DEBUG
/*
 *************************************************************************
 ** PopUpBox
 *************************************************************************
 ** Display two lines of text in a popup box
 *************************************************************************
 ** Requires:  pszString1 = first line to display
 **            pszString2 = second line to display
 ** Returns :  none
 *************************************************************************
 */
static void _PopUpBox(char *pszString1, char *pszString2) {
	Window mPopUpWin;


	mPopUpWin = w_open(WIN_COL + 5, WIN_ROW + 5,
	                   ex_crt_columns - WIN_COL - 10, 4);
	w_putsat(mPopUpWin, 1, 0, pszString1);
	w_putsat(mPopUpWin, 1, 1, pszString2);
	w_puts_centered(mPopUpWin, 3, "< Hit any key >");
	k_getkey();

	w_close(mPopUpWin);

	return;
}  // End _PopUpBox
#endif  // DEBUG

