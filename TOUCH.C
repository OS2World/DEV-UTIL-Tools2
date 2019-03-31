/**

    TOUCH.C - UNIX style file date modification command

**/

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <time.h>
#include <ctype.h>
#include <sys\types.h>
#include <sys\utime.h>
#include <sys\stat.h>
#include <errno.h>
#include <io.h>

/**

    Static variables for TOUCH

**/

#define TRUE	    1
#define FALSE	    0
#define ERROR	    -1

static int Verbose ;		/* TRUE for verbose mode */
static int NoCreate ;		/* True to not create 0 length file */

static struct utimbuf ModTime ; /* time to set modification date to */

static int days_in_month[] =
{ 
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/**

    ParseTime() - parse the time string MMDDHHMM[YY]

    Return TRUE if able to extract time, FALSE otherwise

**/

static int ParseTime( buffer )
char *buffer ;
{
	char *s ;
	time_t timeval ;
	struct tm *timebuf ;
	int month, day, hour, min, year, i ;

	putenv("TZ=GMT") ;
	tzset() ;

	if ( (strlen(buffer) != 8) && (strlen(buffer) !=10) )
		return FALSE ;

	for ( s = buffer ; *s ; s++ )
		if ( !isdigit(*s) )
			return FALSE ;

	if ( strlen(buffer) == 10 )
		year  = ((buffer[8]-'0') * 10) + (buffer[9]-'0') ;
	else
	{
		time( &timeval ) ;
		timebuf = localtime( &timeval ) ;
		year = timebuf->tm_year ;
	}
	if ( year < 70 )
		return FALSE ;

	month = ((buffer[0]-'0') * 10) + (buffer[1]-'0') ;
	if ( month < 1 || month > 12 )
		return FALSE ;

	if ( !(year % 4) )
		days_in_month[1]++ ;

	day   = ((buffer[2]-'0') * 10) + (buffer[3]-'0') ;
	if ( day < 1 || day > days_in_month[month-1] )
		return FALSE ;

	hour  = ((buffer[4]-'0') * 10) + (buffer[5]-'0') ;
	if ( hour > 23 )
		return FALSE ;

	min   = ((buffer[6]-'0') * 10) + (buffer[7]-'0') ;
	if ( min > 59 )
		return FALSE ;

	if ( (month == 2) && (day == 29) && (year % 4) )
		return FALSE ;

	/*-- calculate the number of seconds since 00:00:00 01/01/1970 ---*/

	timeval = 0L ;

	for ( i = 70 ; i < year ; i++ )
		if ( i % 4 )
			timeval += 365 ;
		else
			timeval += 366 ;

	for ( i = 0 ; i < month-1 ; i ++ )
		timeval += days_in_month[i] ;

	timeval += (day-1) ;		/* time contains number of days */

	timeval *= 24 ;
	timeval += hour ;		/* time contains number of hours */

	timeval *= 60 ;
	timeval += min ;		/* time contains number of minutes */

	timeval *= 60 ;			/* time contains number of seconds */

	ModTime.actime  = timeval + timezone ;
	ModTime.modtime = timeval + timezone ;

	return TRUE ;
}

/**

    Banner() - print a program title banner

**/

static void Banner()
{
	fprintf( stderr, "TOUCH\n" ) ;
	fprintf( stderr, "Copyright (c) 1987 by Computer Aided Planning, Inc.\n");
	fprintf( stderr, "All Rights Reserved.\n\n") ;
}

/**

    ShowUsage() - show the usage of the TOUCH command

**/

static void ShowUsage()
{
	fprintf( stderr, "Usage: TOUCH [-cmv?] [mmddhhmm[yy]] file ...\n\n" ) ;
	fprintf( stderr, "\t-c : Do not create file if it doesn't exist\n" ) ;
	fprintf( stderr, "\t-v : Verbose, echo each file name as touched\n") ;
	fprintf( stderr, "\t-m : Set modification time only (no effect)\n") ;
	fprintf( stderr, "\t-? : Display this message\n") ;
	exit(1) ;
}


/**

    MAIN ROUTINE

**/

void main( argc, argv )
int argc ;
char *argv[] ;
{
	time_t timeval ;
	int got_time = FALSE ;
	int i, fh ;

/*+	Banner() ;*/

	if ( argc < 2 )
	{
		ShowUsage() ;
	}

	for ( i = 1 ; i < argc ; i ++ )
	{
		if ( *argv[i] == '-' || *argv[i] == '/' )
		{
			switch( *(argv[i]+1) )
			{
			case 'c' :		/* no create mode */
			case 'C' :
				NoCreate = TRUE ;
				break ;
			case 'v' :		/* verbose mode */
			case 'V' :
				Verbose = TRUE ;
				break ;
			case '?' :		/* help */
				ShowUsage() ;
				break ;
			}
		}
		else
		{
			if ( ! got_time )
			{
				got_time = TRUE ;

				if( ParseTime(argv[i]) )
					continue ;
				else
				{
					time ( &timeval ) ;
					ModTime.actime  = timeval ;
					ModTime.modtime = timeval ;
				}
			}

			if( utime( argv[i], &ModTime ) == ERROR )
			{
				/*--- create a 0 length file ---*/

				if ( errno == ENOENT && ! NoCreate )
				{
					if ( (fh = creat( argv[i], S_IREAD|S_IWRITE)) != ERROR )
					{
						utime( argv[i], &ModTime ) ;
						if ( Verbose )
							fprintf( stderr, "\t%s\n", strupr(argv[i]) ) ;
						close(fh) ;
					}
					else
						fprintf( stderr, "\t%s : E%05d - %s\n", strupr(argv[i]),
						    errno, sys_errlist[errno] ) ;
				}
				else
					fprintf( stderr, "\t%s : E%05d - %s\n", strupr(argv[i]),
					    errno, sys_errlist[errno] ) ;
			}

			if ( Verbose )
				fprintf( stderr, "\t%s\n", strupr(argv[i]) ) ;
		}
	}
}
