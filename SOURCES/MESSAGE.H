

//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  (1) The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  (2) The Software, or any portion of it, may not be compiled for use on any
//  operating system OTHER than FreeDOS without written permission from Rex Conn
//  <rconn@jpsoft.com>
//
//  (3) The Software, or any portion of it, may not be used in any commercial
//  product without written permission from Rex Conn <rconn@jpsoft.com>
//
//  (4) THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.


// MESSAGE.H - Various text messages for 4dos
//   (put here to simplify language conversions)
//   Copyright (c) 1988 - 2003  Rex C. Conn   All rights reserved

#define SINGLE_QUOTE _TEXT('`')		// quote characters
#define DOUBLE_QUOTE _TEXT('"')

#define YES_CHAR _TEXT('Y')
#define NO_CHAR _TEXT('N')
#define ALL_CHAR _TEXT('A')
#define REST_CHAR _TEXT('R')

#define INI_QUIT_CHAR _TEXT('Q')
#define INI_EDIT_CHAR _TEXT('E')

#define LIST_CONTINUE_CHAR _TEXT('C')
#define LIST_PREVIOUS_CHAR _TEXT('B')
#define LIST_EXIT_CHAR _TEXT('Q')
#define LIST_FIND_CHAR _TEXT('F')
#define LIST_FIND_CHAR_REVERSE 6
#define LIST_GOTO_CHAR _TEXT('G')
#define LIST_HIBIT_CHAR _TEXT('H')
#define LIST_EDITOR_CHAR _TEXT('E')
#define LIST_INFO_CHAR _TEXT('I')
#define LIST_LINENUMB_CHAR _TEXT('L')
#define LIST_FIND_NEXT_CHAR _TEXT('N')
#define LIST_FIND_NEXT_CHAR_REVERSE 14
#define LIST_OPEN_CHAR _TEXT('O')
#define LIST_PRINT_CHAR _TEXT('P')
#define LIST_WRAP_CHAR _TEXT('W')
#define LIST_HEX_CHAR _TEXT('X')
#define LIST_PRINT_FILE_CHAR _TEXT('F')
#define LIST_PRINT_PAGE_CHAR _TEXT('P')

#define SELECT_LIST 12


// "Secure" message numbers (add 157 to get message number in .TXT file)
#define SEC_COPYRIGHT 5
#ifdef GERMAN
#define SEC_COPYRIGHT2 6
#endif
#define SEC_BRAND_BAD 9
#define SEC_SHAREWARE_HACK 10


#define SEC_SW_TRREST 11
#define SEC_SW_TRREST_DOS 12
#define SEC_SW_TRNORM 13
#define SEC_SW_TRNORM1 14
#define SEC_SW_ORDER 15
#define SEC_SW_TREXP 16
#define SEC_SW_EXTEND 17
#define SEC_SW_EXTEND2 18
#define SEC_SW_PREXP 19
#define SEC_SW_PRNORM 20
#define SEC_SW_PRREST 21
#define SEC_SW_PBEXP 22
#define SEC_SW_PBNORM 23
#define SEC_SW_PBREST 24
#define SEC_SW_EXIT 25
#define SEC_SW_EXT_ERR 26

#define SEC_SW_ENC_KEY 28
#define SEC_SW_CODE_CHARS 29

#define SEC_NO_HELP_FILE 31
#define SEC_WRONG_HELP_FILE 32

#define SEC_SW_DUP_NAME 34

#define SEC_SW_INI_SEC 36
#define SEC_SW_INI_ITEM 37
#define SEC_SW_INI_FLAG 38

#ifdef DEFINE_GLOBALS

// Global string variables
TCHAR ON[] = _TEXT("ON");
TCHAR OFF[] = _TEXT("OFF");

TCHAR WILD_FILE[] = _TEXT("*.*");

TCHAR WILD_EXT[] = _TEXT(".*");
TCHAR WILD_CHARS[] = _TEXT("[?*");
TCHAR QUOTES[] = _TEXT("`\"");
TCHAR QUOTES_PARENS[] = _TEXT("`\"(");
TCHAR BACK_QUOTE[] = _TEXT("`");
TCHAR NULLSTR[] = _TEXT("");
TCHAR SLASHES[] = _TEXT("\\/");
TCHAR WHITESPACE[] = _TEXT(" \t");

char SCAN_NOCR[] = _TEXT("%[^\r]");

TCHAR FMT_STR[] = _TEXT("%s");
TCHAR FMT_PREC_STR[] = _TEXT("%.*s");
TCHAR FMT_LEFT_STR[] = _TEXT("%-*s");
TCHAR FMT_TWO_EQUAL_STR[] = _TEXT("%s=%s");
TCHAR FMT_STR_CRLF[] = _TEXT("%s\r\n");
TCHAR FMT_DOUBLE_STR[] = _TEXT("%s%s");
TCHAR FMT_PATH_STR[] = _TEXT("%s\\%s");
TCHAR FMT_STR_TO_STR[] = _TEXT("%s -> %s");
TCHAR FMT_FAR_STR[] = _TEXT("%Fs");
TCHAR FMT_FAR_PREC_STR[] = _TEXT("%.*Fs");
TCHAR FMT_FAR_LEFT_STR[] = _TEXT("%-*Fs");
TCHAR FMT_FAR_STR_CRLF[] = _TEXT("%Fs\r\n");
TCHAR FMT_CHAR[] = _TEXT("%c");
TCHAR FMT_INT[] = _TEXT("%d");
TCHAR FMT_UINT[] = _TEXT("%u");
TCHAR FMT_LONG[] = _TEXT("%ld");
TCHAR FMT_ULONG[] = _TEXT("%lu");

TCHAR FMT_FAT32FREE[] = _TEXT("%s = %Lu\r\n");
TCHAR FMT_UINT_LEN[] = _TEXT("%u%n");

TCHAR FMT_DISK[] = _TEXT("%c:");
TCHAR FMT_ROOT[] = _TEXT("%c:\\");
TCHAR FMT_PATH[] = _TEXT("%c:\\%s");

// executable file extensions
TCHAR *executables[] = {
	_TEXT(".com"),
	_TEXT(".exe"),
	_TEXT(".btm"),		// in-memory batch file
	_TEXT(".bat"),
	NULL
};


// video type array for %_VIDEO
TCHAR *video_type[] = {
	"mono",
	"cga",
	"ega",		// monochrome ega
	"ega",		// color ega
	"vga",		// monochrome vga
	"vga"		// color vga
};


// tables for date formatting
TCHAR *daytbl[] = {
	_TEXT("Sunday"),
	_TEXT("Monday"),
	_TEXT("Tuesday"),
	_TEXT("Wednesday"),
	_TEXT("Thursday"),
	_TEXT("Friday"),
	_TEXT("Saturday")
};


TCHAR *montbl[] = {
	_TEXT("Jan"),
	_TEXT("Feb"),
	_TEXT("Mar"),
	_TEXT("Apr"),
	_TEXT("May"),
	_TEXT("Jun"),
	_TEXT("Jul"),
	_TEXT("Aug"),
	_TEXT("Sep"),
	_TEXT("Oct"),
	_TEXT("Nov"),
	_TEXT("Dec")
};


TCHAR *lmontbl[] = {
	_TEXT("January"),
	_TEXT("February"),
	_TEXT("March"),
	_TEXT("April"),
	_TEXT("May"),
	_TEXT("June"),
	_TEXT("July"),
	_TEXT("August"),
	_TEXT("September"),
	_TEXT("October"),
	_TEXT("November"),
	_TEXT("December")
};


TCHAR *dateformat[] = {
	_TEXT("mm%cdd%c[yy]yy"),	// USA date format
	_TEXT("dd%cmm%c[yy]yy"),	// Europe
	_TEXT("[yy]yy%cmm%cdd")		// Japan
};


// 4DOS swapping type
TCHAR *swap_mode[] = {
	"EMS",
	"XMS",
	"Disk",
	"None"
};


// COLORDIR attribute sequences
COLORD colorize_atts[] = {
	_TEXT("dirs"),0x10,
	_TEXT("rdonly"),0x01,
	_TEXT("hidden"),0x02,
	_TEXT("system"),0x04,
	_TEXT("archive"),0x20,
	_TEXT("normal"), 0,
};


// ANSI color sequences (for COLOR)
ANSI_COLORS colors[] = {
	_TEXT("Bla"),30,
	_TEXT("Blu"),34,
	_TEXT("Gre"),32,
	_TEXT("Cya"),36,
	_TEXT("Red"),31,
	_TEXT("Mag"),35,
	_TEXT("Yel"),33,
	_TEXT("Whi"),37,
	_TEXT("Bri Bla"),0,
	_TEXT("Bri Blu"),0,
	_TEXT("Bri Gre"),0,
	_TEXT("Bri Cya"),0,
	_TEXT("Bri Red"),0,
	_TEXT("Bri Mag"),0,
	_TEXT("Bri Yel"),0,
	_TEXT("Bri Whi"),0
};


// 4xxx / TCMD error messages
TCHAR *int_4dos_errors[] = 
{
	_TEXT("Syntax error"),
	_TEXT("Unknown command"),
	_TEXT("Command line too long"),
	_TEXT("No closing quote"),
	_TEXT("Can't open"),
	_TEXT("Can't create"),
	_TEXT("Can't delete"),
	_TEXT("Error reading"),
	_TEXT("Error writing"),
	_TEXT("Can't COPY or MOVE file to itself"),
	_TEXT("Insufficient disk space for"),		// 10
	_TEXT("Contents lost before copy"),
	_TEXT("Infinite COPY or MOVE loop"),
	_TEXT("Not an alias"),
	_TEXT("No aliases defined"),
	_TEXT("Alias loop"),
	_TEXT("Variable loop"),
	_TEXT("UNKNOWN_CMD loop"),
	_TEXT("Executable extension loop"),
	_TEXT("Invalid date"),
	_TEXT("Invalid time"),				// 20
	_TEXT("Directory stack empty"),	
	_TEXT("Can't get directory"),
	_TEXT("Label not found"),
	_TEXT("Out of environment space"),
	_TEXT("Out of alias space"),
	_TEXT("Not in environment"),
	_TEXT("4DOS internal stack overflow"),

	_TEXT("Command only valid in batch file"),
	_TEXT("Batch file missing"),
	_TEXT("Exceeded batch nesting limit"),		// 30
	_TEXT("Invalid batch file"),
	_TEXT("Encrypted batch files require the runtime version"),
	_TEXT("Missing ENDTEXT"),
	_TEXT("Missing GOSUB"),
	_TEXT("Environment already saved"),
	_TEXT("Missing SETLOCAL"),
	_TEXT("Exceeded SETLOCAL nesting limit"),
	_TEXT("Unbalanced parentheses"),
	_TEXT("No expression"),
	_TEXT("Overflow"),				// 40
	_TEXT("Underflow"),
	_TEXT("Divide by zero"),
	_TEXT("Loss of significance"),
	_TEXT("File is empty"),
	_TEXT("Not a directory"),
	_TEXT("Clipboard is in use by another program"),
	_TEXT("Clipboard is not text format"),
	_TEXT("Already excluded files"),
	_TEXT("No functions defined"),
	_TEXT("Not a function"),			// 50

	_TEXT("KSTACK.COM not loaded"),
	_TEXT("Invalid DOS version"),
	_TEXT("Not in swapping mode"),
	_TEXT("No UMBs; loading low"),
	_TEXT("Syntax error in region number or size"),
	_TEXT("Can't eject media"),
	_TEXT("Can't close tray"),
	_TEXT("IDLE is disabled")

#if _WIN
	,
	_TEXT("Error in command-line directive"),
	_TEXT("Window title not found"),
	_TEXT("Command not supported in this OS version"),
	_TEXT("Unknown process"),
	_TEXT("Listbox is full"),
	_TEXT("File association not found for extension"),
	_TEXT("File type not found or no open command associated with"),
	_TEXT("Invalid key"),
	_TEXT("Missing close paren"),
	_TEXT("Invalid count"),
	_TEXT("String too long"),
	_TEXT("Can't install hook"),
	_TEXT("No SMTP server address"),
	_TEXT("No SMTP user address"),
	_TEXT("Can't end current process"),
	_TEXT("Can't query key type")
#endif
};


