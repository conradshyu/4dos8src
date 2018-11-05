/*
 *************************************************************************
 ** DTEST.C
 *************************************************************************
 ** Test text-based INI dialog
 **
 ** Copyright 1996 JP Software Inc., All rights reserved
 *************************************************************************
 */

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>


#include "product.h" 
#include "typedefs.h"
#include "resource.h"  // IDI_ values
#include "inistruc.h"
INIFILE gaInifile;
#include "inifile.h"
#include "dialog.h"
#include "general.h"

/*
 *************************************************************************
 ** Main
 *************************************************************************
 */
void main(void) {
	int nRC = 0;
	int i;
	char *colors[] = { "Black", "Blue", "Green", "Red", "Yellow", "Orange"};
	int color_max = 6;
	unsigned int uValue = 0;
	unsigned int uValueB = 0;
	char aHold[45];
	int nCategory = 0;
	int nCat = 0;
	unsigned int uHoldMem = 0;


	// Allocate a heap of 64K
	_heapgrow();
	uHoldMem = _memavl();
	printf("Mem avail: %u\n",uHoldMem);

	printf("Dialog open\n"); getchar();
	nRC = DlgOpen(IDI_ID_MAX - IDI_BASE);
	printf("RC = %d\n",nRC);
	printf("Mem avail: %u\n",_memavl());

#ifdef QQQQ
	printf("Dialog build window\n"); getchar();
	nRC = DlgBuildWindow(0);
	printf("RC = %d\n",nRC);
	if (nRC != 0) { getchar(); return;}
	printf("Mem avail: %u\n",_memavl());
	printf("Setting field directives\n"); getchar();

	printf("Dialog build window #2\n"); getchar();
	nRC = DlgBuildWindow(1);
	printf("RC = %d\n",nRC);
	if (nRC != 0) { getchar(); return;}
	printf("Mem avail: %u\n",_memavl());

	printf("Setting radio \n"); getchar();
	nRC = DlgSetFieldVal(IDI_ANSI - IDI_BASE,0,FALSE);
	printf("RC = %d\n",nRC);
	if (nRC != 0) { getchar(); return;}


	printf("Dialog build window #3\n"); getchar();
	nRC = DlgBuildWindow(2);
	printf("RC = %d\n",nRC);
	if (nRC != 0) { getchar(); return;}
	printf("Mem avail: %u\n",_memavl());

	printf("Dialog build window #4\n"); getchar();
	nRC = DlgBuildWindow(3);
	printf("RC = %d\n",nRC);
	if (nRC != 0) { getchar(); return;}
	printf("Mem avail: %u\n",_memavl());

	printf("Dialog build window #5\n"); getchar();
	nRC = DlgBuildWindow(4);
	printf("RC = %d\n",nRC);
	if (nRC != 0) { getchar(); return;}
	printf("Mem avail: %u\n",_memavl());

	printf("Dialog build window #6\n"); getchar();
	nRC = DlgBuildWindow(5);
	printf("RC = %d\n",nRC);
	if (nRC != 0) { getchar(); return;}
	printf("Mem avail: %u\n",_memavl());


//COLORS
	printf("Setting colors \n"); getchar();
	for (i = 0; i<color_max; i++) {
		nRC = DlgSetFieldString(IDI_StdColors - IDI_BASE,colors[i]);
		printf("RC = %d\n",nRC);
		nRC = DlgSetFieldString(IDI_InputColors - IDI_BASE,colors[i]);
		printf("RC = %d\n",nRC);
		nRC = DlgSetFieldString(IDI_HistWinColors - IDI_BASE,colors[i]);
		printf("RC = %d\n",nRC);
	}
	if (nRC != 0) { getchar(); return;}


	printf("Setting default colors (GREEN,RED)\n"); getchar();
	nRC = DlgSetColorDefault(IDI_StdColors - IDI_BASE,2,3);
	printf("RC = %d\n",nRC);
	if (nRC != 0) { getchar(); return;}

	printf("Setting default colors (Black,Blue)\n"); getchar();
	nRC = DlgSetColorDefault(IDI_InputColors - IDI_BASE,0,1);
	printf("RC = %d\n",nRC);
	if (nRC != 0) { getchar(); return;}



	printf("Dialog show window\n"); getchar();
	while (nRC >= 0 && nRC < CATEGORY_MAX) {
		nRC = DlgShowWindow(nRC);
	}
	printf("RC = %d\n",nRC);


	printf("Get check value\n"); getchar();
	nRC = DlgGetFieldVal(IDI_INIQuery - IDI_BASE,&uValue);
	printf("Value = %u    ",uValue);
	printf("RC = %d\n",nRC);
	if (nRC != 0) { getchar(); return;}

	printf("Get integer value\n"); getchar();
	nRC = DlgGetFieldVal(IDI_History - IDI_BASE,&uValue);
	printf("Value = %u    ",uValue);
	printf("RC = %d\n",nRC);
	if (nRC != 0) { getchar(); return;}

	printf("Get string\n"); getchar();
	nRC = DlgGetFieldString(IDI_4StartPath - IDI_BASE,aHold,sizeof(aHold));
	printf("String = %s    ",aHold);
	printf("RC = %d\n",nRC);
	if (nRC < 0) { getchar(); return;}

	printf("Get radio\n"); getchar();
	nRC = DlgGetRadioVal(IDI_ANSI - IDI_BASE,&uValue);
	printf("Value = %u    ",uValue);
	printf("RC = %d\n",nRC);
	if (nRC != 0) { getchar(); return;}

	printf("Get color\n"); getchar();
	nRC = DlgGetColorVal(IDI_InputColors - IDI_BASE,&uValue,&uValueB);
	printf("Value = %u,%u    ",uValue,uValueB);
	printf("RC = %d\n",nRC);
	if (nRC != 0) { getchar(); return;}
#endif

	while (nRC >= 0 && nRC < CATEGORY_MAX) {
//	while (nRC == 0) {
		nCat = nRC;
		//printf("Dialog build window: %d\n", nCat); getchar();
		nRC = DlgBuildWindow(nCat);
		if (nRC >= 0) {
/*
	nRC = DlgSetFieldDirective(IDI_PauseOnError - IDI_BASE,"PauseOnError");
	nRC = DlgSetFieldDirective(IDI_4StartPath - IDI_BASE,"4StartPath");

	printf("Setting integer field\n"); getchar();
	nRC = DlgSetFieldVal(IDI_History - IDI_BASE,1024,FALSE);
	printf("RC = %d\n",nRC);
	if (nRC != 0) { getchar(); return;}

//GORP
//	printf("Setting integer field range\n"); getchar();
//	nRC = DlgSetFieldValRange(IDI_History - IDI_BASE,0,2048,16);
//	printf("RC = %d\n",nRC);
//	if (nRC != 0) { getchar(); return;}


	printf("Setting string field\n"); getchar();
	nRC = DlgSetFieldString(IDI_4StartPath - IDI_BASE,"c:\FIGMO");
	printf("RC = %d\n",nRC);
	if (nRC != 0) { getchar(); return;}

	printf("Setting check \n"); getchar();
	nRC = DlgSetFieldVal(IDI_INIQuery - IDI_BASE,TRUE,FALSE);
	printf("RC = %d\n",nRC);
	if (nRC != 0) { getchar(); return;}

*/

			//printf("Dialog show window: %d\n", nCat); getchar();
			nRC = DlgShowWindow(nCat);
			DlgCloseWindow(nCat);
		}
	}
	printf("RC = %d\n",nRC);

	printf("Mem avail (start, now): %u, %u\n",uHoldMem, _memavl());

	printf("Dialog close\n"); getchar();
	nRC = DlgClose();
	printf("RC = %d\n",nRC);

	printf("Mem avail (start, now): %u, %u\n",uHoldMem, _memavl());

	printf("All done\n"); getchar();
}
