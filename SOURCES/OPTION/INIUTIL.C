/*
 *************************************************************************
 ** INIUTIL.C
 *************************************************************************
 ** General utility routines
 **
 ** Copyright 1996 JP Software Inc., All rights reserved
 *************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>

#include "typedefs.h"
#include "general.h"
#include "product.h"
#include "argparse.h"
#include "iniutil.h"

#define NULLSTR ""

#ifndef ANSI_COLORS
// structure used by COLOR to set screen colors via ANSI escape sequences
typedef struct
{
	unsigned char *shade;
	unsigned char ansi;
} ANSI_COLORS;
#endif


/*
 *************************************************************************
 ** strip_leading
 *************************************************************************
 ** Strip the specified leading characters
 *************************************************************************
 */
void strip_leading( char *arg, char *delims ) {
	int i;


	for (i = 0; (*(arg + i) != '\0') && (strchr(delims, *(arg + i)) != NULL); i++)
		;

	if (i > 0)
		memmove(arg, arg + i, strlen(arg + i) + 1);
}


/*
 *************************************************************************
 ** strip_trailing
 *************************************************************************
 ** Strip the specified trailing characters
 *************************************************************************
 */
void strip_trailing(register char *arg, char *delims) {
	register int i;


	for ( i = strlen( arg );
	     ((--i >= 0 ) && ( strchr( delims, arg[i] ) != NULL )); )
		arg[i] = '\0';
}


/*
 *************************************************************************
 ** skipspace
 *************************************************************************
 ** Skip past leading white space and return pointer to first non-space char
 *************************************************************************
 */
char *skipspace( char *line )
{
	while (( *line == ' ' ) || ( *line == '\t' ))
		line++;

	return line;
}


/*
 *************************************************************************
 ** StrEnd
 *************************************************************************
 ** Return a pointer to the end of the string
 *************************************************************************
 */
char *StrEnd( char *s )
{
	return ( s + strlen( s ));
}


/*
 *************************************************************************
 ** FatalError
 *************************************************************************
 ** Print error message and exit program
 *************************************************************************
 */
void FatalError(int nExitRC, char *pszMessage) {

	printf("\n\nERROR: ");

#ifdef DEBUG
	perror(pszMessage);
#else
	printf(pszMessage);
#endif

	printf("\n");

	fflush(stdout);
	exit(nExitRC);
}  // End FatalError



/*
 *************************************************************************
 ** ParseColors
 *************************************************************************
 ** Get foreground & background attributes from an ASCII string
 *************************************************************************
 */
char *ParseColors( char *line, int *nFG, int *nBG )
{
	register char *arg;
	register int i, nIntensity = 0, nAttrib;
	char *nthptr;


	for ( ; ; ) {

		if (( arg = NthArg( line, 0, &nthptr)) == NULL )
			return NULL;

		if ( _strnicmp( arg, "Bri ", 3 ) == 0 ) {
			// set intensity bit
			nIntensity |= 0x08;
		} else if ( _strnicmp( arg, "Bli ", 3 ) == 0 ) {
			// set blinking bit
			nIntensity |= 0x80;
		} else
			break;

		// skip BRIGHT or BLINK
		line = (( NthArg( line, 1, &nthptr) != NULL ) ? nthptr : NULLSTR);
	}

	// check for foreground color match
	if (( nAttrib = color_shade( arg )) <= 15)
		*nFG = nIntensity + nAttrib;

	// "ON" is optional
	i = 1;
	if ((( arg = NthArg( line, 1, &nthptr)) != NULL ) && ( _stricmp( arg, "ON" ) == 0 ))
		i++;

	// check for BRIGHT background
	if (((arg = NthArg(line, i, &nthptr)) != NULL) && (_strnicmp(arg, "Bri ", 3) == 0)) {
		nIntensity = 0x08;
		i++;
	} else
		nIntensity = 0;

	// check for background color match
	if ((nAttrib = color_shade(NthArg(line, i, &nthptr))) <= 15 ) {
		*nBG = nAttrib + nIntensity;
		NthArg(line, ++i, &nthptr);
	}

	return nthptr;
}