// USAGE messages
TCHAR ALIAS_USAGE[] = _TEXT("ALIAS [/P /R ?] name=value");
TCHAR ATTRIB_USAGE[] = _TEXT("ATTRIB [/A:-rhsda /D /E /I /N /P /Q /S] [+|-AHRS] ?...");
TCHAR BEEP_USAGE[] = _TEXT("BEEP [frequency duration]");
TCHAR BREAK_USAGE[] = _TEXT("BREAK [ON | OFF]");
TCHAR CALL_USAGE[] = _TEXT("CALL ?");
TCHAR CDD_USAGE[] = _TEXT("CDD [/A /D /N /S /U] ~");
TCHAR COPY_USAGE[] = _TEXT("COPY [/A:-rhsda /CEGHIKMNOPQRSTUVXZ] ?...[/A /B] ?[/A /B]");
TCHAR COLOR_USAGE[] = _TEXT("COLOR # [BORDER bc]");
TCHAR CTTY_USAGE[] = _TEXT("CTTY device");
TCHAR DATE_USAGE[] = _TEXT("DATE [/T] [date]");
TCHAR DELETE_USAGE[] = _TEXT("DEL [/A:-rhsda /EFIKNPQSTWXYZ] ?...");
TCHAR DESCRIBE_USAGE[] = _TEXT("DESCRIBE [/A:-rhsda /I /D]?...");
TCHAR DIR_USAGE[] = _TEXT("DIR [/A:-rhsda /C[hp] /O:-cdeginrsu /T:acw /124BDEFHJKLMNPSTUVW] ?...");
TCHAR DIRHISTORY_USAGE[] = _TEXT("DIRHISTORY [/A /F /P /R ?]");
TCHAR DO_USAGE[] = _TEXT("DO [repetitor] [WHILE][UNTIL] ...] ... ENDDO");
TCHAR DRAWBOX_USAGE[] = _TEXT("DRAWBOX top left bottom right style # [FILL bg] [SHA] [ZOOM]");
TCHAR DRAWHLINE_USAGE[] = _TEXT("DRAWHLINE row col len style #");
TCHAR DRAWVLINE_USAGE[] = _TEXT("DRAWVLINE row col len style #");
TCHAR ESET_USAGE[] = _TEXT("ESET [/A /M] name...");
TCHAR EXCEPT_USAGE[] = _TEXT("EXCEPT [/I] (?...) ...");
TCHAR FFIND_USAGE[] = _TEXT("FFIND [/A:-rhsda /O:-acdeginrsu /D[a-z] /BCEFIKLMNPRSVY] [/T|X\"text\"] ?...");
TCHAR FOR_USAGE[] = _TEXT("FOR [/A[:-rhsda] /D /F /H /I /L /R] %var IN (set) DO ... [args]");
TCHAR FUNCTION_USAGE[] = _TEXT("FUNCTION [/P /R ?] name=value");
TCHAR GLOBAL_USAGE[] = _TEXT("GLOBAL [/H /I /P /Q] ...");
TCHAR HEAD_USAGE[] = _TEXT("HEAD [/A:-rhsda /Cn /I /Nn /P /Q /V] ?...");
TCHAR HISTORY_USAGE[] = _TEXT("HISTORY [/A /F /P /R ?]");
TCHAR IDLE_USAGE[] = _TEXT("IDLE [ON | OFF]");
TCHAR IF_USAGE[] = _TEXT("IF [/I] [NOT] condition ...");
TCHAR IFF_USAGE[] = _TEXT("IFF [NOT] condition THEN ^ ... [ELSE[IFF] ^ ...] ENDIFF");
TCHAR INKEY_USAGE[] = _TEXT("INKEY [/CDMPX /K\"mask\" /Wn] [text] %%var");
TCHAR INPUT_USAGE[] = _TEXT("INPUT [/CDENPX /Ln /Wn] [text] %%var");
TCHAR KEYBD_USAGE[] = _TEXT("KEYBD [/Cn /Nn /Sn]");
TCHAR KEYSTACK_USAGE[] = _TEXT("KEYSTACK [\"text\"] [n] [/Wn] [!]");
TCHAR LIST_USAGE[] = _TEXT("LIST [/A:-rhsda /Ln /T\"text\" /HINRSWX] ?...");
TCHAR LOADBTM_USAGE[] = _TEXT("LOADBTM [ON | OFF]");
TCHAR LFNFOR_USAGE[] = _TEXT("LFNFOR [ON | OFF]");
TCHAR LOADHIGH_USAGE[] = _TEXT("LOADHIGH [/L:r1,n1;r2,n2;... /S] ?");
TCHAR LOG_USAGE[] = _TEXT("LOG [/H] [ON | OFF | /W ?][text]");
TCHAR MD_USAGE[] = _TEXT("MD [/N /S] ~...");
TCHAR MOVE_USAGE[] = _TEXT("MOVE [/A:-rhsda /CDEFGHIMNOPQRSTUVWZ] ?[... ?]");
TCHAR ON_USAGE[] = _TEXT("ON [BREAK | ERROR | ERRORMSG] ...");
TCHAR POPD_USAGE[] = _TEXT("POPD [*]");
TCHAR RD_USAGE[] = _TEXT("RD [/I] ~...");
TCHAR REBOOT_USAGE[] = _TEXT("REBOOT [/C /M /P /S /V]");
TCHAR RENAME_USAGE[] = _TEXT("REN [/A:-rhsda /EINPQST] ?... ?");
TCHAR SCREEN_USAGE[] = _TEXT("SCREEN row col [text]");
TCHAR SCRPUT_USAGE[] = _TEXT("SCRPUT row col # text");
TCHAR VSCRPUT_USAGE[] = _TEXT("VSCRPUT row col # text");
TCHAR SELECT_USAGE[] = _TEXT("SELECT [/A:-rhsda /C[hp] /1DEHIJLTXZ /O:-cdeginrsu /T:acw] ... (?) ...");
TCHAR SETERROR_USAGE[] = _TEXT("SETERROR errorlevel");;
TCHAR SET_USAGE[] = _TEXT("SET [/AEMPR ?] name=value");
TCHAR SWAPPING_USAGE[] = _TEXT("SWAPPING [ON | OFF]");
TCHAR TAIL_USAGE[] = _TEXT("TAIL [/A:-rhsda /Cn /I /Nn /P /Q /V] ?...");
TCHAR TEE_USAGE[] = _TEXT("TEE [/A] ?...");
TCHAR TIME_USAGE[] = _TEXT("TIME [/T] [hh:mm:ss]");
TCHAR TIMER_USAGE[] = _TEXT("TIMER [/123QS] [ON | OFF]");
TCHAR TITLE_USAGE[] = _TEXT("TITLE title");
TCHAR TOUCH_USAGE[] = _TEXT("TOUCH [/A:-rhsda /C /E /F /I /Q /S /R[:acw]? /D[acw]date /T[acw]time] ?...");
TCHAR TRANSIENT_USAGE[] = _TEXT("TRANSIENT [ON | OFF]");
TCHAR TREE_USAGE[] = _TEXT("TREE [/A /B /F /H /P /S /T[acw]] dir...");
TCHAR TRUENAME_USAGE[] = _TEXT("TRUENAME ?");
TCHAR TYPE_USAGE[] = _TEXT("TYPE [/A:-rhsda /ILPV] ?...");
TCHAR UNALIAS_USAGE[] = _TEXT("UNALIAS [/Q /R] name...");
TCHAR UNFUNCTION_USAGE[] = _TEXT("UNFUNCTION [/Q /R] name...");
TCHAR UNSET_USAGE[] = _TEXT("UNSET [/M /Q /R ?] name...");
TCHAR VER_USAGE[] = _TEXT("VER [/R]");
TCHAR VERIFY_USAGE[] = _TEXT("VERIFY [ON | OFF]");
TCHAR WHICH_USAGE[] = _TEXT("WHICH name...");


// BATCH.C

TCHAR DEBUGGER_PROMPT[] =  _TEXT("T(race) S(tep) J(ump) X(pand)  L(ist) V(ars) A(liases)  O(ff) Q(uit)");

TCHAR ENDTEXT[] = _TEXT("ENDTEXT");
TCHAR ECHO_IS[] = _TEXT("ECHO is %s\r\n");
TCHAR LOADBTM_IS[] = _TEXT("LOADBTM is %s\r\n");
TCHAR PAUSE_PAGE_PROMPT[] = _TEXT("Press ESC to quit or another key to continue...");
TCHAR PAUSE_PAGE_CLEAR[] = _TEXT("\r                                               \r");
TCHAR PAUSE_PROMPT[] = _TEXT("Press any key when ready...");
TCHAR DO_DO[] = _TEXT("do");
TCHAR DO_BY[] = _TEXT("by");
TCHAR DO_FOREVER[] = _TEXT("forever");
TCHAR DO_LEAVE[] = _TEXT("leave");
TCHAR DO_ITERATE[] = _TEXT("iterate");
TCHAR DO_END[] = _TEXT("enddo");
TCHAR DO_WHILE[] = _TEXT("while");
TCHAR DO_UNTIL[] = _TEXT("until");
TCHAR FOR_IN[] = _TEXT("in");
TCHAR FOR_DO[] = _TEXT("do");
TCHAR IF_NOT[] = _TEXT("not");
TCHAR IF_OR[] = _TEXT(".OR.");
TCHAR IF_XOR[] = _TEXT(".XOR.");
TCHAR IF_AND[] = _TEXT(".AND.");

