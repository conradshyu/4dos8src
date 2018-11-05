/*
 *************************************************************************
 ** WRTTEST.C
 *************************************************************************
 ** Test INI file write
 **
 ** Copyright 1996 JP Software Inc., All rights reserved
 *************************************************************************
 */

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "typedefs.h"
#include "iniio.h"
#include "general.h"


/*
 *************************************************************************
 ** Main
 *************************************************************************
 */
void main(int argc, char *argv[]) {
	int i;


	if (argc < 4) {
		printf("Usage: WTEST section directive value ini_filename\n\n");
		return;
	}

	// If parm = "N", replace with NULL
	for (i = 0; i <= argc; i++)
		if ((*argv[i] == 'N') && (*(argv[i] + 1) == '\0'))
			argv[i] = NULL;


	printf("About to call write function\n");

	WriteINIFileStr(argv[1], argv[2], argv[3], argv[4]);
	//WriteINIFileStr("SectionName", "EdsParam", "DINK", "test.ini");
	//WriteINIFileStr("SectionName", "EdsParam", "DINK", "c:\temp\test.ini");

	printf("All done, man.\n");


}
