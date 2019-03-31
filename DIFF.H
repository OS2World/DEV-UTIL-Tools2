/**

	DIFF.H
	
	Header file for DIFF.C
	
**/

/* Constant Values */
#ifndef TRUE
#define TRUE		1
#endif
#ifndef FALSE
#define FALSE		0
#endif

#define PAGESIZE	4096	/* default line page size */
#define MASK		0x0FFF	/* 12 bit hash value */
#define MAXLINE		256	/* maximum line length */

#define CHANGE		0	/* change a range of lines */
#define DELETE		1	/* delete a range of lines */
#define ADD		2	/* add a range of lines */

/* Data Types */

typedef int BOOL ;

typedef struct 
{
	long addr ;		/* address of string in file */
	unsigned int hash ;	/* hash value for string */
	int linenum ;		/* line number in file */
}
LINE ;

/**
	Function prototypes
**/	
static unsigned int Hash(char *);
static LINE *ReadFile(FILE *, int *);
static BOOL CheckHashes(LINE *, LINE *, int); 
static int CheckStrings(FILE *, LINE *, FILE *, LINE *, int);
static void PrintLines(FILE *, LINE *, int , FILE *, LINE *, int);
static void DoDiff(FILE *, LINE *, int, FILE *, LINE *, int, int);
static void banner(void);
void main(int, char **);