TCHAR IF_DEFINED[] = _TEXT("defined");
TCHAR IF_DIREXIST[] = _TEXT("direxist");
TCHAR IF_EXIST[] = _TEXT("exist");
TCHAR IF_ISDIR[] = _TEXT("isdir");
TCHAR IF_ISINTERNAL[] = _TEXT("isinternal");
TCHAR IF_ISALIAS[] = _TEXT("isalias");
TCHAR IF_ISFUNCTION[] = _TEXT("isfunction");
TCHAR IF_ISLABEL[] = _TEXT("islabel");

TCHAR IF_ERRORLEVEL[] = _TEXT("errorlevel");
TCHAR EQ[] = _TEXT("EQ");
TCHAR GT[] = _TEXT("GT");
TCHAR GE[] = _TEXT("GE");
TCHAR LT[] = _TEXT("LT");
TCHAR LE[] = _TEXT("LE");
TCHAR NE[] = _TEXT("NE");
TCHAR EQC[] = _TEXT("EQC");

TCHAR THEN[] = _TEXT("then");
TCHAR IFF[] = _TEXT("iff");
TCHAR ELSEIFF[] = _TEXT("elseiff");
TCHAR ELSE[] = _TEXT("else");
TCHAR ENDIFF[] = _TEXT("endiff");
TCHAR ON_BREAK[] = _TEXT("break");
TCHAR ON_ERROR[] = _TEXT("error");
TCHAR ON_ERRORMSG[] = _TEXT("errormsg");


// DIRCMDS.C
TCHAR ONE_FILE[] = _TEXT("file");
TCHAR MANY_FILES[] = _TEXT("files");
TCHAR ONE_DIR[] = _TEXT("dir");
TCHAR MANY_DIRS[] = _TEXT("dirs");
TCHAR DIRECTORY_OF[] = _TEXT(" Directory of  %s");
TCHAR DIR_FILE_SIZE[] = _TEXT("KMG");

TCHAR DIR_BYTES_IN_FILES[] = _TEXT("%15s bytes in %Lu %s and %Lu %s");
TCHAR DIR_BYTES_ALLOCATED[] = _TEXT("    %s bytes allocated");

TCHAR DIR_BYTES_FREE[] = _TEXT("%15s bytes free");

TCHAR DIR_TOTAL[] = _TEXT("    Total for:  %s");
TCHAR DIR_LABEL[] =          _TEXT(" <DIR>    ");
TCHAR DIR_PERCENT_RATIO[] = _TEXT("  %2d%%");

TCHAR DIR_RATIO[] = _TEXT("  %2d.%d to 1.0");
TCHAR DIR_AVERAGE_RATIO[] = _TEXT("%14s to 1.0 average compression ratio");
TCHAR DIR_AVERAGE_PERCENT_RATIO[] = _TEXT("%14s%% average compression ratio");

TCHAR LFN_DIR_LABEL[] =          _TEXT("        <DIR>  ");

TCHAR COLORDIR[] = _TEXT("colordir");
TCHAR DESCRIBE_PROMPT[] = _TEXT("Describe \"%s\" : ");


// xxxCALLS.C

TCHAR CLIP[] = _TEXT("clip:");

// DOSCALLS.C
TCHAR COMMAND_COM[] = _TEXT("command.com");
TCHAR HELP_EXE[] = _TEXT("4help.exe");
TCHAR HELP_TEXT[] = _TEXT("4dos.hlp");
TCHAR HELP_NX[] = _TEXT("/nx");
TCHAR OPTION_EXE[] = _TEXT("option.exe");


// xxxCMDS.C
TCHAR KBD_CAPS_LOCK[] = _TEXT("Caps=%s\r\n");
TCHAR KBD_NUM_LOCK[] = _TEXT("Num=%s\r\n");
TCHAR KBD_SCROLL_LOCK[] = _TEXT("Scroll=%s\r\n");
TCHAR ENTER_STANDBY[] = _TEXT("Confirm system standby");
TCHAR TURN_MONITOR_OFF[] = _TEXT("Confirm monitor shutdown");
TCHAR REBOOT_IT[] = _TEXT("Confirm system reboot");
TCHAR SHUT_IT_DOWN[] = _TEXT("Confirm system shutdown");
TCHAR TOTAL_ENVIRONMENT[] = _TEXT("\r\n%15Lu bytes total environment\r\n");

TCHAR TOTAL_ALIAS[] = _TEXT("\r\n%15Lu characters total alias\r\n");
TCHAR TOTAL_FUNCTION[] = _TEXT("\r\n%15Lu characters total function\r\n");
TCHAR TOTAL_HISTORY[] = _TEXT("\r\n%15Lu characters total history\r\n");
TCHAR TOTAL_DISK_USED[] = _TEXT("%15s bytes total disk space\r\n%15s bytes used\r\n");

TCHAR DISK_PERCENT[] = _TEXT("%15s %% in use\r\n");

TCHAR START_BG_STR[] = _TEXT("bg");
TCHAR START_DOS_STR[] = _TEXT("dos");
TCHAR START_ICON_STR[] = _TEXT("icon=");

TCHAR START_TRANSIENT_STR[] = _TEXT("c");
TCHAR START_FS_STR[] = _TEXT("fs");
TCHAR START_INV_STR[] = _TEXT("inv");
TCHAR START_KEEP_STR[] = _TEXT("k");
TCHAR START_MAX_STR[] = _TEXT("max");
TCHAR START_MIN_STR[] = _TEXT("min");
TCHAR START_PGM_STR[] = _TEXT("pgm");
TCHAR START_WIN_STR[] = _TEXT("win");

TCHAR START_NO_STR[] = _TEXT("n");
TCHAR WIN_OS2[] = _TEXT("WINOS2");
TCHAR START_FG_STR[] = _TEXT("fg");
TCHAR START_PM_STR[] = _TEXT("pm");
TCHAR START_WIN3_STR[] = _TEXT("win3");

TCHAR START_LOCAL_STR[] = _TEXT("l");
TCHAR START_LA_STR[] = _TEXT("la");
TCHAR START_LD_STR[] = _TEXT("ld");
TCHAR START_LF_STR[] = _TEXT("lf");
TCHAR START_LH_STR[] = _TEXT("lh");
TCHAR START_POS_STR[] = _TEXT("pos=");

TCHAR LOCK_WARNING[] = _TEXT("Warning: LOCK will allow programs to directly access a disk drive, possibly\r\nresulting in damage to filenames or the loss of data.\r\n");
TCHAR TOTAL_EMS[] = _TEXT("\r\n%15Lu bytes total EMS memory\r\n");
TCHAR TOTAL_EXTENDED[] = _TEXT("\r\n%15Lu bytes total EXTENDED memory\r\n");
TCHAR XMS_FREE[] = _TEXT("\r\n%15Lu bytes free XMS memory  ");
TCHAR SWAPPING_IS[] = _TEXT("%s (%s) is %s\r\n");
TCHAR WARNING_NO_REGIONS[] = _TEXT("No UMB regions defined, switches ignored\r\n");
TCHAR WARNING_INVALID_REGION[] = _TEXT("Invalid region number or size \"%s\", ignored\r\n");
TCHAR TOTAL_DOS_RAM[] = _TEXT("\r\n%15Lu bytes total DOS RAM\r\n");
TCHAR HMA_FREE[] = _TEXT("(HMA free)");
TCHAR HMA_USED[] = _TEXT("(HMA in use)");

// xxxINIT.C
// Embedded copyright notice
TCHAR COPYRIGHT[] = _TEXT("Copyright 1988-2004  Rex Conn & JP Software Inc.  All Rights Reserved\r\n");
TCHAR COMSPEC[] = _TEXT("COMSPEC");

TCHAR RUNTIME_ERROR[] = _TEXT("\r\nNo runtime filename given ... press any key to exit");

TCHAR NAME_HEADER[] = _TEXT("4DOS");
TCHAR PROGRAM[16];
TCHAR SET_PROGRAM[] = _TEXT("4DOS %u%c%02u");
TCHAR *DOS_NAME = _TEXT("4DOS.COM");
TCHAR *SHORT_NAME = _TEXT("4DOS");
TCHAR FOURDOS_REV[] = _TEXT("4DOS Build %u (%s)");

TCHAR DOS_VERSION[] = _TEXT("\r\n%s   %s %u%c%02u\r\n");
TCHAR COMSPEC_DOS[] = _TEXT("%s=%Fs");
TCHAR MS95VER[] = _TEXT("(Win95) DOS");
TCHAR MS98VER[] = _TEXT("(Win98) DOS");
TCHAR MSMEVER[] = _TEXT("(WinME) DOS");

TCHAR UNKVER[] = _TEXT("DOS");
TCHAR MSVER[] = _TEXT("MS-DOS");
TCHAR LZVER[] = _TEXT("LZ-DOS");
TCHAR OS2VER[] = _TEXT("OS/2");
TCHAR WARPVER[] = _TEXT("OS/2 Warp");
TCHAR DRVER[] = _TEXT("DR DOS");
TCHAR NOVVER[] = _TEXT("Novell DOS");
TCHAR IBMVER[] = _TEXT("PC DOS");
TCHAR WENVER[] = _TEXT("Wendin-DOS");
TCHAR XDVER[] = _TEXT("X-DOS");
TCHAR FDVER[] = _TEXT("FreeDOS");
TCHAR RDVER[] = _TEXT("ROM-DOS");
TCHAR GSVER[] = _TEXT("DOS-ROM");
TCHAR RXVER[] = _TEXT("RXDOS");
TCHAR SDVER[] = _TEXT("S/DOS");
TCHAR PTSVER[] = _TEXT("PTS-DOS");
TCHAR DOS_REVISION[] = _TEXT("\r\nDOS Revision %c");
TCHAR OS2_REVISION[] = _TEXT("\r\nOS/2 Revision %u");
TCHAR DOS_LOCATION[] = _TEXT("; DOS is in %s");
TCHAR DOS_HMA[] = _TEXT("HMA");
TCHAR DOS_ROM[] = _TEXT("ROM");
TCHAR DOS_LOW[] = _TEXT("low memory");


// ERROR.C
TCHAR USAGE_MSG[] = _TEXT("Usage : ");
TCHAR FILE_SPEC[] = _TEXT("[d:][path]name");
TCHAR PATH_SPEC[] = _TEXT("[d:]pathname");
TCHAR COLOR_SPEC[] = _TEXT("[BRI][BLI] fg ON [BRI] bg");


// EXPAND.C
TCHAR TEMP_DISK[] = _TEXT("TEMP");
TCHAR TMP_DISK[] = _TEXT("TMP");
TCHAR TEMP4DOS_DISK[] = _TEXT("TEMP4DOS");

