#ifndef yae_misc_h_
#define yae_misc_h_

//----------------------------------------------------------------------
//-- (System) Include Files
//----------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#ifndef ArraySize
	#define ArraySize(a) ( sizeof(a) / sizeof(a[0]) )
#endif

#define misc_str2int(s) misc_str2int_r(s,10)

//----------------------------------------------------------------------
//-- Public Function-Prototypes
//----------------------------------------------------------------------
extern char misc_int2char( int val );
extern unsigned int misc_char2int( char ch );
extern unsigned int misc_str2int_r( const char *str, int r );
extern int misc_strcasecmp( const char *s1, const char *s2 );
extern int misc_strncasecmp( const char *s1, const char *s2, int length );
extern int misc_strlen( const char *s );
extern char *misc_SkipSpaces( char *s );
extern int misc_Separate( char *str, char sepa, char **dst );

#endif
