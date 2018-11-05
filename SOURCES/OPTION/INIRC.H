/*
 *************************************************************************
 ** INIRC.H
 *************************************************************************
 ** Structure containing screen controls
 **
 ** Copyright 1996 JP Software Inc., All rights reserved
 *************************************************************************
 */

//List of all controls:
//  First item in each category must be the tab
//  Second item in each category will have the initial focus
//  Order in this list determines <Tab> order
//
//  INI_CTL_RADIO- Radio button group #s must be unique for each group
//                 Radio buttons with the same group # must be sequential
//                 Radio buttons with the same group # must have the same
//                    IDI value
//
//  INI_CTL_COLOR- A foreground color combo box must be followed by a
//                    background color combo box
//
//  INI_CTL_NULL entry defines end of category

static INI_CONTROL gaControlList[] = {

// Begin Startup
{ INI_CTL_TAB, 0, NULL, 0, 0, "~Startup", "Startup settings  (Memory, Buffers, 4START/4EXIT, AUTOEXEC.BAT, Installation)", NULL, NULL, 0, 0 },
{ INI_CTL_BOX,  0, NULL, 1, 0, "Memory", "", NULL, NULL, 64, 4 },
{ INI_CTL_COMBO,  IDI_UMBLoad, NULL, 3, 1,  "Resident in ~UMB", "Load resident part of 4DOS in UMB", NULL, NULL, 3, 9 },
{ INI_CTL_TEXT,  IDI_Swapping, NULL, 3, 2, "S~wapping", "Swapping type(s)", NULL, "_Swapping_", 17, MAX_PATH_LEN },
{ INI_CTL_INT,  IDI_StackSize, NULL, 35, 1, "Stack Si~ze", "Internal stack size", NULL, NULL, 5, 1 },

{ INI_CTL_BOX,  0, NULL, 1, 4,  "Buffers", "", NULL, NULL, 64, 8 },
{ INI_CTL_STATIC,  0, NULL, 22, 5,  "Size      UMB ", NULL, NULL, NULL, 0, 0 },
{ INI_CTL_INT,  IDI_History, NULL, 3, 6,  "Command Histor~y  ", "Size of history list", NULL, "_History_", 5, 1 },
{ INI_CTL_COMBO,  IDI_UMBHistory, NULL, 30, 6,  "~", "Load history in UMB", NULL, NULL, 3, 9 },
{ INI_CTL_CHECK,  IDI_LocalHistory, NULL, 40, 6,  "Local", "Local vs. global history", NULL, NULL, 0, 0 },
{ INI_CTL_INT,  IDI_DirHistory, NULL, 3, 7,  "~Directory History", "Size of directory history list", NULL, "_DirHistory_", 5, 1 },
{ INI_CTL_COMBO,  IDI_UMBDirHistory, NULL, 30, 7,  "~", "Load global directory history in UMB", NULL, NULL, 3, 9 },
{ INI_CTL_CHECK,  IDI_LocalDirHistory, NULL, 40, 7,  "Local", "Local vs. global directory history", NULL, NULL, 0, 0 },
{ INI_CTL_INT,  IDI_Alias, NULL, 3, 8,  "A~lias            ", "Size of alias list", NULL, "_Alias_", 5, 1 },
{ INI_CTL_COMBO,  IDI_UMBAlias, NULL, 30, 8,  "~", "Load global aliases in UMB", NULL, NULL, 3, 9 },
{ INI_CTL_CHECK,  IDI_LocalAliases, NULL, 40, 8,  "Local", "Local vs. global aliases", NULL, NULL, 0, 0 },
{ INI_CTL_INT,  IDI_Function, NULL, 3, 9,  "~Function         ", "Size of function list", NULL, "_Function_", 5, 1 },
{ INI_CTL_COMBO,  IDI_UMBFunction, NULL, 30, 9,  "~", "Load global functions in UMB", NULL, NULL, 3, 9 },
{ INI_CTL_CHECK,  IDI_LocalFunctions, NULL, 40, 9,  "Local", "Local vs. global functions", NULL, NULL, 0, 0 },
{ INI_CTL_INT,  IDI_Environment, NULL, 3, 10,  "~Environment      ", "Size of environment", NULL, "_Environment_", 5, 1 },
{ INI_CTL_COMBO,  IDI_UMBEnvironment, NULL, 30, 10,  "~", "Load master environment in UMB", NULL, NULL, 3, 9 },
{ INI_CTL_INT,  IDI_EnvFree, NULL, 40, 10,  "F~ree Space", "Required free space in environment", NULL, NULL, 5, 1 },

{ INI_CTL_BOX,  0, NULL, 1, 12,  "4START and 4EXIT", "", NULL, NULL, 64, 3 },
{ INI_CTL_TEXT,  IDI_4StartPath, NULL, 3, 13,  "~Path ", "Path for 4START and 4EXIT", NULL, NULL, 53, MAX_PATH_LEN },

{ INI_CTL_BOX,  0, NULL, 1, 15,  "AUTOEXEC.BAT", "", NULL, NULL, 64, 4 },
{ INI_CTL_TEXT,  IDI_AutoExecPath, NULL, 3, 16, "P~ath ", "Path and optional file name for AUTOEXEC.BAT", NULL, NULL, 53, MAX_PATH_LEN },
{ INI_CTL_TEXT,  IDI_AutoExecParms, NULL, 3, 17, "Par~ms", "Parameters for AUTOEXEC.BAT", NULL, NULL, 53, 100 },

{ INI_CTL_NULL, 0, NULL, 0, 0, NULL, NULL, NULL, 0, 0 },


// Begin Display
{ INI_CTL_TAB, 0, NULL, 0, 0, "~Display", "Display settings  (ANSI, Screen Dimensions and Colors, List Box Colors)", NULL, NULL, 0, 0 },
{ INI_CTL_BOX,  0, NULL, 1, 0, "Settings", "", NULL, NULL, 31, 6 },
{ INI_CTL_STATIC,  0, NULL, 3, 1, "ANSI:", "", NULL, NULL, 0, 0 },
{ INI_CTL_RADIO,  IDI_ANSI, NULL, 10, 1,  "~Auto", "Allow 4DOS to determine ANSI driver state", NULL, "_ANSI_", 1, 0 },
{ INI_CTL_RADIO,  IDI_ANSI, NULL, 10, 2,  "~Yes", "ANSI driver loaded", NULL, "_ANSI_", 1, 0 },
{ INI_CTL_RADIO,  IDI_ANSI, NULL, 10, 3,  "~No", "ANSI driver not loaded", NULL, "_ANSI_", 1, 0 },
{ INI_CTL_CHECK,  IDI_BrightBG, NULL, 3, 4,  "~Bright Background", "Bright background colors", NULL, NULL, 0, 0 },

{ INI_CTL_BOX,  0, NULL, 34, 0,  "Text Dimensions", "", NULL, NULL, 31, 5 },
{ INI_CTL_INT,  IDI_ScreenColumns, NULL, 36, 1,  "~Width ", "Screen width", NULL, NULL, 4, 1 },
{ INI_CTL_INT,  IDI_ScreenRows, NULL, 36, 2,  "H~eight", "Screen height", NULL, NULL, 4, 1 },
{ INI_CTL_INT,  IDI_Tabs, NULL, 36, 3,  "~Tabs  ", "Tab stops", NULL, NULL, 4, 1 },

{ INI_CTL_BOX,  0, NULL, 1, 6,  "Screen Colors", "", NULL, NULL, 64, 5 },
{ INI_CTL_STATIC,  0, NULL, 11, 7,  "Foreground", "", NULL, NULL, 0, 0 },
{ INI_CTL_STATIC,  0, NULL, 30, 7,  "Background", "", NULL, NULL, 0, 0 },
{ INI_CTL_COLOR,  IDI_StdColorsFG, NULL, 3, 8,  "~Output", "Standard display foreground color", NULL, NULL, 14, 11 },
{ INI_CTL_COLOR,  IDI_StdColorsBG, NULL, 28, 8,  "~", "Standard display background color", NULL, NULL, 14, 11 },
{ INI_CTL_COLOR,  IDI_InputColorsFG, NULL, 3, 9,  "~Input ", "Input foreground color", NULL, NULL, 14, 11 },
{ INI_CTL_COLOR,  IDI_InputColorsBG, NULL, 28, 9,  "~", "Input background color", NULL, NULL, 14, 11 },

{ INI_CTL_NULL, 0, NULL, 0, 0, NULL, NULL, NULL, 0, 0 },

// Begin Command Line
{ INI_CTL_TAB, 0, NULL, 0, 0, "Command ~Line", "Command Line settings  (Editing, Filename Completion, Command History)", NULL, NULL, 0, 0 },
{ INI_CTL_BOX,  0, NULL, 1, 0,  "Editing", "", NULL, NULL, 64, 6 },
{ INI_CTL_STATIC,  0, NULL, 3, 1,  "Default Mode:", "", NULL, NULL, 0, 0 },
{ INI_CTL_RADIO,  IDI_EditModeOver, NULL, 17, 1,  "~Overstrike", "Command line editor starts in overstrike mode", NULL, NULL, 2, 0 },
{ INI_CTL_RADIO,  IDI_EditModeIns, NULL, 17, 2,  "~Insert", "Command line editor starts in insert mode", NULL, NULL, 2, 0 },
{ INI_CTL_RADIO,  IDI_EditModeInitOver, NULL, 17, 3,  "Init Overstri~ke", "Command line editor starts in insert mode", NULL, NULL, 2, 0 },
{ INI_CTL_RADIO,  IDI_EditModeInitIns, NULL, 17, 4,  "Init Inser~t", "Command line editor starts in insert mode", NULL, NULL, 2, 0 },
{ INI_CTL_STATIC,  0, NULL, 36, 1,  "Cursor:", "", NULL, NULL, 0, 0 },
{ INI_CTL_INT,  IDI_CursorOver, NULL, 44, 1,  "O~verstrike", "Cursor size as % of total character cell size", NULL, NULL, 3, 1 },
{ INI_CTL_STATIC,  0, NULL, 62, 1,  "%", "", NULL, NULL, 0, 0 },
{ INI_CTL_INT,  IDI_CursorIns, NULL, 44, 2,  "I~nsert    ", "Cursor size as % of total character cell size", NULL, NULL, 3, 1 },
{ INI_CTL_STATIC,  0, NULL, 62, 2,  "%", "", NULL, NULL, 0, 0 },

{ INI_CTL_BOX,  0, NULL, 1, 6,  "Filename Completion", "", NULL, NULL, 64, 4 },
{ INI_CTL_CHECK,  IDI_AppendToDir, NULL, 3, 7, "~Add '\\' to Directories", "Add '\\' on directory names in filename completion", NULL, NULL, 0, 0 },
{ INI_CTL_CHECK,  IDI_CompleteHidden, NULL, 40, 7, "Hi~dden Files", "Search for hidden & system files", NULL, NULL, 0, 0 },
{ INI_CTL_TEXT,  IDI_FileCompletion, NULL, 3, 8, "O~ptions", "Customization string for filename completion", NULL, "_FileCompletion_", 51, MAX_PATH_LEN },

{ INI_CTL_BOX,  0, NULL, 1, 10,  "Command History", "", NULL, NULL, 64, 5 },
{ INI_CTL_INT,  IDI_HistMin, NULL, 3, 11,  "Minimum ~Saved", "Minimum command length to save", NULL, NULL, 3, 1 },
{ INI_CTL_STATIC,  0, NULL, 24, 11,  "characters", "", NULL, NULL, 0, 0 },
{ INI_CTL_CHECK,  IDI_HistCopy, NULL, 3, 12,  "Copy to ~End", "Copy selected command to end of history list", NULL, NULL, 0, 0 },
{ INI_CTL_CHECK,  IDI_HistMove, NULL, 26, 12,  "~Move to End", "Move selected command to end of history list", NULL, NULL, 0, 0 },
{ INI_CTL_CHECK,  IDI_HistWrap, NULL, 49, 12,  "~Wrap", "Wrap past end when selecting from history list", NULL, NULL, 0, 0 },
{ INI_CTL_STATIC,  0, NULL, 3, 13,  "Duplicates:", "", NULL, NULL, 0, 0 },
{ INI_CTL_RADIO,  IDI_HistDups, NULL, 18, 13,  "O~ff", "Ignore duplicate history entries", NULL, NULL, 8, 0 },
{ INI_CTL_RADIO,  IDI_HistDups, NULL, 28, 13,  "F~irst", "Save first matching history entry", NULL, NULL, 8, 0 },
{ INI_CTL_RADIO,  IDI_HistDups, NULL, 40, 13,  "~Last", "Save last matching history entry", NULL, NULL, 8, 0 },

{ INI_CTL_BOX,  0, NULL, 1, 15,  "Extended Directory Search", "", NULL, NULL, 64, 4 },
{ INI_CTL_STATIC,  0, NULL, 3, 16,  "Search Level:", "", NULL, NULL, 0, 0 },
{ INI_CTL_RADIO,  IDI_FuzzyCD, NULL, 18, 16,  "~0", "Extended Directory Search level 0", NULL, NULL, 6, 0 },
{ INI_CTL_RADIO,  IDI_FuzzyCD, NULL, 26, 16,  "~1", "Extended Directory Search level 1", NULL, NULL, 6, 0 },
{ INI_CTL_RADIO,  IDI_FuzzyCD, NULL, 34, 16,  "~2", "Extended Directory Search level 2", NULL, NULL, 6, 0 },
{ INI_CTL_RADIO,  IDI_FuzzyCD, NULL, 42, 16,  "~3", "Extended Directory Search level 3", NULL, NULL, 6, 0 },
{ INI_CTL_TEXT,  IDI_TreePath, NULL, 3, 17,  "T~ree Path", "Location of JPSTREE.IDX (Extended Dir Search index)", NULL, NULL, 49, MAX_PATH_LEN },

{ INI_CTL_NULL, 0, NULL, 0, 0, NULL, NULL, NULL, 0, 0 },


// Begin Windows
{ INI_CTL_TAB, 0, NULL, 0, 0, "~Windows", "Window settings  (Popup Windows)", NULL, NULL, 0, 0 },
{ INI_CTL_BOX,  0, NULL, 1, 0,  "Popup Windows", "", NULL, NULL, 64, 9 },
{ INI_CTL_STATIC,  0, NULL, 26, 1,  "Foreground", "", NULL, NULL, 0, 0 },
{ INI_CTL_STATIC,  0, NULL, 44, 1,  "Background", "", NULL, NULL, 0, 0 },
{ INI_CTL_COLOR,  IDI_PopupWinColorsFG, NULL, 3, 2,  "~Popup Colors         ", "Popup window foreground color", NULL, NULL, 13, 11 },
{ INI_CTL_COLOR,  IDI_PopupWinColorsBG, NULL, 42, 2,  "~", "Popup window background color", NULL, NULL, 13, 11 },
{ INI_CTL_COLOR,  IDI_CDDWinColorsFG, NULL, 3, 3,  "~Ext Dir Search Colors", "Extended Directory Search window foreground color", NULL, NULL, 13, 11 },
{ INI_CTL_COLOR,  IDI_CDDWinColorsBG, NULL, 42, 3,  "~", "Extended Directory Search window background color", NULL, NULL, 13, 11 },
{ INI_CTL_STATIC,  0, NULL, 19, 5,  "Left", "", NULL, NULL, 0, 0 },
{ INI_CTL_STATIC,  0, NULL, 29, 5,  "Top", "", NULL, NULL, 0, 0 },
{ INI_CTL_STATIC,  0, NULL, 39, 5,  "Width", "", NULL, NULL, 0, 0 },
{ INI_CTL_STATIC,  0, NULL, 49, 5,  "Height", "", NULL, NULL, 0, 0 },
{ INI_CTL_INT,  IDI_PopupWinLeft, NULL, 3, 6,  "Pop~up         ", "Popup window left side position", NULL, NULL, 4, 1 },
{ INI_CTL_INT,  IDI_PopupWinTop, NULL, 27, 6,  "", "Popup window top position", NULL, "PopupWinLeft", 4, 1 },
{ INI_CTL_INT,  IDI_PopupWinWidth, NULL, 37, 6,  "", "Popup window width", NULL, "PopupWinLeft", 4, 1 },
{ INI_CTL_INT,  IDI_PopupWinHeight, NULL, 47, 6,  "", "Popup window height", NULL, "PopupWinLeft", 4, 1 },
{ INI_CTL_INT,  IDI_CDDWinLeft, NULL, 3, 7,  "Ext ~Dir Search", "Extended Directory Search window left side position", NULL, NULL, 4, 1 },
{ INI_CTL_INT,  IDI_CDDWinTop, NULL, 27, 7,  "", "Extended Directory Search window top position", NULL, "CDDWinLeft", 4, 1 },
{ INI_CTL_INT,  IDI_CDDWinWidth, NULL, 37, 7,  "", "Extended Directory Search window width", NULL, "CDDWinLeft", 4, 1 },
{ INI_CTL_INT,  IDI_CDDWinHeight, NULL, 47, 7,  "", "Extended Directory Search window height", NULL, "CDDWinLeft", 4, 1 },

{ INI_CTL_BOX,  0, NULL, 1, 9,  "Mouse", "", NULL, NULL, 64, 3 },
{ INI_CTL_STATIC,  0, NULL, 3, 10,  "Mouse:", "", NULL, NULL, 0, 0 },
{ INI_CTL_RADIO,  IDI_Mouse, NULL, 10, 10,  "~Auto", "Use mouse if one is found", NULL, NULL, 4, 0 },
{ INI_CTL_RADIO,  IDI_Mouse, NULL, 21, 10,  "~Yes", "Always enable mouse", NULL, NULL, 4, 0 },
{ INI_CTL_RADIO,  IDI_Mouse, NULL, 33, 10,  "~No", "Disable mouse", NULL, NULL, 4, 0 },

{ INI_CTL_NULL, 0, NULL, 0, 0, NULL, NULL, NULL, 0, 0 },


// Begin Options 1
{ INI_CTL_TAB, 0, NULL, 0, 0, "Options ~1", "Options 1 settings  (Descriptions, Special Characters, Beep, Time ...)", NULL, NULL, 0, 0 },
{ INI_CTL_BOX,  0, NULL, 1, 0,  "Descriptions", "", NULL, NULL, 64, 3 },
{ INI_CTL_CHECK,  IDI_Descriptions, NULL, 3, 1,  "E~nable", "Enable description processing", NULL, NULL, 0, 0 },
{ INI_CTL_INT,  IDI_DescriptionMax, NULL, 20, 1,  "M~aximum Length", "Maximum length of file descriptions", NULL, NULL, 3, 1 },

{ INI_CTL_BOX,  0, NULL, 1, 3,  "Special Characters", "", NULL, NULL, 64, 5 },
{ INI_CTL_TEXT,  IDI_CommandSep, NULL, 3, 4,  "~Separator", "Multiple command separator character", NULL, NULL, 1, 1 },
{ INI_CTL_TEXT,  IDI_EscapeChar, NULL, 21, 4,  "~Escape", "4DOS escape character", NULL, NULL, 1, 1 },
{ INI_CTL_TEXT,  IDI_ParameterChar, NULL, 36, 4,  "Pa~rameter", "Alias / batch file parameter character", NULL, NULL, 1, 1 },
{ INI_CTL_STATIC,  0, NULL, 3, 5,  "Decimal:", "", NULL, NULL, 0, 0 },
{ INI_CTL_RADIO,  IDI_DecimalChar, NULL, 14, 5,  "A~uto", "Use decimal character for current country code", NULL, NULL, 3, 0 },
{ INI_CTL_RADIO,  IDI_DecimalChar, NULL, 25, 5,  "Per~iod", "Use period for decimal character", NULL, NULL, 3, 0 },
{ INI_CTL_RADIO,  IDI_DecimalChar, NULL, 37, 5,  "C~omma", "Use comma for decimal character", NULL, NULL, 3, 0 },
{ INI_CTL_STATIC,  0, NULL, 3, 6,  "Thousands:", "", NULL, NULL, 0, 0 },
{ INI_CTL_RADIO,  IDI_ThousandsChar, NULL, 14, 6,  "Au~to", "Use thousands character for current country code", NULL, NULL, 4, 0 },
{ INI_CTL_RADIO,  IDI_ThousandsChar, NULL, 25, 6,  "Perio~d", "Use period for thousands character", NULL, NULL, 4, 0 },
{ INI_CTL_RADIO,  IDI_ThousandsChar, NULL, 37, 6,  "Co~mma", "Use comma for thousands character", NULL, NULL, 4, 0 },

{ INI_CTL_BOX,  0, NULL, 1, 8,  "Default Beep", "", NULL, NULL, 64, 3 },
{ INI_CTL_INT,  IDI_BeepLength, NULL, 3, 9,  "Len~gth", "Default beep length", NULL, NULL, 2, 1 },
{ INI_CTL_STATIC,  0, NULL, 16, 9,  "Ticks", "", NULL, NULL, 0, 0 },
{ INI_CTL_INT,  IDI_BeepFreq, NULL, 27, 9,  "Fre~quency", "Default beep frequency", NULL, NULL, 5, 1 },
{ INI_CTL_STATIC,  0, NULL, 46, 9,  "Hz", "", NULL, NULL, 0, 0 },

{ INI_CTL_BOX,  0, NULL, 1, 11,  "Time", "", NULL, NULL, 64, 3 },
{ INI_CTL_RADIO,  IDI_AmPm, NULL, 3, 12,  "Countr~y", "Display time in country format", NULL, NULL, 5, 0 },
{ INI_CTL_RADIO,  IDI_AmPm, NULL, 17, 12,  "~12-hour", "Display time in 12-hour format with a trailing 'a' or 'p'", NULL, NULL, 5, 0 },
{ INI_CTL_RADIO,  IDI_AmPm, NULL, 29, 12,  "~24-hour", "Display time in 24-hour format", NULL, NULL, 5, 0 },

{ INI_CTL_BOX,  0, NULL, 1, 14,  "Options", "", NULL, NULL, 64, 7 },
{ INI_CTL_CHECK,  IDI_UpperCase, NULL, 3, 15,  "~Force Upper Case", "Force file names to upper case", NULL, NULL, 0, 0 },
{ INI_CTL_CHECK,  IDI_BatchEcho, NULL, 3, 16,  "Default ~Batch Echo", "Echo all batch file commands", NULL, NULL, 0, 0 },
{ INI_CTL_CHECK,  IDI_NoClobber, NULL, 3, 17,  "Protect Redirected Output Fi~les", "Keep redirection from overwriting existing files", NULL, NULL, 0, 0 },
{ INI_CTL_CHECK,  IDI_Win95SFNSearch, NULL, 3, 18,  "Search on Short File Names (~Win 95/98/ME)", "Search for SFNs if no matching LFN is found", NULL, NULL, 0, 0 },
{ INI_CTL_CHECK,  IDI_UnixPaths, NULL, 3, 19,  "Support Unix ~Path Separators", "Supports either \\ or / as path separators", NULL, NULL, 0, 0 },

{ INI_CTL_NULL, 0, NULL, 0, 0, NULL, NULL, NULL, 0, 0 },

// Begin Options 2
{ INI_CTL_TAB, 0, NULL, 0, 0, "Options ~2", "Options 2 settings  (Logging, EVAL, Help, REXX)", NULL, NULL, 0, 0 },
{ INI_CTL_BOX,  0, NULL, 1, 0,  "Logging", "", NULL, NULL, 64, 5 },
{ INI_CTL_CHECK,  IDI_LogOn, NULL, 3, 1,  "Comman~d", "Enable command logging", NULL, "Log", 0, 0 },
{ INI_CTL_TEXT,  IDI_LogName, NULL, 19, 1,  "~File", "Log file name", NULL, NULL, 38, MAX_PATH_LEN },
{ INI_CTL_CHECK,  IDI_LogErrors, NULL, 3, 2,  "~Errors", "Enable error logging", NULL, "Log", 0, 0 },
{ INI_CTL_CHECK,  IDI_HistLogOn, NULL, 3, 3,  "H~istory", "Enable history logging", NULL, "Log", 0, 0 },
{ INI_CTL_TEXT,  IDI_HistLogName, NULL, 19, 3,  "Fi~le", "History log file name", NULL, NULL, 38, MAX_PATH_LEN },

{ INI_CTL_BOX,  0, NULL, 1, 5,  "EVAL", "", NULL, NULL, 64, 3 },
{ INI_CTL_STATIC,  0, NULL, 3, 6,  "Precision:", "", NULL, NULL, 0, 0 },
{ INI_CTL_INT,  IDI_EvalMin, NULL, 15, 6,  "Mi~nimum", "Minimum digits after decimal point in @EVAL", NULL, "_EvalMin_", 2, 1 },
{ INI_CTL_STATIC,  0, NULL, 28, 6,  "digits", "", NULL, NULL, 0, 0 },
{ INI_CTL_INT,  IDI_EvalMax, NULL, 40, 6,  "~Maximum", "Maximum digits after decimal point in @EVAL", NULL, "_EvalMax_", 2, 1 },
{ INI_CTL_STATIC,  0, NULL, 54, 6,  "digits", "", NULL, NULL, 0, 0 },

{ INI_CTL_BOX,  0, NULL, 1, 8,  "Help", "", NULL, NULL, 64, 3 },
{ INI_CTL_TEXT,  IDI_HelpOptions, NULL, 3, 9,  "~Options", "Options for help system", NULL, NULL, 51, 100 },

{ INI_CTL_BOX,  0, NULL, 1, 11,  "REXX", "", NULL, NULL, 64, 3 },
{ INI_CTL_TEXT,  IDI_REXXPath, NULL, 3, 12,  "Pa~th", "Path to REXX interpreter in PC-DOS 7 and above", NULL, NULL, 54, MAX_PATH_LEN },

{ INI_CTL_BOX,  0, NULL, 1, 14,  "Installation", "", NULL, NULL, 64, 3 },
{ INI_CTL_TEXT,  IDI_InstallPath, NULL, 3, 15, "~Path ", "Path where 4DOS files are stored", NULL, NULL, 53, MAX_PATH_LEN },

{ INI_CTL_NULL, 0, NULL, 0, 0, NULL, NULL, NULL, 0, 0 },


// Begin Commands
{ INI_CTL_TAB, 0, NULL, 0, 0, "~Commands", "Command settings  (COPY,DIR, LIST, SELECT)", NULL, NULL, 0, 0 },

{ INI_CTL_BOX,  0, NULL, 1, 0,  "COPY", "", NULL, NULL, 64, 3 },
{ INI_CTL_CHECK,  IDI_CopyPrompt, NULL, 3, 1,  "Copy~Prompt", "Prompt before overwriting files", NULL, NULL, 0, 0 },
{ INI_CTL_BOX,  0, NULL, 1, 3,  "DIR", "", NULL, NULL, 64, 3 },
{ INI_CTL_TEXT,  IDI_ColorDir, NULL, 3, 4,  "C~olors", "Directory colors", NULL, NULL, 52, 511 },

{ INI_CTL_BOX,  0, NULL, 1, 6,  "LIST", "", NULL, NULL, 64, 7 },
{ INI_CTL_STATIC,  0, NULL, 15, 7,  "Foreground", "", NULL, NULL, 0, 0 },
{ INI_CTL_STATIC,  0, NULL, 34, 7,  "Background", "", NULL, NULL, 0, 0 },
{ INI_CTL_COLOR,  IDI_ListColorsFG, NULL, 3, 8,  "~Text      ", "LIST display foreground color", NULL, NULL, 14, 11 },
{ INI_CTL_COLOR,  IDI_ListColorsBG, NULL, 32, 8,  "~", "LIST display background color", NULL, NULL, 14, 11 },
{ INI_CTL_COLOR,  IDI_ListStatBarColorsFG, NULL, 3, 9,  "Status ~Bar", "LIST status bar foreground color", NULL, NULL, 14, 11 },
{ INI_CTL_COLOR,  IDI_ListStatBarColorsBG, NULL, 32, 9,  "~", "LIST status bar background color", NULL, NULL, 14, 11 },

{ INI_CTL_TEXT,  IDI_Printer, NULL, 3, 11,  "Print ~Device", "LIST print device", NULL, NULL, 32, 32 },

{ INI_CTL_BOX,  0, NULL, 1, 13,  "SELECT", "", NULL, NULL, 64, 5 },
{ INI_CTL_STATIC,  0, NULL, 15, 14,  "Foreground", "", NULL, NULL, 0, 0 },
{ INI_CTL_STATIC,  0, NULL, 34, 14,  "Background", "", NULL, NULL, 0, 0 },
{ INI_CTL_COLOR,  IDI_SelectColorsFG, NULL, 3, 15,  "T~ext      ", "SELECT display foreground color", NULL, NULL, 14, 7 },
{ INI_CTL_COLOR,  IDI_SelectColorsBG, NULL, 32, 15,  "~", "SELECT display background color", NULL, NULL, 14, 7 },
{ INI_CTL_COLOR,  IDI_SelectStatBarColorsFG, NULL, 3, 16,  "St~atus Bar", "SELECT status bar foreground color", NULL, NULL, 14, 6 },
{ INI_CTL_COLOR,  IDI_SelectStatBarColorsBG, NULL, 32, 16,  "~", "SELECT status bar background color", NULL, NULL, 14, 6 },

{ INI_CTL_NULL, 0, NULL, 0, 0, NULL, NULL, NULL, 0, 0 }

};	 // End control_list

