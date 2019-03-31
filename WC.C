/**

	WC.C
	
	A UNIX-style Word Count utility.

**/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <process.h>

/**
	Constants
**/
#define TRUE 	1		/* Boolean Truth */
#define FALSE	0		/* Boolean Falsehood */
#define MAXLINE	2048		/* maximum line size */

/**
	Statics
**/
static char szBuffer[MAXLINE] ;
static unsigned long lChars ;	/* Number of characters */
static unsigned long lWords ;	/* Number of words */
static unsigned long lLines ;	/* Number of lines */
static unsigned long lTChars ;	/* Total characters */
static unsigned long lTWords ;	/* Total words */
static unsigned long lTLines ;	/* Total lines */
static int bChars ;		/* print character count */
static int bWords ;		/* print word count */
static int bLines ;		/* print line count */
static int bTotals ;		/* print a total count */

/**
	Function prototypes
**/
void main( int, char ** ) ;
static void Count( char * ) ;
static void PrintCount( long, long, long, char * ) ;

/**

	PrintCount()

	Print the count of words, characters and lines.

**/

static void PrintCount( lCCnt, lWCnt, lLCnt, szName ) 
long lCCnt ;
long lWCnt ;
long lLCnt ;
char *szName ;
{
	if ( bChars )		/* print character count only */
		printf( "%ld %s\n", lCCnt, szName ) ;
	else if ( bWords )	/* print word count only */
		printf( "%ld %s\n", lWCnt, szName ) ;
	else if ( bLines )	/* print line count only */
		printf( "%ld %s\n", lLCnt, szName ) ;
	else			/* print all three */
		printf( "%ld\t%ld\t%ld %s\n", lLCnt, lWCnt, lCCnt, szName ) ;
}

/**

	Count()
	
	Count number of words, characters and lines in a file.
	Set lChars, lWords, and lLines accordingly.

**/
static void Count( szName ) 
char *szName ;
{
	FILE *pFile ;
	int c ;
	int bNonBlank = FALSE ;
	char *szStd = "" ;
	
	lChars = 0L ;
	lWords = 0L ;
	lLines = 0L ;

	if ( szName )
	{
		pFile = fopen( szName, "rb" ) ;
		if ( pFile == NULL )
		{
			printf( "ERROR: could not open file %s\n", szName ) ;
			exit(1) ;
		}
	}
	else
	{
		pFile = stdin ;		/* NULL name, read standard input */
		szName = szStd ;
	}
	
	while ( TRUE )
	{
		c = fgetc(pFile) ;

		if ( c == EOF )
			break ;

		++lChars ;
			
		if ( isspace(c) )
		{
			if ( bNonBlank )
				++lWords ;
			bNonBlank = FALSE ;
		}
		else
			bNonBlank = TRUE ;

		if ( c == '\n' )
			++lLines ;
	}

	/* close file if not stdin */
	if ( pFile != stdin )
		fclose( pFile ) ;

	/* print out the line count */
	PrintCount( lChars, lWords, lLines, szName ) ;

	/* Add to the totals */
	lTChars += lChars ;
	lTWords += lWords ;
	lTLines += lLines ;
}

/**

	MAIN ROUTINE

**/

void main( argc, argv )
int argc ;
char *argv[] ;
{
	int nArg;

	/* Print a cute littel banner */
	fprintf( stderr, "WC - A Word Count Utility\n" ) ;
	fprintf( stderr, "By Ralph E. Brendler\n\n" ) ; 
	
	nArg = 1 ;
	while ( nArg < argc )
	{
		if ( *argv[nArg] != '-' && *argv[nArg] != '/' )
			break ;

		/* process command line switches */
		switch( *(argv[nArg]+1) )
		{
		case 'c':	/* print character count */
			bChars = TRUE ;
			bWords = bLines = FALSE ;
			break ;
		case 'w':	/* print word count */
			bWords = TRUE ;
			bChars = bLines = FALSE ;
			break ;
		case 'l':	/* print line count */
			bLines = TRUE ;
			bChars = bWords = FALSE ;
			break ;
		case 't':	/* print totals */
			bTotals = TRUE ;
			break ;
		case '?':	/* print usage */
			printf( "Usage: wc [-cltw?] [file...]\n" ) ;
			exit(0) ;
			break ;
		default:	/* bad switch */
			printf( "ERROR: Bad switch %s\n", argv[nArg] ) ;
			printf( "Type \"wc -?\" for help\n" ) ;
			exit(1) ;
			break ;
		}

		++nArg ;
	}

	/* go through each file, counting stuff */
	if ( nArg == argc )
		Count( NULL ) ;		/* count stdin */
	else
		while ( nArg < argc )	/* count filename */
			Count( argv[nArg++] ) ;

	/* Print out the totals */
	if ( bTotals )
		PrintCount( lTChars, lTWords, lTLines, "Total Count" ) ;

	exit(0) ;
}

