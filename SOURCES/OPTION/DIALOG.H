/*
 *************************************************************************
 ** dialog.h
 *************************************************************************
 ** Copyright (c) 1996  JP Software, All rights reserved
 *************************************************************************
 */

#ifndef DIALOG_H
#define DIALOG_H

#define CATEGORY_MAX		7
#define B_USE				CATEGORY_MAX + 1
#define B_SAVE				CATEGORY_MAX + 2
#define B_CANCEL			CATEGORY_MAX + 3
#define B_RESUME			CATEGORY_MAX + 4
#define B_HELP_FIELD		CATEGORY_MAX + 5
#define B_HELP_OPTION	CATEGORY_MAX + 6
#define B_HELP_TOC		CATEGORY_MAX + 7
#define B_HELP_INI		CATEGORY_MAX + 8
#define B_HELP_KEYS		CATEGORY_MAX + 9

/*
 *************************************************************************
 ** Prototypes
 *************************************************************************
 */
int DlgOpen(unsigned int uINIControlCount);
int DlgBuildWindow(unsigned int uCategory);
int DlgShowWindow(unsigned int uCategory);
int DlgCloseWindow(unsigned int uCategory);
int DlgClose(void);

int DlgSetHelpExe(char *pszExeName);

int DlgSetFieldDirective(unsigned int uIndex, char *pszName);
int DlgSetFieldVal(unsigned int uIndex, unsigned int uValue);
int DlgSetFieldValRange(unsigned int uIndex, int nMin, int nMax,
                        unsigned int uIncrement);
int DlgSetFieldString(unsigned int uIndex, char *pszString);
int DlgSetComboDefault(unsigned int uIndex, unsigned int uListIndex);
int DlgSetColorDefault(unsigned int uIndex, unsigned int uFGIndex,
                       unsigned int uBGIndex);

int DlgGetFieldVal(unsigned int uIndex, unsigned int *puValue);
int DlgGetFieldString(unsigned int uIndex, char *pszString, int nStringMax);
int DlgGetRadioVal(unsigned int uIndex, unsigned int *puValue);
int DlgGetComboVal(unsigned int uIndex, unsigned int *puValue);
int DlgGetColorVal(unsigned int uIndex, unsigned int *puFGIndex,
                   unsigned int *puBGIndex);

#ifdef DEBUG
int DlgIndexTest(unsigned int uINIControlIndex, unsigned int uCtrlType);
#endif

#endif  // DIALOG_H

