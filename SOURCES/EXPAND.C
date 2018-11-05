

/*
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  (1) The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  (2) The Software, or any portion of it, may not be compiled for use on any
  operating system OTHER than FreeDOS without written permission from Rex Conn
  <rconn@jpsoft.com>

  (3) The Software, or any portion of it, may not be used in any commercial
  product without written permission from Rex Conn <rconn@jpsoft.com>

  (4) THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/


// EXPAND.C
//   Copyright (c) 1988 - 2002  Rex C. Conn  All rights reserved.
//
//   Expand command aliases
//   Expand environment variables, internal variables, and variable functions
//   Perform redirection
//   History processing

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <dos.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <malloc.h>
#include <math.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <share.h>
#include <string.h>
#include <time.h>

#include "4all.h"
#include "inifile.h"

extern TCHAR gaPushdQueue[];
#ifndef __WATCOMC__
extern int __dst_adjust;
#endif
extern struct tm *_gmtime( const time_t *, struct tm * );

static int nExecStrRet = 0;

void _fastcall SeekToEnd( int );
static int _near _fastcall UserFunctions( LPTSTR, LPTSTR );
static int _near _fastcall VariableFunctions( LPTSTR, LPTSTR );
static LPTSTR _near _fastcall var_internal( LPTSTR );
static int _near _fastcall AttributeString( LPTSTR, int * );
static void _near FormatLong( LPTSTR, LPCTSTR, ULONG_PTR );
static int _near Format64bit( LPTSTR, LPCTSTR, t_int64 * );
static int _fastcall __history( LPTSTR, int );


// duplicate a file handle, and close the old handle
void _fastcall dup_handle( register unsigned int uOldFH, unsigned int uNewFH )
{
	register int nFH;

	// the dup & close are required to kludge around a bug in Netware
	//   (Netware fails to flush the file buffers);
	//   Netware has *another* bug that breaks things if you try
	//   this with STDIN!
	if (( uOldFH != STDIN ) && ( uNewFH != STDIN )) {
		nFH = _dup( uNewFH );
		_close( nFH );
	}

	if ( _dup2( uOldFH, uNewFH ) < 0 )
		_close( uNewFH );
	_close( uOldFH );
}


// kludge to back up over a ^Z at EoF
void _fastcall SeekToEnd( register int nFH )
{
	TCHAR cEOF = 0;

	long lOffset = -1L;


	if ( _lseek( nFH, lOffset, SEEK_END ) >= 0L ) {

		_read( nFH, (char *)&cEOF, sizeof(TCHAR) );

		// back up so we overwrite the ^Z
		if ( cEOF == 0x1A )
			_lseek( nFH, lOffset, SEEK_CUR );

		// kludge for MSC 7 & DOS bug
	} else
		QuerySeekSize( nFH );
}


// Redirecting to CLIP: - build filename in TMP directory
void _fastcall RedirToClip( LPTSTR pszName, int fStd )
{
	register LPTSTR pszClipName;
	TCHAR _far *lpszTempDirectory;

	// look for TEMP disk area defined in environment, or default to boot
	lpszTempDirectory = GetTempDirectory( pszName );


	// if no temp directory, or temp directory is invalid, use root
	//   directory on boot drive
	if ( lpszTempDirectory == 0L )
		sprintf( pszName, FMT_ROOT, gpIniptr->BootDrive );

	if ( fStd == 0 )
		pszClipName = _TEXT("CLIP_IN.JPS");
	else if ( fStd == 1 )
		pszClipName = _TEXT("CLIP_OUT.JPS");
	else if ( fStd == 2 )
		pszClipName = _TEXT("CLIP_ERR.JPS");
	else
		pszClipName = _TEXT("CLIP_TMP.JPS");
	mkdirname( pszName, pszClipName );
}


// copy the file to the clipboard
int _fastcall CopyToClipboard( int nFH )
{

	unsigned int uBytesRead = 0, uSize;
	char _far *lpMemory;
	long lMemSize;

	// set clipboard buffer size
	if (( lMemSize = QuerySeekSize( nFH )) >= 0xFFEE )
		lMemSize = 0xFFEE;
	RewindFile( nFH );

	uSize = (UINT)lMemSize + 2;

	if (( lpMemory = AllocMem( &uSize )) == 0L )
		return ( error( ERROR_NOT_ENOUGH_MEMORY, NULL ));

	lMemSize = uSize - 2;

	// save file 
	if ( lMemSize > 0 )
		_dos_read( nFH, lpMemory, (UINT)lMemSize, &uBytesRead );
	lpMemory[ uBytesRead ] = '\0';

	if ( QueryClipAvailable() ) {
		OpenClipboard();
		SetClipData( lpMemory, (long)uBytesRead+1 );
		CloseClipboard();
	} else
		return ( error( ERROR_4DOS_CLIPBOARD_INUSE, NULL ));

	FreeMem( lpMemory );

	return 0;
}


int _fastcall CopyTextToClipboard( LPTSTR pszLine, int nLength )
{
	if ( QueryClipAvailable() ) {
		OpenClipboard();
		SetClipData( ( char _far *)pszLine, nLength );
		CloseClipboard();
	} else
		return ERROR_4DOS_CLIPBOARD_INUSE;

	return 0;
}


// Redirecting from CLIP: - copy from the clipboard to the specified file
int _fastcall CopyFromClipboard( LPTSTR pszFilename )
{
	unsigned int uBytesWritten, uSize;
	int nFH, nReturn = 0;
	long lMemSize;
	char _far *lpMemory;

	// remove leftover CLIP file
	remove( pszFilename );

	if ( QueryClipAvailable() ) {

		HoldSignals();
		if (( nFH = _sopen( pszFilename, (_O_WRONLY | _O_BINARY | _O_CREAT | _O_TRUNC), _SH_DENYWR, _S_IWRITE | _S_IREAD )) < 0 )
			return ( error( _doserrno, pszFilename ));

		OpenClipboard();
		if (( lMemSize = QueryClipSize()) > 0 ) {

			if ( lMemSize > 0xFFEEL)
				uSize = 0xFFF0;
			else
				uSize = (UINT)lMemSize + 2;

			// Allocate the buffer, holler if we can't
			if (( lpMemory = AllocMem( &uSize )) == 0L )
				nReturn = error( ERROR_NOT_ENOUGH_MEMORY, NULL );

			else {

				if ( lMemSize > 0xFFEEL ) {
					if (( lpMemory = ReallocMem( lpMemory, lMemSize )) == 0L ) {
						nReturn = error( ERROR_NOT_ENOUGH_MEMORY, NULL );
						goto EndCopy;
					}
				}

				// Read the clipboard data and write to the file
				lpMemory[0] = '\0';
				ReadClipData( lpMemory );
				lpMemory[ lMemSize-1 ] = '\0';
				_dos_write( nFH, lpMemory, _fstrlen( lpMemory ), &uBytesWritten );
			}

EndCopy:
			CloseClipboard();
			FreeMem( lpMemory );

		} else
			nReturn = error( ERROR_4DOS_CLIPBOARD_NOT_TEXT, NULL );

		_close( nFH );
		EnableSignals();
	}

	return nReturn;
}


// Perform I/O redirection (except pipes).  All > and <'s will be removed.
// We have to work around a Netware bug that precludes using the same
//   code as 4NT - closing a dup'd file closes the other handle as well!
int _fastcall redir( char *pszStartLine, REDIR_IO *redirect )
{
	static char szDelimiters[] = "  \t;,<>|`+=[]";
	register char *pszLine, *pszArg;
	int nFH, i, fClip, nOpenMode = 0, nShareMode, nc, nNameLength;
	char szFileName[MAXFILENAME], caRedirections[MAXREDIR];

	// set the current switch character (usually /)
	szDelimiters[1] = gpIniptr->SwChr;

	if ( gpIniptr->Expansion & EXPAND_NO_REDIR )
		return 0;

	for ( ; ; ) {

		// check for I/O redirection (< or >)
		if (( pszLine = scan( pszStartLine, "<>", (( *pszStartLine == _TEXT('(') ) ? QUOTES_PARENS : QUOTES ))) == BADQUOTES )
			return ERROR_EXIT;

		if ( *pszLine == _TEXT('\0') )
			return 0;

		// set the NOCLOBBER default
		nc = gpIniptr->NoClobber;

		nShareMode = _SH_COMPAT;

		// save pointer to redirection character
		pszArg = pszLine++;

		// clear I/O redirection flags
		for ( i = 0; ( i < MAXREDIR ); i++ )
			caRedirections[i] = 0;

		// force batch file close (for Netware bug)
		close_batch_file();

		if ( *pszArg == _TEXT('>') ) {

			// redirecting output (stdout and/or stderr)
			caRedirections[ STDOUT ] = 1;

			if ( *pszLine == _TEXT('>') ) {
				// append to output file
				nOpenMode = _O_RDWR | _O_BINARY;
				pszLine++;
			} else		// overwrite the output file
				nOpenMode = _O_WRONLY | _O_BINARY | _O_TRUNC;

			if ( *pszLine == '&' ) {

				// redirect STDERR too (>&) or STDERR only (>&>)
				caRedirections[ STDERR ] = 1;

				if ( *(++pszLine ) == _TEXT('>') ) {
					// redirect STDERR only
					caRedirections[ STDOUT ] = 0;
					pszLine++;
				}
			}

			if ( *pszLine == _TEXT('!') ) {	// override NOCLOBBER
				nc = 0;
				pszLine++;
			}

		} else {		// redirecting input (stdin)
			// kludge for morons who do "file << file2"
			while ( *pszLine == _TEXT('<') )
				pszLine++;
			caRedirections[ STDIN ] = 1;
		}

		// get the file name (can't use sscanf() with []'s)
		pszLine = skipspace( pszLine );
		if (( nNameLength = ( scan( pszLine, szDelimiters, QUOTES ) - pszLine )) > (MAXFILENAME- 1 ))
			nNameLength = ( MAXFILENAME - 1 );
		sprintf( szFileName, FMT_PREC_STR, nNameLength, pszLine );
		EscapeLine( szFileName );

		// if not a device, expand filename ("dir > ...\file" )
		if ( QueryIsDevice( szFileName )) {

			nOpenMode |= _O_CREAT;
			nShareMode = _SH_DENYNO;

		} else if ( mkfname( szFileName, 0 ) == NULL )
			return ERROR_EXIT;

		for ( i = 0; ( i < MAXREDIR ); i++ ) {

			// if redirecting this handle, save it
			if ( caRedirections[ i ] ) {

				// already redirected this handle?
				if ( redirect->naStd[ i ] )
					return ( error( ERROR_ALREADY_ASSIGNED, pszArg ));
				redirect->naStd[ i ] = _dup( i );
			}
		}

		if ( *pszArg == _TEXT('>') ) {

			// if target is a device, _O_CREAT will already be set
			if (( nOpenMode & _O_CREAT ) == 0 ) {

				// if NOCLOBBER set, don't overwrite an existing
				//   file, and ensure file exists on an append
				//   (_O_CREAT will not be set)
				if ( nc ) {
					if ( nOpenMode & _O_TRUNC )
						nOpenMode |= ( _O_CREAT | _O_EXCL );
				} else
					nOpenMode |= _O_CREAT;
			}

			// close the handle(s) so the file will be opened in
			//   their handle slot
			if ( caRedirections[ STDOUT ] )
				_close( STDOUT );
			if ( caRedirections[ STDERR ] )
				_close( STDERR );

			// if it's a named pipe, turn off _O_TRUNC switch
			//   because of OS/2 2.0 fatal bug
			if ( QueryIsPipeName( szFileName ))
				nOpenMode &= ~_O_TRUNC;

			// open the redirection target
			fClip = 0;
			if ( stricmp( szFileName, CLIP ) == 0 ) {

				// build temp file name
				RedirToClip( szFileName, (( caRedirections[ STDOUT ] ) ? 1 : 2 ));
				fClip = 1;

				if ( nOpenMode & _O_RDWR ) {
					if (( nFH = CopyFromClipboard( szFileName )) != 0 )
						return nFH;
					nOpenMode = _O_RDWR | _O_CREAT | _O_BINARY;
				} else
					nOpenMode = _O_RDWR | _O_CREAT | _O_BINARY | _O_TRUNC;
				nShareMode = _SH_DENYRW;
			}

			if (( nFH = _sopen( szFileName, nOpenMode, nShareMode, ( _S_IREAD | _S_IWRITE ))) < 0 ) {

				nFH = (( errno == EEXIST ) ? ERROR_FILE_EXISTS : _doserrno );

				// undo redirection so we can display an error
				unredir( redirect, NULL );
				return ( error( nFH, szFileName ));
			}

			// set "writing to CLIPBOARD" flag
			if ( fClip )
				redirect->faClip[ nFH ] = 1;

			// if appending, go the end of the file
			if (( nOpenMode & _O_TRUNC ) == 0 )
				SeekToEnd( nFH );

			// if both STDOUT & STDERR redirected, dup STDOUT to
			//    STDERR
			if (( caRedirections[ STDOUT ] ) && ( caRedirections[ STDERR ]))
				_dup2( nFH, STDERR );

		} else {

			// close STDIN; file will be opened in its handle slot
			_close( STDIN );

			// if CLIP:, copy clipboard to file & open it
			if ( stricmp( szFileName, CLIP ) == 0 ) {
				// build temp file name
				RedirToClip( szFileName, 0 );
				if (( nFH = CopyFromClipboard( szFileName )) != 0 )
					return nFH;
			}

			// open the redirection source
			if ( _sopen( szFileName, _O_RDONLY | _O_BINARY, _SH_DENYNO ) < 0 )
				return ( error( _doserrno, szFileName ));
		}

		// now rub out the filename & redirection characters
		strcpy( pszArg, pszLine + nNameLength );
	}
}



// Reset all redirection: stdin = stdout = stderr = console.
//   if DOS & a pipe is open, close it and reopen stdin to the pipe.
void _fastcall unredir( register REDIR_IO *redirect, int *pnError )
{
	register int i;

	// clean up STDIN, STDOUT, and STDERR, and close open pipe
	for ( i = 0; ( i < MAXREDIR ); i++ ) {

		if ( redirect->naStd[ i ] ) {

			// read CLIP: temp file & stuff it into the clipboard
			if ( i <= STDERR ) {
				if ( redirect->faClip[i] )
					CopyToClipboard( i );
			}

			dup_handle( redirect->naStd[ i ], i );
			redirect->naStd[ i ] = 0;

			// delete the temp clipboard file
			if ( i <= STDERR ) {
				if ( redirect->faClip[i] ) {
					TCHAR szClipName[MAXFILENAME];
					RedirToClip( szClipName, i );
					remove( szClipName );
				}
			}

			redirect->faClip[ i ] = 0;

		}
	}

	if ( redirect->fPipeOpen ) {

		// delete old input pipe ( if any)
		if ( redirect->pszInPipe != NULL ) {
			remove( redirect->pszInPipe );
			free( redirect->pszInPipe );
		}

		// set input name to last output file
		redirect->pszInPipe = redirect->pszOutPipe;

		if (( i = _sopen( redirect->pszInPipe, (_O_RDONLY | _O_BINARY), _SH_DENYWR )) < 0 )
			error( _doserrno, redirect->pszInPipe );
		else {
			redirect->naStd[ STDIN ] = _dup( STDIN );
			dup_handle( i, STDIN );
		}

		redirect->fPipeOpen = 0;
	}
}


// Expand aliases ( first argument on the command line)
int _fastcall alias_expand( register LPTSTR pszLine )
{
	static TCHAR szDelimiters[] = _TEXT("%[^ <>=+|]%n");
	register LPTSTR pszAliasArg;
	TCHAR szAliasLine[CMDBUFSIZ+1], szScratch[5];
	TCHAR *pszArg, *pszEOL, cEOL;
	int nArgNum, nLoopCounter, nVarsExist;
	char _far *lpszAliasValue;

	INT_PTR nAliasLength;

	// check for alias expansion disabled
	if ( gpIniptr->Expansion & EXPAND_NO_ALIASES )
		return 0;

	// beware of aliases with no whitespace before ^ or |
	szDelimiters[3] = gpIniptr->CmdSep;

	for ( nLoopCounter = 0; ; nLoopCounter++ ) {

		nVarsExist = 0;

		// skip past ? (prompt before executing line) character
		if ( *pszLine == _TEXT('?') ) {

			pszLine = skipspace( pszLine+1 );
			if ( *pszLine == _TEXT('\0') )
				return 0;

			// skip optional prompt string
			if ( *pszLine == _TEXT('"') )
				pszLine = skipspace( scan( pszLine, WHITESPACE, QUOTES+1 ));
		}

		// parse the first command in the line
		if (( pszArg = first_arg( pszLine )) == NULL )
			return ERROR_EXIT;

		// a '*' means don't do alias expansion on this command name
No_Alias:
		if ( *pszArg == _TEXT('*') ) {
			strcpy( gpNthptr, gpNthptr+1 );
			return 0;
		}

		// skip past '@' (no history save) character
		if ( *pszArg == _TEXT('@') ) {
			if ( *(++pszArg) == _TEXT('\0') )
				return 0;
			gpNthptr++;
			goto No_Alias;
		}

		// strip things like <, >, |, etc.
		sscanf( pszArg, szDelimiters, pszArg, &nAliasLength );

		// adjust for leading whitespace
		nAliasLength += (INT_PTR)( gpNthptr - pszLine );

		// search the environment for the alias
		if (( lpszAliasValue = get_alias( pszArg )) == 0L )
			return 0;

		// look for alias loops
		if ( nLoopCounter > 16 )
			return (error(ERROR_4DOS_ALIAS_LOOP, NULL ));

		// check if alias is too long
		if ( _fstrlen( lpszAliasValue ) >= (CMDBUFSIZ - 1 ))
			return ( error( ERROR_4DOS_COMMAND_TOO_LONG, NULL ));
		_fstrcpy( szAliasLine, lpszAliasValue );

		// get the end of the first command & its args
		if (( pszEOL = scan( pszLine, NULL, QUOTES )) == BADQUOTES )
			return ERROR_EXIT;

		cEOL = *pszEOL;
		*pszEOL = _TEXT('\0');

		// process alias arguments
		for ( pszAliasArg = szAliasLine; ( *pszAliasArg != _TEXT('\0') ); ) {

			if (( pszAliasArg = scan( pszAliasArg, _TEXT("%"), BACK_QUOTE )) == BADQUOTES )
				return ERROR_EXIT;

			if ( *pszAliasArg != _TEXT('%') )
				break;

			// check for alias count (%#) or alias arg (%n&)
			if ( pszAliasArg[1] == _TEXT('#') ) {

				strcpy( pszAliasArg, pszAliasArg+2 );

				// %# evaluates to number of args in alias line
				for ( nArgNum = 1; ( ntharg( pszLine, nArgNum | 0x2000 | 0x8000 ) != NULL ); nArgNum++ )
					;

				IntToAscii( nArgNum-1, szScratch );
				strins( pszAliasArg, szScratch );
				continue;

			} else if (( isdigit(pszAliasArg[1]) == 0 ) && ( pszAliasArg[1] != gpIniptr->ParamChr )) {

				// Not an alias arg; probably an environment
				//   variable.  Ignore it.

				pszAliasArg++;

				for ( ; (( isalnum( *pszAliasArg )) || ( *pszAliasArg == _TEXT('_') ) || ( *pszAliasArg == _TEXT('$') )); pszAliasArg++ )
					;

				// ignore %% variables
				if ( *pszAliasArg == _TEXT('%') )
					pszAliasArg++;
				continue;
			}

			// strip the '%'
			strcpy( pszAliasArg, pszAliasArg+1 );

			// %& defaults to %1&
			nArgNum = (( *pszAliasArg == gpIniptr->ParamChr ) ? 1 : atoi( pszAliasArg ));
			while ( isdigit( *pszAliasArg ))
				strcpy( pszAliasArg, pszAliasArg+1 );

			// flag highest arg processed
			if ( nArgNum > nVarsExist)
				nVarsExist = nArgNum;

			// get matching argument from command line
			pszArg = ntharg( pszLine, nArgNum | 0x2000 );

			if ( *pszAliasArg == gpIniptr->ParamChr ) {
				// get command tail
				strcpy( pszAliasArg, pszAliasArg+1 );
				pszArg = gpNthptr;
				nVarsExist = 0xFF;
			}

			if ( pszArg == NULL )
				continue;

			// expand alias line
			if (( strlen( szAliasLine ) + strlen( pszArg )) >= (CMDBUFSIZ - 1 ))
				return ( error( ERROR_4DOS_COMMAND_TOO_LONG, NULL ));
			strins( pszAliasArg, pszArg );

			// don't try to expand the argument we just added
			pszAliasArg += strlen( pszArg );
		}

		// restore the end-of-command character (^, |, '\0', etc.)
		*pszEOL = cEOL;

		// if alias variables exist, delete command line until
		//   highest referenced command, compound command char, pipe,
		//   conditional, or EOL ); else just collapse the alias name
		if ( nVarsExist ) {

			pszArg = scan( pszLine + nAliasLength, NULL, QUOTES );

			if (( ntharg( pszLine + nAliasLength, nVarsExist | 0x2000 | 0x8000 ) != NULL ) && ( pszArg > gpNthptr )) {
				pszArg = gpNthptr;
				// preserve original whitespace (or lack)
				if ( iswhite( pszArg[-1] ))
					pszArg--;
			}

		} else
			pszArg = pszLine + nAliasLength;

		strcpy( pszLine, pszArg );

		// check for overlength line
		if (( strlen( pszLine ) + strlen( szAliasLine )) >= ( CMDBUFSIZ - 1 ))
			return (error(ERROR_4DOS_COMMAND_TOO_LONG, NULL ));

		// insert the alias
		strins( pszLine, szAliasLine );

		// check for nested variables disabled
		if ( gpIniptr->Expansion & EXPAND_NO_NESTED_ALIASES )
			return 0;
	}
}




// expand shell variables
int _fastcall var_expand( LPTSTR pszStartLine, int fRecurse )
{
	static int nRecursiveLoop;
	LPTSTR pszVariable, pszArg;
	TCHAR szBuffer[MAXLINESIZ+8], szFunction[32];
	LPTSTR pszLine, pszStartVar, pszOldVar = NULL;
	int i, n, nLoopCount = 0, nLength, fAliasFunc, fUserFunc, fExpansion, fAsterisk = 0;
	char _far *lpszVarValue;

	// check for variable expansion disabled
	if ( gpIniptr->Expansion & EXPAND_NO_VARIABLES )
		return 0;

	// reset nLoopCount if initial entry into var_expand()
	if ( fRecurse == 0 )
		nRecursiveLoop = 0;

	// save current Expansion flags
	fExpansion = gpIniptr->Expansion;

	for ( pszLine = pszStartLine; ; ) {

		// Get the end of the first command & its args.
		//   We do this in two parts because scan() displays
		//   an error message which we don't want to see twice.

		if (( pszVariable = scan( pszStartLine, NULL, ((( *pszStartLine == _TEXT('(') ) && ( fRecurse == 0 )) ? QUOTES_PARENS : QUOTES ))) == BADQUOTES )
			return ERROR_EXIT;

		// reset expansion flags
		gpIniptr->Expansion = fExpansion;

		// don't do variable expansion inside command groups
		if (( pszLine = scan( pszLine, _TEXT("%"), ((( *pszLine == _TEXT('(') ) && ( fRecurse == 0 )) ? _TEXT("`(") : BACK_QUOTE ))) == BADQUOTES )
			return ERROR_EXIT;

		// make sure we're not getting infinite loopy here
		if ( pszLine == pszOldVar ) {
			if ( ++nLoopCount > 16 )
				return ( error( ERROR_4DOS_VARIABLE_LOOP, pszLine ));
		} else {
			nLoopCount = 0;
			pszOldVar = pszLine;
		}

		fAliasFunc = fUserFunc = 0;

		// only do variable substitution for first command on line

		// strip the %
		if (( pszLine == NULL ) || ( *pszLine == _TEXT('\0') ) || ( pszLine >= pszVariable ) || ( *strcpy( pszLine, pszLine+1 ) == _TEXT('\0') ) || ( pszLine+1 >= pszVariable ))
			return 0;
		pszVariable = pszLine;

		if ( *pszVariable == _TEXT('%') ) {

			// kludge for COMMAND.COM/CMD.EXE compatibility
			//   FOR variables use two %%'s!
			sprintf( szFunction, _TEXT("\001%c"), pszVariable[1] );

			if ( get_variable( szFunction ) != 0L )
				strcpy( pszVariable, pszVariable + 1 );		// strip extra %
			else {
				// skip %% after stripping first one
				pszLine++;
				continue;
			}
		}

		// check for variable function or explicit variable
		// call var_expand() recursively if necessary to resolve nested variables

		szFunction[0] = _TEXT('\0');

		if (( *pszVariable == _TEXT('@') ) || ( *pszVariable == _TEXT('[') )) {

			if ( *pszVariable == _TEXT('@') ) {
				// get the variable function name
				sscanf( ++pszVariable, _TEXT("%31[^[]%n"), szFunction, &n );

				if ( szFunction[0] == _TEXT('\0') )
					return ( error( ERROR_4DOS_BAD_SYNTAX, pszLine ));
				pszVariable += n;
			}

			// save args & call var_expand() recursively
			// check for nested variable functions (delay incrementing "pszVariable" so we can get nested functions)
			if (( *pszVariable != _TEXT('[') ) || (*( pszArg = scan( pszVariable, _TEXT("]"), _TEXT("\"`[") )) == _TEXT('\0') ))
				return ( error( ERROR_4DOS_BAD_SYNTAX, pszLine ));

			pszStartVar = ++pszVariable;

			// build new line composed of the function arguments
			if (( nLength = (int)( pszArg - pszStartVar )) >= MAXLINESIZ-1 ) {
var_too_long:
				return ( error( ERROR_4DOS_COMMAND_TOO_LONG, NULL ));
			}

			sprintf( szBuffer, FMT_PREC_STR, nLength, pszStartVar );

			// collapse the function name & args
			strcpy( pszLine, pszArg+1 );

			// are there nested vars to resolve?
			if ( strchr( szBuffer, _TEXT('%') ) != NULL ) {

				// make sure we don't go "infinite loopy" here
				if ( ++nRecursiveLoop > 10 ) {
					nRecursiveLoop = 0;
					return ( error( ERROR_4DOS_VARIABLE_LOOP, NULL ));
				}

				// call ourselves to resolve nested vars
				// set "fRecurse" so we don't do alias expansion
				n = var_expand( szBuffer, 1 );

				nRecursiveLoop--;

				if ( n )
					return n;
			}

			if ( szFunction[0] ) {

				// process the variable function
				pszArg = szBuffer;

				if (( n = VariableFunctions( szFunction, pszArg )) == -6666 ) {
					// don't recurse %@ALIAS or @FUNCTION or @REGQUERY!
					n = 0;
					fAliasFunc = 1;
				} else if ( n == -6667 ) {
					// but we have to recurse user-defined functions!
					n = 0;
					fUserFunc =  1;
				}

				if ( n > 0 ) {
					// need to rebuild the variable for the error message
					pszArg = (LPTSTR)_alloca( ( 20 + strlen( szBuffer )) * sizeof(TCHAR) );
					sprintf( pszArg, _TEXT("%%@%s[%s]"), szFunction, szBuffer );
					return ( error( n, pszArg ));
				} else if ( n < 0 )
					return -n;

			} else
				pszArg = NULL;

		} else {

			// kludge for CMD.EXE FOR syntax - %fabc means "%f" + "abc", but "%fabc%"
			//   means "substitute the fabc env variable"
			if ((( pszArg = first_arg( pszVariable )) != NULL ) && (( isalnum( *pszArg ) ) || ( *pszArg == _TEXT('_') ) || ( *pszArg == _TEXT('$') ))) {

				// kludge for quoted filename: "%var%" "%anothervar%" would otherwise be seen as var%" "%anothervar%"
				for ( i = 0; ( pszArg[i] != _TEXT('\0')); i++ ) {

					if ( pszArg[i] == _TEXT('%') ) {
						pszArg[i] = _TEXT('\0');
						if ( get_variable( pszArg ) != NULL )
							goto NotForVariable;
						break;
					}

					if (( isdelim( pszArg[i] )) || ( pszArg[i] == _TEXT('=')) || ( pszArg[i] == _TEXT('"')))
						break;
				}
			}

			// kludge for COMMAND.COM/CMD.EXE compatibility:
			//   FOR variables may have a max length of 1!
			//   Have to check FOR vars _before_ batch vars
			sprintf( szFunction, _TEXT("\001%c"), *pszVariable );

			if (( lpszVarValue = get_variable( szFunction )) != 0L ) {

				// collapse the variable spec
				strcpy( pszLine, pszVariable+1 );

				goto for_variable;
			}
NotForVariable:
			// kludge for CMD.EXE compatibility - %* means
			//   "all the arguments" - i.e., %1$
			if (( *pszVariable == _TEXT('*') ) && (( pszVariable[1] == _TEXT('\0') ) || ( strchr( _TEXT(" \t,\"|<>"), pszVariable[1] ) != NULL )))
				fAsterisk = *pszVariable = gpIniptr->ParamChr;

			// get the environment (or internal) variable name
			if (( cv.bn >= 0 ) && (( isdigit( *pszVariable )) || ( *pszVariable == gpIniptr->ParamChr ))) {

				// parse normal batch file variables (%5)

				// kludge for "%01" idiocy w/COMMAND.COM
				if ( *pszVariable == _TEXT('0') )
					pszVariable++;
				else {
					while ( isdigit( *pszVariable ))
						pszVariable++;
				}

				// parse batch file %& variables
				// FIXME - if %~a$, what should we do???
				if ( *pszVariable == gpIniptr->ParamChr )
					pszVariable++;

			} else if (( *pszVariable == _TEXT('#') ) || ( *pszVariable == _TEXT('?') )) {

				// %# evaluates to number of args in batch file
				// %? evaluates to exit code of last external program
				// %?? evaluates to high byte of exit code (DOS)
				pszVariable++;

				if ( *pszVariable == _TEXT('?') )
					pszVariable++;

			} else {

				// parse environment variables (%var%)

				// test for %_? ( internal errorlevel)
				if (( pszVariable[0] == _TEXT('_') ) && ( pszVariable[1] == _TEXT('?') ))
					pszVariable += 2;

				else {

					for ( ; (( isalnum( *pszVariable )) || ( *pszVariable == _TEXT('_') ) || ( *pszVariable == _TEXT('$') )); pszVariable++ )
						;

					// skip a trailing '%'
					if ( *pszVariable == _TEXT('%') ) {
						strcpy( pszVariable, pszVariable + 1 );
					}
				}
			}

			// variable names can't be > 128 characters
			if (( n = ( int)( pszVariable - pszLine )) > 128 )
				n = 128;
			else if ( n == 0 )
				continue;
			sprintf( szBuffer, FMT_PREC_STR, n, pszLine );
			pszArg = NULL;

			// collapse the variable spec
			strcpy( pszLine, pszVariable );
		}

		// if not a variable function, expand it
		if (( pszArg == NULL ) && ( szBuffer[0] != _TEXT('\0') )) {


			// first, try expanding batch vars (%n, %&, %n&, and %#);
			//   then check for internal variables
			if ( cv.bn >= 0 ) {

				// check for %n, %&, and %n&
				for ( i = 0; ( szBuffer[i] != _TEXT('\0') ); i++ ) {
					if (( isdigit( szBuffer[i] ) == 0 ) && ( szBuffer[i] != gpIniptr->ParamChr ))
						break;
				}

				// expand %n, %&, and %n&
				if ( szBuffer[i] == _TEXT('\0') ) {

					// %& defaults to %1&
					pszVariable = szBuffer;

					// check for %* (return all arguments, UNSHIFTED!)
					if ( fAsterisk ) {
						fAsterisk = 0;
						i = 1;
					} else
						i = (( *pszVariable == gpIniptr->ParamChr ) ? 1 : atoi( pszVariable )) + bframe[cv.bn].Argv_Offset;

					for ( pszVariable = szBuffer; isdigit( *pszVariable ); pszVariable++ )
						;

					for ( n = 0; ( n < i ) && ( bframe[cv.bn].Argv[n] != NULL ); n++ )
						;

						if ( *pszVariable == gpIniptr->ParamChr ) {

							// get command tail
							pszArg = szBuffer;
							for ( *pszArg = _TEXT('\0'); ( bframe[cv.bn].Argv[n] != NULL ); n++ ) {

								if (( strlen( pszArg ) + strlen( bframe[cv.bn].Argv[n] )) >= MAXLINESIZ - 2 )
									goto var_too_long;

								if ( *pszArg )
									strcat( pszArg, _TEXT(" ") );
								strcat( pszArg, bframe[cv.bn].Argv[n] );
							}

						} else
							pszArg = bframe[cv.bn].Argv[n];

				} else if ( szBuffer[0] == _TEXT('#') ) {

					// %# evaluates to number of args in batch file
					//   (after adjusting for SHIFT)
					for ( n = 0; ( bframe[cv.bn].Argv[ n + bframe[cv.bn].Argv_Offset ] != NULL ); n++ )
						;

					pszArg = szBuffer;
					IntToAscii((( n > 0 ) ? n - 1 : 0 ), pszArg );
				}
			}

			// check for an environment variable
			if ( pszArg == NULL ) {

				// check for GOSUB local variables
				if (( cv.bn >= 0 ) && ( bframe[ cv.bn ].pszGosubArgs )) {

					sprintf( szFunction, FMT_CHAR, bframe[ cv.bn ].uGosubStack+1 );
					strins( szBuffer, szFunction );
					if (( lpszVarValue = get_variable( szBuffer )) != 0L )
						goto for_variable;
					else
						strcpy( szBuffer, szBuffer+1 );
				}

				if (( lpszVarValue = get_variable( szBuffer )) != 0L ) {

					// kludge for nitwit MS "foo=c:\;c:\dos;%foo%"
					if ( fRecurse == 0 ) {

						strins( szBuffer, _TEXT("%") );
						strcat( szBuffer, _TEXT("%") );

						n = strlen( szBuffer );
						for ( i = 0; ( i <= (int)( _fstrlen( lpszVarValue ) - n )); i++ ) {
							if ( _fstrnicmp( lpszVarValue + i, szBuffer, n ) == 0 ) {
								_fstrcpy( szBuffer, lpszVarValue );
								strins( pszLine, szBuffer );
								return 0;
							}
						}

					}

for_variable:
					if ( _fstrlen( lpszVarValue ) >= MAXLINESIZ-1 )
						goto var_too_long;

					pszArg = szBuffer;
					_fstrcpy( pszArg, lpszVarValue );

				} else {
					// not a batch or env var; check for internal
					pszArg = var_internal( szBuffer );
				}

			}
		}

		// insert the variable
		if ( pszArg != NULL ) {

			i = strlen( pszArg );
			if (( strlen( pszStartLine ) + i ) >= (unsigned int)(( pszStartLine == gszCmdline ) ? CMDBUFSIZ - 1 : MAXLINESIZ - 1 ))
				goto var_too_long;

			strins( pszLine, pszArg );

			// kludge to allow alias expansion when variable is
			//   first arg on command line
			pszArg = first_arg( pszStartLine );
			if (( pszArg != NULL ) && ( fRecurse == 0 ) && ((unsigned int)( pszLine - pszStartLine ) < strlen( pszArg ))) {
				if ( alias_expand( pszStartLine ) != 0 )
					return ERROR_EXIT;
			}

			if ( fUserFunc ) {
			
				// check for user-defined functions disabled
				if ( gpIniptr->Expansion & EXPAND_NO_USER_FUNCTIONS )
					pszLine += i;

			// check for nested variables disabled
			} else if (( fAliasFunc ) || ( gpIniptr->Expansion & EXPAND_NO_NESTED_VARIABLES ))
				pszLine += i;
		}
	}
}


// get the specified line from the clipboard
int _fastcall GetClipboardLine( int nLine, LPTSTR pszLine, int nMaxLength )
{
	unsigned int nReturn = 0;
	unsigned int uEOL;
	char _far *lpClipMemory;
	long lOffset;

	*pszLine = _TEXT('\0');

	// See if there;s data in the clipboard, and reserve memory for it
	if ( QueryClipAvailable() ) {
		OpenClipboard();
		if (( lOffset = QueryClipSize() ) == 0 ) {
			CloseClipboard();
			return ERROR_EXIT;
		}
		uEOL = (unsigned int)( lOffset & 0xFFFF );
		if (( lOffset > 0xFFFFL ) || (( lpClipMemory = AllocMem( &uEOL )) == 0L )) {
			CloseClipboard();
			return ERROR_NOT_ENOUGH_MEMORY;
		}
	} else
		return ERROR_EXIT;

	// Read the data into the buffer
	ReadClipData( lpClipMemory );
	CloseClipboard();

	// copy clipboard to the buffer
	for ( ; ; nLine-- ) {

		if ( *lpClipMemory == '\0' ) {
			nReturn = ERROR_EXIT;
			break;
		}

		sscanf_far( lpClipMemory, _TEXT("%*[^\r\n]%n"), &uEOL );
		if ( nLine == 0 ) {
			if ( uEOL > (unsigned int)nMaxLength )
				uEOL = nMaxLength;
			sprintf( pszLine, FMT_FAR_PREC_STR, uEOL, lpClipMemory );
			break;
		}

		lpClipMemory += uEOL;
		while (( *lpClipMemory == '\r' ) || ( *lpClipMemory == '\n' ))
			lpClipMemory++;
	}

	FreeMem( lpClipMemory );

	return nReturn;
}


// strip leading & trailing whitespace, process escape chars, and resolve path components
static int _fastcall ProcessFileName( LPTSTR pszFile, int fMakePath )
{
	trim( pszFile, WHITESPACE );
	EscapeLine( pszFile );
	if (( fMakePath ) || ( strstr( pszFile, _TEXT("...") ) != NULL )) {
		if ( mkfname( pszFile, 0 ) == NULL )
			return -ERROR_EXIT;
	} else
		StripQuotes( pszFile );

	return 0;
}


// process a user-defined variable function (%@...)
static int _near _fastcall UserFunctions ( LPTSTR pszFunction, LPTSTR pszBuffer )
{
	TCHAR _far * lpszList;
	TCHAR szFunctionName[32];
	LPTSTR pszVar, pszArg, pszArguments;
	int n, nArgNum;

	if ( glpFunctionList ) {

		for ( lpszList = glpFunctionList; ( *lpszList != _TEXT( '\0' )); lpszList = next_env( lpszList )) {

			szFunctionName[0] = _TEXT('\0');
			sscanf_far( lpszList, _TEXT("%31[^= \t]%n"), szFunctionName, &n );

			if ( stricmp( szFunctionName, pszFunction ) == 0 ) {

				// got a match -- substitute variables and pass back to var_expand
				pszArguments = strcpy( (LPTSTR)_alloca( ( strlen( pszBuffer ) + 1 ) * sizeof(TCHAR) ), pszBuffer );

				sprintf( pszBuffer, FMT_FAR_PREC_STR, MAXLINESIZ, lpszList + n + 1 );

				// OK, pszBuffer now contains the function definition, & pszArguments the args to substitute

				// process user function arguments
				for ( pszVar = pszBuffer; ( *pszVar != _TEXT('\0') ); ) {

					if (( pszVar = scan( pszVar, _TEXT("%"), BACK_QUOTE )) == BADQUOTES )
						return ERROR_EXIT;

					if ( *pszVar != _TEXT('%') )
						break;		// no more variables

					if (( isdigit( pszVar[1] ) == 0 ) && ( pszVar[1] != gpIniptr->ParamChr )) {
						// Not a function arg; probably an environment variable.  Ignore it.
						pszVar++;
						// ignore %% variables
						if ( *pszVar == _TEXT('%') )
							pszVar++;
						continue;
					}
				
					// strip the %
					strcpy( pszVar, pszVar + 1 );

					// strip the variable
					sscanf( pszVar, _TEXT("%u%n"), &nArgNum, &n );
					strcpy( pszVar, pszVar + n );

					if ( *pszVar == gpIniptr->ParamChr ) {

						strcpy( pszVar, pszVar + 1 );
						if ( n == 0 )
							nArgNum = 1;

						// get command tail
						ntharg( pszArguments, --nArgNum | 0xA000 );
						pszArg = gpNthptr;

					} else {

						// get matching argument from command line
						if ( nArgNum == 0 )
							pszArg = pszFunction;
						else {
							if (( pszArg = ntharg( pszArguments, --nArgNum | 0x2000 )) == NULL )
								continue;
						}
					}

					if ( pszArg != NULL ) {

						// expand line
						if (( strlen( pszBuffer ) + strlen( pszArg )) >= ( MAXLINESIZ - 1 ))
							return ( error( ERROR_4DOS_COMMAND_TOO_LONG, NULL ));

						strins( pszVar, pszArg );

						// don't try to expand the argument we just added
						pszVar += strlen( pszArg );
					}
				}

				*pszFunction = _TEXT('\0');
				break;
			}
		}
	}

	return -6667;
}


// process a variable function (%@...)
static int _near _fastcall VariableFunctions( LPTSTR lpszFunction, LPTSTR szBuffer )
{
	extern const TCHAR *FUNC_ARRAY[];
	static FILESEARCH ffdir;
	static TCHAR szFindFilename[MAXFILENAME];
	static TCHAR szNumeric[] = _TEXT("%[, 0123456789]");
	LPTSTR pszVar, pszFileName;
	int nLength, nOffset = 0, i = 0, n = 0, nEditFlags = EDIT_DATA, nFH;
	unsigned int uDay, uMonth, uYear, fClip = 0;
	unsigned long ulTemp = 0L;
	long lOffset = 0L;
	TCHAR _far *lpPtr;
	TCHAR *pszArg, szDrive[512];
	QDISKINFO DiskInfo;
	FILESEARCH dir;

	// variable functions MUST have an argument!
	if (( pszFileName = first_arg( szBuffer )) == NULL )
		pszFileName = NULLSTR;

	// check for user function first
	n = UserFunctions( lpszFunction, szBuffer );
	if ( *lpszFunction == _TEXT('\0') )
		return n;

	n = 0;

	// set drive arg (so both "a" and "a:" will work)
	// Check for quoted UNC and remove quotes
	if (( _strnicmp( pszFileName, _TEXT("\\\\"), 2 ) == 0 ) || ( _strnicmp( pszFileName, _TEXT("\"\\\\"), 3 )) == 0 ) {
		strcpy( szDrive, pszFileName );
		StripQuotes( szDrive );
	} else
		sprintf( szDrive, FMT_DISK, ((( pszFileName[0] == _TEXT('\0') ) || (( pszFileName[1] != _TEXT('\0') ) && ( pszFileName[1] != _TEXT(':') ))) ? gcdisk( NULL ) + 64 : *pszFileName ));

	for ( nOffset = 0, pszVar = szBuffer; ( FUNC_ARRAY[nOffset] != NULL ); nOffset++ ) {

		// check FUNC_ARRAY for a function name match
		if ( _stricmp( lpszFunction, FUNC_ARRAY[nOffset]) == 0 ) {

		switch ( nOffset ) {
		case FUNC_ABS:

			// return the absolute value
			for ( pszVar = skipspace( pszVar );  (( *pszVar ) && ( isdigit( *pszVar ) == 0 )); pszVar++ )
				;
			for ( pszArg = pszVar; ( *pszArg != _TEXT('\0') ) && ( isdigit( *pszArg ) || ( *pszArg == gaCountryInfo.szThousandsSeparator[0] ) || ( *pszArg == gaCountryInfo.szDecimal[0] )); pszArg++ )
				;
			*pszArg = _TEXT('\0');
			break;

		case FUNC_AGEDATE:

			// make a formatted date/time from the DOS "age" value
			sscanf( pszVar, _TEXT("%lu,%u"), &ulTemp, &i );

			if (( i < 0 ) || ( i > 6 ))
				return ERROR_INVALID_PARAMETER;

			uYear =  (unsigned)( ulTemp >> 25 ) + 80;
			uMonth = (unsigned)( ulTemp >> 21 ) & 0xF;
			uDay =	 (unsigned)( ulTemp >> 16 ) & 0x1F;

			// replace leading space with a 0
			if (*( pszVar = FormatDate( uMonth, uDay, uYear, i )) == _TEXT(' ') )
				*pszVar = _TEXT('0');

			uYear = (unsigned)( ulTemp >> 11 ) & 0x1F;	// hours
			uMonth = (unsigned)( ulTemp >> 5 ) & 0x3F;	// minutes
			uDay =	 (unsigned)( ulTemp & 0x1F ) << 1;	// seconds
			sprintf( pszVar + strlen( pszVar ), ",%02u%c%02u%c%02u", uYear, gaCountryInfo.szTimeSeparator[0], uMonth, gaCountryInfo.szTimeSeparator[0], uDay );
			break;

		case FUNC_ALIAS:

			// return the alias definition
			EscapeLine( pszVar );
			if (( lpPtr = get_alias( pszVar )) == 0L )
				lpPtr = NULLSTR;
			sprintf( szBuffer, FMT_FAR_PREC_STR, MAXLINESIZ-1, lpPtr );
			return -6666;

		case FUNC_ALTNAME:

			// return alternate (FAT-format) filename
			if ( path_part( pszVar ) != NULL )
				GetShortName( pszVar );
			else {

				if ( ProcessFileName( pszVar, 1 ) != 0 )
					return -ERROR_EXIT;

				dir.szAlternateFileName[0] = _TEXT('\0');
				if ( find_file( 0x4E, pszVar, 0x17 | FIND_NO_ERRORS | FIND_CLOSE, &dir, NULL ) != NULL ) {
					// if no alternate name, it's already a FAT name,
					//   so just return the original
					if ( dir.szAlternateFileName[0] != _TEXT('\0') )
						strcpy( pszVar, dir.szAlternateFileName );
					else
						strcpy( pszVar, dir.szFileName );
				} else
					pszVar = NULLSTR;
			}

			return -6666;

		case FUNC_ASCII:


			// return the ASCII value of the specified character(s)
			//   first, check for something like ASCII[^X`]
			szDrive[0] = _TEXT('\0');
			EscapeLine( pszVar );
			for ( pszArg = pszVar; ( *pszArg ); pszArg++ ) {

				if ( pszArg != pszVar )
					strcat( szDrive, _TEXT(" ") );

				IntToAscii( *pszArg, strend( szDrive ));
				if ( strlen( szDrive ) >= ( sizeof( szDrive ) - 4 ))
					break;
			}
			pszVar = szDrive;
			break;

		case FUNC_ATTRIB:

			// check file attributes
			if ( ProcessFileName( pszFileName, 1 ) != 0 )
				return -ERROR_EXIT;

			if (( n = QueryFileMode( pszFileName, (unsigned int *)&i)) == 0 ) {

				// if no second arg, return current attributes as string
				if (( pszFileName = ntharg( pszVar, 1 )) == NULL ) {
					strcpy( pszVar, show_atts( i ));
					break;
				}

				// strip wacky bits (including extra Win2K bits)
				i &= 0x37;

				// get the desired test (NRHSDA)
				if ( AttributeString( pszFileName, &n ) != 0 )
					return ERROR_INVALID_PARAMETER;

				// test file attributes against flags
				if ((( pszFileName = ntharg( pszVar, 2 )) != NULL ) && ( _ctoupper( *pszFileName ) == _TEXT('P') ))
					IntToAscii((( i & n) == n), pszVar );
				else
					IntToAscii(( i == n ), pszVar );

			} else
				*pszVar = _TEXT('\0');

			break;

		case FUNC_CAPS:

			// get requested delimiters
			if ( *pszFileName == _TEXT('"') ) {
				i = strlen( pszFileName );
				if ( pszFileName[i-1] == _TEXT('"') )
					pszFileName[i-1] = _TEXT('\0');
				EscapeLine( ++pszFileName );
				pszVar = skipspace( pszVar + i + 1 );
			} else {
				pszFileName = _TEXT(" \t");
				pszVar = skipspace( pszVar );
			}

			pszArg = pszVar;

			// capitalize first letter in each word
			for ( i = 1; ( *pszArg ); pszArg++ ) {

				// whitespace char?
				if ( strchr( pszFileName, *pszArg ) != NULL )
					i = 1;
				else if ( i ) { 
					if ( isalpha( *pszArg ))
						*pszArg = _ctoupper( *pszArg );
					i = 0;
				}
			}

			break;

		case FUNC_CDROM:

			// return 1 if the drive is a CD-ROM
			IntToAscii( QueryIsCDROM( gcdisk( szDrive )), pszVar );
			break;

		case FUNC_CHAR:

			// return the character for the specified ASCII / Unicode value
			sprintf( szDrive, FMT_PREC_STR, 511, pszVar );
			*pszVar = _TEXT('\0');

			for ( pszArg = szDrive; ( *pszArg ); ) {

				if ( sscanf( pszArg, _TEXT("%d %n"), &i, &nLength ) == 2 ) {
					pszArg += nLength;

					sprintf( strend(pszVar), FMT_CHAR, i );

				} else
					break;
			}

			break;

		case FUNC_CLIP:

			// paste from the clipboard
			if (( i = atoi( pszVar )) < 0 )	// line to read
				return ERROR_INVALID_PARAMETER;

			if ( GetClipboardLine( i, pszVar, MAXLINESIZ-1 ) != 0 )
				pszVar = _TEXT("**EOC**");
			break;

		case FUNC_CLIPW:

			// copy the text to the clipboard
			EscapeLine( pszVar );
			if (( n = CopyTextToClipboard( pszVar, strlen( pszVar ) )) != 0 )
				return n;

			pszVar = _TEXT("0");
			break;

		case FUNC_CLUSTSIZE:

			if ( QueryDiskInfo( szDrive, &DiskInfo, 0 ))
				return -ERROR_EXIT;
			sprintf( pszVar, FMT_LONG, DiskInfo.ClusterSize );
			break;

		case FUNC_CODEPAGE:

			// device (CON or PRN) codepage, SELECTed by MODE
			strip_trailing( pszFileName, _TEXT(":") );
			if (( n = QueryDeviceCodePage( pszFileName )) < 0 )
				return ERROR_INVALID_PARAMETER;
			IntToAscii( n, pszVar );
			break;

		case FUNC_COM:

			// return ready (1) or not ready (0) for the serial port
			IntToAscii( QuerySerialReady( atoi( pszFileName ) - 1 ), pszVar );
			break;

		case FUNC_COMMA:

			// format a long integer by inserting commas (or other
			// character specified by country_info.szThousandsSeparator)
			AddCommas( pszVar );
			break;

		case FUNC_COMPARE:

			// compare the specified files
			pszFileName = scan( pszVar, _TEXT(","), QUOTES );
			if ( pszFileName == pszVar || pszFileName == BADQUOTES || *pszFileName != _TEXT(',') || pszFileName[1] == _TEXT('\0'))
				return ERROR_INVALID_PARAMETER;
			*pszFileName++ = _TEXT('\0');

			strcpy( szFindFilename, pszFileName );// allow expansion
			if ( ProcessFileName( pszVar, 1 ) || ProcessFileName( szFindFilename, 1 ))
				return -ERROR_EXIT;

			IntToAscii( CompareFiles( pszVar, szFindFilename ), pszVar );
			break;

		case FUNC_CONVERT:
			{

			// convert between two number bases
			if (( sscanf( pszVar, _TEXT("%d ,%d , %n"), &i, &n, &nOffset ) < 2 ) || ( i < 2 ) || ( n < 2 ) || ( n > 36 ))
				return ERROR_INVALID_PARAMETER;

			// "i" is input base; "n" is output base
			pszVar += nOffset;

			for ( ; ( isalnum( *pszVar )); pszVar++ ) {
				nOffset = (( isdigit( *pszVar )) ? *pszVar - _TEXT('0') : ( _ctoupper( *pszVar ) - _TEXT('A')) + 10 );
				if ( nOffset >= i )
					return ERROR_INVALID_PARAMETER;

				ulTemp = ( ulTemp * i ) + nOffset;
			}

			pszVar = szBuffer;

			_ultoa( ulTemp, pszVar, n );

			strupr( pszVar );
			break;
			}

		case FUNC_COUNT:

			// count occurrences of a character in a string
			escape( pszVar );
			if ( sscanf( pszVar, _TEXT("%c%*[^,],%n"), (TCHAR *)&i, &n ) < 2 )
				return ERROR_INVALID_PARAMETER;

			for ( pszVar += n, n = 0; *pszVar; pszVar++ )
				if ( *pszVar == (TCHAR)i )
					n++;
			IntToAscii( n, pszVar = szBuffer );
			break;

		case FUNC_CRC32:
		case FUNC_MD5:
		case FUNC_SHA1:
			// calculate a CRC32, MD5 or SHA1 for the specified file
			if ( ProcessFileName( pszVar, 1 ) != 0 )
				return -ERROR_EXIT;

			if ( is_file( pszVar ) == 0 )
				IntToAscii( -1, pszVar );
			else if ( nOffset != FUNC_CRC32 ) {
				TCHAR ucDigest[20];
				if ( MD5SHA1( pszVar, ucDigest, nOffset == FUNC_SHA1 ))
					IntToAscii( -1, pszVar );
				else for ( i = 0; i < ( nOffset == FUNC_SHA1 ? 20 : 16 ); i++ )
					sprintf( &pszVar[2*i], _TEXT("%02x"), ucDigest[i] );
			} else {
				ulTemp = CRC32( pszVar );
				sprintf( pszVar, _TEXT("%08lx"), ulTemp );
			}

			break;

		case FUNC_CWD:

			// current working directory
			if (( pszVar = gcdir( szDrive, 0 )) == NULL)
				return -ERROR_EXIT;
			break;

		case FUNC_CWDS:

			// cwd w/backslash guaranteed
			if (( pszVar = gcdir( szDrive, 0 )) == NULL )
				return -ERROR_EXIT;
			mkdirname( pszVar, NULLSTR );
			break;

		case FUNC_DOW:		// day of week (Sun - Sat)
		case FUNC_DOWF:		// day of week (Sunday - Saturday)
		case FUNC_DOWI:		// day of week (1-7)
		case FUNC_ISODOWI:	// day of week (1-7, ISO)

			if (( n = MakeDaysFromDate(  &lOffset, pszVar )) != 0 )
				return n;
			n = (int)(( lOffset + 2 ) % 7 );
			if ( nOffset == FUNC_DOW )
				sprintf( pszVar, _TEXT("%.3s"), daytbl[n] );
			else if ( nOffset == FUNC_DOWF )
				strcpy( pszVar, daytbl[n] );
			else if ( nOffset == FUNC_ISODOWI )
				IntToAscii( n ? n : 7, pszVar );
			else
				IntToAscii( n+1, pszVar );
			break;

		case FUNC_DOY:		// day of year (1-366)
		case FUNC_DAY:
		case FUNC_MONTH:
		case FUNC_MONTHF:
		case FUNC_YEAR:
		case FUNC_ISOWEEK:	// week of year (ISO)
		case FUNC_ISOWYEAR:

			if ( GetStrDate( pszVar, &uMonth, &uDay, &uYear ))
				return ERROR_4DOS_INVALID_DATE;

			if ( nOffset == FUNC_DOY ) {
				ISOweekDOY( &uDay, uMonth, &uYear, &n );
				IntToAscii( n, pszVar );
			} else if ( nOffset == FUNC_DAY )
				IntToAscii( uDay, pszVar );
			else if ( nOffset == FUNC_MONTH )
				IntToAscii( uMonth, pszVar );
			else if ( nOffset == FUNC_MONTHF ) {
				if ( uMonth > 0 && uMonth <= 12 )
					strcpy( pszVar, lmontbl[uMonth-1] );
				else
					return ERROR_4DOS_INVALID_DATE;
			} else if ( nOffset == FUNC_ISOWEEK || nOffset == FUNC_ISOWYEAR ) {
				n = ISOweekDOY( &uDay, uMonth, &uYear, &i );
				IntToAscii( nOffset == FUNC_ISOWEEK ? n : uYear, pszVar );
			} else	// year needs to be at least 2 digits, with leading 0 if necessary
				sprintf( pszVar, _TEXT("%02d"), uYear );
			break;

		case FUNC_DATE:

			// return number of days since 1/1/80 for a specified date
			if (( n = MakeDaysFromDate(( long *)&ulTemp, pszVar )) != 0 )
				return n;

			sprintf( pszVar, FMT_LONG, ulTemp );
			break;

		case FUNC_DATECONV:

			// convert date from one format to another
			for ( pszFileName = pszVar; *pszFileName && *pszFileName != _TEXT(','); pszFileName++ )
				;		// search for a comma
			if ( *pszFileName ) {	// found one
				*pszFileName++ = _TEXT('\0'); // terminate date
				if ( sscanf( pszFileName, _TEXT("%d"), &i ) < 1 || i < 0 || i > 6 )
					return ERROR_INVALID_PARAMETER;
			} // else i = 0 still

			if ( GetStrDate( pszVar, &uMonth, &uDay, &uYear ))
				return ERROR_4DOS_INVALID_DATE;

 			// replace leading space with a 0
			if (*( pszVar = FormatDate( uMonth, uDay, uYear, i )) == _TEXT(' ') )
				*pszVar = _TEXT('0');
			break;

		case FUNC_DECIMAL:

			// return the decimal (fractional) part
			if (( pszArg = strchr( pszVar, gaCountryInfo.szDecimal[0] )) != NULL )
				sscanf( pszArg+1, _TEXT("%[0123456789]"), pszVar );

			if (( pszArg == NULL ) || ( *pszVar == _TEXT('\0') ))
				pszVar = _TEXT("0");
			break;

		case FUNC_DESCRIPT:

			// return description for specified file
			szDrive[0] = _TEXT('\0');
			if ( ProcessFileName( pszVar, 1 ) != 0 )
				return -ERROR_EXIT;

			process_descriptions( pszVar, szDrive, DESCRIPTION_READ | DESCRIPTION_PROCESS );
			pszVar = szDrive;
			break;

		case FUNC_DEVICE:

			// return a 1 if the name is a character device
			IntToAscii( QueryIsDevice( pszFileName ), pszVar );
			break;

		case FUNC_DIGITS:

			// returns 1 if the argument is all digits
			for ( ; ( isdigit( *pszVar )); pszVar++ )
				;
			IntToAscii( (( *pszVar == _TEXT('\0')) && ( pszVar != szBuffer )), szBuffer );
			pszVar = szBuffer;
			break;

		case FUNC_DIRSTACK:

			// return nth entry in the directory stack or its length
			n = *pszFileName ? atoi( pszFileName ) : INT_MAX;
			// still i = 0
			for ( lpPtr = gaPushdQueue; *lpPtr && i < n; i++ )
				lpPtr = next_env( lpPtr );
			if ( n < INT_MAX )
				_fstrcpy( pszVar, lpPtr );
			else
				IntToAscii( i, pszVar );
			break;

		case FUNC_DISKFREE:
		case FUNC_DISKTOTAL:
		case FUNC_DISKUSED:
		case FUNC_HDDSIZE:
			{

				// return the disk stats
				if ( QueryDiskInfo( szDrive, &DiskInfo, 0 ))
					return -ERROR_EXIT;

				pszFileName = ntharg( pszVar, 1 );

				if ( pszFileName != NULL ) {
					i = *pszFileName;
					n = pszFileName[1];
				}

				if ( nOffset == FUNC_DISKFREE )
					strcpy( pszVar, DiskInfo.szBytesFree );
				else if ( nOffset == FUNC_DISKTOTAL )
					strcpy( pszVar, DiskInfo.szBytesTotal );
				else if ( nOffset == FUNC_DISKUSED )
					sprintf( pszVar, "(%s-%s)", DiskInfo.szBytesTotal, DiskInfo.szBytesFree );
				else {	// FUNC_HDDSIZE
					static LBA buf = { 26 };
					buf.uFlags = 0;	// PhoenixBIOS 4.0R6.0 bug
					GetHDDParams( GetDrivePhysUnit( gcdisk( szDrive )), &buf );
					sprintf( pszVar, "%s*%u", Format64( &buf.llTotalSectors ), buf.uSectSize );
				}

				// get the size value (B, K, M or G)

				if ( i != 0 ) {
					if ( i == 'k' )
						strcat( pszVar, "\\1000" );
					else if ( i == 'K' )
						strcat( pszVar, "\\1024" );
					else if ( i == 'm' )
						strcat( pszVar, "\\1000000" );
					else if ( i == 'M' )
						strcat( pszVar, "\\1048576" );
					else if ( i == 'g' )
						strcat( pszVar, "\\1000000000" );
					else if ( i == 'G' )
						strcat( pszVar, "\\1073741824" );
				}

				evaluate( pszVar );
				if ( tolower( n ) == 'c' ) 
					AddCommas( pszVar );

				break;
			}

		case FUNC_DOSMEM:

			// get free RAM

			FormatLong( pszVar, pszFileName, (unsigned long)(unsigned int)ServCtrl( SERV_AVAIL, 0 ) << 4 );
			break;

		case FUNC_DRIVETYPE:

			// return drive type (0-6)
			IntToAscii( QueryDriveType( szDrive ), pszVar );
			break;

		case FUNC_DDCSTR:

			// return a Display Data Channel (DDC)
			// Enhanced Display Identification Data (EDID) string
			if (( pszVar = GetDDCstring( atoi( pszFileName ), pszVar )) != NULL )
				strip_trailing( pszVar, _TEXT("\n "));
			break;

		case FUNC_ERRTEXT:

			// returns OS text for the specified code
			// 0 == last system error
			if ( isdigit( *pszVar ) == 0 )
				return ERROR_INVALID_PARAMETER;

			i = atoi( pszVar );
			szBuffer[0] = _TEXT('\0');

			if ( i <= 0 )
				i = _doserrno;
			GetError( i, szBuffer );

			pszVar = szBuffer;
			break;

		case FUNC_CEILING:
		case FUNC_FLOOR:

			pszArg = strend( pszVar );
			sprintf( pszArg, _TEXT("+0%c4999999999=0%c0"), gaCountryInfo.szDecimal[0], gaCountryInfo.szDecimal[0] );
			if ( nOffset == FUNC_FLOOR )
				*pszArg = _TEXT('-');
			goto Eval;

		case FUNC_INC:
		case FUNC_DEC:
			// shorthand for %@EVAL[pszVar+1] or %@EVAL[pszVar-1]
			sprintf( strend(pszVar), (( nOffset == FUNC_INC ) ? _TEXT("+1=0%c10") : _TEXT("-1=0%c10") ), gaCountryInfo.szDecimal[0] );
			//lint -fallthrough

		case FUNC_EVAL:
Eval:
			// calculate simple algebraic expressions
			if ( evaluate( pszVar ) != 0 )
				return -ERROR_EXIT;
			break;

		case FUNC_EXECUTE:
		case FUNC_EXECSTR:

			// execute a command & return its result
			if ( pszFileName == NULLSTR )
				return ERROR_INVALID_PARAMETER;

			// save gszCmdline onto stack
			lpszFunction = strcpy( (LPTSTR)_alloca( ( strlen( gszCmdline ) + 1 ) * sizeof(TCHAR) ), gszCmdline );
			i = ( *pszVar == _TEXT('@') );

			// return the (1st line) of the command output
			if ( nOffset == FUNC_EXECSTR ) {

				if ( GetTempDirectory( pszFileName ) == 0L )
					*pszFileName = _TEXT('\0');

				UniqueFileName( pszFileName );
				AddQuotes( pszFileName );
				pszArg = (LPTSTR)_alloca( ( strlen( pszFileName ) + 1 ) * sizeof(TCHAR) );
				pszFileName = strcpy( pszArg, pszFileName );
				strins( pszVar, _TEXT("( ") );
				sprintf( strend( pszVar ), _TEXT(" ) >!%s"), pszFileName ); 
			}

			// kludge for Netware bug
			close_batch_file();

			// convert line to INT 2Eh format
			n = strlen( pszVar );
			memmove( pszVar+1, pszVar, n+1 );
			*pszVar = (char)n;

			if ( nOffset == FUNC_EXECSTR ) {

				// disable echoing so we don't get the wrong result
				if ( cv.bn >= 0 ) {
					n = bframe[cv.bn].uEcho;
					bframe[cv.bn].uEcho = 0;
				}
				i = cv.fVerbose;
				cv.fVerbose = 0;

				nExecStrRet = DoINT2E( pszVar );
				if ( cv.bn >= 0 )
					bframe[cv.bn].uEcho = n;

				cv.fVerbose = i;
				lOffset = 0;

				// restore gszCmdline
				strcpy( gszCmdline, lpszFunction );
				StripQuotes( pszFileName );
				goto GetFirstLine;
			}

			// don't return result if line began with '@'
			n =	DoINT2E( pszVar );
			if ( i == 1 )
				pszVar = NULLSTR;
			else
				IntToAscii( n, pszVar );

			// restore gszCmdline
			strcpy( gszCmdline, lpszFunction );
			break;

		case FUNC_EXPAND:

			// do a wildcard expansion of the filename argument
			if ( ProcessFileName( pszFileName, 0 ) != 0 )
				return -ERROR_EXIT;

			if ( *pszFileName == _TEXT('\0') )
				return ERROR_INVALID_PARAMETER;

			pszArg = (LPTSTR)_alloca( ( strlen( pszFileName ) + 1 ) * sizeof(TCHAR) );
			pszFileName = strcpy( pszArg, pszFileName );
			GetSearchAttributes( ntharg( pszVar, 1 ));

			// return all matching files
			*pszVar = _TEXT('\0');
			for ( n = FIND_FIRST; ( find_file( n, pszFileName, 0x17 | FIND_NO_ERRORS | FIND_BYATTS | FIND_NO_DOTNAMES, &dir, szDrive ) != NULL ); n = FIND_NEXT ) {
				if ( strlen( pszVar ) + strlen( szBuffer ) + 2 >= (MAXLINESIZ - 1)) {
					FindClose( dir.hdir );
					return ERROR_4DOS_COMMAND_TOO_LONG;
				}
				if ( n != FIND_FIRST )
					strcat( pszVar, _TEXT(" ") );
				AddQuotes( szDrive );
				strcat( pszVar, szDrive );
			}
			break;

		case FUNC_EXTENDED:

			// get free extended memory ( if any) for 286's and 386's
			ulTemp = get_extended();
			FormatLong( pszVar, pszFileName, ( ulTemp << 10 ));
			break;

		case FUNC_EXTENSION:

			// extension
			trim( pszVar, WHITESPACE );
			EscapeLine( pszVar );
			if ((( pszVar = ext_part( pszVar )) != NULL ) && ( *pszVar == _TEXT('.') ))
				pszVar++;
			break;

		case FUNC_FILECLOSE:

			// close the specified file handle
			if (( i = atoi( pszVar )) <= 2 )
				return ERROR_INVALID_HANDLE;
			IntToAscii( _close( i ), pszVar );
			break;

		case FUNC_FILENAME:

			// filename
			trim( pszVar, WHITESPACE );
			EscapeLine( pszVar );
			pszVar = fname_part( pszVar );
			break;

		case FUNC_FILEAGE:	// age of file (as a long)
		case FUNC_FILEDATE:	// file date
		case FUNC_FILETIME:	// file time

			if ( ProcessFileName( pszFileName, 1 ) != 0 )
				return -ERROR_EXIT;

			if ( *pszFileName == _TEXT('\0') )
				return ERROR_INVALID_PARAMETER;

			if ( find_file( FIND_FIRST, pszFileName, 0x17 | FIND_CLOSE, &dir, NULL ) != NULL ) {

				// check for LastWrite, LastAccess, or Creation
				n = 0;
				pszArg = scan( pszVar, _TEXT(","), QUOTES );
				if (( pszArg == BADQUOTES) || (( *pszArg ) && ( *pszArg != _TEXT(',') )))
					return ERROR_INVALID_PARAMETER;

				if ( *pszArg )
					pszArg++;
				strupr( pszArg );
				if ( *pszArg == _TEXT('A') )
					n = 1;
				else if ( *pszArg == _TEXT('C') )
					n = 2;
				while (( *pszArg ) && ( *pszArg++ != _TEXT(',')))
					;

				if ( nOffset == FUNC_FILEAGE ) {

					if ( n == 1 )
						FileTimeToDOSTime( &(dir.ftLastAccessTime ), &(dir.ft.wr_time ), &(dir.fd.wr_date ) );
					else if ( n == 2 )
						FileTimeToDOSTime( &(dir.ftCreationTime ), &(dir.ft.wr_time ), &(dir.fd.wr_date ) );

					ulTemp = dir.fd.wr_date;
					ulTemp = (ulTemp << 16) + dir.ft.wr_time;

					sprintf( pszVar, FMT_ULONG, ulTemp );

				} else if ( nOffset == FUNC_FILEDATE ) {

					// optional date format
					if (( i = atoi( pszArg )) > 5 )
						return ERROR_INVALID_PARAMETER;

					// replace leading space with a 0

					if ( n == 1 )
						FileTimeToDOSTime( &(dir.ftLastAccessTime), &(dir.ft.wr_time ), &(dir.fd.wr_date ) );
					else if ( n == 2 )
						FileTimeToDOSTime( &(dir.ftCreationTime), &(dir.ft.wr_time ), &(dir.fd.wr_date ) );

					if (*( pszVar = FormatDate( dir.fd.file_date.months, dir.fd.file_date.days, dir.fd.file_date.years+80, i )) == ' ' )
						*pszVar = '0';

				} else {
					// file time

					if ( n == 1 )
						FileTimeToDOSTime( &(dir.ftLastAccessTime ), &(dir.ft.wr_time ), &(dir.fd.wr_date ) );
					else if ( n == 2 )
						FileTimeToDOSTime( &(dir.ftCreationTime ), &(dir.ft.wr_time ), &(dir.fd.wr_date ) );

					if ( _ctoupper( *pszArg ) == 'S' )
						sprintf( pszVar, "%02u%c%02u%c%02u", dir.ft.file_time.hours, gaCountryInfo.szTimeSeparator[0], dir.ft.file_time.minutes, gaCountryInfo.szTimeSeparator[0], ( dir.ft.file_time.seconds * 2 ) % 60 );
					else
						sprintf( pszVar, "%02u%c%02u", dir.ft.file_time.hours, gaCountryInfo.szTimeSeparator[0], dir.ft.file_time.minutes );

				}

			} else
				*pszVar = _TEXT('\0');
			break;

		case FUNC_FILEOPEN:

			// open the specified file & return its handle
			if (( pszFileName = ntharg( pszVar, 1 )) == NULL )
				return ERROR_INVALID_PARAMETER;

			if ( _ctoupper( *pszFileName ) == _TEXT('R') )
				n = _O_RDONLY | _O_BINARY;
			else if ( _ctoupper( *pszFileName ) == _TEXT('W') )
				n = _O_WRONLY | _O_BINARY | _O_CREAT | _O_TRUNC;
			else if ( _ctoupper( *pszFileName ) == _TEXT('A') )
				n = _O_RDWR | _O_BINARY | _O_CREAT;
			else
				return ERROR_INVALID_PARAMETER;

			// check for "binary" mode
			if ((( pszFileName = ntharg( pszVar, 2 )) != NULL ) && (( *pszFileName == _TEXT('B') ) || ( *pszFileName == _TEXT('b') ))) {
				// binary mode assumes non-truncate!
				if ( n & _O_WRONLY )
					n &= ~_O_TRUNC;
			}

			pszFileName = first_arg( pszVar );

			if ( ProcessFileName( pszFileName, 1 ) != 0 )
				return -ERROR_EXIT;

			// first try a normal open
			// can't test for _O_RDONLY because it's == 0!
			if (( i = _sopen( pszFileName, n, ((( n & ( _O_WRONLY | _O_RDWR )) == 0 ) ? _SH_DENYWR : _SH_COMPAT ), ( _S_IREAD | _S_IWRITE) )) >= 0 ) {

				// if appending, go to EoF
				if ( n & _O_RDWR ) {
					if ( n & _O_BINARY )
						QuerySeekSize( i );
					else
						SeekToEnd( i );
				}
			}

			pszVar = szBuffer;
			IntToAscii( i, pszVar );
			break;

		case FUNC_FILEREAD:
		case FUNC_FILEREADB:

			// read a line from the specified file handle
			// second argument is read length, optional for FILEREAD
			if (( sscanf( pszVar, _TEXT("%d ,%d"), &i, &n ) < ( nOffset == FUNC_FILEREAD ? 1 : 2 )) || ( i < 3 ) || ( n < ( nOffset == FUNC_FILEREAD ? 0 : 1 )))
				return ERROR_INVALID_PARAMETER;

			if ( n == 0 ) {
				if ( getline( i, pszVar, MAXLINESIZ-1, nEditFlags ) <= 0 )
					pszVar = END_OF_FILE_STR;
			} else {

				if ( n >= MAXLINESIZ )
					n = MAXLINESIZ - 1;

				if (( n = _read( i, pszVar, n )) <= 0 )
					pszVar = END_OF_FILE_STR;
				else
					pszVar[n] = _TEXT('\0');
			}
			if ( nOffset == FUNC_FILEREADB ) {
				szDrive[0] = _TEXT('\0');
				for ( pszArg = pszVar; n > 0 && strlen( szDrive ) < sizeof( szDrive ) - 4; pszArg++, n-- ) {
					if ( pszArg != pszVar )
						strcat( szDrive, _TEXT(" ") );
					IntToAscii( *pszArg, strend( szDrive ));
				}
				pszVar = szDrive;
			}
			break;

		case FUNC_FILES:

			// get the desired test (NRHSDA)
			init_dir();

			// kludge for %@files[xxx,n]
			pszFileName = ntharg( pszVar, 1 );
			lOffset = ((( pszFileName != NULL ) && (_stricmp( pszFileName, _TEXT("n") ) == 0)) ? 0x07 | FIND_NO_ERRORS | FIND_BYATTS : 0x17 | FIND_NO_ERRORS | FIND_BYATTS );

			// set inclusive/exclusive modes
			GetSearchAttributes( pszFileName );
			if (( pszFileName = first_arg( pszVar )) == NULL )
				return ERROR_INVALID_PARAMETER;

			if ( ProcessFileName( pszFileName, 1 ) != 0 )
				return -ERROR_EXIT;

			// return number of matching files
			for ( i = 0, n = FIND_FIRST; ( find_file( n, pszFileName, lOffset, &dir, NULL ) != NULL ); i++, n = FIND_NEXT )
				;
			IntToAscii( i, pszVar );
			break;

		case FUNC_FILESEEK:

			// seek to the specified offset
			if (( sscanf( pszVar, _TEXT("%d ,%ld ,%d"), &i, &lOffset, &n ) != 3 ) || ( i < 3 ) || ((unsigned int)n > 2 ))
				return ERROR_INVALID_PARAMETER;

			sprintf( pszVar, FMT_LONG, (long)_lseek( i, lOffset, n ));
			break;

		case FUNC_FILESEEKL:

			if (( sscanf( pszVar, _TEXT("%d ,%ld"), &i, &lOffset ) != 2 ) || ( i < 3 ))
				return ERROR_INVALID_PARAMETER;

			// rewind & read a line from the file
			RewindFile( i );
			nEditFlags = EDIT_DATA | EDIT_NO_PIPES;

			for ( ; ( lOffset > 0L ); lOffset-- ) {
				if ( getline( i, pszVar, MAXLINESIZ-1, nEditFlags ) <= 0 )
					break;
			}
			sprintf( pszVar, FMT_LONG, _lseek( i, 0L, SEEK_CUR ));
			break;

		case FUNC_FILESIZE:
			{

				t_int64 llTemp;
				// display allocated size?
				if ((( pszVar = ntharg( pszVar, 2 )) != NULL ) && ( _ctoupper( *pszVar ) == _TEXT('A') ))
					i = 1;

				if (( pszFileName = first_arg( szBuffer )) == NULL )
					return ERROR_INVALID_PARAMETER;

				if ( ProcessFileName( pszFileName, 1 ) != 0 )
					return -ERROR_EXIT;

				// file size test

				if ( QueryFileSize64( pszFileName, i, &llTemp ) == 0 ) {

					// get the size value (B, K, or M)
					if (( pszFileName = ntharg( szBuffer, 1 )) == NULL )
						pszFileName = NULLSTR;

					// round file sizes upwards (to match DIR /4)
					if ( *pszFileName == _TEXT('k') )
						Add32To64( &llTemp, 499L );
					else if ( *pszFileName == _TEXT('K') )
						Add32To64( &llTemp, 511L );
					else if ( *pszFileName == _TEXT('m') )
						Add32To64( &llTemp, 499999L );
					else if ( *pszFileName == _TEXT('M') )
						Add32To64( &llTemp, 524287L );
					else if ( *pszFileName == _TEXT('g') )
						Add32To64( &llTemp, 499999999L );
					else if ( *pszFileName == _TEXT('G') )
						Add32To64( &llTemp, 536870911L );

					pszVar = szBuffer;

					if ( Format64bit( pszVar, pszFileName, &llTemp ))
						return ERROR_INVALID_PARAMETER;	// unknown type

				} else
					pszVar = _TEXT("-1");

				break;
			}

		case FUNC_FILEWRITE:

			// write a line to the specified file
			sscanf( pszVar, _TEXT("%d %*1[,]%n"), &i, &n );

			if ( i < 1 )
				return ERROR_INVALID_PARAMETER;

			n = qprintf( i, FMT_STR_CRLF, pszVar + n );

			IntToAscii( n, pszVar );
			break;

		case FUNC_FILEWRITEB:

			// write chars to the specified file
			sscanf( pszVar, _TEXT("%d,%d %*1[,]%n"), &i, &nLength, &n );

			if ( i < 3 )
				return ERROR_INVALID_PARAMETER;

			if ( nLength < 0 ) {
				int j, k, l = 1, m;
				for ( j = n, n = 0; l; j += l, n += m )
					m = sscanf( pszVar + j, FMT_UINT_LEN, &k, &l ) > 1 && l ? _write( i, &k, 1 ) : 0;
			} else
				n = _write( i, pszVar + n, nLength );

			IntToAscii( n, pszVar );
			break;

		case FUNC_FINDFIRST:
			// initialize the search structure
			memset( &ffdir, '\0', sizeof(FILESEARCH) );

			// get filespec
			if ( ProcessFileName( pszFileName, 0 ) != 0 )
				return -ERROR_EXIT;

			copy_filename( szFindFilename, pszFileName );
			//lint -fallthrough

		case FUNC_FINDNEXT:

			// find first/next file
			init_dir();

			// set inclusive/exclusive modes
			GetSearchAttributes( ntharg( pszVar, 1 ));

			if ( find_file((( nOffset == FUNC_FINDFIRST) ? FIND_FIRST : FIND_NEXT), szFindFilename, 0x17 | FIND_NO_DOTNAMES | FIND_BYATTS | FIND_NO_ERRORS, &ffdir, pszVar ) == NULL ) {
				*pszVar = _TEXT('\0');
				break;
			}

			nOffset = FUNC_FINDNEXT;
			break;

		case FUNC_FINDCLOSE:

			// close the "find file" search handle
			IntToAscii((( FindClose( ffdir.hdir ) == TRUE ) ? 0 : GetLastError()), pszVar );
			break;

		case FUNC_FORMAT:

			// format a string
			pszFileName = scan( pszVar, _TEXT(","), QUOTES );
			if (( pszFileName == BADQUOTES ) || ( *pszFileName != _TEXT(',') ) || ( pszFileName[1] == _TEXT('\0') ))
				return ERROR_INVALID_PARAMETER;
			*pszFileName++ = _TEXT('\0');

			// save format and source strings
			pszVar = strcpy( (LPTSTR)_alloca( ( strlen( pszVar ) + 3 ) * sizeof(TCHAR) ), pszVar );
			pszFileName = strcpy( (LPTSTR)_alloca( ( strlen( pszFileName ) + 1 ) * sizeof(TCHAR) ), pszFileName );
			strins( pszVar, _TEXT("%") );
			strcat( pszVar, _TEXT("s") );

			sprintf( szBuffer, pszVar, pszFileName );
			pszVar = szBuffer;
			break;

		case FUNC_FSTYPE:

			// return file system type
			pszVar = gszFileSystemName;
			pszVar[0] = _TEXT('\0');
			if ( ifs_type( szDrive ) && pszVar[0] == _TEXT('\0') )
				pszVar = _TEXT("?");	// NTFS for Windows 98
			break;

		case FUNC_FULLNAME:

			// return fully qualified filename
			if ( ProcessFileName( pszVar, 1 ) != 0 )
				return -ERROR_EXIT;
			break;

		case FUNC_FUNCTION:

			// return the function definition
			EscapeLine( pszVar );
			if (( lpPtr = get_list( pszVar, glpFunctionList )) == 0L )
				lpPtr = NULLSTR;
			sprintf( szBuffer, FMT_FAR_PREC_STR, MAXLINESIZ-1, lpPtr );
			return -6666;

		case FUNC_HISTORY:

			// return a line or a word from the command history
			n = -1;	// still i = 0
			if ( sscanf( pszVar, "%d,%d", &i, &n ) < 1 || i < 0 )
				return ERROR_INVALID_PARAMETER;
			for ( lpPtr = NULL; i >= 0; i-- )
				lpPtr = prev_hist( lpPtr );
			_fstrcpy( pszVar, lpPtr );
			if ( n >= 0 ) {	// now i = -1
				for ( pszArg = pszVar; n >= 0 && *pszArg; pszArg++ ) {
					if ( isdelim( *pszArg ))
						i = TRUE;
					else {
						if ( i )
							n--;
						i = FALSE;
						pszVar = pszArg;
					}
				}
				if ( *pszArg == _TEXT('\0') ) {
					pszVar = pszArg;
					break;// not reached - return empty string
				}
				while ( !isdelim( *pszArg ))
					pszArg++;
				*pszArg = _TEXT('\0');
			}
			break;

		case FUNC_IF:
			// return a value based upon the result of the condition
			pszVar = scan( pszVar, _TEXT(","), QUOTES );
			if (( pszVar == BADQUOTES) || ( *pszVar != _TEXT(',') ) || ( pszVar[1] == _TEXT('\0') ))
				return ERROR_INVALID_PARAMETER;

			*pszVar++ = _TEXT('\0');
			pszArg = (LPTSTR)_alloca( ( strlen( pszVar ) + 1 ) * sizeof(TCHAR) );
			pszVar = strcpy( pszArg, pszVar );

			n = TestCondition( szBuffer, 2 );

			// reset the default switch character

			gpIniptr->SwChr = QuerySwitchChar();

			if ( n == -USAGE_ERR )
				return ERROR_4DOS_BAD_SYNTAX;

			pszArg = scan( pszVar, _TEXT(","), QUOTES );
			if (( pszArg == BADQUOTES) || (( *pszArg ) && ( *pszArg != _TEXT(',') )))
				return ERROR_INVALID_PARAMETER;

			szBuffer[0] = _TEXT('\0');
			if ( n > 0 ) {
				*pszArg = _TEXT('\0');
				strcpy( szBuffer, pszVar );
			} else {
				// n == 0
				if ( *pszArg )
					pszArg++;
				sscanf( pszArg, _TEXT("%[^\032]"), szBuffer );
			}
			pszVar = szBuffer;
			break;

		case FUNC_INDEX:

			// return the index of the first part of the source string
			//   that includes the search substring
			pszVar = scan( pszVar, _TEXT(","), QUOTES );
			if (( pszVar == BADQUOTES ) || (*pszVar != _TEXT(',') ) || ( pszVar[1] == _TEXT('\0') ))
				return ERROR_INVALID_PARAMETER;

			*pszVar++ = _TEXT('\0');

			// look for (optional) 3rd argument
			pszArg = scan( pszVar, _TEXT(","), QUOTES );
			if ( pszArg == BADQUOTES )
				return ERROR_INVALID_PARAMETER;
			
			if ( *pszArg == _TEXT(',') ) {
				*pszArg++ = _TEXT('\0');
				// if offset == 0, return the number of matches
				if (( nOffset = atoi( pszArg )) == 0 )
					nOffset = 0x7FFF;
			} else
				nOffset = 1;

			// Perform simple case-insensitive search
			EscapeLine( szBuffer );
			EscapeLine( pszVar );
			
			pszArg = ((( nOffset > 0 ) || ( szBuffer[0] == _TEXT('\0') )) ? szBuffer : strend( szBuffer ) - 1 );
			for ( n = strlen( pszVar ); ; (( nOffset > 0 ) ? pszArg++ : pszArg-- )) {

				for ( ; (( *pszArg ) && ( _strnicmp( pszArg, pszVar, n ) != 0 )); (( nOffset > 0 ) ? pszArg++ : pszArg-- ))
					;

				if ( *pszArg == _TEXT('\0') ) {
					// a request for the number of matches?
					if ( nOffset > 0x4000 )
						IntToAscii( 0x7FFF - nOffset, szBuffer );
					else	// no match
						IntToAscii( -1, szBuffer );
					return 0;
				}

				if ( nOffset > 0 ) {
					if ( --nOffset <= 0 )
						break;
				} else if ( ++nOffset >= 0 )
					break;
			}

			// return position
			IntToAscii( (INT)( pszArg - szBuffer), szBuffer );
			return 0;

		case FUNC_INIREAD:
			{
				LPTSTR pszKey, pszSection;

				// read an entry from an .INI file
				// @iniread[inifilename, section, key]
				pszFileName = pszVar;
				pszSection = scan( pszVar, _TEXT(","), QUOTES );
				if (( pszSection == BADQUOTES ) || (*pszSection != _TEXT(',') ) || ( pszSection[1] == _TEXT('\0') ))
					return ERROR_INVALID_PARAMETER;
				*pszSection++ = _TEXT('\0');

				pszKey = scan( pszSection, _TEXT(","), QUOTES );
				if (( pszKey == BADQUOTES ) || (*pszKey != _TEXT(',') ) || ( pszKey[1] == _TEXT('\0') ))
					return ERROR_INVALID_PARAMETER;
				*pszKey++ = _TEXT('\0');

				StripQuotes( pszFileName );
				StripQuotes( pszSection );

				if ( IniReadWrite( 0, pszFileName, pszSection, pszKey, szDrive ) == NULL )
					szDrive[0] = _TEXT('\0');

				pszVar = szDrive;
				break;
			}

		case FUNC_INIWRITE:
			{
				LPTSTR pszKey, pszValue, pszSection;

				// write an entry to an .INI file
				// @iniwrite[inifilename, section, key, value]
				pszFileName = pszVar;
				pszSection = scan( pszVar, _TEXT(","), QUOTES );
				if (( pszSection == BADQUOTES ) || (*pszSection != _TEXT(',') ) || ( pszSection[1] == _TEXT('\0') ))
					return ERROR_INVALID_PARAMETER;
				*pszSection++ = _TEXT('\0');

				pszKey = scan( pszSection, _TEXT(","), QUOTES );
				if (( pszKey == BADQUOTES ) || (*pszKey != _TEXT(',') ) || ( pszKey[1] == _TEXT('\0') ))
					return ERROR_INVALID_PARAMETER;
				*pszKey++ = _TEXT('\0');

				pszValue = scan( pszKey, _TEXT(","), QUOTES );
				if (( pszValue == BADQUOTES ) || (*pszValue != _TEXT(',') ))
					return ERROR_INVALID_PARAMETER;
				*pszValue++ = _TEXT('\0');

				StripQuotes( pszFileName );
				StripQuotes( pszSection );
				StripQuotes( pszKey );

				if ( IniReadWrite( 1, pszFileName, pszSection, pszKey, pszValue ) != NULL )
					pszVar = "0";
				else
					pszVar = "1";

				break;
			}

		case FUNC_INTEGER:

			// return the integer part
			szNumeric[2] = gaCountryInfo.szThousandsSeparator[0];

			// kludge for stupid people who do %@integer[.1234]
			if ( *pszVar == gaCountryInfo.szDecimal[0] ) {
				pszVar = _TEXT("0");
				break;
			}

			// kludge for stupid people who do "%@integer[10-10-94]"
			i = ((( *pszVar == _TEXT('-') ) || ( *pszVar == _TEXT('+') )) && ( isdigit( pszVar[1] )));
			sscanf( pszVar+i, szNumeric, pszVar+i );

			// kludge for +/- 0
			if (( i > 0 ) && ( stricmp( pszVar+i, _TEXT("0") ) == 0 ))
				strcpy( pszVar, pszVar+i );
			break;

		case FUNC_INSERT:
		case FUNC_SUBST:

			// insert a string into or substitute in another
			if (( sscanf( pszVar, _TEXT("%d ,%n"), &i, &n ) < 1 ) || ( i < 0 ))
				return ERROR_INVALID_PARAMETER;

			pszVar += n;
			pszFileName = pszVar;
			pszVar = scan( pszVar, _TEXT(","), QUOTES );
			if (( pszVar == BADQUOTES ) || ( *pszVar != _TEXT(',') ))
				return ERROR_INVALID_PARAMETER;
			*pszVar++ = _TEXT('\0');

			n = strlen( pszVar );
			if ( i > n )
				i = n;
			if (( nOffset == FUNC_INSERT ? n : i ) + 2 * strlen( pszFileName ) > MAXLINESIZ )
				return ERROR_4DOS_COMMAND_TOO_LONG;
			if ( nOffset == FUNC_INSERT )
				strins( pszVar + i, pszFileName );
			else
				strcpy( pszVar + i, pszFileName );
			break;

		case FUNC_ISALNUM:
			n = _UPPER | _LOWER | _DIGIT;
			goto WhatChars;

		case FUNC_ISALPHA:
			n = _UPPER | _LOWER;
			goto WhatChars;

		case FUNC_ISASCII:
#ifdef __WATCOMC__
			n = _CNTRL | _PRINT;
#else
			n = _CONTROL | _BLANK | _PUNCT | _UPPER | _LOWER | _DIGIT;
#endif
			goto WhatChars;

		case FUNC_ISCNTRL:
#ifdef __WATCOMC__
			n = _CNTRL;
#else
			n = _CONTROL;
#endif
			goto WhatChars;

		case FUNC_ISDIGIT:
			n = _DIGIT;
			goto WhatChars;

		case FUNC_ISLOWER:
			n = _LOWER;
			goto WhatChars;

		case FUNC_ISPRINT:
#ifdef __WATCOMC__
			n = _PRINT;
#else
			n = _BLANK | _PUNCT | _UPPER | _LOWER | _DIGIT;
#endif
			goto WhatChars;

		case FUNC_ISPUNCT:
			n = _PUNCT;
			goto WhatChars;

		case FUNC_ISSPACE:
			n = _SPACE;
			goto WhatChars;

		case FUNC_ISUPPER:
			n = _UPPER;
			goto WhatChars;

		case FUNC_ISXDIGIT:
#ifdef __WATCOMC__
			n = _XDIGT;
#else
			n = _HEX;
#endif
WhatChars:
			for ( i = 0; pszVar[i]; i++ )
#ifdef __WATCOMC__
				if (!(_IsTable[pszVar[i]+1] & n ))
#else
				if (!((_ctype+1)[pszVar[i]] & n ))
#endif
					break;
			IntToAscii( !pszVar[i], pszVar );
			break;

		case FUNC_LABEL:

			// get the volume label for the specified drive
			if ( QueryVolumeInfo( szDrive, pszVar, &ulTemp ) == NULL )
				*pszVar = _TEXT('\0');
			break;

		case FUNC_LEFT:
		case FUNC_RIGHT:

			// substring - check for valid syntax
			//   LEFT[length,string]    RIGHT[length,string]

			if (( sscanf( pszVar, _TEXT("%d ,%n"), &i, &n ) < 1 ) || ( n < 0 ))
				return ERROR_INVALID_PARAMETER;

			pszVar += n;

			n = strlen( pszVar );
			if (( i < 0 ) && ( -i > n ))
				i = -n;

			// if i is negative, take all but the rightmost (@LEFT) or leftmost (@RIGHT) i characters
			if ( nOffset == FUNC_LEFT )
				sprintf( szBuffer, FMT_PREC_STR, (( i < 0 ) ? n + i : i ), pszVar );
			else {
				if ( i > n )
					i = n;
				sprintf( szBuffer, FMT_STR, strend( pszVar ) - (( i < 0 ) ? n + i : i ));
			}
			pszVar = szBuffer;
			break;

		case FUNC_LENGTH:

			// length of string
			IntToAscii( strlen( pszVar ), pszVar );
			break;

		case FUNC_LINE:

			// get line from file
			if (( sscanf( pszVar, _TEXT("%*[^,],%ld"), &lOffset ) != 1 ) || ( lOffset < 0L ))
				return ERROR_INVALID_PARAMETER;
			//lint -fallthrough

		case FUNC_LINES:	// get # of lines in file

			if ( nOffset == FUNC_LINES )
				lOffset = LONG_MAX;

			// kludge for Netware bug
			close_batch_file();

			if ( pszFileName == NULLSTR )
				return ERROR_INVALID_PARAMETER;

			EscapeLine( pszFileName );

			if ( QueryIsCON( pszFileName ))
				nFH = STDIN;

			else if ( stricmp( pszFileName, CLIP ) == 0 )
				fClip = 1;

			else {

// FIXME - for ^C
				if (( *pszFileName ) && ( mkfname( pszFileName, 0 ) == NULL ))
					return -ERROR_EXIT;
GetFirstLine:
				if (( nFH = _sopen( pszFileName, (_O_RDONLY | _O_BINARY), _SH_DENYWR )) < 0 )
					return _doserrno;
			}

			// read a line from the file
			for ( ; ( lOffset >= 0L ); lOffset-- ) {

				if ( fClip ) {
					n = (( GetClipboardLine( i, pszVar, MAXLINESIZ-1 ) == 0 ) ? 1 : 0 );
					i++;
				} else
					n = getline( nFH, pszVar, MAXLINESIZ-1, nEditFlags );

				if ( n <= 0 ) {
					if ( nOffset == FUNC_LINE )
						pszVar = END_OF_FILE_STR;
					break;
				}
			}

			if (( fClip == 0 ) && ( nFH ))
				_close( nFH );

			if ( nOffset == FUNC_LINES )
				sprintf( pszVar, FMT_LONG, ( LONG_MAX - ( lOffset + 1 )));
			else if ( nOffset == FUNC_EXECSTR )
				remove( pszFileName );

			break;

		case FUNC_LOWER:

			// shift string to lower case
			strlwr( pszVar );
			break;

		case FUNC_LPT:

			// return ready (1 ) or not ready (0 ) for the printer
			IntToAscii( QueryPrinterReady( atoi( pszFileName ) - 1 ), pszVar );
			break;

		case FUNC_MAKEAGE:

			// make an "age long" from the specified date & time
			uMonth = uDay = uYear = 0;
			if ( GetStrDate( pszVar, &uMonth, &uDay, &uYear ))
				return ERROR_4DOS_INVALID_DATE;

			uYear -= 1980;
			ulTemp = ( uDay + ( uMonth << 5 ) + ( uYear << 9 ));
			ulTemp <<= 16;

			for ( pszFileName = pszVar; (( *pszFileName != _TEXT('\0') ) && ( *pszFileName++ != _TEXT(',') )); )
				;

			uDay = uMonth = uYear = 0;
			sscanf( pszFileName, _TEXT("%u%*c%u%*c%u"), &uDay, &uMonth, &uYear );
			if (( uDay >= 24 ) || ( uMonth >= 60 ) || ( uYear >= 60 ))
				return ERROR_4DOS_INVALID_TIME;

			ulTemp += (ULONG)(( uYear / 2 ) + ( uMonth << 5 ) + ( uDay << 11 ));
			sprintf( pszVar, FMT_ULONG, ulTemp );
			break;

		case FUNC_MAKEDATE:

			// make a formatted date from the number of days
			sscanf( pszVar, _TEXT("%lu,%u"), &ulTemp, &i );

			if (( n = MakeDateFromDays( ulTemp, &uYear, &uMonth, &uDay )) != 0 )
				return n;

			if (( i < 0 ) || ( i > 6 ))
				return ERROR_INVALID_PARAMETER;

			// replace leading space with a 0
			if (*( pszVar = FormatDate( uMonth, uDay, uYear, i )) == _TEXT(' ') )
				*pszVar = _TEXT('0');
			break;

		case FUNC_MAKETIME:

			// make a formatted time from the number of seconds
			sscanf( pszVar, FMT_ULONG, &ulTemp );

			// don't allow anything past 23:59:59
			if ( ulTemp >= 86400L )
				return ERROR_INVALID_PARAMETER;

			sprintf( pszVar, _TEXT("%02lu%c%02lu%c%02lu"), ( ulTemp / 3600 ), gaCountryInfo.szTimeSeparator[0], ((ulTemp % 3600 ) / 60 ), gaCountryInfo.szTimeSeparator[0], (ulTemp % 60 ) );
			break;

		case FUNC_MAX:
		case FUNC_MIN:
		case FUNC_AVERAGE:
			{
				long lNum = 0;

				// return largest/smallest/avergage number in list
				for ( pszArg = pszVar; ( *pszArg != _TEXT('\0') ); ) {

					if ( sscanf( pszArg, _TEXT("%ld%*[ ,\t]%n"), &lOffset, &i ) < 2 )
						return ERROR_INVALID_PARAMETER;

					if ( pszArg == pszVar )
						lNum = lOffset;
					else if ( nOffset == FUNC_MAX ) {
						if ( lOffset > lNum )
							lNum = lOffset;
					} else if ( nOffset == FUNC_MIN ) {
						if ( lOffset < lNum )
							lNum = lOffset;
					} else if ( nOffset == FUNC_AVERAGE ) {
						lNum += lOffset;
						n++;
					}

					pszArg += i;
				}

				sprintf( pszVar, FMT_LONG, lNum );

				if ( nOffset == FUNC_AVERAGE && n ) {
					char s[8];
					sprintf( s, "/%d", n + 1 );
					strcat( pszVar, s );
					evaluate( pszVar );
				}
				break;
			}

		case FUNC_NAME:

			// filename
			trim( pszVar, WHITESPACE );
			EscapeLine( pszVar );
			pszVar = fname_part( pszVar );
			if (( pszFileName = strrchr( pszVar, _TEXT('.') )) != NULL )
				*pszFileName = _TEXT('\0');
			break;

		case FUNC_NUMERIC:

			// returns 1 if the argument is numeric
			IntToAscii( QueryIsNumeric( pszVar ), szBuffer );
			pszVar = szBuffer;
			break;


		case FUNC_PATH:

			// path part
			trim( pszVar, WHITESPACE );
			EscapeLine( pszVar );
			pszVar = path_part( pszVar );
			break;

		case FUNC_QUOTE:

			// enclose name containing whitespace by double quotes
			AddQuotes( pszVar );
			break;

		case FUNC_RANDOM:
			{
				// get min & max
				long lStart, lEnd;
				if (( sscanf( pszVar, _TEXT("%ld ,%ld"), &lStart, &lEnd ) != 2 ) || ( lStart > lEnd))
					return ERROR_INVALID_PARAMETER;

				sprintf( pszVar, FMT_LONG, GetRandom() % ( ++lEnd - lStart ) + lStart );
				break;
			}

		case FUNC_READSCR:

			// read a string from the screen
			if (( GetCursorRange( pszVar, &i, &n ) != 0 ) || ( sscanf( pszVar, _TEXT("%*d ,%*d ,%d"), &nLength ) != 1 ))
				return ERROR_INVALID_PARAMETER;

			if ( nLength >= MAXLINESIZ - 1 )
				nLength = MAXLINESIZ - 2;

			for ( nOffset = 0; ( nOffset < nLength ); nOffset++, n++ )
				ReadCellStr((TCHAR _far *)(pszVar + nOffset), 2, i, n );
			pszVar[nOffset] = _TEXT('\0');

			break;

		case FUNC_READY:

			// is specified drive ready?
			IntToAscii( QueryDriveReady( gcdisk( szDrive )), pszVar );
			break;


		case FUNC_REMOTE:

			// is specified drive remote?
			if ( is_net_drive( szDrive ))
				IntToAscii( 1, pszVar );
			else
				IntToAscii( QueryDriveRemote( gcdisk( szDrive )), pszVar );
			break;

		case FUNC_REMOVABLE:

			// is specified drive removable?
			IntToAscii( QueryDriveRemovable( gcdisk( szDrive )), pszVar );
			break;

		case FUNC_REPEAT:

			// repeat character "n" times
			escape( pszVar );
			if (( sscanf( pszVar, _TEXT("%c%*[^,],%d"), (TCHAR *)&i, &n ) < 1 ) || ( (unsigned int)n > MAXLINESIZ-1 ))
				return ERROR_INVALID_PARAMETER;

			// can't use memset() with Unicode
			wcmemset( pszVar, (TCHAR)i, n );
			pszVar[n] = _TEXT('\0');
			break;

		case FUNC_REPLACE:
			{
				// replaces occurrences of "str1" with "str2"
				pszVar = scan( szBuffer + (( szBuffer[0] == gpIniptr->EscChr ) ? 2 : 1 ), _TEXT(","), QUOTES );
				if (( pszVar == BADQUOTES ) || ( *pszVar != _TEXT(',') ) || ( pszVar[1] == _TEXT('\0') ))
					return ERROR_INVALID_PARAMETER;

				*pszVar++ = _TEXT('\0');
				EscapeLine( szBuffer );
				pszFileName = strdup( szBuffer );

				pszArg = scan( pszVar, _TEXT(","), QUOTES );
				if (( pszArg == BADQUOTES ) || ( *pszArg != _TEXT(',') ) || ( pszArg[1] == _TEXT('\0') ))
					return ERROR_INVALID_PARAMETER;

				*pszArg++ = _TEXT('\0');
				EscapeLine( pszVar );
				pszVar = strdup( pszVar );
				strcpy( szBuffer, pszArg );

				pszArg = szBuffer;
				if ( *pszArg == _TEXT('\0') )
					return ERROR_INVALID_PARAMETER;

				while ((( pszArg = strstr( pszArg, pszFileName )) != NULL ) && (( strlen( szBuffer ) + 2 ) < MAXLINESIZ )) {
					strcpy( pszArg, pszArg + strlen( pszFileName ));
					strins( pszArg, pszVar );
					pszArg += strlen( pszVar );
				}

				free( pszFileName );
				free( pszVar );
				pszVar = szBuffer;
				break;
			}

		case FUNC_REVERSE:

			// reverse string
			_strrev( pszVar );
			break;


		case FUNC_SEARCH:

			if ( pszFileName == NULLSTR )
				return ERROR_INVALID_PARAMETER;

			// search for filename
			// defaults to search in PATH
			if (( pszVar = strchr( pszVar, _TEXT(',') )) != NULL )
				pszVar = skipspace( pszVar+1 );

			StripQuotes( pszFileName );
			EscapeLine( pszFileName );
			if (( pszVar = searchpaths( pszFileName, pszVar, TRUE, NULL )) != NULL )
				mkfname( pszVar, 0 );
			break;

		case FUNC_SELECT:		// popup a selection window
			{

				int nTop, nLeft, nBottom, nRight, nSort = 0;
				// 4DOS gets 64K; everything else gets realloc'd
				unsigned int uListSize = 0xFFF0;
				unsigned long ulSize = 0L;
				TCHAR _far * _far *lppList = 0L;
				TCHAR szTitle[80];

				// get line from file - check for valid syntax
				if (( sscanf( pszVar, _TEXT("%*[^,],%d ,%d ,%d ,%d %*[,]%79[^,],%d"), &nTop, &nLeft, &nBottom, &nRight, szTitle, &nSort ) < 4 ) || (( nBottom - nTop ) < 2 ) || (( nRight - nLeft ) < 2 ))
					return ERROR_INVALID_PARAMETER;

				// kludge for Netware bug
				close_batch_file();

				nEditFlags = EDIT_DATA;

				// only set fd to STDIN if we're reading from a disk file
				if ( QueryIsCON( pszFileName ))
					n = STDIN;

				else {

					// if CLIP:, copy clipboard to file & open it
					if ( stricmp( pszFileName, CLIP ) == 0 ) {
						// build temp file name
						RedirToClip( pszFileName, 0 );
						if (( n = CopyFromClipboard( pszFileName )) != 0 )
							return n;
					}

					if (( *pszFileName ) && ( mkfname( pszFileName, 0 ) == NULL ))
						return -ERROR_EXIT;

					if (( n = _sopen( pszFileName, (_O_RDONLY | _O_BINARY), _SH_DENYNO)) < 0 )
						return _doserrno;

				}

				HoldSignals();

				lpPtr = (TCHAR _far *)AllocMem( &uListSize );
				uListSize -= 0xFF;

				for ( i = 0; ; i++ ) {

					// allocate memory for 256 entries at a time
					if (( i % 256 ) == 0 ) {

						ulSize += 1024;

						lppList = (TCHAR _far * _far *)ReallocMem( lppList, ulSize );
						if (( i == 0 ) && ( lppList != 0L ))
							lppList[0] = lpPtr;
					}

					if ( getline( n, szBuffer, MAXLINESIZ-1, nEditFlags ) <= 0 )
						break;

					nLength = strlen( szBuffer ) + 1;

					if (( lppList == 0L ) || ( lppList[0] == 0L ) || ((((unsigned int)( lpPtr - lppList[0]) + nLength ) * sizeof(TCHAR) ) >= uListSize )) {

							_close( n );
							FreeMem( lppList[0] );
							FreeMem( lppList );
							return ERROR_NOT_ENOUGH_MEMORY;

					}

					_fstrcpy( lpPtr, szBuffer );
					lppList[i] = lpPtr;
					lpPtr += nLength;
				}

				// if reading from STDIN, reset it to the console
				if ( n == STDIN )
					_dup2( STDOUT, STDIN );
				else
					_close( n );

				EnableSignals();

				if ( i == 0 ) {
					FreeMem( lppList[0]);
					FreeMem( lppList );
					return ERROR_4DOS_FILE_EMPTY;
				}

				*(++lpPtr ) = _TEXT('\0');

				// call the popup window
				if (( lpPtr = wPopSelect( nTop, nLeft, ( nBottom - nTop ) - 1, ( nRight - nLeft ) - 1, lppList, i, 1, szTitle, NULL, NULL, nSort )) != 0L )
					_fstrcpy( pszVar, lpPtr );
				else
					pszVar = NULL;

				// free the pointer array & list memory
				FreeMem( lppList[0] );
				FreeMem( lppList );

				// reenable signal handling after cleanup
				EnableSignals();

				// if we aborted wPopSelect with ^C, bomb after cleanup
				if ( cv.fException )
					longjmp( cv.env, -1 );

				break;
			}

		case FUNC_SERIAL:

			// get the volume serial number for the specified drive
			if ( QueryVolumeInfo( szDrive, pszVar, &ulTemp ) == NULL )
				*pszVar = _TEXT('\0');
			else
				sprintf( pszVar, _TEXT("%04lx:%04lx"), ulTemp >> 16, ulTemp & 0xFFFF );
			break;

		case FUNC_SMBSTR:

			// get the n-th string of type i from SMBIOS (DMI)
			escape( pszVar );
			if (( sscanf( pszVar, _TEXT("%d%*[^,],%d"), &i, &n ) < 1 ) || i < 0 || i > 127 || n <= 0 )
				return ERROR_INVALID_PARAMETER;

			_fstrcpy( pszVar, GetDMIstring((BYTE)i, n ));
			break;

		case FUNC_LTRIM:
		case FUNC_RTRIM:
		case FUNC_STRIP:

			// remove specified characters from string
			pszFileName = scan( pszVar, _TEXT(","), QUOTES );
			if (( pszFileName == BADQUOTES ) || ( *pszFileName != _TEXT(',') ) || ( pszFileName[1] == _TEXT('\0') ))
				return ERROR_INVALID_PARAMETER;
			*pszFileName++ = _TEXT('\0');
			i = strlen( pszVar ) - 1;
			if (( *pszVar == _TEXT('"') ) && ( pszVar[i] == _TEXT('"') )) {
				pszVar[ i ] = _TEXT('\0');
				strcpy( pszVar, pszVar + 1 );
			}

			// remove escapes from the chars to remove
			EscapeLine( pszVar );

			// remove specified leading or trailing characters?
			if ( nOffset == FUNC_LTRIM )
				strip_leading( pszFileName, pszVar );
			else if ( nOffset == FUNC_RTRIM )
				strip_trailing( pszFileName, pszVar );
			else for ( pszVar = pszFileName; ( *pszVar != _TEXT('\0') ); ) {

				// skip escaped characters
				if (( *pszVar == gpIniptr->EscChr ) && (( gpIniptr->Expansion & EXPAND_NO_ESCAPES ) == 0 )) {
					pszVar++;
					if ( *pszVar )
						pszVar++;
					continue;
				}

				if ( strchr( szBuffer, *pszVar ) != NULL )
					strcpy( pszVar, pszVar + 1 );
				else
					pszVar++;
			}

			pszVar = pszFileName;
			break;

		case FUNC_INSTR:

			// substring - check for valid syntax
			//   INSTR[start,length,string]

			if ((( n = sscanf( pszVar, _TEXT("%d ,%d %*1[,]%n"), &i, &nLength, &nOffset )) < 2 ) || ( i < 0 ))
				return ERROR_INVALID_PARAMETER;
			if ( n <= 2 )
				n = INT_MAX;
			else
				n = nLength;
			pszVar += nOffset;
			goto get_substr;

		case FUNC_SUBSTR:

			// substring - check for valid syntax
			//   SUBSTR[string,start,length]

			pszVar = scan( pszVar, _TEXT(","), QUOTES );
			if (( pszVar == BADQUOTES ) || ( *pszVar != _TEXT(',') ))
				return ERROR_INVALID_PARAMETER;

			*pszVar++ = _TEXT('\0');

			// default to remainder of string
			n = INT_MAX;
			if (( sscanf( pszVar, _TEXT("%d ,%d"), &i, &n ) < 1 ) || ( i < 0 ))
				return ERROR_INVALID_PARAMETER;
			pszVar = szBuffer;
get_substr:
			nLength = strlen( pszVar );

			// point to requested offset
			if ( n > 0 )
				pszVar += (( nLength > i ) ? i : nLength );
			else {
				n = -n;
				nLength--;		// adjust for offset 0
				if ( i > nLength )
					*pszVar = _TEXT('\0');
				else {
					// point to end and then back up
					pszVar += ( nLength - i );
				}
			}

			sprintf( pszVar, FMT_PREC_STR, n, pszVar );
			break;

		case FUNC_TIME:

			// return number of seconds since midnight
			if (( sscanf( pszVar, _TEXT("%lu%*c%u%*c%u"), &ulTemp, &i, &n ) < 1 ) || ( ulTemp > 24 ) || ( i >= 60 ) || ( n >= 60 ))
				return ERROR_4DOS_INVALID_TIME;
			sprintf( pszVar, FMT_ULONG, ( ulTemp * 3600L ) + (unsigned long)(( i * 60 ) + n ));
			break;

		case FUNC_TIMER:

			// return current elapsed time for specified timer
			i = atoi( pszVar );
			if (( i > 3 ) || ( i < 1 ))
				return ERROR_INVALID_PARAMETER;

			// system date
			_timer( i - 1, pszVar );
			break;

		case FUNC_TRIM:

			// return string with leading & trailing whitespace stripped
			trim( pszVar, WHITESPACE );
			break;

		case FUNC_TRUENAME:

			// true filename
			if ( ProcessFileName( pszVar, 0 ) != 0 )
				return -ERROR_EXIT;

			if ( true_name( pszVar, szDrive ) == NULL )
				return -ERROR_EXIT;
			strcpy( pszVar, szDrive );
			break;

		case FUNC_TRUNCATE:

			// truncate the specified file at its current position
			if (( i = atoi( pszVar )) <= 2 )
				return ERROR_INVALID_HANDLE;
			IntToAscii( _chsize( i, _lseek( i, 0, SEEK_CUR )), pszVar );
			break;

		case FUNC_UNIQUE:

			// Creat a unique filename
			// if no path specified, use the current directory
			if ( ProcessFileName( pszVar, 0 ) != 0 )
				return -ERROR_EXIT;

			if ( *pszVar == _TEXT('\0') )
				strcpy( pszVar, gcdir( NULL, 1 ));

			if ( mkfname( pszVar, 0 ) == NULL )
				return -ERROR_EXIT;

			if (( n = UniqueFileName( pszVar )) != 0 )
				return n;

			break;

		case FUNC_UNQUOTE:

			// strip double quotes
			StripQuotes( pszVar );
			break;

		case FUNC_UNQUOTES:

			// strip leading and trailing double quotes
			if ( *pszVar == DOUBLE_QUOTE )
				strcpy( pszVar, pszVar + 1 );
			pszArg = strlast( pszVar );
			if ( *pszArg == DOUBLE_QUOTE )
				*pszArg = _TEXT('\0');
			break;

		case FUNC_UPPER:

			// shift string to upper case
			strupr( pszVar );
			break;

		case FUNC_WILD:
		case FUNC_LCS:
		case FUNC_SIMILAR:

			// wildcard comparison or Longest Common Subsequence
			pszFileName = pszVar;
			pszVar = scan( pszVar, _TEXT(","), QUOTES );
			if (( pszVar == BADQUOTES ) || ( *pszVar != _TEXT(',') ))
				return ERROR_INVALID_PARAMETER;
			*pszVar++ = _TEXT('\0');

			if ( nOffset == FUNC_WILD )
				pszVar = (( wild_cmp( pszVar, pszFileName, 1, TRUE ) == 0 ) ? _TEXT("1") : _TEXT("0") );
			else if ( nOffset == FUNC_LCS )
				pszVar = LCS( pszVar, pszFileName );
			else	// FUNC_SIMILAR
				IntToAscii( similar_text( pszVar, pszFileName ), pszVar );
			break;

		case FUNC_FIELD:
		case FUNC_FIELDS:
		case FUNC_WORD:
		case FUNC_WORDS:

			// get the i'th word from the line (base 0 )
			// or the total number of words on the line (@WORDS)

			// were custom delimiters requested?
			if ( *pszVar == _TEXT('"') ) {
				i = strlen( pszFileName );
				if ( pszFileName[i-1] == _TEXT('"') )
					pszFileName[i-1] = _TEXT('\0');
				EscapeLine( ++pszFileName );
				pszVar += i + 1; // string may begin with space so don't wkip it
			} else
				pszFileName = _TEXT(" ,\t");

			if ( nOffset == FUNC_WORD || nOffset == FUNC_FIELD ) {
				// check for negative offset (scan backwards from end)
				if ( *pszVar == _TEXT('-') ) {
					n = 1;
					pszVar++;
				}
				if ( sscanf( pszVar, _TEXT("%d ,%[^\n]"), &i, pszVar ) < 1 )
					return ERROR_INVALID_PARAMETER;
			} else
				i = 0x4000;

			pszVar = strcpy( szBuffer, pszVar );

			if ( n )
				pszVar = strlast( pszVar );

			for ( ; ; ) {

				TCHAR cQuote, *pszPtr;

				// find start of arg[i]
				while (( *pszVar != _TEXT('\0') ) && ( pszVar >= szBuffer ) && ( strchr( pszFileName, *pszVar ) != NULL )) {

					// if @FIELD[S] and first char is delimiter, don't skip it!
					if ( nOffset == FUNC_FIELD || nOffset == FUNC_FIELDS ) {

						if ( fClip > 0 )
							pszVar += ( n ? -1 : 1 );

						// only look for a single delimiter for @FIELD[S]
						break;

					} else
						pszVar += ( n ? -1 : 1 );
				}

				fClip++;
				if ( *pszVar == _TEXT('\0') || pszVar < szBuffer )
					break;

				// search for next delimiter character
				for ( pszPtr = pszVar, cQuote = 0; (( *pszVar != _TEXT('\0') ) && ( pszVar >= szBuffer )); ) {

					// ignore whitespace inside quotes
					if ((( *pszVar == _TEXT('`') ) || ( *pszVar == _TEXT('"') )) && ( strchr( pszFileName, *pszVar ) == NULL ))  {
						if ( cQuote == *pszVar )
							cQuote = 0;
						else if ( cQuote == 0 )
							cQuote = *pszVar;
					} else if (( cQuote == 0 ) && ( strchr( pszFileName, *pszVar ) != NULL ))
						break;

					pszVar += ( n ? -1 : 1 );
				}

				if ( i == 0 ) {

					// this is the argument I want - copy it & return
					if (( n = (int)( pszVar - pszPtr )) < 0 ) {
						n = -n;
						pszVar++;
					} else
						pszVar = pszPtr;

					if ( n >= MAXLINESIZ )
						n = MAXLINESIZ-1;
					pszVar[n] = _TEXT('\0');
					break;
				}

				i += (( i < 0 ) ? 1 : -1 );

				if ( *pszVar == _TEXT('\0') || pszVar < szBuffer )
					break;
			}

			if ( nOffset == FUNC_WORD || nOffset == FUNC_FIELD ) {
				if ( i != 0 || pszVar < szBuffer )
					pszVar = NULL;
			} else
				IntToAscii(( 0x4000 - i ), pszVar );
			break;


		case FUNC_XMS:

			// get free XMS memory ( if any)
			ulTemp = get_xms( (unsigned *)&i );
			FormatLong( pszVar, pszFileName, (ulTemp << 10 ));
			break;


		case FUNC_EMS:

			// get free EMS memory ( if any)
			get_expanded( (unsigned *)&ulTemp ); // MSW = 0 already
			FormatLong( pszVar, pszFileName, ulTemp << 14 );
			break;


		case FUNC_LFN:
			// return the LFN form of the filename

			// trailing '\' causes GetLongName to fail
			strip_trailing( ((( pszVar[1] == _TEXT(':') ) && ( pszVar[2] )) ? pszVar + 3 : pszVar + 1 ), _TEXT("\\/ ") );

			if ( ProcessFileName( pszVar, 1 ) != 0 )
				return -ERROR_EXIT;

			if ( GetLongName( pszVar ) == 0 )
				*pszVar = _TEXT('\0');
			return -6666;

		case FUNC_SFN:
			// return the SFN form of the filename
			if ( ProcessFileName( pszVar, 1 ) != 0 )
				return -ERROR_EXIT;

			if ( GetShortName( pszVar ) == 0 )
				*pszVar = _TEXT('\0');
			return -6666;

		default:

			// Bogus match, dude!  Probably 4dos looking for a
			//   Win32-specific function
			continue;
			}

			// we got a match, so exit now
			if ( pszVar != szBuffer )
				sprintf( szBuffer, FMT_PREC_STR, MAXLINESIZ-1, (( pszVar == NULL ) ? NULLSTR : pszVar ));
			return 0;
		}
	}

	// unknown variable function?
	return ERROR_INVALID_FUNCTION;
}


// convert an attribute text string to its binary equivalent
static int _near _fastcall AttributeString( LPTSTR pszAttributes, register int *pnAttribute )
{
	if ( pszAttributes == NULL )
		return 0;

	strupr( pszAttributes );

	for ( ; ( *pszAttributes != _TEXT('\0') ); pszAttributes++ ) {

		if ( *pszAttributes == _TEXT('N') )
			*pnAttribute = 0;
		else if ( *pszAttributes == _TEXT('R') )
			*pnAttribute |= _A_RDONLY;
		else if ( *pszAttributes == _TEXT('H') )
			*pnAttribute |= _A_HIDDEN;
		else if ( *pszAttributes == _TEXT('S') )
			*pnAttribute |= _A_SYSTEM;
		else if ( *pszAttributes == _TEXT('D') )
			*pnAttribute |= _A_SUBDIR;
		else if ( *pszAttributes == _TEXT('A') )
			*pnAttribute |= _A_ARCH;
		else if ( *pszAttributes != _TEXT('_') )
			return ERROR_INVALID_PARAMETER;
	}

	return 0;
}


// format a long value for FUNC_EMS, FUNC_XMS, etc.
static void _near FormatLong( LPTSTR pszTarget, LPCTSTR pszFormat, ULONG_PTR ulVal )
{
	// lVal comes in as a value in B - if user wants K or M, shift it
	if ( pszFormat != NULL ) {
		if ( *pszFormat == _TEXT('k') )
			ulVal /= 1000L;
		else if ( *pszFormat == _TEXT('K') )
			ulVal >>= 10;
		else if ( *pszFormat == _TEXT('m') )
			ulVal /= 1000000L;
		else if ( *pszFormat == _TEXT('M') )
			ulVal >>= 20;
	}

	sprintf( pszTarget, ((( pszFormat != NULL ) && ( tolower( pszFormat[1] ) == _TEXT('c') )) ? _TEXT("%Lu") : FMT_ULONG), ulVal );
}


// format a 64-bit value for FUNC_FILESIZE, etc.
// return 0 if OK, !0 if invalid format
static int _near Format64bit( LPTSTR pszTarget, LPCTSTR pszFormat, t_int64 *llVal )
{
	// *llVal comes in as a value in B - if user wants K, M or G, divide it
#ifdef NATIVE_INT64
	t_int64 divisor = 1L;

	if ( *pszFormat ) {
		if ( *pszFormat == _TEXT('k') )
			divisor = 1000L;
		else if ( *pszFormat == _TEXT('K') )
			divisor = 1024L;
		else if ( *pszFormat == _TEXT('m') )
			divisor = 1000000L;
		else if ( *pszFormat == _TEXT('M') )
			divisor = 1048576L;
		else if ( *pszFormat == _TEXT('g') )
			divisor = 1000000000L;
		else if ( *pszFormat == _TEXT('G') )
			divisor = 1073741824L;
		else if ( _ctoupper( *pszFormat ) != _TEXT('B') )
			return -1;
	}
#else
	t_int64 divisor = {1, 0};

	if ( *pszFormat ) {
		if ( *pszFormat == _TEXT('k') )
			divisor.ulLowPart = 1000L;
		else if ( *pszFormat == _TEXT('K') )
			divisor.ulLowPart = 1024L;
		else if ( *pszFormat == _TEXT('m') )
			divisor.ulLowPart = 1000000L;
		else if ( *pszFormat == _TEXT('M') )
			divisor.ulLowPart = 1048576L;
		else if ( *pszFormat == _TEXT('g') )
			divisor.ulLowPart = 1000000000L;
		else if ( *pszFormat == _TEXT('G') )
			divisor.ulLowPart = 1073741824L;
		else if ( _ctoupper( *pszFormat ) != _TEXT('B') )
			return -1;
	}
#endif
	strcpy( pszTarget, Divide64By64( llVal, &divisor, 0, 1 ) );
	return 0;
}


// process the internal variables (%_...)
static LPTSTR _near _fastcall var_internal( LPTSTR lpszVar )
{
	extern const TCHAR *VAR_ARRAY[];
	static TCHAR szBrandString[48];
	TCHAR *ptr;
	int i = 0, n, nOffset;
	DATETIME sysDateTime;
	ULONG ulUnixTime;
	t_int64 lla, llb;
	struct tm t;

	if (( _stricmp( lpszVar, _TEXT("?") ) == 0 ) || ( _stricmp( lpszVar, _TEXT("ERRORLEVEL") ) == 0 )) {
		// exit code of last external program?
		IntToAscii( gnErrorLevel, lpszVar );
		return lpszVar;

	} else if ( _stricmp( lpszVar, _TEXT("??") ) == 0 ) {
		// high byte of exit code of last external program?
		IntToAscii( gnHighErrorLevel, lpszVar );
		return lpszVar;
	}

	for ( nOffset = 0; ; nOffset++ ) {

		// if it doesn't begin with an _, it's not an internal pszVar
		if (( *lpszVar != _TEXT('_') ) || ( VAR_ARRAY[nOffset] == NULL ))
			return NULL;

		if ( _stricmp( lpszVar + 1, VAR_ARRAY[nOffset]) == 0 ) {

			switch ( nOffset ) {

		case VAR_4VER:
		case VAR_VERSION:

			// Version of 4DOS
			sprintf( lpszVar, _TEXT("%u%c%02u"), VER_MAJOR, nOffset == VAR_4VER ? gaCountryInfo.szDecimal[0] : _TEXT('.'), VER_MINOR );
			break;

		case VAR_ALIAS:

			// free alias space
			IntToAscii((( glpAliasList + gpIniptr->AliasSize ) - ( end_of_env( glpAliasList) + 1 )), lpszVar );
			break;

		case VAR_ALT:

			// is Alt key depressed?
			IntToAscii(( bios_shiftstate() >> 3 & 1 ), lpszVar );
			break;

		case VAR_ANSI:

			// ANSI loaded?
			IntToAscii( QueryIsANSI(), lpszVar );
			break;

		case VAR_APMAC:		// get AC status
		case VAR_APMBATT:	// get battery status
		case VAR_APMLIFE:	// get remaining battery life
			{
				char chAC, chBattery, chLife;
				_asm {
					mov	ax,5300h
						xor	bx,bx
						int	15h
						jnc	APMInstalled	; @TODO!! for __WATCOMC__
				}
				// APM not installed!
				*lpszVar = _TEXT('\0');
				break;

				// get power status
				_asm {
APMInstalled:
					mov	ax, 530Ah
						mov	bx, 1
						int	15h
						mov	chAC, bh
						mov	chBattery, bl
						mov	chLife, cl
				}
				if ( nOffset == VAR_APMAC )
					lpszVar = ACList[ chAC ];
				else if ( nOffset == VAR_APMBATT) {
					if ( chBattery > 4 )
						chBattery = 4;
					lpszVar = BatteryList[ chBattery ];
				} else {
					// if chLife == 255, return "unknown"
					if ( chLife > 100 )
						strcpy( lpszVar, BatteryList[4] );
					else
						IntToAscii( chLife, lpszVar );
				}
				break;
			}

		case VAR_BATCH:

			// current batch nesting level
			IntToAscii( cv.bn+1, lpszVar );
			break;

		case VAR_BATCHLINE:

			// current batch line
			IntToAscii((( cv.bn >= 0 ) ? bframe[cv.bn].uBatchLine : -1 ), lpszVar );
			break;

		case VAR_BATCHNAME:

			// current batch name
			strcpy( lpszVar, (( cv.bn >= 0 ) ? filecase( bframe[cv.bn].pszBatchName ) : NULLSTR ));
			break;

		case VAR_BATCHTYPE:

			// current batch type:
			// -1 not in batch, 0 normal, 1 compressed, 2 encrypted
			if ( cv.bn < 0 )
				n = -1;
			else if ( bframe[cv.bn].nFlags & BATCH_ENCRYPTED )
				n = 2;
			else if ( bframe[cv.bn].nFlags & BATCH_COMPRESSED )
				n = 1;
			else
				n = 0;
			IntToAscii( n, lpszVar );
			break;

		case VAR_BDEBUGGER:

			// batch debugger active
			IntToAscii( gpIniptr->SingleStep, lpszVar );
			break;

		case VAR_BG_COLOR:
		case VAR_FG_COLOR:

			// foreground or background color at current cursor position
			GetAtt( (unsigned int *)&i, (unsigned int *)&n );

			if ( gpIniptr->BrightBG )
				strcpy( lpszVar, (( nOffset == VAR_FG_COLOR) ? colors[( i&0xF)].szShade : colors[( i>>4)].szShade ));

			else {

				if ( nOffset == VAR_FG_COLOR)
					sprintf( lpszVar, _TEXT("%s%s%s"), (( i & 0x08) ? BRIGHT : NULLSTR), (( i & 0x80 ) ? BLINK : NULLSTR), colors[i&0x7].szShade );
				else
					strcpy( lpszVar, colors[( i >> 4) & 0x7 ].szShade );
			}
			break;

		case VAR_BOOT:

			// boot drive
			sprintf( lpszVar, FMT_CHAR, gpIniptr->BootDrive );
			break;

		case VAR_BUILD:

			// build number
			IntToAscii( VER_BUILD, lpszVar );
			break;

		case VAR_CAPSLOCK:

			// is CapsLock on?
			IntToAscii(( bios_shiftstate() >> 6 & 1 ), lpszVar );
			break;

		case VAR_CDROMS:
		case VAR_DRIVES:
		case VAR_HDRIVES:
		case VAR_READY:

			// return list of CD-ROM, present, hard or ready drives
			for ( ptr = lpszVar, i = 1; i <= 26; i++ ) {
				if ( QueryDriveExists( i )) {
					if ( i == 2 && GetDrivePhysUnit( 2 ) == GetDrivePhysUnit( 1 ))
						continue; // phantom floppy B:
					if ( nOffset == VAR_CDROMS ) {
						if ( !QueryIsCDROM( i ))
							continue;
					} else if ( nOffset == VAR_HDRIVES ) {
						if ( GetDrivePhysUnit( i ) < 0x80 )
							continue;
					} else if ( nOffset == VAR_READY ) {
						if ( !QueryDriveReady( i ))
							continue;
					}
					lpszVar += sprintf( lpszVar, "%c: ", i + '@');
				}
			}
			if ( ptr < lpszVar )
				lpszVar--;
			*lpszVar = _TEXT('\0');
			lpszVar = ptr;
			break;

		case VAR_CI:
			IntToAscii( gpIniptr->CursI, lpszVar );
			break;

		case VAR_CMDLINE:
			{
				extern LPTSTR pszEgetsBase;

				lpszVar = pszEgetsBase;
				break;
			}

		case VAR_CMDPROC:
			lpszVar = SHORT_NAME;
			break;

		case VAR_CMDSPEC:
			sprintf( lpszVar, FMT_FAR_STR, _pgmptr );
			break;

		case VAR_CO:
			IntToAscii( gpIniptr->CursO, lpszVar );
			break;

		case VAR_CODEPAGE:

			// current code page
			IntToAscii( QueryCodePage(), lpszVar );
			break;

		case VAR_COLUMN:
		case VAR_ROW:

			// current cursor row or column position
			GetCurPos( &i, &n );
			IntToAscii( (( nOffset == VAR_ROW) ? i : n), lpszVar );
			break;

		case VAR_COLUMNS:

			// number of screen columns
			IntToAscii( GetScrCols(), lpszVar );
			break;

		case VAR_COUNTRY:
			// current country code
			IntToAscii( gaCountryInfo.nCountryID, lpszVar );
			break;

		case VAR_CPU:

			// get the CPU type or brand string, if supported
			if ( gnCPU > 586 && !get_cpu_brand( szBrandString ) ) {
				trim( szBrandString, WHITESPACE );
				lpszVar = szBrandString;
			} else
				lpszVar = cpu_Type();
			break;

		case VAR_CPUSPEED:

			// CPU speed in MHz
			if ( gnCPU >= 586 && ( gpIniptr->WinMode || !InV86mode() )) {
				if ( gpIniptr->WinMode ) {
					_asm {	// start critical section
						mov	ax,1681h
						int	2Fh
					}
					for ( ulUnixTime = GetWinMs(); ulUnixTime == GetWinMs(); )
						;	// await new tick
				}
				GetTSC( &lla );
				SysWait( 50, 2 );	// wait 50 ms
				GetTSC( &llb );
				if ( gpIniptr->WinMode ) {
					_asm {	// end critical section
						mov	ax,1682h
						int	2Fh
					}
				}
				Subtract64From64( &llb, &lla );
#ifdef NATIVE_INT64
				llb = 50000L;
#else
				llb.ulHighPart = 0L;
				llb.ulLowPart = 50000L;
#endif
				lpszVar = Divide64By64( &lla, &llb, 0, 1 );
			} else
				IntToAscii( cpu_Speed(), lpszVar );
			break;

		case VAR_CTRL:

			// is Ctrl key depressed?
			IntToAscii(( bios_shiftstate() >> 2 & 1 ), lpszVar );
			break;

		case VAR_CWD:

			// current working directory
			lpszVar = gcdir( NULL, 0 );
			break;

		case VAR_CWDS:

			// cwd w/backslash guaranteed
			if (( lpszVar = gcdir( NULL, 0 )) != NULL )
				mkdirname( lpszVar, NULLSTR );
			break;

		case VAR_CWP:

			// current working directory with no drive
			if (( lpszVar = gcdir( NULL, 0 )) != NULL )
				lpszVar += 2;
			break;

		case VAR_CWPS:

			// cwd with no drive w/backslash guaranteed
			if (( lpszVar = gcdir( NULL, 0 )) != NULL ) {
				lpszVar += 2;
				mkdirname( lpszVar, NULLSTR);
			}

			break;

		case VAR_DATE:
		case VAR_ISODATE:
		case VAR_ISORDATE:
		case VAR_ISOWDATE:
		case VAR_DATETIME:

			// system date
			QueryDateTime( &sysDateTime );

			if ( nOffset == VAR_DATETIME ) {
				sprintf( lpszVar, _TEXT("%4u%02u%02u%02u%02u%02u"), sysDateTime.year, sysDateTime.month, sysDateTime.day, sysDateTime.hours, sysDateTime.minutes, sysDateTime.seconds );
				break;
			}
			if ( nOffset == VAR_ISODATE )
				n = 4;
			else if ( nOffset == VAR_ISOWDATE )
				n = 5;
			else if ( nOffset == VAR_ISORDATE )
				n = 6;
			else
				n = 0;

			// replace leading space with a 0
			if (*( lpszVar = FormatDate( sysDateTime.month, sysDateTime.day, sysDateTime.year, n )) == _TEXT(' ') )
				*lpszVar = _TEXT('0');
			break;

		case VAR_DAY:

			// system date
			QueryDateTime( &sysDateTime );
			IntToAscii( sysDateTime.day, lpszVar );
			break;

		case VAR_DNAME:
			// set DescriptionName

			lpszVar = (( gpIniptr->DescriptName == INI_EMPTYSTR) ? DESCRIPTION_FILE : ( gpIniptr->StrData + gpIniptr->DescriptName ));
			break;

		case VAR_DISK:

			// current disk
			*lpszVar = (TCHAR)( gcdisk( NULL ) + 64 );
			lpszVar[1] = _TEXT('\0');


			break;

		case VAR_DOS:

			// operating system flavor (DOS, WIN, or NT)

			lpszVar = pszOSname;
			break;

		case VAR_DOSVER:

			// get the DOS or Windows version
			lpszVar = gszOsVersion;
			break;

		case VAR_DOW:
		case VAR_DOWF:
		case VAR_DOWI:
		case VAR_ISODOWI:

			// get the day of week (Mon, Tue, etc. or 1 - 7)
			QueryDateTime( &sysDateTime );

			if ( nOffset == VAR_DOW )
				sprintf( lpszVar, _TEXT("%.3s"), daytbl[(int)sysDateTime.weekday] );
			else if ( nOffset == VAR_DOWF )
				strcpy( lpszVar, daytbl[(int)sysDateTime.weekday] );
			else if ( nOffset == VAR_ISODOWI )
				IntToAscii( sysDateTime.weekday ? sysDateTime.weekday : 7, lpszVar );
			else
				IntToAscii( sysDateTime.weekday+1, lpszVar );
			break;

		case VAR_DOY:
		case VAR_ISOWEEK:
		case VAR_ISOWYEAR:

			// return the day (1-366) or the week (0-53) of the year
			QueryDateTime( &sysDateTime );
			n = sysDateTime.day;
			n = ISOweekDOY( &n, sysDateTime.month, &sysDateTime.year, &i );

			// at this point, n = ISO week number, i = day of year
			if ( nOffset == VAR_DOY )
				n = i;
			else if ( nOffset == VAR_ISOWYEAR )
				n = sysDateTime.year;
			// else ( nOffset == VAR_ISOWEEK ) use n;
			IntToAscii( n, lpszVar );
			break;

		case VAR_DPMI:
			// return the DPMI version number, or 0 if it's not loaded
			{
				_asm {
					push	si
						push	di
						xor	dx, dx
						mov	ax, 1687h
						int	02Fh
						pop	di
						pop	si
						mov	i, ax		; if AX != 0, DPMI not present
						mov	n, dx		; get version level
				}
DPMI_version:
				if ( i != 0 )
					lpszVar = _TEXT("0");
				else
					sprintf( lpszVar, _TEXT("%d%c%02d"), ( n >> 8 ), gaCountryInfo.szDecimal[0], ( n & 0xFF));
				break;
			}

		case VAR_VCPI:
			// return the VCPI version number, or 0 if not present
			_asm {
				mov	ax,3567h
				push	es
				int	21h
				mov	dx,es
				pop	es
				or	bx,dx	; Int 67h vector zero?
				jz	no_vcpi	; if so, don't call it!
				mov	ax,0DE00h
				int	67h
				mov	al,0
			no_vcpi:mov	i,ax	; if AH != 0, VCPI not present
				mov	n,bx	; get version level
			}
			goto DPMI_version;

		case VAR_VDS:
			// return the VDS version number, or 0 if not present
			_asm {
				push	es
				xor	dx,dx
				mov	es,dx
				test	es:[47Bh],100000b;bit 5 clear = no VDS
				pop	es
				mov	ax,8102h	; get VDS version
				jz	no_vds
				push	cx
				push	dx
				push	si
				push	di
				int	4Bh
				pop	di
				pop	si
				pop	dx
				pop	cx
				jc	no_vds
				xchg	ax,dx	; now DX = version, AX = 0
			no_vds:	mov	i,ax	; if AX != 0, VDS not present
				mov	n,dx	; get version level
			}
			goto DPMI_version;

		case VAR_VERMAJOR:

			// 4DOS major version
			IntToAscii( VER_MAJOR, lpszVar );
			break;

		case VAR_VERMINOR:

			// 4DOS minor version
			IntToAscii( VER_MINOR, lpszVar );
			break;

		case VAR_DST:
		case VAR_MJD:
		case VAR_STZN:
		case VAR_STZO:
		case VAR_TZN:
		case VAR_TZO:
		case VAR_UNIXTIME:
		case VAR_UTCDATE:
		case VAR_UTCDATETIME:
		case VAR_UTCHOUR:
		case VAR_UTCISODATE:
		case VAR_UTCMINUTE:
		case VAR_UTCSECOND:
		case VAR_UTCTIME:

			// UTC-related variables
			QueryDateTime( &sysDateTime );
			t.tm_sec = sysDateTime.seconds;
			t.tm_min = sysDateTime.minutes;
			t.tm_hour = sysDateTime.hours;
			t.tm_mday = sysDateTime.day;
			t.tm_mon = sysDateTime.month - 1;
			t.tm_year = sysDateTime.year - 1900;
			t.tm_isdst = -1; // unknown: see Watcom's mktime()
			ulUnixTime = mktime( &t );// seconds past 1-1-1970
			n = t.tm_isdst;	 // will be cleared by _gmtime()
			_gmtime( &ulUnixTime, &t );
			if ( nOffset == VAR_DST )
				IntToAscii( n, lpszVar );
			else if ( nOffset == VAR_MJD ) {
				sprintf( lpszVar, "40587+%lu/86400+%d/8640000=6", ulUnixTime, (int)sysDateTime.hundredths );
				evaluate( lpszVar );
			} else if ( nOffset == VAR_STZN )
				lpszVar = tzname[0];
			else if ( nOffset == VAR_STZO )
				IntToAscii( (int)(timezone / 60), lpszVar );
			else if ( nOffset == VAR_TZN )
				lpszVar = n ? tzname[1] : tzname[0];
			else if ( nOffset == VAR_TZO )
				IntToAscii( (int)((timezone - (n ? __dst_adjust : 0)) / 60), lpszVar );
			else if ( nOffset == VAR_UNIXTIME )
				sprintf( lpszVar, FMT_LONG, ulUnixTime );
			else if ( nOffset == VAR_UTCDATE || nOffset == VAR_UTCISODATE ) {
				// replace leading space with a 0
				if (*( lpszVar = FormatDate( t.tm_mon + 1, t.tm_mday, t.tm_year + 1900, nOffset == VAR_UTCISODATE ? 4 : 0 )) == _TEXT(' ') )
					*lpszVar = _TEXT('0');
			} else if ( nOffset == VAR_UTCDATETIME )
				sprintf( lpszVar, _TEXT("%4u%02u%02u%02u%02u%02u"), t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec );
			else if ( nOffset == VAR_UTCHOUR )
				IntToAscii( t.tm_hour, lpszVar );
			else if ( nOffset == VAR_UTCMINUTE )
				IntToAscii( t.tm_min, lpszVar );
			else if ( nOffset == VAR_UTCSECOND )
				IntToAscii( t.tm_sec, lpszVar );
			else	// VAR_UTCTIME
				sprintf( lpszVar, _TEXT("%02u%c%02u%c%02u"), t.tm_hour, gaCountryInfo.szTimeSeparator[0], t.tm_min, gaCountryInfo.szTimeSeparator[0], t.tm_sec );
			break;

		case VAR_DV:

			// is DESQview loaded?
			IntToAscii(( int)( gpIniptr->DVMode ), lpszVar );
			break;

		case VAR_ENVIRONMENT:

			// free environment space
			IntToAscii((( glpEnvironment + gpIniptr->EnvSize ) - (end_of_env( glpEnvironment) + 1 )), lpszVar );
			break;

		case VAR_ECHO:

			// returns echo state
			IntToAscii( (( cv.bn >= 0 ) ? bframe[cv.bn].uEcho : cv.fVerbose ), lpszVar );
			break;

		case VAR_EDITMODE:

			IntToAscii( gnEditMode, lpszVar );
			break;

		case VAR_EXECSTR:

			// integer return code of last @EXECSTR function
			IntToAscii( nExecStrRet, lpszVar );
			break;

		case VAR_EXPANSION:

			// current expansion mode (SETDOS /X)
			ptr = lpszVar;
			if ( ( n = gpIniptr->Expansion ) == 0 )
				*ptr++ = _TEXT('0');
			else for ( i = _TEXT('1'); n > 0 ; n >>= 1, i++ )
				if ( n & 1 )
					*ptr++ = i;
			*ptr = _TEXT('\0');
			break;

		case VAR_FONTPAGE:

			// current font page (0 if no Arabic or Hebrew loaded)
			IntToAscii( GetFontPage(), lpszVar );
			break;

		case VAR_HLOGFILE:

			// name of command log file
			strcpy( lpszVar, (( gpIniptr->HistLogOn) ? GetLogName(1 ) : NULLSTR));
			break;

		case VAR_HOUR:

			// system date
			QueryDateTime( &sysDateTime );
			IntToAscii( sysDateTime.hours, lpszVar );
			break;

		case VAR_IERRORLEVEL:

			// exit code of last internal command
			IntToAscii( gnInternalErrorLevel, lpszVar );
			break;

		case VAR_ININAME:

			// name of INI file
			strcpy( lpszVar, gpIniptr->PrimaryININame == INI_EMPTYSTR ? NULLSTR : gpIniptr->StrData + gpIniptr->PrimaryININame );
			break;

		case VAR_KBHIT:
			// is a key waiting?
			IntToAscii(( _kbhit() != 0 ), lpszVar );
			break;

		case VAR_KEYSTACKED:

			// number of keystrokes left in the KSTACK buffer
			_asm	xor	cx,cx	; the old KSTACK won't touch CX
			if ( QueryKSTACK() ) {
				_asm {
					cmp	cx,2	; function 2 supported?
					jb	KSret	; no - old KSTACK.COM!
					mov	ax,0D44Fh
					mov	bx,2	; get the number of the
					int	2Fh	; keystrokes in buffer
					mov	i,ax
				KSret:
				}
			}
			IntToAscii( i, lpszVar );
			break;

		case VAR_KSTACK:

			// KSTACK loaded?
			IntToAscii( QueryKSTACK(), lpszVar );
			break;

		case VAR_LALT:

			// is left Alt key depressed?
			IntToAscii(( bios_shiftstate() >> 9 & 1 ), lpszVar );
			break;

		case VAR_LASTDIR:

			// previous directory
			{
				extern TCHAR szLastDirectory[];
				lpszVar = szLastDirectory;
			}
			break;

		case VAR_LASTDISK:

			// return last active disk
			for ( i = 26; ( i > 2 ); i-- ) {
				if ( QueryDriveExists( i ))
					break;
			}

			sprintf( lpszVar, FMT_CHAR, i+64);
			break;

		case VAR_LCTRL:

			// is left Ctrl key depressed?
			IntToAscii(( bios_shiftstate() >> 8 & 1 ), lpszVar );
			break;

		case VAR_LOGFILE:

			// name of command log file
			strcpy( lpszVar, (( gpIniptr->LogOn) ? GetLogName( 0 ) : NULLSTR));
			break;

		case VAR_LSHIFT:

			// is Left Shift key depressed?
			IntToAscii(( bios_shiftstate() >> 1 & 1 ), lpszVar );
			break;

		case VAR_MACHINE:

			*lpszVar = _TEXT('\0');
			GetMachineName( lpszVar );
			strip_trailing( lpszVar, _TEXT(" ") );
			break;

		case VAR_MINUTE:

			// system date
			QueryDateTime( &sysDateTime );
			IntToAscii( sysDateTime.minutes, lpszVar );
			break;

		case VAR_MONITOR:

			// get monitor type (MONO or COLOR)
			lpszVar = ((( GetVideoMode() % 2 ) == 0 ) ? MONO_MONITOR : COLOR_MONITOR);
			break;

		case VAR_MONTH:
		case VAR_MONTHF:

			// system date
			QueryDateTime( &sysDateTime );
			if ( nOffset == VAR_MONTHF )
				strcpy( lpszVar, lmontbl[sysDateTime.month-1] );
			else
				IntToAscii( sysDateTime.month, lpszVar );
			break;

		case VAR_MOUSE:

			// mouse driver loaded?
			IntToAscii( QueryMouseReady(), lpszVar );
			break;

		case VAR_NDP:

			// get FPU type
			lpszVar = fpu_Type();
			break;

		case VAR_NETWORK:

			// network installed? (return installed component flags)
			IntToAscii( NetworkInstalled(), lpszVar );
			break;

		case VAR_NLSFUNC:
			n = 0x1400;
TSRcheck:
			// TSR installed? (1=yes, 0=OK to install, -1=not OK
			IntToAscii( -InstalledCheck( n ), lpszVar );
			break;

		case VAR_POWER:

			// POWER.EXE installed?
			IntToAscii( PowerInstalled(), lpszVar );
			break;

		case VAR_PRINT:
			n = 0x100;
			goto TSRcheck;

		case VAR_ASSIGN:
			n = 0x600;
			goto TSRcheck;

		case VAR_DRIVER:
			n = 0x800;
			goto TSRcheck;

		case VAR_SHARE:
			n = 0x1000;
			goto TSRcheck;

		case VAR_MSCDEX:
			n = 0x1100;
			goto TSRcheck;

		case VAR_TASKMAX:
			n = 0x2700;
			goto TSRcheck;

		case VAR_GRAPHICS:
			n = 0xAC00;
			goto TSRcheck;

		case VAR_DISPLAY:
			n = 0xAD00;
			goto TSRcheck;

		case VAR_GRAFTABL:
			n = 0xB000;
			goto TSRcheck;

		case VAR_APPEND:
			n = 0xB700;
			goto TSRcheck;

		case VAR_EGA:
			n = 0xBC00;
			goto TSRcheck;

		case VAR_NUMLOCK:

			// is NumLock on?
			IntToAscii(( bios_shiftstate() >> 5 & 1 ), lpszVar );
			break;

		case VAR_RALT:

			// is right Alt key depressed?
			IntToAscii(( bios_shiftstate() >> 11 & 1 ), lpszVar );
			break;

		case VAR_RCTRL:

			// is right Ctrl key depressed?
			IntToAscii(( bios_shiftstate() >> 10 & 1 ), lpszVar );
			break;

		case VAR_ROWS:

			// number of screen rows
			IntToAscii( GetScrRows() + 1, lpszVar );
			break;

		case VAR_RSHIFT:

			// is Right Shift key depressed?
			IntToAscii(( bios_shiftstate() & 1 ), lpszVar );
			break;

		case VAR_SBDSP:

			// SB DSP version
			i = !(n = GetSBDSPver());
			goto DPMI_version;
			break;

		case VAR_SCROLLLOCK:

			// is ScrollLock on?
			IntToAscii(( bios_shiftstate() >> 4 & 1 ), lpszVar );
			break;

		case VAR_SECOND:

			// system date
			QueryDateTime( &sysDateTime );
			IntToAscii( sysDateTime.seconds, lpszVar );
			break;

		case VAR_SHELL:

			// shell level
			IntToAscii( gpIniptr->ShellNum, lpszVar );
			break;

		case VAR_SHIFT:

			// is either Shift key depressed?
			IntToAscii(( bios_shiftstate() & 3 ? 1 : 0 ), lpszVar );
			break;

		case VAR_SMARTDRV:

			// is SMARTDRV installed?
			IntToAscii( InstalledCheck( 0x4A10 ) == 0xBABE, lpszVar );
			break;

		case VAR_STARTPATH:

			// startup directory
			lpszVar = gszStartPath;
			break;

		case VAR_STDERR:

			// does standard error point to CON?
			IntToAscii( _isatty( STDERR ), lpszVar );
			break;

		case VAR_STDIN:

			// does standard input point to CON?
			IntToAscii( _isatty( STDIN ), lpszVar );
			break;

		case VAR_STDOUT:

			// does standard output point to CON?
			IntToAscii( _isatty( STDOUT ), lpszVar );
			break;

		case VAR_SWAPPING:

			// swapping type
			lpszVar = (( ServCtrl( SERV_SWAP, -1 ) == 0 ) ? OFF : swap_mode[ gpIniptr->SwapMeth ]);
			break;

		case VAR_SYSERR:

			// last DOS/Windows error
			IntToAscii( gnSysError, lpszVar );
			break;

		case VAR_SYSREQ:

			// is SysReq key depressed?
			IntToAscii(( bios_shiftstate() >> 15 ), lpszVar );
			break;

		case VAR_TASKSWITCHER:

			// is DOSSHELL Task Switcher installed?
			IntToAscii( InstalledCheck( 0x4B02 ) == 0, lpszVar );
			break;

		case VAR_TICK:

			// BIOS clock ticks since midnight
			sprintf( lpszVar, FMT_LONG, *(( volatile long _far * )0x46CL ));
			break;

		case VAR_TIME:

			// system time (24-hour); replace leading space with a 0
			if ( *( lpszVar = gtime( 1 )) == _TEXT(' ') )
				*lpszVar = _TEXT('0');
			break;

		case VAR_TRANSIENT:

			// if loaded with /C, gnTransient == 1
			IntToAscii( gnTransient, lpszVar );
			break;

		case VAR_TSC:

			// Time-Stamp Counter of the CPU (586+)
			if ( gnCPU >= 586 ) {
				GetTSC( &lla );
				lpszVar = Format64( &lla );
			} else
				lpszVar = _TEXT("?");
			break;

		case VAR_V86:

			// is CPU in V86 mode?
			IntToAscii( gnCPU >= 386 ? InV86mode() : 0, lpszVar );
			break;

		case VAR_VIDEO:

			// get video adaptor type
			lpszVar = QuerySVGA() ? SVGA_TYPE : video_type[ GetVideoMode() ];
			break;

		case VAR_WIN:

			// is MS Windows loaded?
			IntToAscii( gpIniptr->WinMode, lpszVar );
			break;

		case VAR_WINTICKS:

			// ms since midnight (DOS) or since Windows started
			sprintf( lpszVar, FMT_LONG, GetMs() );
			break;

		case VAR_WINTITLE:

			// get the title of our window
			*lpszVar = _TEXT('\0');
			if ( fWin95 )
				Win95GetTitle( lpszVar );
			else
				ServTtl( lpszVar, 2 );
			break;

		case VAR_YEAR:

			// system date
			QueryDateTime( &sysDateTime );
			IntToAscii( sysDateTime.year, lpszVar );
			break;


		default:
			// Bogus match, dude!  Probably 4DOS looking for a
			//   Win32-specific function
			continue;
			}

			// found a match
			return lpszVar;
		}
	}
}


// collapse escape characters for the entire line
void _fastcall EscapeLine( LPTSTR pszLine )
{
	if (( gpIniptr->Expansion & EXPAND_NO_ESCAPES ) == 0 ) {

		for ( ; ( *pszLine != _TEXT('\0') ); pszLine++ ) {

			if ( *pszLine == gpIniptr->EscChr ) {
				strcpy( pszLine, pszLine + 1 );
				*pszLine = escape_char( *pszLine );
			}
		}
	}
}


// collapse escape characters
void _fastcall escape( LPTSTR pszLine )
{
	if (( *pszLine == gpIniptr->EscChr ) && ( pszLine[1] != _TEXT('\0')) && (( gpIniptr->Expansion & EXPAND_NO_ESCAPES ) == 0 )) {
		strcpy( pszLine, pszLine + 1 );
		*pszLine = escape_char( *pszLine );
	}
}


// convert the specified character to it's escaped equivalent
TCHAR _fastcall escape_char( TCHAR c )
{
	if (( c = tolower( c )) == _TEXT('b') )
		c = _TEXT('\b');		// backspace
	else if ( c == _TEXT('c') )
		c = _TEXT(',');		// comma
	else if ( c == _TEXT('e') )
		c = _TEXT('\033');		// ESC
	else if ( c == _TEXT('f') )
		c = _TEXT('\f');		// form feed
	else if ( c == _TEXT('k') )
		c = _TEXT('`');		// single back quote
	else if ( c == _TEXT('n') )
		c = _TEXT('\n');		// line feed
	else if ( c == _TEXT('q') )
		c = _TEXT('"');		// double quote
	else if ( c == _TEXT('r') )
		c = _TEXT('\r');		// CR
	else if ( c == _TEXT('s') )
		c = _TEXT(' ');		// space
	else if ( c == _TEXT('t') )
		c = _TEXT('\t');		// tab

	return c;
}


#define HISTORY_APPEND 1
#define HISTORY_FREE 2
#define HISTORY_NODUPES 4
#define HISTORY_PAUSE 8
#define HISTORY_READ 0x10


static int _fastcall __history( LPTSTR, int );
#pragma alloc_text( _TEXT, DirHistory_Cmd )
#pragma alloc_text( _TEXT, History_Cmd )

// print the directory history, read it from a file, or clear it
int _near DirHistory_Cmd( LPTSTR pszCmdLine )
{
	return ( __history( pszCmdLine, 1 ));
}


// print the command history, read it from a file, or clear it
int _near History_Cmd( LPTSTR pszCmdLine )
{
	return ( __history( pszCmdLine, 0 ));
}


static int _fastcall __history( LPTSTR pszCmdLine, int fDirHistory )
{
	LPTSTR pszArg;
	int nReturn = 0, nEditFlags = EDIT_DATA, fDeleted, nFH;
	long fHistory;
	TCHAR _far *lpszHistory, _far *lpszDupes, _far *lpList;

	lpList = (( fDirHistory ) ? glpDirHistory : glpHistoryList );

	// check for switch - if we have args, we have to have a switch!
	if (( GetSwitches( pszCmdLine, _TEXT("AFNPR"), &fHistory, 1 ) != 0 ) || (( fHistory == 0 ) && ( pszCmdLine ) && ( *pszCmdLine )))
		return ( Usage( (( fDirHistory ) ? DIRHISTORY_USAGE : HISTORY_USAGE )));

	// clear the history list
	if ( fHistory & HISTORY_FREE ) {

		*lpList = _TEXT('\0');
		lpList[1] = _TEXT('\0');

	} else if ( fHistory & HISTORY_APPEND ) {

		// add string to history list
		glpHptr = NULL;
		if (( pszCmdLine ) && ( *pszCmdLine != _TEXT('\0') )) {
			if ( lpList == glpHistoryList )
				addhist( pszCmdLine );
			else
				SaveDirectory( lpList, pszCmdLine );
		}

	} else if ( fHistory & HISTORY_NODUPES ) {

		// remove duplicates from the history
		for ( lpszDupes = lpList; ( *lpszDupes != _TEXT('\0') ); ) {

			fDeleted = 0;
			for ( lpszHistory = next_env( lpszDupes ); ( *lpszHistory != _TEXT('\0') ); lpszHistory = next_env( lpszHistory )) {

				if ( _fstricmp( lpszDupes, lpszHistory ) == 0 ) {
					// delete the oldest ( first) duplicate entry
					_fmemmove( lpszDupes, next_env( lpszDupes ), ( gpIniptr->HistorySize - (unsigned int)( next_env( lpszDupes ) - lpList )) * sizeof(TCHAR) );
					fDeleted = 1;
					break;
				}
			}

			if ( fDeleted == 0 )
				lpszDupes = next_env( lpszDupes );
		}

	} else  if ( fHistory & HISTORY_READ ) {

		// read a history file
		if (( pszArg = first_arg( pszCmdLine )) == NULL )
			return ( Usage( HISTORY_USAGE ));

		if (( mkfname( pszArg, 0 ) == NULL ) || (( nFH = _sopen( pszArg, (_O_RDONLY | _O_BINARY), _SH_DENYWR )) < 0 ))
			return ( error( _doserrno, pszArg ));

		if ( setjmp( cv.env ) == -1 ) {
			_close( nFH );
			return CTRLC;
		}

		// add the line to the history
		for ( pszArg = gszCmdline, glpHptr = 0L; ( getline( nFH, pszArg, MAXLINESIZ - ((int)( pszArg - gszCmdline ) + 1 ), nEditFlags ) > 0 ); ) {

			// if last char is escape character, append
			//   the next line
			if ( *pszArg ) {
				pszArg += ( strlen( pszArg ) - 1 );
				if ( *pszArg == gpIniptr->EscChr )
					continue;
			}

			// skip blank lines, leading whitespace, & comments
			pszArg = skipspace( gszCmdline );
			if (( *pszArg ) && ( *pszArg != _TEXT(':') )) {
				if ( lpList == glpHistoryList )
					addhist( pszArg );
				else
					SaveDirectory( lpList, pszArg );
			}

			pszArg = gszCmdline;
		}

		_close( nFH );

	} else {

		// display the history, optionally pausing after each page
		init_page_size();

		if ( fHistory & HISTORY_PAUSE ) {
			gnPageLength = GetScrRows();
		}

		for ( lpszHistory = lpList; ( *lpszHistory != _TEXT('\0') ); lpszHistory = next_env( lpszHistory ))
			more_page( lpszHistory, 0 );
	}

	return nReturn;
}


// add a command to the history list
void _fastcall addhist( LPTSTR pszCommand )
{
	register unsigned int uLength;
	TCHAR _far *lpszHistory, _far *lpszNext, _far *lpszLast;

	pszCommand = skipspace( pszCommand );

	uLength = strlen( pszCommand );

	// check for history not modified, or command too short or too long
	if (( glpHptr != 0L ) || ( *pszCommand == _TEXT('@') ) || ( strnicmp( pszCommand, _TEXT("*@"), 2 ) == 0 ) || ( *pszCommand == _TEXT('\0') ) || ( uLength < gpIniptr->HistMin ) || (( uLength + 2 ) > gpIniptr->HistorySize ))
		return;

	// disable task switches under Windows and DESQview
	CriticalSection( 1 );

	if ( gpIniptr->HistoryDups ) {

		// remove duplicates from the history
		for ( lpszHistory = glpHistoryList; ( *lpszHistory != _TEXT('\0') ); lpszHistory = next_env( lpszHistory )) {

			if ( _fstricmp( pszCommand, lpszHistory ) == 0 ) {

				// keep oldest entry?
				if ( gpIniptr->HistoryDups == 1 )
					goto HistoryExit;

				// delete the oldest ( first) duplicate entry
				lpszLast = end_of_env( lpszHistory ) + 1;
				lpszNext = next_env( lpszHistory );
				_fmemmove( lpszHistory, lpszNext, (( lpszLast - lpszNext ) + 1 ) * sizeof(TCHAR) );
			}
		}
	}

	// history entries are separated by a '\0'
	for ( ; ; ) {

		lpszLast = end_of_env( glpHistoryList );
		if (( uLength + ( lpszLast - glpHistoryList ) + 4 ) < gpIniptr->HistorySize )
			break;

		// delete the oldest ( first) history entry
		lpszNext = next_env( glpHistoryList );
		_fmemmove( glpHistoryList, lpszNext, (( lpszLast - lpszNext ) + 1 ) * sizeof(TCHAR) );
	}

	// add to end of history
	lpszHistory = lpszLast;
	_fstrcpy( lpszHistory, pszCommand );
	lpszHistory[ uLength + 1 ] = _TEXT('\0');

HistoryExit:
	// reset history pointer
	glpHptr = 0L;

	// re-enable task switches under Windows and DESQview
	CriticalSection( 0 );
}


#pragma alloc_text( SCREENIO_TEXT, prev_hist )

// return previous command in history list
TCHAR _far * _fastcall prev_hist( TCHAR _far *pszCmd )
{
	if ( pszCmd <= glpHistoryList ) {
		// don't wrap back to end if the user doesn't want to!
		if (( pszCmd != 0L ) && ( gpIniptr->HistoryWrap == 0 ))
			return glpHistoryList;
		pszCmd = end_of_env( glpHistoryList );
	}

	if ( pszCmd > glpHistoryList )
		pszCmd--;

	for ( ; (( pszCmd > glpHistoryList ) && ( pszCmd[-1] != _TEXT('\0') )); pszCmd-- )
		;

	return pszCmd;
}


#pragma alloc_text( SCREENIO_TEXT, next_hist )

// return next command in history list
TCHAR _far * _fastcall next_hist( TCHAR _far *pszCmd )
{
	TCHAR _far *pchEnv;

	if ( pszCmd == 0L ) {
		if ( gpIniptr->HistoryWrap )
			pszCmd = glpHistoryList;	// wrapped around
	} else {
		pchEnv = next_env( pszCmd );
		if ( *pchEnv != _TEXT('\0') )
			pszCmd = pchEnv;
		else if ( gpIniptr->HistoryWrap )
			pszCmd = glpHistoryList;
		else {
			// if HistoryWrap is off, make sure we're at the beginning
			// of the last entry!
			while (( pszCmd > glpHistoryList ) && ( pszCmd[-1] != _TEXT('\0') ))
				pszCmd--;
		}
	}

	return pszCmd;
}
