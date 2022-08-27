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

#include "glib.h"

#include <stdio.h>

#define VIDEO_BEG 0x00000200
#define VIDEO_END 0x000fa200

#define VDC_IO_PORT 0x1b0

/* VDC regs + commands */
#define RVMOD   02000
#define RUSET   02002
#define RPHI9   02003
#define RPMD9   02004
#define RPLO9   02005
#define UVIDM   001
#define UREFR   002
#define UPALT   004
#define UPGAD   010

long dblbuf;
vp   0; /* visible page */
tc   0; /* true color */
vdch 0;
vdcv 0;

static char kbuf[32];
static char kpbeg 0;
static char kpend 0;

static int egapal[16] = {
    0000000,0000052,0005200,0005252,
    0520000,0520052,0522500,0525252,
    0252525,0252577,0257725,0257777,
    0772525,0772577,0777725,0777777
};

static struct vidmode {
    int w, h, m;
} vmodes[] = {
    256, 192, VDCM256_8,
    256, 192, VDCM256TC,

    320, 200, VDCM320_8,
    320, 200, VDCM320TC,

    480, 320, VDCM480_8,

    512, 384, VDCM512_8,

    640, 400, VDCM640_8
};

static vdc_ack  0;
static vdc_sync 0;
static vdc_stat 0;

/* write to VDC */
vdcw(addr, val)
unsigned addr;
unsigned val;
{
	vdc_ack = 0;
	VDC_IO_PORT->iaddr = ((addr & 07777) << 18) | (val & 0777777);
	while (!vdc_ack);
}

setpal(p, l)
register int *p;
register unsigned l;
{    
    extern vdcw();
    
    while (l--) {
        vdcw(l, p[l]);
    }
    vdcw(RUSET, UPALT);
    msdel(100);
}

static
_ISR_generic(interrupt) /* tell compiler this is an interrupt service routine */
{
    asm("movl   #73,0x1d0"); /* ISR\n */
    asm("movl   #83,0x1d0");
    asm("movl   #82,0x1d0");
    asm("movl   #10,0x1d0");
    asm("stop");
    asm("jmp 0"); /* safety */
}

static
setISRs()
{
    register i;
    
    /* not handling any of these specially, just kill system */
    for (i = 2; i < 48; i++) {
        (i * 4)->iaddr = (long) _ISR_generic;
    }
    
    for (i = 0; i < 10; i++) {
        (0x08 + (i * 4))->iaddr = (long) _ISR_generic;
    }
    0x3c->iaddr = (long) _ISR_generic;
    0x60->iaddr = (long) _ISR_generic;
    0x7c->iaddr = (long) _ISR_generic;
}

static
_ISR_kbd_inp(interrupt)
{
    /* save registers d0 d1 and a0 */
    asm("moveml #0xc080,.sp@-");
    asm("movb    kpbeg,.d0");
    asm("andl    #31,.d0");
    asm("addql   #1,.d0");
    asm("movb    kpend,.d1");
    asm("andl    #31,.d1");
    asm("cmpl    .d1,.d0");
    asm("jeq .k000");
    asm("movb    kpbeg,.d0");
    asm("andl    #31,.d0");
    asm("addl    #kbuf,.d0");
    asm("movl    .d0,.a0");
    asm("addqb   #1,kpbeg");
    asm("movb    0x1a0,.a0@"); /* 0x1a0 = KEYB_ADDR */
    asm(".k000:");
    /* restore d0 d1 and a0 */
    asm("moveml .sp@+,#0x103");
    
    /* C version */
    /*  if ((kpbeg + 1) != kpend) {
          kbuf[kpbeg++] = *((char*)KEYB_ADDR);
    }*/
}

