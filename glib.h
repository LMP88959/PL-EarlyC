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

#define VDCM256_8 0
#define VDCM256TC 1
#define VDCM320_8 2
#define VDCM320TC 3
#define VDCM480_8 4
#define VDCM512_8 5
#define VDCM640_8 6
#define NUMMODES  7

int init88();
kill88();

/* set palette, swap front and back buffers, sync (wait for vertical retrace */
setpal(), swap(), sync();

long clockms();

int getch(), kbhit();
clrkb();
