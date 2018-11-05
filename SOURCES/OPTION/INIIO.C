/*
 *************************************************************************
 ** INIIO.C
 *************************************************************************
 ** File I/O routines for .ini files
 **
 ** Copyright 1996 JP Software Inc., All rights reserved
 *************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include <dos.h>

#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <sys\types.h>
#include <share.h>

#include "typedefs.h"
#include "iniio.h"
#include "general.h"
#include "iniutil.h"


#define CH_EOF   26  // End of file character
#define LINE_MAX 1023
#define FILE_MAX 32768

#define NULL_STR      ""
#define WHITE_SPACE   " \t"
#define LINE_END      "\r\n"

#define IO_OPEN_ERROR   -1
#define IO_MEM_ERROR    -2
#define IO_OPEN_MSG     "Could not open INI file."
#define IO_MEM_MSG      "Could not allocate necessary memory."


// make a far pointer from the segment and offset
#define MAKEP(seg,off) ((void _far *)(((unsigned long)(seg) << 16) | \
                                       (unsigned int)(off)))



/*
 *************************************************************************
 ** Prototypes
 *************************************************************************
 */
static int _GetLine(int fd, register char *line, int nMaxSize,
                    int *fEOFatEOL);


/*
 *************************************************************************
 ** WriteINIFileInt
 *************************************************************************
 ** Write an integer to the .INI file
 *************************************************************************
 ** Requires: szSection = section name to add
 **           szItem = directive name to add
 **           nValue = integer value of ini directive
 **           pszIniName = name of ini file
 ** Returns : none
 *************************************************************************
 */
void WriteINIFileInt(char *pszSection, char *pszItem, int nValue,
                     char *pszIniName) {
	char szBuffer[17];

	sprintf(szBuffer, "%d", nValue);
	WriteINIFileStr(pszSection, pszItem, szBuffer, pszIniName);

}  // End WriteINIFileInt


/*
 *************************************************************************
 ** WriteINIFileStr
 *************************************************************************
 ** Write a string to the .INI file
 *************************************************************************
 ** Requires: pszSection = section name to add
 **           pszItem = directive name to add
 **           pszValue = string value of ini directive
 **           pszIniName = name of ini file
 ** Returns : none
 *************************************************************************
 */
