/*
 *************************************************************************
 ** ARGPARSE.C
 *************************************************************************
 ** Argument parsing routines
 **
 ** Copyright 1996 JP Software Inc., All rights reserved
 *************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "general.h"
#include "typedefs.h"
#include "iniutil.h"
#include "resource.h"
#include "inistruc.h"
extern INIFILE gaInifile;
#include "argparse.h"

#define BADQUOTES ((char *)-1)  // flag for unmatched quotes in line
#define QUOTES[] = "`\"";


/*
 *************************************************************************
 ** NthArg
 *************************************************************************
 ** Returns the nth argument in the command line (parsing by whitespace
 **   & switches) or NULL if no nth argument exists
 **
 ** NOTE:  This function is heavily modified from the original ntharg()
 **        Do not use this as a direct replacement
 *************************************************************************
 */
char *NthArg(char *line, int index, char **argptr)
{
	static char buf[ARG_MAX + 1];
	static char delims[] = "  \t,";
	int length;
	INIFILE *pIniptr = (INIFILE *)&gaInifile;
	int fNoComma = 0;  


	// Don't use arg pointer if one was not passed
	if (argptr != NULL)
		*argptr = NULL;

	if (line == NULL)
		return NULL;

	delims[0] = pIniptr->SwChr;
	delims[4] = (char)(( fNoComma == 0 ) ? ',' : '\0' );

	for ( ; ; index--) {
		// find start of arg[i]
		line += strspn(line, delims + 1);
		if ((*line == '\0') || (index < 0))
			break;

		// search for next delimiter or switch character
		while (*line == pIniptr->SwChr)
			line++;

		length = strcspn(line, delims);
		if (length == 0)
			break;

		if (index == 0) {
			if (argptr != NULL)
				*argptr = line;

			// this is the argument I want - copy it & return
			sprintf(buf, "%.*s", MIN(length, ARG_MAX), line);
			return buf;
		}

		line += length;
	}  // End for index

	return NULL;
}  // End NthArg