TCHAR PATH_VAR[] = _TEXT("PATH");
TCHAR PATHEXT[] = _TEXT("PATHEXT");

TCHAR MONO_MONITOR[] = _TEXT("mono");
TCHAR COLOR_MONITOR[] = _TEXT("color");
TCHAR SVGA_TYPE[] = _TEXT("svga");

TCHAR END_OF_FILE_STR[] = _TEXT("**EOF**");


TCHAR *ACList[] = {
	"off-line",
	"on-line",
	"unknown"
};

TCHAR *BatteryList[] = {
	"high",
	"low",
	"critical",
	"charging",
	"unknown"
};


const TCHAR *VAR_ARRAY[] = {
	_TEXT("4ver"),			// 4DOS / 4NT / TC32 version
	_TEXT("?"),			// return code of previous internal command
	_TEXT("alias"),			// free alias space
	_TEXT("alt"),			// Alt key depressed
	_TEXT("ansi"),			// ANSI driver loaded
	_TEXT("apmac"),			// APM AC line status
	_TEXT("apmbatt"),		// APM battery status
	_TEXT("apmlife"),		// APM remaining battery life
	_TEXT("append"),		// APPEND loaded (0 or 1)
	_TEXT("assign"),		// ASSIGN loaded (0, 1 or -1)
	_TEXT("batch"),			// batch nesting level
	_TEXT("batchline"),		// line # in current batch file
	_TEXT("batchname"),		// name of current batch file
	_TEXT("batchtype"),		// type of current batch file
	_TEXT("bdebugger"),		// batch debugger active
	_TEXT("bg"),			// background color at cursor
	_TEXT("boot"),			// boot drive
	_TEXT("build"),			// internal 4DOS / 4NT / TC32 build number
	_TEXT("capslock"),		// CapsLock on
	_TEXT("cdroms"),		// list of CD-ROM drives
	_TEXT("ci"),			// insert cursor shape
	_TEXT("cmdline"),		// current (partial?) command line
	_TEXT("cmdproc"),		// command processor
	_TEXT("cmdspec"),		// command processor full pathname
	_TEXT("co"),			// overstrike cursor shape
	_TEXT("codepage"),		// active codepage
	_TEXT("column"),		// current column position
	_TEXT("columns"),		// # of columns on active display
	_TEXT("country"),		// country code
	_TEXT("cpu"),			// cpu type
	_TEXT("cpuspeed"),		// cpu speed [MHz] (586+)
	_TEXT("ctrl"),			// Ctrl key depressed
	_TEXT("cwd"),			// current working directory
	_TEXT("cwds"),			// current working directory with trailing '\'
	_TEXT("cwp"),			// current working path
	_TEXT("cwps"),			// current working path with trailing '\'
	_TEXT("date"),			// current date
	_TEXT("datetime"),		// current date and time, yyyyMMddhhmmss
	_TEXT("day"),			// current day
	_TEXT("disk"),			// current disk
	_TEXT("display"),		// DISPLAY.SYS loaded (0 or 1)
	_TEXT("dname"),			// description file name
	_TEXT("dos"),			// OS type
	_TEXT("dosver"),		// OS version #
	_TEXT("dow"),			// day of week (3-char)
	_TEXT("dowf"),			// day of week (full name)
	_TEXT("dowi"),			// day of week as integer
	_TEXT("doy"),			// day of year (1 - 366)
	_TEXT("dpmi"),			// DPMI version
	_TEXT("driver"),		// DRIVER.SYS loaded (0, 1 or -1)
	_TEXT("drives"),		// list of all available drives
	_TEXT("dst"),			// daylight savings time in effect
	_TEXT("dv"),			// DESQview loaded flag
	_TEXT("echo"),			// ECHO state (batch or command line)
	_TEXT("editmode"),		// current line editor state (0 or 1)
	_TEXT("ega"),			// EGA.SYS loaded (0, 1 or -1)
	_TEXT("env"),			// free environment space
	_TEXT("execstr"),		// return code of the last @EXECSTR
	_TEXT("expansion"),		// current expansion mode (SETDOS /X)
	_TEXT("fg"),			// foreground color at cursor
	_TEXT("fontpage"),		// active fontpage
	_TEXT("graftabl"),		// GRAFTABL loaded (0, 1 or -1)
	_TEXT("graphics"),		// GRAPHICS loaded (0 or 1)
	_TEXT("hdrives"),		// list of hard (fixed) drives
	_TEXT("hlogfile"),		// name of current history log file
	_TEXT("hour"),			// current hour
	_TEXT("idow"),			// international day (short name)
	_TEXT("idowf"),			// international day (long name)
	_TEXT("ininame"),		// pathname of the current INI file
	_TEXT("isodate"),		// yyyy-mm-dd (ISO format)
	_TEXT("isodowi"),		// day of week as integer (ISO)
	_TEXT("isordate"),		// yyyy-ddd (ISO ordinal date format)
	_TEXT("isowdate"),		// yyyy-Www-d (ISO week date format)
	_TEXT("isoweek"),		// week of year (ISO)
	_TEXT("isowyear"),		// year of ISO week date
	_TEXT("kbhit"),			// != 0 if key is waiting
	_TEXT("keystacked"),		// number of keystrokes in KSTACK buffer
	_TEXT("kstack"),		// != 0 if KSTACK is loaded
	_TEXT("lalt"),			// left Alt key depressed
	_TEXT("lastdir"),		// previous directory
	_TEXT("lastdisk"),		// last disk in use
	_TEXT("lctrl"),			// left Ctrl key depressed
	_TEXT("logfile"),		// name of current log file
	_TEXT("lshift"),		// left Shift key depressed
	_TEXT("machine"),		// machine name in the network
	_TEXT("minute"),		// current minute
	_TEXT("mjd"),			// modified Julian day
	_TEXT("monitor"),		// monitor type (mono or color)
	_TEXT("month"),			// current month
	_TEXT("monthf"),		// current month (full name)
	_TEXT("mouse"),			// mouse loaded (0 or 1)
	_TEXT("mscdex"),		// MSCDEX loaded (0, 1 or -1)
	_TEXT("ndp"),			// math coprocessor type
	_TEXT("network"),		// network components installed
	_TEXT("nlsfunc"),		// NLSFUNC installed (0, 1 or -1)
	_TEXT("numlock"),		// NumLock on
	_TEXT("pipe"),			// currently in a pipe (0 or 1)
	_TEXT("power"),			// POWER loaded (0 or 1)
	_TEXT("print"),			// PRINT loaded (0, 1 or -1)
	_TEXT("ralt"),			// right Alt key depressed
	_TEXT("rctrl"),			// right Ctrl key depressed
	_TEXT("ready"),			// list of all ready drives
	_TEXT("row"),			// current row
	_TEXT("rows"),			// # of rows on active display
	_TEXT("rshift"),		// right Shift key depressed
	_TEXT("sbdsp"),			// SB DSP version
	_TEXT("scrolllock"),		// ScrollLock on
	_TEXT("second"),		// current second
	_TEXT("share"),			// SHARE installed (0, 1 or -1)
	_TEXT("shell"),			// shell level (0 - 99)
	_TEXT("shift"),			// Shift key depressed
	_TEXT("smartdrv"),		// SMARTDRV installed (0 or 1)
	_TEXT("startpath"),		// startup directory of current shell
	_TEXT("stderr"),		// standard error points to CON
	_TEXT("stdin"),			// standard input points to CON
	_TEXT("stdout"),		// standard output points to CON
	_TEXT("stzn"),			// name of time zone for standard time
	_TEXT("stzo"),			// offset in minutes from UTC for standard time
	_TEXT("swapping"),		// current 4DOS swapping mode
	_TEXT("syserr"),		// last system error #
	_TEXT("sysreq"),		// SysReq key depressed
	_TEXT("taskmax"),		// DR DOS TASKMAX or TASKMGR loaded
	_TEXT("taskswitcher"),		// DOSSHELL task switcher present
	_TEXT("tick"),			// BIOS clock ticks since midnight
	_TEXT("time"),			// current time
	_TEXT("transient"),		// transient or resident shell
	_TEXT("tsc"),			// time stamp counter (586+ only)
	_TEXT("tzn"),			// name of current time zone
	_TEXT("tzo"),			// offset in minutes from UTC for current time zone
	_TEXT("unixtime"),		// number of seconds since 1-1-1970
	_TEXT("utcdate"),		// current UTC date
	_TEXT("utcdatetime"),		// current UTC date and time
	_TEXT("utchour"),		// current UTC hour
	_TEXT("utcisodate"),		// current UTC date in ISO format
	_TEXT("utcminute"),		// current UTC minute
	_TEXT("utcsecond"),		// current UTC second
	_TEXT("utctime"),		// current UTC time
	_TEXT("v86"),			// CPU in V86 mode (386+ only)
	_TEXT("vcpi"),			// VCPI version
	_TEXT("vds"),			// VDS version
	_TEXT("vermajor"),		// 4DOS major version
	_TEXT("verminor"),		// 4DOS minor version
	_TEXT("version"),		// 4DOS version in major.minor format
	_TEXT("video"),			// video type (cga, mono, ega, vga)
	_TEXT("win"),			// Windows loaded flag
	_TEXT("winticks"),		// ms since midnight/Windows started
	_TEXT("wintitle"),		// window title
	_TEXT("year"),			// current year

	NULL
};