void WriteINIFileStr(char *pszSection, char *pszItem,
                     char *pszValue, char *pszIniName) {
	int i;
	int fd;
	int nLength = 0;
	int fEOFatEOL = FALSE;
	int fLineEndsInEOF = FALSE;
	unsigned int uSize = FILE_MAX;
	unsigned int uBytesRead = 0;
	unsigned int uBytesWritten = 0;
	unsigned short usSegment = 0;
	long lOffset = 0L;
	char szBuffer[LINE_MAX + 1];
	char szWriteBuffer[LINE_MAX + 1];
	char _far *pchTail;

	// Flags
	unsigned int fDone = FALSE;
	unsigned int fEOF = FALSE;
	unsigned int fInSection = FALSE;
	unsigned int fFoundMatch = FALSE;


	// Open the INI file
	if (( fd = sopen( pszIniName, (O_RDWR | O_BINARY | O_CREAT), SH_DENYWR,
	    (S_IREAD | S_IWRITE) )) <= 0 ) {
		// Error
		FatalError(IO_OPEN_ERROR, IO_OPEN_MSG);
	}

	// If no section is specified, we want to put new directive before any
	//  section headers
	if (pszSection == NULL)
		fInSection = TRUE;
	else {
		// Find section
		fEOF = _GetLine(fd, szBuffer, LINE_MAX, &fEOFatEOL) <= 0;

		while ((! fInSection ) && (! fEOF)) {
			strip_leading(szBuffer, WHITE_SPACE);
			if (szBuffer[0] != '\0') {
				// Record starting position of line following last non-blank line
				lOffset = _lseek(fd, 0L, SEEK_CUR);
				fLineEndsInEOF = fEOFatEOL;
			}

			strip_leading(szBuffer, " \t[");
			strip_trailing(szBuffer, " \t]");

			if (_stricmp(szBuffer, pszSection) == 0)
				fInSection = TRUE;
			else
				fEOF = _GetLine(fd, szBuffer, LINE_MAX, &fEOFatEOL) <= 0;

		}  // End while
	}  // End else find section

	if (! fInSection) {
		// If section was not found, add to end of file

		// Set pointer to line following last non-blank line
		_lseek(fd, lOffset, SEEK_SET);

		// If the last non-blank line read ends with the EOF char, insert EOL
		if (fLineEndsInEOF)
			write(fd, LINE_END, strlen(LINE_END));

		// Add section
		sprintf(szWriteBuffer, "\r\n[%s]\r\n", pszSection);
		write(fd, szWriteBuffer, strlen(szWriteBuffer));

		// Add directive
		if ((pszItem != NULL) && (pszValue != NULL)) {
			sprintf(szWriteBuffer, "%s=%s\r\n", pszItem, pszValue);
			write(fd, szWriteBuffer, strlen(szWriteBuffer));
		}

		lOffset = _lseek(fd, 0L, SEEK_CUR);
		chsize(fd, lOffset);
	}
	else if (pszItem != NULL) {
		nLength = strlen(pszItem);

		// We are in the correct section; now look for matching item
		while (! fDone) {
			szBuffer[0] = '\0';
			fEOF = _GetLine(fd, szBuffer, LINE_MAX, &fEOFatEOL) <= 0;

			if (! fEOF) {
				// Examine buffer
				strip_leading(szBuffer, WHITE_SPACE);

				if (szBuffer[0] != '\0') {
					// Hit a non-blank line
					fLineEndsInEOF = fEOFatEOL;

					if (szBuffer[0] == '[') {
						// Hit another section header
						fInSection = FALSE;
					}
					else {
						// Not a section header
						// Is this the directive?
						if (strnicmp(szBuffer, pszItem, nLength) == 0) {
							// Skip spaces between directive and '='
							for (i = nLength; isspace(szBuffer[i]); i++)
								;
				         if (szBuffer[i] == '=' ) {
								// Found the matching directive name
								fFoundMatch = TRUE;
							}
						}
					}  // End else not a section header
				}  // End if not a blank line
			}  // End if not EOF


			if (!fInSection || fFoundMatch || fEOF) {
				// Found the spot to write

				// Allocate space and save rest of file
				if (_dos_allocmem(((uSize + 0xF ) >> 4 ), (unsigned *)&usSegment) != 0) {
					// Error
					close(fd);
					FatalError(IO_MEM_ERROR, IO_MEM_MSG);
				}
				pchTail = (char _far *)MAKEP(usSegment, 0);
				_dos_read(fd, pchTail, uSize, &uBytesRead);

				// Set pointer to line following last non-blank line
				_lseek(fd, lOffset, SEEK_SET);

				// If the last non-blank line read ends with the EOF char
				//  and we hit the real end of file, insert EOL
				if (fLineEndsInEOF && fEOF)
					write(fd, LINE_END, strlen(LINE_END));

				// Write new directive line or skip (delete) if no value
				if (pszValue != NULL) {
					sprintf(szWriteBuffer, "%s=%s\r\n", pszItem, pszValue);
					write(fd, szWriteBuffer, strlen(szWriteBuffer));
				}

				if (! fInSection) {
					// Write current line (contains section)
					sprintf(szWriteBuffer, "\r\n%s\r\n", szBuffer);
					write(fd, szWriteBuffer, strlen(szWriteBuffer));
				}

				// Restore rest of file
				if (uBytesRead > 0)
					_dos_write(fd, pchTail, uBytesRead, &uBytesWritten);

				_dos_freemem(usSegment);

				// Trunc file
				lOffset = _lseek(fd, 0L, SEEK_CUR);
				chsize(fd, lOffset);

				fDone = TRUE;
			}  // End if found the spot to write


			if (szBuffer[0] != '\0') {
				// Record starting position of line following last non-blank line
				lOffset = _lseek(fd, 0L, SEEK_CUR);
			}

		}  // End while not Done
	}  // End else in the correct section

	close(fd);

	return;
}  // End WriteINIFileStr
					

/*
 *************************************************************************
 ** _GetLine
 *************************************************************************
 ** Read a line
 *************************************************************************
 ** Requires: fd = file descriptor
 **           line = buffer for new line
 **           nMaxSize = maximum length of buffer
 **           fEOFatEOL = flag
 ** 
 ** Returns : Number of chars read
 ** 
 **           fEOFatEOL = TRUE if found EOF at end of line just read in
 **                       FALSE otherwise
 *************************************************************************
 */
static int _GetLine(int fd, register char *line, int nMaxSize,
                    int *fEOFatEOL) {
	register int i;


	*fEOFatEOL = FALSE;
	nMaxSize = read(fd, line, nMaxSize);

	// get a line and set the file pointer to the next line
	for (i = 0; ; i++, line++) {

		if (i >= nMaxSize)
			break;

		if (*line == CH_EOF) {
			// Set flag if we have hit an EndOfFile char (26) at the end of
			//  the line
			*fEOFatEOL = TRUE;
			break;
		}

		if ((*line == '\r') || (*line == '\n')) {
			// skip a LF following a CR or LF
			if ((++i < nMaxSize) && (line[1] == '\n'))
				i++;

			break;
		}
	}

	// truncate the line
	*line = '\0';

	if (i >= 0) {
		// save the next line's position
		(void)_lseek(fd, (long)( i - nMaxSize ), SEEK_CUR);
	}

	return i;
}  // End _GetLine
