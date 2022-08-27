/*****************************************************************************/
/*
 * Early C implementation of an extra lite version of the PiSHi engine.
 * 
 *   by EMMIR 2018-2022
 *   
 *   YouTube: https://www.youtube.com/c/LMP88
 *   
 * This software is released into the public domain.
 */
/*****************************************************************************/

#include <stdio.h>

exit(status)
{
    asm("jmp 0");
}

eirpts()
{
    asm("andi #0xf8ff,.sr");
}

puts(s)
char *s;
{
    putstr(s);
    putchar('\n');
}

putstr(s)
char *s;
{
    while (*s) {
        putchar(*s++);
    }
}

putchar(c)
{
    TERM_ADDR->iaddr = c;
}

printd(n)
{
    static char s[16];
	
	itoa(n, s, 10);
	puts(s);
}

printx(n)
{
	static char s[16];
	
	itoa(n, s, 16);
	puts(s);
}
