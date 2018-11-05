/* This is a terrible, awful, ugly hack. The 4DOS library replacement
 * routines written in assembly all use the __cdecl calling convention.
 * Of course, code built with Watcom C uses register calling convention
 * instead. To work around the problem without rewriting assembly code,
 * we just create a few thunks. Not nice, not fast, but works.
 */
 
#include <stdlib.h>
#include <stdarg.h>

_WCRTLINK extern int real_write( int, const void *, int );
#pragma aux real_write "write_";

int __cdecl _write( int handle, const void *buf, int len )
{
    return( real_write( handle, buf, len ) );
}


extern int __cdecl lfn_rename( const char *, const char * );
#pragma aux lfn_rename "_rename";

_WCRTLINK int rename( const char *oldname, const char *newname )
{
    return( lfn_rename( oldname, newname ) );
}


extern int __cdecl lfn_mkdir( const char * );
#pragma aux lfn_mkdir "__mkdir";

_WCRTLINK int mkdir( const char *path )
{
    return( lfn_mkdir( path ) );
}


extern int __cdecl lfn_rmdir( const char * );
#pragma aux lfn_rmdir "__rmdir";

_WCRTLINK int rmdir( const char *path )
{
    return( lfn_rmdir( path ) );
}


extern int __cdecl lfn_chdir( const char * );
#pragma aux lfn_chdir "__chdir";

_WCRTLINK int chdir( const char *path )
{
    return( lfn_chdir( path ) );
}


extern int __cdecl lfn_remove( const char * );
#pragma aux lfn_remove "_remove";

_WCRTLINK int remove( const char *path )
{
    return( lfn_remove( path ) );
}


extern unsigned __cdecl lfn_dgfa( const char *, unsigned * );
#pragma aux lfn_dgfa "__dos_getfileattr";

_WCRTLINK unsigned _dos_getfileattr( const char *path, unsigned *attr )
{
    return( lfn_dgfa( path, attr ) );
}


extern unsigned __cdecl lfn_sgfa( const char *, unsigned );
#pragma aux lfn_sgfa "__dos_setfileattr";

_WCRTLINK unsigned _dos_setfileattr( const char *path, unsigned attr )
{
    return( lfn_sgfa( path, attr ) );
}

#if 0
/* The open() and sopen() functions take a variable number of arguments.
 * As a consequence, the Watcom library routines already use a stack calling
 * convention very much like __cdecl. Just generate linker aliases for those.
 *
 * Unfortunately, the open flags are *different*, so this doesn't work!
 * It may be possible to use _open_osfhandle() to solve this.
 */
 
#pragma alias( "_sopen_", "__sopen" )
#pragma alias( "_open_", "__open" )
#pragma alias( "open_", "__open" )
#endif