static
_ISR_vdc_inp(interrupt)
{
    /* save d0, d1 */
    asm("moveml #0xc000,.sp@-");
    asm("movl    0x1b0,.d0");
    asm("cmpl    #118,.d0");
    asm("jne .vdcn0");
    asm("movl    #1,vdc_ack");
    asm(".vdcn0:");
    asm("cmpl    #115,.d0");
    asm("jne .vdcn1");
    asm("movl    #1,vdc_sync");
    asm(".vdcn1:");
    asm("movl    .d0,.d1");
    asm("andl    #0x7f,.d1");
    asm("cmpl    #116,.d1");
    asm("jne .vdcend");
    asm("movl    #14,.d1");
    asm("asrl    .d1,.d0");
    asm("andl    #0777,.d0");
    asm("movl    .d0,vdc_stat");
    asm("movl    #1,vdc_ack");
    asm(".vdcend:");
    asm("moveml .sp@+,#0x0003");
    /* C version */
 /*   long rcv;
	rcv = *(long *) VDC_IO_PORT;
	if (rcv == 'v') { vdc_ack = 1; }
    if (rcv == 's') { vdc_sync = 1; }
    if ((rcv & 0177) == 't') { vdc_stat = rcv >> 14 & 0777; vdc_ack = 1; }*/
}

static
msdel(t)
unsigned t;
{
    register long s;

    s = clockms();
    while ((clockms() - s) < t) {
        asm("nop");
    }
}

int
init88(mode)
{
    extern eirpts(), vdcw();
    extern puts(), swap();
    register countdown = 0;
    struct vidmode *vm;
    register unsigned i;
   
	setISRs();
	IRQ_L1->iaddr = (long) _ISR_kbd_inp;
	IRQ_L2->iaddr = (long) _ISR_vdc_inp;
    eirpts(); /* enable interrupts */
    
    /* verify the VDC is available */
    countdown = 1024;
    vdc_stat  = 0;
    vdc_ack   = 0;
    VDC_IO_PORT->iaddr = (02040 << 18) | 0100; /* CAL */
    msdel(100);
    while (!vdc_ack && countdown) { countdown--; }
    if (!vdc_ack) {
        puts("\nVDC unresponsive\n");
        goto error;
    }
    if ((vdc_stat & 0206) != 0206) {
        puts("\nVDC cal error\n");
        goto error;
    }

    vdcw(RPHI9, 0);
    vdcw(RPMD9, 0);
    vdcw(RPLO9, 0);
    vdcw(RUSET, UPGAD);
    msdel(100);
    vdcw(RVMOD, mode);
    vdcw(RUSET, UVIDM);
    msdel(100);
    setpal(egapal, 16);
    vdc_sync = 0;
    while (!vdc_sync);
    /* test read/writing to/from VRAM */
    for (i = VIDEO_BEG; i < VIDEO_END; i =+ 4) {
        i->iaddr = (egapal[(i>>4)&0xf]<<14);
        if (i->iaddr != (egapal[(i>>4)&0xf]<<14)) {
            puts("\nvideo memory error\n");
            goto error;
        }
    }
    vdc_sync = 0;
    while (!vdc_sync);
    msdel(400);

    vm = NULL;
    for (i = 0; i < NUMMODES; i++) {
        if (vmodes[i].m == mode) {
            vm = &vmodes[i];
            break;
        }
    }
    
    if (vm == NULL) {
        puts("\nno suitable video mode found\n");
        goto error;
    }
    
    vdch = vm->w;
    vdcv = vm->h;
    tc = (mode == VDCM256TC) || (mode == VDCM320TC);

    swap();
    
    return(1);
error:
	return(0);
}

kill88() {}

swap()
{
    extern vdcw();
    register int page;
    unsigned vsz;
    
    vp = 1 - vp;
    vsz = (vdch * vdcv);
    if (tc) {
        vsz =* 4;
    }
    page = vp * vsz;
    dblbuf = VIDEO_BEG + (1 - vp) * vsz;
    vdcw(RPHI9, page >> 18 & 0777);
    vdcw(RPMD9, page >> 9 & 0777);
    vdcw(RPLO9, page >> 0 & 0777);
    vdcw(RUSET, UPGAD);
}

sync()
{
    /* wait for next retrace */
    vdc_sync = 0;
    while (!vdc_sync);
}

long
clockms()
{
    return(0x1c0->iaddr);
}

int
getch()
{
	char r = -1;

	if (kpbeg != kpend) {
		r = kbuf[kpend];
		kpend++;
	}
	return(r);
}

clrkb()
{
	kpbeg = kpend = 0;
}

int
kbhit()
{
	return(kpbeg != kpend);
}
