/*
 *************************************************************************
 ** general.h
 *************************************************************************
 ** Copyright (c) 1996  JP Software, All rights reserved
 *************************************************************************
 */

#ifndef GENERAL_H
#define GENERAL_H

#define TRUE  1
#define FALSE 0

#define MAXFILENAME   260
#define ARG_MAX       255

// Make a far pointer from the segment and offset
#define MAKEP(seg,off) ((void _far *)(((unsigned long)(seg) << 16) | \
                                      (unsigned int)(off)))

// Macros
#define MAX(x,y)	(((x) > (y)) ? (x) : (y))
#define MIN(x,y)	(((x) < (y)) ? (x) : (y))

#endif  // GENERAL_H
