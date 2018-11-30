#include <ctype.h>


/**
 *  Converts a lowercase character to uppercase
 *
 *  @param[in] c  The character.
 *  @returns An uppercase character of `c`.
 *
 */
int toupper(int c) {

    if ('a' <= c && c <= 'z')
        c -= 0x20;

    return c;
}
