/**

	CHMOD.C

	UNIX style file attribute modification program.  This program 
	uses OS/2 specific code to get and set file attributes.

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <os2.h>

/**
	Constants
**/
#define F_READONLY	0x0001		/* read-only attribute */
#define F_HIDDEN	0x0002		/* hidden attribute */
#define F_SYSTEM	0x0004		/* system attribute */
#define F_SUBDIR	0x0010		/* subdirectory attribute */
#define F_ARCHIVE	0x0020		/* archive attribute */

/**
	Function Prototypes
**/
static void SetAttribute(char *, unsigned) ;
void main(int, char **) ;

/**

	SetAttribute()

	Set the attribute for a file to a specific number.  Uses OS/2
	dependant code.

	This routine will return 0xFFFF if unable to set attribute.

**/
static void SetAttribute(szName, uAtt)
char *szName ;
unsigned uAtt ;
{
	printf( "%s: (" , szName ) ;

	if ( uAtt == 0 )
		printf( " Normal" ) ;
	if ( uAtt & F_READONLY )
		printf( " Read-Only" ) ;
	if ( uAtt & F_HIDDEN )
		printf( " Hidden" ) ;
	if ( uAtt & F_SYSTEM )
		printf( " System" ) ;
	if ( uAtt & F_ARCHIVE )
		printf( " Archive" ) ;
	if ( uAtt & F_SUBDIR )
		printf( " Subdirectory" ) ;

	printf( " )" ) ;

	if ( DosSetFileMode( szName, (uAtt & ~F_SUBDIR), 0L ) )
		printf( " - Error!" ) ;

	printf( "\n" ) ;
}

/**

	MAIN ROUTINE

**/

void main(argc, argv)
int argc ;
char *argv[] ;
{
	unsigned fTurnOn, fTurnOff ;
	unsigned *fFlag ;
	int nArg ;

	fprintf( stderr, "CHMOD - A file attribute change utility\n" ) ;
	fprintf( stderr, "By Ralph Brendler\n\n" ) ;

	if ( argc < 2 )
	{
		printf( "Type \"chmod -?\" for help\n" ) ;
		exit(1) ;
	}

	fTurnOn = 0 ;			/* bits to turn on */
	fTurnOff = 0 ;			/* bits to turn off */

	/* parse out the command line switches */
	nArg = 1 ;
	while ( *argv[nArg] == '-' || *argv[nArg] == '+' || *argv[nArg] == '=' )
	{
		char *szTmp ;

		if ( nArg >= argc )
		{
			printf( "ERROR: no file names\n" ) ;
			exit(1) ;
		}
		
		szTmp = argv[nArg] ;
		
		if ( *szTmp == '-' )
			fFlag = &fTurnOff ;
		else if ( *szTmp == '+' )
			fFlag = &fTurnOn ;
		else	/* *szTmp == '=' */
		{
			fFlag = &fTurnOn ;
			fTurnOff = 0xFFFF ;
		}

		while ( *(++szTmp) )
		{
			switch( *szTmp ) 
			{
			case 'a':		/* archive */
				*fFlag |= F_ARCHIVE ;
				break ;

			case 'h':		/* hidden */
				*fFlag |= F_HIDDEN ;
				break ;

			case 'r':		/* read only */
				*fFlag |= F_READONLY ;
				break ;

			case 's':		/* system */
				*fFlag |= F_SYSTEM ;
				break ;

			case '?':		/* help */
				printf( "Usage: chmod {+-=}{ahrs} [file...]\n" ) ;
				exit(0) ;
				/* unreachable - break implied */

			default:		/* bad input */
				printf( "ERROR: unrecognized switch %s\n",
							argv[nArg] ) ;
				exit(1) ;
				/* unreachable - break implied */
			}
		}
		++nArg ;
	}

	/* Run down list of files, setting attributes.	*/

	/* Note that this section uses the DosGetFirst	*/
	/* and DosGetNext funcs, rather than linking to */
	/* the SETARGV object, since SETARGV will not	*/
	/* find hidden or read-only files.		*/
	while ( nArg < argc )
	{
		HDIR hDir = 1 ;
		unsigned uCnt = 1 ;
		FILEFINDBUF FileBuf ;
		unsigned uAtt ;
		char szPath[80] ;
		char *szName ;

		/* read the first file name */		
		DosFindFirst( argv[nArg], &hDir, 0x37, &FileBuf, 
					sizeof(FileBuf), &uCnt, 0L ) ;

		/* strip off the path information */
		strcpy( szPath, argv[nArg] ) ;
		szName = strrchr( szPath, '\\' ) ;
		if ( szName )
			++szName ;
		else
			szName = szPath ;

		while ( uCnt > 0 )
		{
			uAtt = FileBuf.attrFile ;
			strncpy(szName, FileBuf.achName, FileBuf.cchName) ;
			szName[FileBuf.cchName] = '\0' ;

			if ( uAtt & F_SUBDIR )	/* hidden only */
			{
				/* ignore '.' and '..' directories */
				if ( *szName != '.' )
				{
					if ( (fTurnOn & ~F_HIDDEN) 
							|| (fTurnOff & ~F_HIDDEN) )
						printf( "WARNING: Can only change Hidden attribute for a directory\n" ) ;
					if ( fTurnOn & F_HIDDEN || fTurnOff & F_HIDDEN )
					{
						uAtt &= (~(fTurnOff & F_HIDDEN)) ;
						uAtt |= (fTurnOn & F_HIDDEN) ;
						SetAttribute( szPath, uAtt ) ;
					} 
				}
			}
			else
			{
				uAtt &= (~fTurnOff) ;
				uAtt |= fTurnOn ;
				SetAttribute( szPath, uAtt ) ;
			}

			/* read the next file name */
			hDir = 1 ;
			uCnt = 1 ;
			DosFindNext(hDir, &FileBuf, sizeof(FileBuf), &uCnt) ;
		}
		++nArg ;
	}
}
