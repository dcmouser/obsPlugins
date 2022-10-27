#include "jrfuncs.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <ctype.h>


//---------------------------------------------------------------------------
// from https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
char* jrtrim(char *str)
{
    size_t len = 0;
    char *frontp = str;
    char *endp = NULL;

    if( str == NULL ) { return NULL; }
    if( str[0] == '\0' ) { return str; }

    len = strlen(str);
    endp = str + len;

    /* Move the front and back pointers to address the first non-whitespace
     * characters from each end.
     */
    while( isspace((unsigned char) *frontp) ) { ++frontp; }
    if( endp != frontp )
    {
        while( isspace((unsigned char) *(--endp)) && endp != frontp ) {}
    }

    if( frontp != str && endp == frontp )
            *str = '\0';
    else if( str + len - 1 != endp )
            *(endp + 1) = '\0';

    /* Shift the string so that it starts at str so that if it's dynamically
     * allocated, we can still free it on the returned pointer.  Note the reuse
     * of endp to mean the front of the string buffer now.
     */
    endp = str;
    if( frontp != str )
    {
            while( *frontp ) { *endp++ = *frontp++; }
            *endp = '\0';
    }

    return str;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// ATTN: note no bounds checking be careful
bool jrLeftCharpSplit(char* str, char* leftPart, char separator) {
    if (str[0] == '\0') {
        strcpy(leftPart, "");
        return false;
    }
    char* charp = strchr(str, separator);
    if (!charp) {
        // last part
        strcpy(leftPart, str);
        strcpy(str, "");
    }
    else {
        strncpy(leftPart, str, charp - str);
        leftPart[charp - str] = '\0';
        strcpy(str, charp + 1);
    }
        return true;
}
//---------------------------------------------------------------------------










