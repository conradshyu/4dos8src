/*
 *************************************************************************
 ** inikpar.h
 *************************************************************************
 ** Copyright (c) 1997  JP Software, All rights reserved
 *************************************************************************
 */

#ifndef INIKPAR_H
#define INIKPAR_H

#define FMT_FAR_PREC_STR  "%.*Fs"

// KPARSE.C
int keyparse(char _far *keystr, int klen);
int toklist(char *token, TL_HEADER *tlist, int *index);


#endif  // INIKPAR_H