const TCHAR *FUNC_ARRAY[] = {
	_TEXT("abs"),
	_TEXT("agedate"),
	_TEXT("alias"),
	_TEXT("altname"),
	_TEXT("ascii"),
	_TEXT("attrib"),
	_TEXT("average"),
	_TEXT("caps"),
	_TEXT("cdrom"),
	_TEXT("ceiling"),
	_TEXT("char"),
	_TEXT("clip"),			// clipboard read
	_TEXT("clipw"),			// clipboard paste
	_TEXT("clustsize"),
	_TEXT("codepage"),
	_TEXT("com"),
	_TEXT("comma"),
	_TEXT("compare"),
	_TEXT("convert"),
	_TEXT("count"),
	_TEXT("crc32"),
	_TEXT("cwd"),
	_TEXT("cwds"),
	_TEXT("date"),
	_TEXT("dateconv"),
	_TEXT("day"),
	_TEXT("ddcstr"),
	_TEXT("dec"),
	_TEXT("decimal"),
	_TEXT("descript"),
	_TEXT("device"),
	_TEXT("digits"),
	_TEXT("dirstack"),
	_TEXT("diskfree"),
	_TEXT("disktotal"),
	_TEXT("diskused"),
	_TEXT("dosmem"),
	_TEXT("dow"),
	_TEXT("dowf"),
	_TEXT("dowi"),
	_TEXT("doy"),			// day of year (1-366)
	_TEXT("drivetype"),
	_TEXT("ems"),
	_TEXT("errtext"),
	_TEXT("eval"),
	_TEXT("exec"),
	_TEXT("execstr"),
	_TEXT("expand"),
	_TEXT("ext"),
	_TEXT("extended"),
	_TEXT("field"),
	_TEXT("fields"),
	_TEXT("fileage"),
	_TEXT("fileclose"),
	_TEXT("filedate"),
	_TEXT("filename"),
	_TEXT("fileopen"),
	_TEXT("fileread"),
	_TEXT("filereadb"),
	_TEXT("files"),
	_TEXT("fileseek"),
	_TEXT("fileseekl"),
	_TEXT("filesize"),
	_TEXT("filetime"),
	_TEXT("filewrite"),
	_TEXT("filewriteb"),
	_TEXT("findclose"),
	_TEXT("findfirst"),
	_TEXT("findnext"),
	_TEXT("floor"),
	_TEXT("format"),
	_TEXT("fstype"),
	_TEXT("full"),
	_TEXT("function"),
	_TEXT("hddsize"),
	_TEXT("history"),
	_TEXT("if"),
	_TEXT("inc"),
	_TEXT("index"),
	_TEXT("iniread"),
	_TEXT("iniwrite"),
	_TEXT("insert"),
	_TEXT("instr"),
	_TEXT("int"),
	_TEXT("isalnum"),
	_TEXT("isalpha"),
	_TEXT("isascii"),
	_TEXT("iscntrl"),
	_TEXT("isdigit"),
	_TEXT("islower"),
	_TEXT("isodowi"),
	_TEXT("isoweek"),
	_TEXT("isowyear"),
	_TEXT("isprint"),
	_TEXT("ispunct"),
	_TEXT("isspace"),
	_TEXT("isupper"),
	_TEXT("isxdigit"),
	_TEXT("label"),
	_TEXT("lcs"),
	_TEXT("left"),
	_TEXT("len"),
	_TEXT("lfn"),
	_TEXT("line"),
	_TEXT("lines"),
	_TEXT("lower"),
	_TEXT("lpt"),
	_TEXT("ltrim"),
	_TEXT("makeage"),
	_TEXT("makedate"),
	_TEXT("maketime"),
	_TEXT("master"),
	_TEXT("max"),
	_TEXT("md5"),
	_TEXT("min"),
	_TEXT("month"),
	_TEXT("monthf"),
	_TEXT("mouse"),
	_TEXT("name"),
	_TEXT("numeric"),
	_TEXT("path"),
	_TEXT("quote"),
	_TEXT("random"),
	_TEXT("readscr"),
	_TEXT("ready"),
	_TEXT("remote"),
	_TEXT("removable"),
	_TEXT("repeat"),
	_TEXT("replace"),
	_TEXT("reverse"),
	_TEXT("right"),
	_TEXT("rtrim"),
	_TEXT("search"),
	_TEXT("select"),
	_TEXT("serial"),
	_TEXT("sfn"),
	_TEXT("sha1"),
	_TEXT("similar"),
	_TEXT("smbstr"),
	_TEXT("strip"),
	_TEXT("subst"),
	_TEXT("substr"),
	_TEXT("time"),
	_TEXT("timer"),
	_TEXT("trim"),
	_TEXT("truename"),
	_TEXT("truncate"),
	_TEXT("unique"),
	_TEXT("unquote"),
	_TEXT("unquotes"),
	_TEXT("upper"),
	_TEXT("wild"),
	_TEXT("word"),
	_TEXT("words"),
	_TEXT("xms"),
	_TEXT("year"),

	NULL
};


// FILECMDS.C
TCHAR REPLACE[] = _TEXT(" (Replace)");
TCHAR FILES_COPIED[] = _TEXT("%6Lu %s copied\r\n");
TCHAR MOVE_CREATE_DIR[] = _TEXT("Create \"%s\"");
TCHAR FILES_MOVED[] = _TEXT("%6Lu %s moved");
TCHAR FILES_RENAMED[] = _TEXT("%6Lu %s renamed\r\n");
TCHAR FILES_DELETED[] = _TEXT("%6Lu %s deleted");

TCHAR FILES_BYTES_FREED[] = _TEXT(" %15Lu bytes freed");
TCHAR ARE_YOU_SURE[] = _TEXT("Are you sure");
TCHAR DELETE_QUERY[] = _TEXT("Delete ");
TCHAR DELETING_FILE[] = _TEXT("Deleting %s\r\n");

// INIPARSE.C
TCHAR INI_ERROR[] = _TEXT("Error on line %d of %s:\r%s  \"%.32s\"");


// LINES.C
TCHAR BOX_FILL[] = _TEXT("fil");
TCHAR BOX_SHADOW[] = _TEXT("sha");
TCHAR BOX_ZOOM[] = _TEXT("zoo");


// LIST.C
TCHAR FFIND_FILE_HEADER[] = _TEXT("---- %s");
TCHAR FFIND_OFFSET[] = _TEXT("Offset: %Lu  (%lxh)");
TCHAR FFIND_TEXT_FOUND[] = _TEXT("  %Lu %s in");
TCHAR FFIND_ONE_LINE[] = _TEXT("line");
TCHAR FFIND_MANY_LINES[] = _TEXT("lines");

TCHAR FFIND_FOUND[] = _TEXT(" %6Lu %s");
TCHAR FFIND_CONTINUE[] = _TEXT("Continue search? ");
TCHAR LIST_STDIN_MSG[] = _TEXT("STDIN");
TCHAR LIST_DELETE[] = _TEXT("Delete this file? ");
TCHAR LIST_DELETE_TITLE[] = _TEXT(" Delete ");
TCHAR LIST_GOTO[] = _TEXT("Line: ");
TCHAR LIST_GOTO_OFFSET[] = _TEXT("Hex Offset: ");
TCHAR LIST_INFO_PIPE[] = _TEXT("LIST is displaying the output of a pipe.");

TCHAR LIST_INFO_FAT[] = _TEXT("File:  %s\nDesc:  %.64s\nSize:  %Ld bytes\nDate:  %s\nTime:  %02u%c%02u%c%02u\n");
TCHAR LIST_INFO_LFN[] = _TEXT("File:        %s\nDescription: %.55s\nSize:        %Ld bytes\nLast Write:  %-8s  %02u%c%02u%c%02u\nLast Access: %-8s  %02u%c%02u%c%02u\nCreated:     %-8s  %02u%c%02u%c%02u");

TCHAR LIST_HEADER[] = _TEXT(" %-12.12s %c F1 Help %c Commands: BFGHILNPWX %c");
TCHAR LIST_LINE[] = _TEXT("Col %-3d  Line %-9Lu%3d%%");
TCHAR LIST_WAIT[] = _TEXT("WAIT");
TCHAR LIST_GOTO_TITLE[] = _TEXT(" Goto Line / Offset ");
TCHAR LIST_FIND[] = _TEXT("Find: ");
TCHAR LIST_FIND_WAIT[] = _TEXT("Finding \"%.64s\"");
TCHAR LIST_FIND_TITLE[] = _TEXT(" Find Text ");
TCHAR LIST_FIND_TITLE_REVERSE[] = _TEXT(" Find Text (Reverse) ");
TCHAR LIST_FIND_BTITLE_CASE[] = _TEXT(" Case-sensitive search ");
TCHAR LIST_FIND_HEX[] = _TEXT("Hex search (Y/N)? ");
TCHAR LIST_NOT_FOUND[] = _TEXT("Not found--press a key to continue ");
TCHAR LIST_PRINT_TITLE[] = _TEXT(" Print ");
TCHAR LIST_QUERY_PRINT[] = _TEXT("Print File or Page (%c/%c)? ");
TCHAR LIST_PRINTING[] = _TEXT("Printing--press ESC to quit ");
TCHAR LIST_SAVE_TITLE[] = _TEXT(" Save As ");
TCHAR LIST_QUERY_SAVE[] = _TEXT("Filename: ");
TCHAR LIST_TABSIZE[] = _TEXT("Tab size: ");
TCHAR LIST_TABSIZE_TITLE[] = _TEXT(" Tabs ");


// MISC.C
TCHAR NO_SEP[] = _TEXT("NO_SEP");
TCHAR DESCRIPTION_FILE[144] = _TEXT("DESCRIPT.ION");
TCHAR DESCRIPTION_SCAN[] = _TEXT("%*[ ,\t]%511[^\r\n\004\032]");
TCHAR CONSOLE[] = _TEXT("CON");
TCHAR DEV_CONSOLE[] = _TEXT("\\DEV\\CON");
LPTSTR PRINTERS[] = { _TEXT("PRN"), _TEXT("LPT1"), _TEXT("LPT2"), _TEXT("LPT3"), _TEXT("\\DEV\\PRN"), _TEXT("\\DEV\\LPT1"), _TEXT("\\DEV\\LPT2"), _TEXT("\\DEV\\LPT3") };
TCHAR YES_NO[] = _TEXT("Y/N");
TCHAR YES_NO_ALL[] = _TEXT("Y/N/A/R");
TCHAR BRIGHT[] = _TEXT("Bri ");
TCHAR BLINK[] = _TEXT("Bli ");
TCHAR BORDER[] = _TEXT("Bor");


// PARSER.C
TCHAR CANCEL_BATCH_JOB[] = _TEXT("\r\nCancel batch job %s ? (Y/N/A) : ");
TCHAR INSERT_DISK[] = _TEXT("\r\nInsert disk with \"%s\"\r\nPress any key when ready...");
TCHAR UNKNOWN_COMMAND[] = _TEXT("UNKNOWN_CMD");
TCHAR COMMAND_GROUP_MORE[] = _TEXT("More? ");
TCHAR CMDLINE_VAR[] = _TEXT("CMDLINE=");
TCHAR PROMPT_NAME[] = _TEXT("PROMPT");
TCHAR TITLE_PROMPT[] = _TEXT("TITLEPROMPT");

TCHAR DOS_HDR[] = _TEXT("  DOS      Ctrl+Esc = Window List                            F1 or HELP = help  ");


// SCREENIO.C
TCHAR HISTORY_TITLE[] = _TEXT(" History ");
TCHAR DIRECTORY_TITLE[] = _TEXT(" Directories ");
TCHAR FILENAMES_TITLE[] = _TEXT(" Filenames ");
TCHAR VARIABLES_TITLE[] = _TEXT(" Variables ");


// SELECT.C
TCHAR SELECT_HEADER[] = _TEXT("     chars %c Cursor keys select %c +Mark  -Clear %c ENTER to run %c");
TCHAR MARKED_FILES[] = _TEXT("  Marked: %4u files  %9LuK");
TCHAR SELECT_PAGE_COUNT[] = _TEXT("Page %2u of %2u");


// SYSCMDS.C
TCHAR BUILDING_INDEX[] = _TEXT("Indexing %s\r\n");
TCHAR CDPATH[] = _TEXT("_cdpath");
TCHAR UNLABELED[] = _TEXT("unlabeled");
TCHAR VOLUME_LABEL[] = _TEXT(" Volume in drive %c is %-12s");

