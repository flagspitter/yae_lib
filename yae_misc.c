/***********************************************************************

  yae_misc.c
  Collection of miscellaneous functions.

***********************************************************************/

//----------------------------------------------------------------------
//-- Include Files
//----------------------------------------------------------------------
#include "yae_lib.h"

//----------------------------------------------------------------------
//-- Public Functions : misc
//----------------------------------------------------------------------

/***************************************************************
	Name        misc_int2char
	Purpose     Convert ( 0 to 15 ) value to char
***************************************************************/
char misc_int2char( int val )
{
	char ret;
	
	if( ( val >= 0 ) && ( val <= 9 ) )
	{
		ret = '0' + val;
	}
	else if( ( val >= 10 ) && ( val <= 15 ) )
	{
		ret = 'A' + ( val - 10 );
	}
	else
	{
		ret = '0';
	}
	
	return ret;
}

/***************************************************************
	Name        misc_char2int
	Purpose     Convert char to int
***************************************************************/
unsigned int misc_char2int( char ch )
{
	unsigned int ret = 0;
	if( ( ch >= '0' ) && ( ch <= '9' ) )
	{
		ret = ch - '0';
	}
	else if( ( ch >= 'A' ) && ( ch <= 'F' ) )
	{
		ret = ch - 'A' + 10;
	}
	else if( ( ch >= 'a' ) && ( ch <= 'f' ) )
	{
		ret = ch - 'a' + 10;
	}
	else
	{
		ret = 0;
	}
	return ret;
}

/***************************************************************
	Name        misc_str2int_r
	Purpose     Convert string to int (Specify radix)
***************************************************************/
unsigned int misc_str2int_r( const char *str, int r )
{
	int radix = r;
	int pos = 0;
	unsigned int ret = 0;
	bool minus = false;
	
	if( str == NULL )
	{
		ret = 0;
	}
	else
	{
		if( str[0] == '0' )
		{
			if( ( str[1] == 'x' ) || ( str[1] == 'X' ) )
			{
				radix = 16;
				pos = 2;
			}
			
			if( ( radix != 16 ) && ( str[1] == 'b' ) || ( str[1] == 'B' ) )
			{
				radix = 2;
				pos = 2;
			}
		}
		
		if( ( radix == 10 ) && ( str[0] == '-' ) )
		{
			minus = true;
			pos++;
		}
		
		while( str[pos] != '\0' )
		{
			if( str[pos] != '_' ) // underbars will be ignored
			{
				ret *= radix;
				ret += misc_char2int( str[pos] );
			}
			pos++;
		}
		
		if( minus )
		{
			ret *= -1;
		}
	}
	
	return ret;
}

/***************************************************************
	Name        CmpCase
	Purpose     Case insensitive char comparison
***************************************************************/
static bool CmpCase( char c1, char c2 )
{
	char t1, t2;
	
	t1 = ( ( c1 >= 'A' ) && ( c1 <= 'Z' ) ) ? ( c1 + 'a' - 'A' ) : c1;
	t2 = ( ( c2 >= 'A' ) && ( c2 <= 'Z' ) ) ? ( c2 + 'a' - 'A' ) : c2;
	
	return (t1 == t2);
}

/***************************************************************
	Name        misc_strcasecmp
	Purpose     Case insensitive string comparison
***************************************************************/
int misc_strcasecmp( const char *s1, const char *s2 )
{
	int i = 0;
	while( CmpCase( s1[i], s2[i] ) && ( s1[i] != '\0' ) && ( s2[i] != '\0' ) )
	{
		i++;
	}
	
	return s1[i] - s2[i];
}

/***************************************************************
	Name        misc_strncasecmp
	Purpose     Case insensitive char comparison ( with length limit )
***************************************************************/
int misc_strncasecmp( const char *s1, const char *s2, int length )
{
	int i = 0;
	
	while( ( i < length ) && CmpCase( s1[i], s2[i] ) && ( s1[i] != '\0' ) && ( s2[i] != '\0' ) )
	{
		i++;
	}
	
	int ret;
	
	if( length == 0 )
	{
		ret = 0;
	}
	else if( i >= length )
	{
		ret = s1[i-1] - s2[i-1];
	}
	else
	{
		ret = s1[i] - s2[i];
	}
	
	return ret;
}

/***************************************************************
	Name        misc_strlen
	Purpose     
***************************************************************/
int misc_strlen( const char *s )
{
	int i = 0;
	while( s[i] != '\0' )
	{
		i++;
	}
	return i;
}

char *misc_SkipSpaces( char *s )
{
	char *p = s;
	
	while( ( *p == ' ' ) || ( *p == '\t' ) || ( *p == '\r' ) || ( *p == '\n' ) )
	{
		p++;
	}
	
	return p;
}

int misc_Separate( char *str, char sepa, char **dst )
{
	int argc = 0;
	char *p = str;
	
	while( ( *p != '\0' ) && ( *p != '\r' ) && ( *p != '\n' ) )
	{
		p = misc_SkipSpaces( p );
		
		if( ( *p != '\0' ) && ( *p != '\r' ) && ( *p != '\n' ) )
		{
			dst[ argc ] = p;
			argc++;
		}
		
		// Search Separator
		while( ( *p != '\0' ) && ( *p != '\r' ) && ( *p != '\n' ) && ( *p != sepa ) )
		{
			p++;
		}
		
		// SkipSeparator
		if( *p == sepa )
		{
			*p = '\0';
			p++;
		}
	}
	
	return argc;
}
