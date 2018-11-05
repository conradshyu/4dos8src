/*
 *************************************************************************
 ** SCRDEF.H
 *************************************************************************
 ** Default color definitions
 **
 ** Copyright (c) 1997  JP Software, All rights reserved
 *************************************************************************
 */
#ifndef SCRDEF_H
#define SCRDEF_H


#define SC_FC						CYAN+BRIGHT
#define SC_HI						YELLOW+BRIGHT
#define SC_BACK					BLACK
#define SC_GRAY 					BLACK+BRIGHT
#define SC_TEXT 					WHITE
#define SC_TEXT_HILIGHT			WHITE+BRIGHT


#define SC_SHADOW 				COLOR(BLACK+BRIGHT,SC_BACK)

// Windows
#define SC_WIN						COLOR(SC_TEXT,SC_BACK)
#define SC_WIN_BOX				COLOR(SC_TEXT_HILIGHT,SC_BACK)
#define SC_WIN_HELP 				COLOR(BLACK,CYAN)
#define SC_WIN_LINE 				COLOR(SC_TEXT_HILIGHT,CYAN)

#define SC_BORDER					COLOR(SC_TEXT_HILIGHT,SC_BACK)
#define SC_BORDER_HELP			COLOR(BLACK,CYAN)

// Menus
#define SC_MENU_BACK				BLUE
#define SC_MENU					COLOR(SC_TEXT_HILIGHT,SC_MENU_BACK)
#define SC_MENU_HILITE			COLOR(SC_HI,SC_MENU_BACK)
#define SC_MENU_TITLE			COLOR(SC_TEXT_HILIGHT,SC_MENU_BACK)
#define SC_MENU_DISABLE			COLOR(SC_TEXT,SC_MENU_BACK)
#define SC_MENU_SELECT			COLOR(SC_FC,BLACK)  // Selection bar

// Controls
#define SC_DATA					COLOR(BLACK,WHITE)
#define SC_DATA_FOCUS			COLOR(SC_FC,BLACK+BRIGHT)

#define SC_FOCUS 					COLOR(SC_FC,SC_BACK)
#define SC_NO_FOCUS	 			COLOR(SC_TEXT,SC_BACK)
#define SC_HILITE					COLOR(SC_HI,SC_BACK)

#define SC_FOCUS_BUTTON			COLOR(SC_FC,SC_BACK)
#define SC_NO_FOCUS_BUTTON		COLOR(SC_TEXT,SC_BACK)
#define SC_HILITE_BUTTON		COLOR(SC_HI,SC_BACK)
#define SC_SHADOW_BUTTON		COLOR(BLACK,SC_BACK)

#define SC_NO_FOCUS_TAB			COLOR(SC_TEXT,SC_BACK)
#define SC_COMBO_SELECT			COLOR(SC_FC,BLACK+BRIGHT)  // Selection bar

// PopUp
#define SC_POPUP					COLOR(SC_TEXT_HILIGHT,SC_MENU_BACK)


/*
 *************************************************************************
 ** Prototypes
 *************************************************************************
 */
void SetScreenDefaults(int nPalette);

#endif  // SCRDEF_H