TCHAR VOLUME_SERIAL[] = _TEXT("   Serial number is %04lx:%04lx");
TCHAR NO_PATH[] = _TEXT("No PATH");

TCHAR BYTES_FREE[] = _TEXT("%15s bytes free\r\n");

TCHAR LBYTES_FREE[] = _TEXT("%15Lu bytes free\r\n");

TCHAR CODE_PAGE[] = _TEXT("Active code page: %u\r\n");
TCHAR COUNTRY_CODE[] = _TEXT("Current country code: %u\r\n");

TCHAR LOG_IS[] = _TEXT("LOG (%s) is %s\r\n");

TCHAR BREAK_IS[] = _TEXT("BREAK is %s\r\n");
TCHAR LFNFOR_IS[] = _TEXT("LFNFOR is %s\r\n");

TCHAR VERIFY_IS[] = _TEXT("VERIFY is %s\r\n");
TCHAR IDLE_IS[] = _TEXT("IDLE detection is %s\r\n");

TCHAR SETDOS_IS[] = _TEXT("ANSI=%d\r\nBRIGHTBG=%d\r\nCOMPOUND=%c\r\nDESCRIPTIONS=%u  (%s)\r\nESCAPE=%c\r\n\
EVAL=%d%c%d\r\nEXPANSION=%s\r\nINPUT=%u\r\nMODE=%u\r\nNOCLOBBER=%u\r\nPARAMETERS=%c\r\nROWS=%u\r\nCURSOR OVERSTRIKE=%d\r\nCURSOR INSERT=%d\r\nUPPER CASE=%u\r\nVERBOSE=%u\r\nSWITCH=%c\r\nSINGLESTEP=%u\r\n");
TCHAR LOG_FILENAME[] = _TEXT("4DOSLOG");
TCHAR HLOG_FILENAME[] = _TEXT("4DOSHLOG");

TCHAR GLOBAL_DIR[] = _TEXT("\r\nGLOBAL: %s");
TCHAR TIMER_NUMBER[] = _TEXT("Timer %d ");
TCHAR TIMER_ON[] = _TEXT("on: %s\r\n");
TCHAR TIMER_OFF[] = _TEXT("off: %s ");
TCHAR TIMER_SPLIT[] = _TEXT("%u%c%02u%c%02u%c%02u");
TCHAR TIMER_ELAPSED[] = _TEXT(" Elapsed: %s\r\n");
TCHAR TIME_FMT[] = _TEXT("%2u%c%02u%c%02u%c");
TCHAR DATE_FMT[] = _TEXT("%u%*1s%u%*1s%u");

TCHAR NEW_DATE[] = _TEXT("\r\nNew date (%s): ");
TCHAR NEW_TIME[] = _TEXT("\r\nNew time (hh%cmm%css): ");
TCHAR WHICH_UNKNOWN_CMD[] = _TEXT("%s is an unknown command\r\n");
TCHAR WHICH_EXTERNAL[] = _TEXT("%s is an external : %s\r\n");
TCHAR WHICH_EXECUTABLE_EXT[] = _TEXT("%s is an executable extension : %Fs %s\r\n");
TCHAR WHICH_BATCH[] = _TEXT("%s is a batch file : %s\r\n");
TCHAR WHICH_INTERNAL[] = _TEXT("%s is an internal command\r\n");

#else			// DEFINE_GLOBALS

extern TCHAR ON[];
extern TCHAR OFF[];
extern TCHAR WILD_FILE[];
extern TCHAR WILD_EXT[];
extern TCHAR WILD_CHARS[];
extern TCHAR QUOTES[];
extern TCHAR QUOTES_PARENS[];
extern TCHAR BACK_QUOTE[];
extern TCHAR NULLSTR[];
extern TCHAR SLASHES[];
extern TCHAR WHITESPACE[];

extern char SCAN_NOCR[];

extern TCHAR FMT_STR[];
extern TCHAR FMT_PREC_STR[];
extern TCHAR FMT_LEFT_STR[];
extern TCHAR FMT_TWO_EQUAL_STR[];
extern TCHAR FMT_DOUBLE_STR[];
extern TCHAR FMT_PATH_STR[];
extern TCHAR FMT_STR_CRLF[];
extern TCHAR FMT_STR_TO_STR[];
extern TCHAR FMT_FAR_STR[];
extern TCHAR FMT_FAR_PREC_STR[];
extern TCHAR FMT_FAR_LEFT_STR[];
extern TCHAR FMT_FAR_STR_CRLF[];
extern TCHAR FMT_CHAR[];
extern TCHAR FMT_INT[];
extern TCHAR FMT_UINT[];
extern TCHAR FMT_LONG[];
extern TCHAR FMT_ULONG[];
extern TCHAR FMT_QUAD[];
extern TCHAR FMT_FAT32FREE[];
extern TCHAR FMT_UINT_LEN[];
extern TCHAR FMT_DISK[];
extern TCHAR FMT_ROOT[];
extern TCHAR FMT_PATH[];

extern TCHAR *executables[];

#define COM executables[0]
#define EXE executables[1]
#define BTM executables[2]
#define BAT executables[3]

extern TCHAR *daytbl[];
extern TCHAR *montbl[];
extern TCHAR *lmontbl[];
extern TCHAR *dateformat[];
extern TCHAR *video_type[];
extern TCHAR *swap_mode[];

extern COLORD colorize_atts[];
extern ANSI_COLORS colors[];
extern TCHAR *int_4dos_errors[];

// USAGE messages
extern TCHAR ACTIVATE_USAGE[];
extern TCHAR ALIAS_USAGE[];
extern TCHAR ASSOC_USAGE[];
extern TCHAR ATTRIB_USAGE[];
extern TCHAR BEEP_USAGE[];
extern TCHAR BREAK_USAGE[];
extern TCHAR CALL_USAGE[];
extern TCHAR CDD_USAGE[];
extern TCHAR COLOR_USAGE[];
extern TCHAR COPY_USAGE[];
extern TCHAR CTTY_USAGE[];
extern TCHAR DATE_USAGE[];
extern TCHAR DELETE_USAGE[];
extern TCHAR DESCRIBE_USAGE[];
extern TCHAR DIR_USAGE[];
extern TCHAR DIRHISTORY_USAGE[];
extern TCHAR DO_USAGE[];
extern TCHAR DRAWBOX_USAGE[];
extern TCHAR DRAWHLINE_USAGE[];
extern TCHAR DRAWVLINE_USAGE[];
extern TCHAR ESET_USAGE[];
extern TCHAR EVENTLOG_USAGE[];
extern TCHAR EXCEPT_USAGE[];
extern TCHAR FFIND_USAGE[];
extern TCHAR FOR_USAGE[];
extern TCHAR FUNCTION_USAGE[];
extern TCHAR GLOBAL_USAGE[];
extern TCHAR HEAD_USAGE[];
extern TCHAR HISTORY_USAGE[];
extern TCHAR IDLE_USAGE[];
extern TCHAR IF_USAGE[];
extern TCHAR IFF_USAGE[];
extern TCHAR INKEY_USAGE[];
extern TCHAR INPUT_USAGE[];
extern TCHAR KEYBD_USAGE[];
extern TCHAR KEYSTACK_USAGE[];
extern TCHAR LIST_USAGE[];
extern TCHAR LOG_USAGE[];
extern TCHAR LFNFOR_USAGE[];
extern TCHAR LOADHIGH_USAGE[];
extern TCHAR LOADBTM_USAGE[];
extern TCHAR MD_USAGE[];
extern TCHAR MKLNK_USAGE[];
extern TCHAR MOVE_USAGE[];
extern TCHAR QUERYBOX_USAGE[];
extern TCHAR ON_USAGE[];
extern TCHAR PLAYAVI_USAGE[];
extern TCHAR PLAYSOUND_USAGE[];
extern TCHAR POPD_USAGE[];
extern TCHAR PRINT_USAGE[];
extern TCHAR RD_USAGE[];
extern TCHAR REBOOT_USAGE[];
extern TCHAR RENAME_USAGE[];
extern TCHAR SCREEN_USAGE[];
extern TCHAR SCRPUT_USAGE[];
extern TCHAR SELECT_USAGE[];
extern TCHAR SETERROR_USAGE[];
extern TCHAR SET_USAGE[];
extern TCHAR SWAPPING_USAGE[];
extern TCHAR TAIL_USAGE[];
extern TCHAR TEE_USAGE[];
extern TCHAR TIME_USAGE[];
extern TCHAR TIMER_USAGE[];
extern TCHAR TITLE_USAGE[];
extern TCHAR TOUCH_USAGE[];
extern TCHAR TRANSIENT_USAGE[];
extern TCHAR TREE_USAGE[];
extern TCHAR TRUENAME_USAGE[];
extern TCHAR WHICH_USAGE[];
extern TCHAR TYPE_USAGE[];
extern TCHAR UNALIAS_USAGE[];
extern TCHAR UNFUNCTION_USAGE[];
extern TCHAR UNSET_USAGE[];
extern TCHAR VER_USAGE[];
extern TCHAR VERIFY_USAGE[];
extern TCHAR VSCRPUT_USAGE[];
extern TCHAR WINDOW_USAGE[];


// BATCH.C
extern TCHAR DEBUGGER_PROMPT[];
extern TCHAR ENDTEXT[];
extern TCHAR ECHO_IS[];
extern TCHAR LOADBTM_IS[];
extern TCHAR PAUSE_PAGE_PROMPT[];
extern TCHAR PAUSE_PAGE_CLEAR[];
extern TCHAR PAUSE_PROMPT[];
extern TCHAR DO_DO[];
extern TCHAR DO_BY[];
extern TCHAR DO_FOREVER[];
extern TCHAR DO_LEAVE[];
extern TCHAR DO_ITERATE[];
extern TCHAR DO_END[];
extern TCHAR DO_WHILE[];
extern TCHAR DO_UNTIL[];
extern TCHAR FOR_IN[];
extern TCHAR FOR_DO[];
extern TCHAR IF_NOT[];
extern TCHAR IF_OR[];
extern TCHAR IF_XOR[];
extern TCHAR IF_AND[];

extern TCHAR IF_DEFINED[];
extern TCHAR IF_DIREXIST[];
extern TCHAR IF_EXIST[];
extern TCHAR IF_ISDIR[];
extern TCHAR IF_ISINTERNAL[];
extern TCHAR IF_ISALIAS[];
extern TCHAR IF_ISFUNCTION[];
extern TCHAR IF_ISLABEL[];

extern TCHAR IF_ERRORLEVEL[];
extern TCHAR EQ[];
extern TCHAR GT[];
extern TCHAR GE[];
extern TCHAR LT[];
extern TCHAR LE[];
extern TCHAR NE[];
extern TCHAR EQC[];