/*
 *************************************************************************
 ** color_shade
 *************************************************************************
 ** match color against list
 *************************************************************************
 */
int color_shade( char *arg )
{
	register int i;
	// ANSI color sequences (for COLOR)
	ANSI_COLORS colors[] = {
		"Bla",30,
		"Blu",34,
		"Gre",32,
		"Cya",36,
		"Red",31,
		"Mag",35,
		"Yel",33,
		"Whi",37,
		"Bri Bla",0,
		"Bri Blu",0,
		"Bri Gre",0,
		"Bri Cya",0,
		"Bri Red",0,
		"Bri Mag",0,
		"Bri Yel",0,
		"Bri Whi",0
	};


	if ( arg != NULL ) {

		// allow 0-15 as well as Blue, Green, etc.
		if ( isdigit( *arg ))
			return ( atoi( arg ));

		for ( i = 0; ( i <= 7 ); i++ ) {
			// check for color match
			if ( _strnicmp( arg, colors[i].shade, 3 ) == 0 )
				return i;
		}
	}

	return 0xFF;
}


/*
 *************************************************************************
 ** copy_filename
 *************************************************************************
 ** copy a filename, max of 260 characters
 *************************************************************************
 */
void copy_filename(char *target, char *source )
{
	sprintf( target, "%.*s", MAXFILENAME-1, source );
}


/*
 *************************************************************************
 ** path_part
 *************************************************************************
 ** return the path stripped of the filename (or NULL if no path)
 *************************************************************************
 */
char *path_part( char *s )
{
	static char buffer[MAXFILENAME];

	copy_filename( buffer, s );

	// search path backwards for beginning of filename
	for ( s = StrEnd( buffer ); ( --s >= buffer ); ) {

		// accept either forward or backslashes as path delimiters
		if (( *s == '\\' ) || ( *s == '/' ) || ( *s == ':' )) {
			// take care of arguments like "d:.." & "..\.."
			if ( _stricmp( s+1, ".." ) != 0 )
				s[1] = '\0';
			return buffer;
		}
	}

	return NULL;
}


/*
 *************************************************************************
 ** is_dir
 *************************************************************************
 ** Returns 1 if it's a directory, 0 otherwise
 **
 ** Dummy function to replace complex one that has snowballing
 **  dependencies on other 4xxx product specific code
 *************************************************************************
 */
int is_dir( char *pszFileName ) {
	unsigned int uHoldAttr = 0;
	int nRC = 0;


	if (_dos_getfileattr(pszFileName, &uHoldAttr) == 0) {
		// Successful call
		if (uHoldAttr & _A_SUBDIR)
			nRC = 1;
	}

	return nRC;
}  // End is_dir


/*
 *************************************************************************
 ** _ctoupper
 *************************************************************************
 ** Map character to upper case
 ** 
 ** Dummy function to replace assembler code
 ** Will not specifically handle foreign language chars as the assembler
 **  does
 *************************************************************************
 */
int _ctoupper( int c ) {
	return toupper(c);
}


/*
 *************************************************************************
 ** ConcatDirFile
 *************************************************************************
 ** Make a file name from a directory name by appending '\' (if necessary)
 **   and then appending the filename
 *************************************************************************
 */
int ConcatDirFileNames(char *pszDirName, char *pszFileName) {
	int nLen;
	int nRC = FALSE;


	nLen = strlen(pszDirName);

	if (nLen < (MAXFILENAME-2)) {
		if ((*pszDirName) && (strchr( "/\\:", pszDirName[nLen-1]) == NULL )) {
			strcat(pszDirName, "\\");
			nLen++;
		}

		sprintf(StrEnd(pszDirName), "%.*s", ((MAXFILENAME-1)-nLen),
		        pszFileName);
		nRC = TRUE;
	}

	return(nRC);
}  // End ConcatDirFileNames

