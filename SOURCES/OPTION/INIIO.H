/*
 *************************************************************************
 ** iniio.h
 *************************************************************************
 ** Copyright 1996 JP Software Inc., All rights reserved
 *************************************************************************
 */

#ifndef INIIO_H
#define INIIO_H

/*
 *************************************************************************
 ** Prototypes
 *************************************************************************
 */
void WriteINIFileInt(char *pszSection, char *pszItem, int nValue,
                     char *pszIniName);
void WriteINIFileStr(char *pszSection, char *pszItem,
                     char *pszValue, char *pszIniName);

#endif  // INIIO_H
