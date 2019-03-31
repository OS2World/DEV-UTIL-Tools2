/**

	CTAGS.C
	
	Unix-style CTAGS utility for MS OS/2 and MS DOS 3.X.

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**

	Constants and Types

**/

#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif

#define TRUE		1
#define FALSE		0
#define MAXLINE		256
#define MAXNAME		31

/**

	Static Variables

**/

static int nLevel ;		/* nesting level */
static char szLine[MAXLINE] ;	/* line read from file */
static char *szCur ;		/* current position in buffer */
static FILE *pTags ;		/* TAGS file pointer */

/**

	SkipComment()
	
	Skip from the current point in the file to the next end of comment
	line characters. This function should be called with the current
	line in the buffer szLine, and the current point in the	buffer
	pointed to by szCur.
	
	If the end of the comment is found in szLine, szCur will point to 
	the character after the comment in szLine, and the function will
	return TRUE.
	
	If the end of the comment is not found in the buffer, the function
	will return FALSE.

**/

int SkipComment()
{
	int bGotStar = FALSE ;		/* TRUE if we have '*' */

	while ( *szCur )
	{
		switch( *szCur )
		{
		case '/':		/* end of comment? */
			if ( bGotStar )
			{
				++szCur ;
				return TRUE;
			}
			break ;
		case '*':		/* set star flag */
			bGotStar = TRUE ;
			break ;
		default:
			bGotStar = FALSE ;
		}
		++szCur ;
	}
	return FALSE ;
}

/**

	SkipUntil()
	
	Skip from the current point in the file to the end of a quoted
	string. This function should be called with the current	line in 
	the buffer szLine, and the current point in the	buffer pointed
	to by szCur.
	
	If the end of the string is found in szLine, szCur will point to 
	the character after the string in szLine, and the function will
	return TRUE.
	
	If the end of the string is not found in the buffer, the function
	will return FALSE.

**/

int SkipUntil( cBreak )
int cBreak ;			/* break character */
{
	int bSkipNext = FALSE ;		/* TRUE if we have '\' */
	
	while ( *(++szCur) )
	{
		if ( *szCur == (char)cBreak && ! bSkipNext  )
		{
			++szCur ;
			return TRUE;
		}
		else switch( *szCur )
		{
		case '\\':		/* set escaping flag */
			bSkipNext = ! bSkipNext ;
			break ;
		default:
			bSkipNext = FALSE ;
		}
	}
	return FALSE ;
}

/**

	ProcessDecl() - process a function declaration

	This routine will determine whether the line in the buffer is
	the start of a function or a forward reference.  It will make
	its determination by whether it encounters a newline ,semicolon,
	or comma after the function declaration.

**/

void ProcessDecl( szFile )
char *szFile ;
{
	char szFuncName[MAXNAME] ;
	char *szTmp ;
	int nLength ;
	int nParenLevel = 0 ;
	
	/*--- check to see if prototype or declaration ---*/
	szTmp = szCur ;
	while ( *szTmp )
	{
		if ( *szTmp == '\n' )
		{
			/*--- back up to find the function name ---*/
			*szTmp = '\0' ;	
			szTmp = szCur - 1 ;

			while ( isspace( *szTmp ) )
				--szTmp ;
			while ( !isspace(*szTmp) && szTmp >= szLine && 
							*szTmp != '*' )
				--szTmp ;
			++szTmp ;
			nLength = (szCur - szTmp) ;
			strncpy( szFuncName, szTmp, nLength ) ;
			szFuncName[nLength] = '\0' ; 
			fprintf( pTags, "%s \t%s \t^", szFuncName, szFile) ;
			szTmp = szLine ;
			while ( *szTmp )
			{
				/* 'escape' special GREP chars */
				if ( strchr("\\^$.:*+[]", *szTmp) )
					putc( '\\', pTags ) ;
				putc( *szTmp++, pTags ) ;
			}
			fprintf( pTags, "$\n" ) ;
			break ;
		}
		else if ( *szTmp == ')' )
		{
			nParenLevel-- ;
			++szTmp ;
		}
		else if ( *szTmp == '(' )
		{
			nParenLevel++ ;
			++szTmp ;
		}
		else if ( *szTmp == ',' )
		{
			if ( nParenLevel == 0 )
				break ;
			++szTmp ;
		}
		else if ( *szTmp == ';' )
			break ;
		else
			++szTmp ;
	}
}

/**

	FindDecls()
	
	Find a C declaration.

**/

void FindDecls( szFile )
char *szFile ;
{
	FILE *fp ;
	int bComment = FALSE ;
	int Done = FALSE ;
	
	if ( (fp = fopen(szFile, "rt")) == NULL )
	{
		fprintf( stderr, "Unable to open input file %s.\n", szFile ) ;
		return ;
	}
	
	while ( ! feof(fp) )
	{
		fgets( szLine, MAXLINE-1, fp ) ;
		
		szCur = szLine ;
		Done = FALSE ;
		
		while ( ! Done )
		{
			switch ( *szCur )
			{
			case '\n':	/* end of string */
			case '\0':
				Done = TRUE ;
				break ;
			case '/':	/* set comment flag */
				bComment = TRUE ;
				break ;
			case '*':
				if ( bComment )
				{
					while ( ! SkipComment() && ! feof(fp) )
					{
						szCur = szLine ;
						fgets( szLine, MAXLINE-1, fp ) ;
					}
				}
				bComment = FALSE ;
				break ;
			case '\"':	/* process double quoted string */
				bComment = FALSE ;
				while ( ! SkipUntil('\"') && ! feof(fp) ) 
				{
					fgets( szLine, MAXLINE-1, fp ) ;
					szCur = szLine ;
				}
				break ;
			case '\'':	/* process single quoted constant */
				bComment = FALSE ;
				while ( ! SkipUntil('\'') && ! feof(fp) ) 
				{
					fgets( szLine, MAXLINE-1, fp ) ;
					szCur = szLine ;
				}
				break ;
			case '{':
				bComment = FALSE ;
				++nLevel ;
				break ;
			case '}':
				bComment = FALSE ;
				--nLevel ;
				break ;
			case '(':	/* start of function call */
				if ( nLevel == 0 )
					ProcessDecl( szFile ) ;
				break ;
			}
			++szCur ;
		}
	}
	fclose( fp ) ;
}

/**

	Main Routine

**/

void main( argc, argv )
int argc ;
char *argv[] ;
{
	int count ;
	
	fprintf( stderr, "CTAGS - C file tag utility\n" ) ;
	fprintf( stderr, "Copyright (c)1988 Computer Aided Planning, Inc.\n" ) ;
	fprintf( stderr, "All Rights Reserved\n\n" ) ;
	
	if ( argc < 2 )
	{
		fprintf( stderr, "Usage: CTAGS file [file...]\n" ) ;
		exit(1) ;
	}
	
	if ( (pTags = fopen("TAGS", "wt")) == NULL )
	{
		fprintf( stderr, "Unable to open output file TAGS\n" ) ;
		exit(1) ;
	}
	
	count = 1 ;
	while ( count < argc )
	{
		nLevel = 0 ;
		FindDecls( argv[count++] ) ;
	}
		
	fclose( pTags ) ;
}
