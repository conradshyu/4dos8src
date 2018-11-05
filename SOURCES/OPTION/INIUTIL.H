/*
 *************************************************************************
 ** iniutil.h
 *************************************************************************
 ** Copyright (c) 1997  JP Software, All rights reserved
 *************************************************************************
 */

#ifndef INIUTIL_H
#define INIUTIL_H

/*
 *************************************************************************
 ** Prototypes
 *************************************************************************
 */
void strip_leading(char *arg, char *delims);
void strip_trailing(register char *arg, char *delims);
void FatalError(int nExitRC, char *pszMessage);
char *skipspace( char *line );
char *StrEnd( char *s );
char *ParseColors( char *line, int *nFG, int *nBG );
int color_shade( char *arg );
void copy_filename(char *target, char *source );
char *path_part( char *s );
int is_dir( char *pszFileName );
int _ctoupper( int c );
int ConcatDirFileNames(char *pszDirName, char *pszFileName);

#endif  // INIUTIL_H