extern TCHAR THEN[];
extern TCHAR IFF[];
extern TCHAR ELSEIFF[];
extern TCHAR ELSE[];
extern TCHAR ENDIFF[];
extern TCHAR ON_BREAK[];
extern TCHAR ON_ERROR[];
extern TCHAR ON_ERRORMSG[];


// DIRCMDS.C
extern TCHAR ONE_FILE[];
extern TCHAR MANY_FILES[];
extern TCHAR ONE_DIR[];
extern TCHAR MANY_DIRS[];
extern TCHAR DIRECTORY_OF[];
// extern TCHAR DIRECTORY_DESCRIPTION[];
extern TCHAR DIR_FILE_SIZE[];
extern TCHAR DIR_BYTES_IN_FILES[];
extern TCHAR DIR_BYTES_ALLOCATED[];
extern TCHAR DIR_BYTES_FREE[];
extern TCHAR DIR_TOTAL[];
extern TCHAR DIR_LABEL[];
extern TCHAR DIR_JUNCTION_LABEL[];
extern TCHAR DIR_PERCENT_RATIO[];

extern TCHAR DIR_RATIO[];
extern TCHAR DIR_AVERAGE_RATIO[];
extern TCHAR DIR_AVERAGE_PERCENT_RATIO[];

extern TCHAR LFN_DIR_LABEL[];
extern TCHAR LFN_DIR_JUNCTION_LABEL[];
extern TCHAR COLORDIR[];
extern TCHAR DESCRIBE_PROMPT[];


// xxxCALLS.C
extern TCHAR CLIP[];

extern TCHAR COMMAND_COM[];
extern TCHAR HELP_EXE[];
extern TCHAR HELP_TEXT[];
extern TCHAR HELP_NX[];
extern TCHAR OPTION_EXE[];


// xxxCMDS.C

extern TCHAR KBD_CAPS_LOCK[];
extern TCHAR KBD_NUM_LOCK[];
extern TCHAR KBD_SCROLL_LOCK[];
extern TCHAR ENTER_STANDBY[];
extern TCHAR TURN_MONITOR_OFF[];
extern TCHAR REBOOT_IT[];
extern TCHAR SHUT_IT_DOWN[];

extern TCHAR START_BG_STR[];
extern TCHAR START_DOS_STR[];
extern TCHAR START_ICON_STR[];

extern TCHAR START_TRANSIENT_STR[];
extern TCHAR START_FS_STR[];
extern TCHAR START_INV_STR[];
extern TCHAR START_KEEP_STR[];
extern TCHAR START_MAX_STR[];
extern TCHAR START_MIN_STR[];
extern TCHAR START_PGM_STR[];
extern TCHAR START_WIN_STR[];

extern TCHAR START_NO_STR[];
extern TCHAR WIN_OS2[];
extern TCHAR START_FG_STR[];
extern TCHAR START_PM_STR[];
extern TCHAR START_TTY[];
extern TCHAR START_WIN3_STR[];

extern TCHAR START_LOCAL_STR[];
extern TCHAR START_LA_STR[];
extern TCHAR START_LD_STR[];
extern TCHAR START_LF_STR[];
extern TCHAR START_LH_STR[];
extern TCHAR START_POS_STR[];

extern TCHAR LOCK_WARNING[];
extern TCHAR WARNING_NO_REGIONS[];
extern TCHAR WARNING_INVALID_REGION[];
extern TCHAR SWAPPING_IS[];
extern TCHAR TOTAL_EMS[];
extern TCHAR TOTAL_EXTENDED[];
extern TCHAR XMS_FREE[];
extern TCHAR TOTAL_DOS_RAM[];
extern TCHAR HMA_MEMORY[];
extern TCHAR HMA_FREE[];
extern TCHAR HMA_USED[];

extern TCHAR TOTAL_ENVIRONMENT[];

extern TCHAR TOTAL_ALIAS[];
extern TCHAR TOTAL_FUNCTION[];
extern TCHAR TOTAL_HISTORY[];
extern TCHAR TOTAL_DISK_USED[];
extern TCHAR DISK_PERCENT[];


// xxxINIT.C
extern TCHAR COPYRIGHT[];
extern TCHAR COMSPEC[];
extern TCHAR TAILIS[];

extern TCHAR PROGRAM[];

extern TCHAR RUNTIME_ERROR[];

extern TCHAR NAME_HEADER[];
extern TCHAR SET_PROGRAM[];
extern TCHAR *SHORT_NAME;
extern TCHAR DOS_VERSION[];
extern TCHAR UNKVER[];
extern TCHAR MSVER[];
extern TCHAR LZVER[];
extern TCHAR MS95VER[];
extern TCHAR MS98VER[];
extern TCHAR MSMEVER[];
extern TCHAR OS2VER[];
extern TCHAR WARPVER[];
extern TCHAR DRVER[];
extern TCHAR NOVVER[];
extern TCHAR IBMVER[];
extern TCHAR WENVER[];
extern TCHAR XDVER[];
extern TCHAR FDVER[];
extern TCHAR RDVER[];
extern TCHAR GSVER[];
extern TCHAR RXVER[];
extern TCHAR SDVER[];
extern TCHAR PTSVER[];
extern TCHAR DOS_REVISION[];
extern TCHAR OS2_REVISION[];
extern TCHAR DOS_LOCATION[];
extern TCHAR DOS_HMA[];
extern TCHAR DOS_ROM[];
extern TCHAR DOS_LOW[];
extern TCHAR COMSPEC_DOS[];
extern TCHAR *DOS_NAME;
extern TCHAR FOURDOS_REV[];


// ERROR.C
extern TCHAR USAGE_MSG[];
extern TCHAR FILE_SPEC[];
extern TCHAR PATH_SPEC[];
extern TCHAR COLOR_SPEC[];


// EXPAND.C
extern TCHAR PATH_VAR[];
extern TCHAR PATHEXT[];

extern TCHAR MONO_MONITOR[];
extern TCHAR COLOR_MONITOR[];
extern TCHAR SVGA_TYPE[];

extern TCHAR END_OF_FILE_STR[];

extern TCHAR *ACList[];
extern TCHAR *BatteryList[];

extern TCHAR TEMP_DISK[];
extern TCHAR TMP_DISK[];
extern TCHAR TEMP4DOS_DISK[];


// defines for internal variable array
enum InternalVariables {
	VAR_4VER,
	VAR_IERRORLEVEL,
	VAR_ALIAS,
	VAR_ALT,
	VAR_ANSI,
	VAR_APMAC,
	VAR_APMBATT,
	VAR_APMLIFE,
	VAR_APPEND,
	VAR_ASSIGN,
	VAR_BATCH,
	VAR_BATCHLINE,
	VAR_BATCHNAME,
	VAR_BATCHTYPE,
	VAR_BDEBUGGER,
	VAR_BG_COLOR,
	VAR_BOOT,
	VAR_BUILD,
	VAR_CAPSLOCK,
	VAR_CDROMS,
	VAR_CI,
	VAR_CMDLINE,
	VAR_CMDPROC,
	VAR_CMDSPEC,
	VAR_CO,
	VAR_CODEPAGE,
	VAR_COLUMN,
	VAR_COLUMNS,
	VAR_COUNTRY,
	VAR_CPU,
	VAR_CPUSPEED,
	VAR_CTRL,
	VAR_CWD,
	VAR_CWDS,
	VAR_CWP,
	VAR_CWPS,
	VAR_DATE,
	VAR_DATETIME,
	VAR_DAY,
	VAR_DISK,
	VAR_DISPLAY,
	VAR_DNAME,
	VAR_DOS,
	VAR_DOSVER,
	VAR_DOW,
	VAR_DOWF,
	VAR_DOWI,
	VAR_DOY,
	VAR_DPMI,
	VAR_DRIVER,
	VAR_DRIVES,
	VAR_DST,
	VAR_DV,
	VAR_ECHO,
	VAR_EDITMODE,
	VAR_EGA,
	VAR_ENVIRONMENT,
	VAR_EXECSTR,
	VAR_EXPANSION,
	VAR_FG_COLOR,
	VAR_FONTPAGE,
	VAR_GRAFTABL,
	VAR_GRAPHICS,
	VAR_HDRIVES,
	VAR_HLOGFILE,
	VAR_HOUR,
	VAR_IDOW,
	VAR_IDOWF,
	VAR_ININAME,
	VAR_ISODATE,
	VAR_ISODOWI,
	VAR_ISORDATE,
	VAR_ISOWDATE,
	VAR_ISOWEEK,
	VAR_ISOWYEAR,
	VAR_KBHIT,
	VAR_KEYSTACKED,
	VAR_KSTACK,
	VAR_LALT,
	VAR_LASTDIR,
	VAR_LASTDISK,
	VAR_LCTRL,
	VAR_LOGFILE,
	VAR_LSHIFT,
	VAR_MACHINE,
	VAR_MINUTE,
	VAR_MJD,
	VAR_MONITOR,
	VAR_MONTH,
	VAR_MONTHF,
	VAR_MOUSE,
	VAR_MSCDEX,
	VAR_NDP,
	VAR_NETWORK,
	VAR_NLSFUNC,
	VAR_NUMLOCK,
	VAR_PIPE,
	VAR_POWER,
	VAR_PRINT,
	VAR_RALT,
	VAR_RCTRL,
	VAR_READY,
	VAR_ROW,
	VAR_ROWS,
	VAR_RSHIFT,
	VAR_SBDSP,
	VAR_SCROLLLOCK,
	VAR_SECOND,
	VAR_SHARE,
	VAR_SHELL,
	VAR_SHIFT,
	VAR_SMARTDRV,
	VAR_STARTPATH,
	VAR_STDERR,
	VAR_STDIN,
	VAR_STDOUT,
	VAR_STZN,
	VAR_STZO,
	VAR_SWAPPING,
	VAR_SYSERR,
	VAR_SYSREQ,
	VAR_TASKMAX,
	VAR_TASKSWITCHER,
	VAR_TICK,
	VAR_TIME,
	VAR_TRANSIENT,
	VAR_TSC,
	VAR_TZN,
	VAR_TZO,
	VAR_UNIXTIME,
	VAR_UTCDATE,
	VAR_UTCDATETIME,
	VAR_UTCHOUR,
	VAR_UTCISODATE,
	VAR_UTCMINUTE,
	VAR_UTCSECOND,
	VAR_UTCTIME,
	VAR_V86,
	VAR_VCPI,
	VAR_VDS,
	VAR_VERMAJOR,
	VAR_VERMINOR,
	VAR_VERSION,
	VAR_VIDEO,
	VAR_WIN,
	VAR_WINTICKS,
	VAR_WINTITLE,
	VAR_YEAR
};
	
