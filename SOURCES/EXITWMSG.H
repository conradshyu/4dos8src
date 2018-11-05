#include <string.h>

#define TZNAME_MAX 128

const char _far * _fastcall get_variable(const char *);

// replacement of getenv() to avoid linking the standard MSVC startup code
static char *getenv(const char *var)
{
	static char TZbuf[TZNAME_MAX*4];
	const char _far *p = get_variable(var);
	return p ? (char *)(long)_fstrcpy(TZbuf, p) : NULL;
}
