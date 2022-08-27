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

#include "pl.h"

#include "glib.h"

#include <stdio.h>

/* cc68 -O -o prog.bin main.c glib.c gfx.c clip.c imode.c math.c pl.c */
static init();
static int update();
static display();

/* special palette where the LS 3-bits is the inverse lookup key */
static int pal[6] = {
  0000000,
  0172321,
  0440802, /* 8 and 9 were allowed in octal literals, they became 010 and 011 */
  0656353,
  0233124,
  0777775,
};

main()
{
    extern vdch, vdcv, tc;
    extern long dblbuf;
    auto int *vm;
    long p, c, n;
    short fc = 0;
    auto dt, k = 1;

    puts("initializing");
    /* initialize with VDCM320_8 to use 8-bit color */
    if (!init88(VDCM320TC)) {
        return;
    }
    
    setpal(pal, 6);
    swap();

    /* if true color, we can render to video memory directly */
    if (tc) {
        vm = (int *) dblbuf;
    } else {
        vm = alloc(vdch * vdcv * sizeof(int));
    }
    pinit(vm, vdch, vdcv);

    init();
    clrkb();
    p = clockms();
    n = clockms() + 1000;
    while (k) {
        c  = clockms();
        dt = c - p;
        if (dt < 33) {
            continue;
        }
        p = c;
        k = update();
        display();
        fc++;
    
        if (clockms() >= n) {
            putstr("FPS: ");
            printd(fc);
            n  =+ 1000;
            fc = 0;
        }
    }
    exit(0);
}

char *
umemgt(n, esz)
unsigned n, esz;
{
    return(alloc(n * esz));
}

umemfr(p)
char *p;
{
    free(p);
}

uerror(id, modnm, msg)
char *modnm, *msg;
{
    printx(id);
    puts(modnm);
    puts(msg);
    kill88();
    exit(0);
}

bufcpy(to, from, size)
register char *to, *from;
register unsigned size;
{
    while (size--) {
        *to++ = *from++;
    }
}

bufset(to, val, size)
register char *to;
register char val;
register unsigned size;
{
    while (size--) {
       *to++ = val;
    }
}

#define CUSZ    128 /* cube size */
#define GRSZ    1   /* grid size */

static struct PL_OBJ *floor;
static struct PL_OBJ *tcube;
static struct PL_TEX tex;
static rot  1;
static incv 0;
static decv 0;
static dir  1;

static
maketex()
{
    static int chk[PTDIM * PTDIM];
    auto i, j, t;
    int c;
    
    t = 0x10;

    for (i = 0; i < PTDIM; i++) {
        for (j = 0; j < PTDIM; j++) {
            if ((i < t) || (j < t) ||
               (i > ((PTDIM - 1) - t)) ||
               (j > ((PTDIM - 1) - t))) {
                /* border color */
                c = pal[1];
            } else {
                /* checkered pattern */
                c = (((i & t)) ^ ((j & t))) ? pal[1] : pal[3];
            }
            if ((i == j) || abs(i - j) < 3) {
                /* thick red line along diagonal */
                chk[i + j * PTDIM] = pal[2];
            } else {
                chk[i + j * PTDIM] = c;
            }
        }
    }
    
    for (i = 0; i < (PTDIM * PTDIM); i++) {
        chk[i] = c18to24(chk[i]);
    }
    tex.texdat = chk;
}

static
init()
{
    maketex();

    imtex(&tex);
    tcube = genbox(CUSZ, CUSZ, CUSZ, PALL, 
            pal[5] >> 12 & 077, 
            pal[5] >>  6 & 077, 
            pal[5] >>  0 & 077);
	imtex(NULL);
	floor = genbox(CUSZ, CUSZ, CUSZ, PTOP,
	        pal[4] >> 12 & 077, 
	        pal[4] >>  6 & 077, 
	        pal[4] >>  0 & 077);
	
	vfov   = 8;
	cullm  = PCBACK;
	rastm  = PRTEX;
    
    /* fixed camera for this demo */
    defcam(-64, 100, 200, 0, 0);
}

static int
update()
{    
    static s 4;
    /* note: compiler supports the old assignment operators */
    switch (getch()) {
        case 033: return(0);
        case 040: rot =^ 1; break;
        case 'a': if (s < 0100) s =<< 1; break;
        case 'd': if (s > 1) s =>> 1; break;
    }
    /*printd(s);*/
    if ((dir == 1) && (incv >= 255)) {
        dir = -1;
        incv = 255;
    } else if ((dir == -1) && (incv <= 0)) {
        dir = 1;
        incv = 0;
    }
    incv =+ (s * dir);
	decv =- 4;

	return(1);
}

static
display()
{
    extern vdch, vdcv, tc;
    extern long dblbuf;
    auto i = 0, j = 0;
    register int vsz;
    register char *ptr;
    register long vbase;

    if (tc) {
        pvideo = (int*) dblbuf;
    }
    
    /* clear viewport */
	pc();

    /* draw tile grid */
    for (i = -GRSZ; i < GRSZ; i++) {
        for (j = -GRSZ; j < GRSZ; j++) {
            mstidt();
            mtrans(0 + i * CUSZ, 0, 600 + j * CUSZ);
            odraw(floor);
        }
    }
    
    /* draw textured cube */
    mstidt();
    mtrans(-64, 100, 500);
    if (rot) {
        mrotx(decv >> 2);
        mroty(decv >> 1);
    }
    mscale(PONE * (incv + 128) >> 8, PONE, PONE);
    odraw(tcube);

    sync();

    if (tc == 0) {
        /* RGB to index */
        vsz = (vdch * vdcv);
        vbase = dblbuf + vsz;
        while (vsz--) {
            /* the code does the following: *((char *) vbase) = ...; vbase--; */
            vbase--->caddr = pvideo[vsz] & 07;
            /* caddr struct member declared in stdio.h */
        }
    }

	swap();
}