// defines for variable function array
enum InternalFunctions {
	FUNC_ABS,
	FUNC_AGEDATE,
	FUNC_ALIAS,
	FUNC_ALTNAME,
	FUNC_ASCII,
	FUNC_ATTRIB,
	FUNC_AVERAGE,
	FUNC_CAPS,
	FUNC_CDROM,
	FUNC_CEILING,
	FUNC_CHAR,
	FUNC_CLIP,
	FUNC_CLIPW,
	FUNC_CLUSTSIZE,
	FUNC_CODEPAGE,
	FUNC_COM,
	FUNC_COMMA,
	FUNC_COMPARE,
	FUNC_CONVERT,
	FUNC_COUNT,
	FUNC_CRC32,
	FUNC_CWD,
	FUNC_CWDS,
	FUNC_DATE,
	FUNC_DATECONV,
	FUNC_DAY,
	FUNC_DDCSTR,
	FUNC_DEC,
	FUNC_DECIMAL,
	FUNC_DESCRIPT,
	FUNC_DEVICE,
	FUNC_DIGITS,
	FUNC_DIRSTACK,
	FUNC_DISKFREE,
	FUNC_DISKTOTAL,
	FUNC_DISKUSED,
	FUNC_DOSMEM,
	FUNC_DOW,
	FUNC_DOWF,
	FUNC_DOWI,
	FUNC_DOY,
	FUNC_DRIVETYPE,
	FUNC_EMS,
	FUNC_ERRTEXT,
	FUNC_EVAL,
	FUNC_EXECUTE,
	FUNC_EXECSTR,
	FUNC_EXPAND,
	FUNC_EXTENSION,
	FUNC_EXTENDED,
	FUNC_FIELD,
	FUNC_FIELDS,
	FUNC_FILEAGE,
	FUNC_FILECLOSE,
	FUNC_FILEDATE,
	FUNC_FILENAME,
	FUNC_FILEOPEN,
	FUNC_FILEREAD,
	FUNC_FILEREADB,
	FUNC_FILES,
	FUNC_FILESEEK,
	FUNC_FILESEEKL,
	FUNC_FILESIZE,
	FUNC_FILETIME,
	FUNC_FILEWRITE,
	FUNC_FILEWRITEB,
	FUNC_FINDCLOSE,
	FUNC_FINDFIRST,
	FUNC_FINDNEXT,
	FUNC_FLOOR,
	FUNC_FORMAT,
	FUNC_FSTYPE,
	FUNC_FULLNAME,
	FUNC_FUNCTION,
	FUNC_HDDSIZE,
	FUNC_HISTORY,
	FUNC_IF,
	FUNC_INC,
	FUNC_INDEX,
	FUNC_INIREAD,
	FUNC_INIWRITE,
	FUNC_INSERT,
	FUNC_INSTR,
	FUNC_INTEGER,
	FUNC_ISALNUM,
	FUNC_ISALPHA,
	FUNC_ISASCII,
	FUNC_ISCNTRL,
	FUNC_ISDIGIT,
	FUNC_ISLOWER,
	FUNC_ISODOWI,
	FUNC_ISOWEEK,
	FUNC_ISOWYEAR,
	FUNC_ISPRINT,
	FUNC_ISPUNCT,
	FUNC_ISSPACE,
	FUNC_ISUPPER,
	FUNC_ISXDIGIT,
	FUNC_LABEL,
	FUNC_LCS,
	FUNC_LEFT,
	FUNC_LENGTH,
	FUNC_LFN,
	FUNC_LINE,
	FUNC_LINES,
	FUNC_LOWER,
	FUNC_LPT,
	FUNC_LTRIM,
	FUNC_MAKEAGE,
	FUNC_MAKEDATE,
	FUNC_MAKETIME,
	FUNC_MASTER,
	FUNC_MAX,
	FUNC_MD5,
	FUNC_MIN,
	FUNC_MONTH,
	FUNC_MONTHF,
	FUNC_MOUSE,
	FUNC_NAME,
	FUNC_NUMERIC,
	FUNC_PATH,
	FUNC_QUOTE,
	FUNC_RANDOM,
	FUNC_READSCR,
	FUNC_READY,
	FUNC_REMOTE,
	FUNC_REMOVABLE,
	FUNC_REPEAT,
	FUNC_REPLACE,
	FUNC_REVERSE,
	FUNC_RIGHT,
	FUNC_RTRIM,
	FUNC_SEARCH,
	FUNC_SELECT,
	FUNC_SERIAL,
	FUNC_SFN,
	FUNC_SHA1,
	FUNC_SIMILAR,
	FUNC_SMBSTR,
	FUNC_STRIP,
	FUNC_SUBST,
	FUNC_SUBSTR,
	FUNC_TIME,
	FUNC_TIMER,
	FUNC_TRIM,
	FUNC_TRUENAME,
	FUNC_TRUNCATE,
	FUNC_UNIQUE,
	FUNC_UNQUOTE,
	FUNC_UNQUOTES,
	FUNC_UPPER,
	FUNC_WILD,
	FUNC_WORD,
	FUNC_WORDS,
	FUNC_XMS,
	FUNC_YEAR
};


// FILECMDS.C

extern TCHAR REPLACE[];
extern TCHAR FILES_COPIED[];
extern TCHAR MOVE_CREATE_DIR[];
extern TCHAR FILES_MOVED[];
extern TCHAR FILES_RENAMED[];
extern TCHAR FILES_DELETED[];
extern TCHAR FILES_BYTES_FREED[];
extern TCHAR ARE_YOU_SURE[];
extern TCHAR DELETE_QUERY[];
extern TCHAR DELETING_FILE[];


// INIPARSE.C
extern TCHAR INI_ERROR[];
extern TCHAR INI_QUERY[];


// LINES.C
extern TCHAR BOX_FILL[];
extern TCHAR BOX_SHADOW[];
extern TCHAR BOX_ZOOM[];


// LIST.C
extern TCHAR FFIND_FILE_HEADER[];
extern TCHAR FFIND_TEXT_FOUND[];
extern TCHAR FFIND_ONE_LINE[];
extern TCHAR FFIND_MANY_LINES[];
extern TCHAR FFIND_FOUND[];

extern TCHAR FFIND_OFFSET[];
extern TCHAR FFIND_CONTINUE[];
extern TCHAR LIST_STDIN_MSG[];
extern TCHAR LIST_DELETE[];
extern TCHAR LIST_DELETE_TITLE[];
extern TCHAR LIST_INFO_FAT[];
extern TCHAR LIST_INFO_LFN[];
extern TCHAR LIST_INFO_PIPE[];
extern TCHAR LIST_LINE[];
extern TCHAR LIST_NOT_FOUND[];
extern TCHAR LIST_GOTO[];
extern TCHAR LIST_GOTO_OFFSET[];

extern TCHAR LIST_HEADER[];
extern TCHAR LIST_WAIT[];
extern TCHAR LIST_GOTO_TITLE[];
extern TCHAR LIST_FIND[];
extern TCHAR LIST_FIND_WAIT[];
extern TCHAR LIST_FIND_TITLE[];
extern TCHAR LIST_FIND_TITLE_REVERSE[];
extern TCHAR LIST_FIND_BTITLE_CASE[];
extern TCHAR LIST_FIND_HEX[];
extern TCHAR LIST_PRINT_TITLE[];
extern TCHAR LIST_SAVE_TITLE[];
extern TCHAR LIST_QUERY_PRINT[];
extern TCHAR LIST_QUERY_SAVE[];
extern TCHAR LIST_PRINTING[];
extern TCHAR LIST_TABSIZE[];
extern TCHAR LIST_TABSIZE_TITLE[];


// MISC.C
extern TCHAR NO_SEP[];
extern TCHAR DESCRIPTION_FILE[];
extern TCHAR DESCRIPTION_SCAN[];

// extern TCHAR IFILE[];
extern TCHAR CONSOLE[];
extern TCHAR DEV_CONSOLE[];
extern LPTSTR PRINTERS[];
extern TCHAR YES_NO[];
extern TCHAR YES_NO_ALL[];
extern TCHAR BRIGHT[];
extern TCHAR BLINK[];
extern TCHAR BORDER[];


// PARSER.C
extern TCHAR CANCEL_BATCH_JOB[];
extern TCHAR INSERT_DISK[];
extern TCHAR UNKNOWN_COMMAND[];
extern TCHAR COMMAND_GROUP_MORE[];
extern TCHAR CMDLINE_VAR[];
extern TCHAR PROMPT_NAME[];
extern TCHAR TITLE_PROMPT[];
extern TCHAR DOS_HDR[];


// SCREENIO.C
extern TCHAR HISTORY_TITLE[];
extern TCHAR DIRECTORY_TITLE[];
extern TCHAR FILENAMES_TITLE[];
extern TCHAR VARIABLES_TITLE[];


// SELECT.C
extern TCHAR MARKED_FILES[];
extern TCHAR SELECT_HEADER[];
extern TCHAR SELECT_PAGE_COUNT[];


// SYSCMDS.C
extern TCHAR BUILDING_INDEX[];
extern TCHAR CDPATH[];
extern TCHAR UNLABELED[];
extern TCHAR VOLUME_LABEL[];

extern TCHAR VOLUME_SERIAL[];
extern TCHAR NO_PATH[];
extern TCHAR BYTES_FREE[];
extern TCHAR LBYTES_FREE[];
extern TCHAR LCHARS_FREE[];
extern TCHAR LOG_FILENAME[];
extern TCHAR HLOG_FILENAME[];
extern TCHAR LOG_IS[];

extern TCHAR BREAK_IS[];
extern TCHAR LFNFOR_IS[];

extern TCHAR VERIFY_IS[];
extern TCHAR IDLE_IS[];

extern TCHAR KEYS_IS[];
extern TCHAR SETDOS_IS[];
extern TCHAR GLOBAL_DIR[];
extern TCHAR TIMER_NUMBER[];
extern TCHAR TIMER_ON[];
extern TCHAR TIMER_OFF[];
extern TCHAR TIMER_SPLIT[];
extern TCHAR TIMER_ELAPSED[];
extern TCHAR TIME_FMT[];
extern TCHAR DATE_FMT[];

extern TCHAR NEW_DATE[];
extern TCHAR NEW_TIME[];
extern TCHAR WHICH_UNKNOWN_CMD[];
extern TCHAR WHICH_ASSOCIATED[];
extern TCHAR WHICH_EXTERNAL[];
extern TCHAR WHICH_EXECUTABLE_EXT[];
extern TCHAR WHICH_BATCH[];
extern TCHAR WHICH_EXTPROC[];
extern TCHAR WHICH_REXX[];
extern TCHAR WHICH_URL[];
extern TCHAR WHICH_INTERNAL[];

extern TCHAR CODE_PAGE[];
extern TCHAR COUNTRY_CODE[];

#endif /* DEFINE_GLOBALS */
