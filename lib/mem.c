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

#define HEAPBEG     0x600000
#define HEAPEND     0xffffff
#define HEAPSZ      (HEAPEND - HEAPBEG)
static int hptr = 0;

char *
alloc(s)
unsigned s;
{
    char *b;
    
    if ((hptr + s) >= HEAPSZ) { return NULL; }
    b = (char *) (HEAPBEG + hptr);
    hptr =+ s;
    return(b);
}

free(p)
char *p;
{
    /* NO-OP because I am lazy :) */
}

int
heapa()
{
    return(HEAPEND - hptr);
}

int
heapu()
{
    return(hptr);
}

char *
memcpy(dst, src, n)
register char *dst;
register char *src;
register unsigned n;
{
    register unsigned i;
    
	if (!dst) {
		return(0);
	}
	/* definitely not a fast memcpy */
	for (i = 0; i < n; i++) {
		dst[i] = src[i];
	}
	return(dst);
}

char *
memset(s, c, n)
register char *s;
register int c;
register unsigned n;
{
	register unsigned i;
	
	if (!s) {
		return(0);
	}
	for (i = 0; i < n; i++) {
		s[i] = (char) c;
	}
	return(s);
}

abs(x) { return(x < 0 ? -x : x); }

static
_reverse(s, len)
register char *s;
{
	register k, i, c;
	int e;

	k = len - 1;
	e = len >> 1;
	for (i = 0; i < e; i++) {
		c = s[k];
		s[k--] = s[i];
		s[i] = c;
	}
}

char *
itoa(n, s, base)
register int n, base;
register char *s;
{
	int i = 0;
	int neg = 0;
	int r;
	
	if (n == 0) {
		s[i++] = '0';
		s[i] = '\0';
		return(s);
	}
	if (n < 0 && base == 10) {
	    neg = 1;
		n = -n;
	}
	while (n != 0) {
		r = n % base;
		s[i++] = (r > 9) ? (r - 10) + 'a' : r + '0';
		n =/ base;
	}
	if (neg) {
		s[i++] = '-';
	}
	s[i] = '\0';
	_reverse(s, i);
	return(s);
}
