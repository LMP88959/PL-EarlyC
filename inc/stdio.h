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

#define INT_MAX 0x7fffffff
#define INT_MIN 0x80000000
#define NULL    0

/* for accessing memory as an: */
struct { int  iaddr; }; /* int  */
struct { char caddr; }; /* char */

#define KEYB_ADDR   0x1a0
#define TERM_ADDR   0x1d0

#define IRQ_L1      0144
#define IRQ_L2      0150
#define IRQ_L3      0154
#define IRQ_L4      0160
#define IRQ_L5      0164
#define IRQ_L6      0170
#define IRQ_L7      0174

puts(), putstr(), putchar(), printd(), printx();

exit();
eirpts(); /* enable interrupts */

char *alloc();
free();
int heapa(); /* heap available */
int heapu(); /* heap used */

int abs();

char *memcpy();
char *memset();
char *itoa();
