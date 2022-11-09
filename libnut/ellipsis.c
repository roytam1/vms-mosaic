/* Copyright (C) 2005, 2006, 2007 - The VMS Mosaic Project */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
#define ELLIPSIS_TEST
*/

#ifdef ELLIPSIS_TEST
void usage_statement(void);
int compact_string(char *main_string, char *ellipsis_string, 
		   int num_chars, int mode, int eLength);

int main(int argc, char *argv[])
{
    char *main_string, *ellipsis_string;
    int i, j, num_chars, mode, result;
    int ellipsisLength = 3;

    /* Check number of args */
    if (argc != 4) {
        usage_statement();
        return(1);
    }

    /* Get args */
    main_string = argv[1];
    num_chars = atoi(argv[2]);
    mode = atoi(argv[3]);

    /* Allocate ellipsis_string */
    ellipsis_string = (char *)calloc(num_chars, 1);

    result = compact_string(main_string, ellipsis_string, num_chars, mode,
			    ellipsisLength);
    if (result == 1) {
	printf("The original string is:\n");
	printf("\t|%s|\n\n", main_string);
	printf("And the compacted string is:\n");
	printf("\t|%s|\n\n", ellipsis_string);
    } else if (result == 2) {
	printf("String short enough already\n");
    } else {
	printf("You screwed something up...\n");
    }
}
#endif


int compact_string(char *main_string, char *ellipsis_string, 
		   int num_chars, int mode, int eLength)
{
    int string_len = strlen(main_string);
    int feem, puff, i;

    if (string_len <= num_chars)
	return(2);

    switch(mode) {
	case 1:
	    puff = num_chars - eLength;
	    feem = string_len - puff;

	    for (i = 0; i < eLength; i++)
		ellipsis_string[i] = '.';

	    ellipsis_string[i] = '\0';

	    strncat(ellipsis_string, main_string + feem, puff);
	    break;

	case 2: {
	    int right_side, left_side;

	    puff = num_chars - eLength;
	    left_side = puff >> 1;
	    right_side = puff - left_side;

	    strncpy(ellipsis_string, main_string, left_side);
	    for (i = left_side; i < num_chars - right_side; i++)
		ellipsis_string[i] = '.';

	    ellipsis_string[num_chars - right_side] = '\0';
	    strncat(ellipsis_string, main_string + (string_len - right_side),
		    right_side);
	    break;
        }
	
	case 3:
	    puff = num_chars - eLength;

	    strncpy(ellipsis_string, main_string, puff);

	    for (i = puff; i < num_chars; i++)
		ellipsis_string[i] = '.';
	    break;
	
        default:
#ifdef ELLIPSIS_TEST
	    usage_statement();
	    exit(0);
#else
	    return(0);
#endif
	
    }
    ellipsis_string[num_chars] = '\0';

    return(1);
}


#ifdef ELLIPSIS_TEST
void usage_statement(void)
{
    printf("Usage: main string num_chars mode; where mode is:\n");
    printf("       1=cut off start, 2=cut off middle, 3=cut off end\n");
}
#endif
