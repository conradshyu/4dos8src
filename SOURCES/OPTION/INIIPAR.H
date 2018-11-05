/*
 *************************************************************************
 ** iniipar.h
 *************************************************************************
 ** Include file for iparse.c
 **
 ** Copyright (c) 1997  JP Software, All rights reserved
 *************************************************************************
 */

#ifndef INIIPAR_H
#define INIIPAR_H

#define SLASHES           "\\/"
#define BORDER            "Bor"
#define FMT_INT           "%d"
#define FMT_UINT          "%u"


// IPARSE.C
void IniClear(INIFILE *);
int IniLine(char *, INIFILE *, int, int, int, char **);
int next_token(char **, int, int, char *, char *);


#endif  // INIIPAR_H
