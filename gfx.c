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

/*  gfx.c
 * 
 * Scans and rasterizes polygons.
 * 
 */

hres 0, vres 0;
*pvideo NULL;
dty *pdepth NULL;

#define ZP     8      /* z precision */
#define TXMSK  ((1 << (PTLOG + PTLOG)) - 1)
#define ATTR   8
#define ATTRB  3
#define SP     8               /* scan conversion precision */
#define SPRND  (1 << (SP - 1))  /* rounding */

static scan_miny, scan_maxy;

/* integer reserve for data locality */
static short g3dresv[PMAXDIM /* x_L */
                 + PMAXDIM /* x_R */
                 + PMAXDIM /* xLc */
                 + PMAXDIM /* xRc */
                 + (ATTR * PMAXDIM) /* abuf */
                 ];

static int *x_L, *x_R;
static short *abuf; /* attribute buffer */

/* cleared versions of scan conversion L/R buffers */
static int *xLc, *xRc;

static short iv15[PMAXDIM];

pinit(video, h, v)
int *video;
{
    extern char *umemgt();
    extern umemfr(), pminit(), uerror();
    register int i;
    
	hres   = h;
	vres   = v;
	defvp(0, 0, h - 1, v - 1, 1);
	
	if (pdepth) {
	    umemfr(pdepth);
	    pdepth = NULL;
	}

	pdepth = umemgt(h * v, sizeof(dty));
	if (pdepth == NULL) {
	    uerror(PERR_NO_MEM, "gfx", "no memory");
	}
	
	pvideo = video;
	
	/* set buffer offsets */
	x_L  = g3dresv + v;
    x_R  = x_L + v;
    xLc  = x_R + v;
    xRc  = xLc + v;
    abuf = xRc + v;

	for (i = 0; i < v; i++) {
        xLc[i] =  077777777;
        xRc[i] = -077777777;
    }
	
	iv15[0] = 1;
	for (i = 1; i < PMAXDIM; i++) {
	    iv15[i] = (1 << 15) / i;
	}
	
	pminit();
}

static
qbzero(s0, s1, n)
register int *s0;
register dty *s1;
register unsigned n;
{   
    while (n--) {
        *s0++ = 0;
        *s1++ = 0;
    }
}

pc()
{    
    qbzero(pvideo, pdepth, hres * vres);
}

/* scan convert polygon */
static int
pscan(stream, dim, len)
int *stream;
{
    extern bufcpy();
    extern int cpolyx();
    extern int cliney();
    int resv[PVDIM + PVDIM + (PPMAXV * PSTEX)];
    short rdim;
    int *vA, *vB;
    register int x, y, dx, dy;
    register int sx, sy, i;
    short mjr, ady, imjr;
    int *AT, *DT, *VS;
    short *AS; /* attribute buffer ptr */
    short *ABL, *ABR;
    
    AT = resv + (0 * PVDIM); /* vertex attributes */
    DT = resv + (1 * PVDIM); /* delta vertex attributes */
    VS = resv + (2 * PVDIM); /* vertex stream (x-clipped) */
    ABL = abuf + 0; /*  left side is +0 */
    ABR = abuf + 1; /* right side is +1 */
    rdim = dim - 2;
    scan_miny =  077777777;
    scan_maxy = -077777777;
    /* clean scan tables */
    bufcpy(x_L, xLc, 2 * vres * sizeof(int));
  
    len = cpolyx(VS, stream, dim, len);
    while (len--) {
        vA = VS;
        vB = VS =+ dim;
        if (!cliney(&vA, &vB, dim, vpminy, vpmaxy)) {
            continue;
        }
        x  = *vA++;
        y  = *vA++;
        dx = *vB++;
        dy = *vB++;
        if (y  < scan_miny) { scan_miny = y; }
        if (y  > scan_maxy) { scan_maxy = y; }
        if (dy < scan_miny) { scan_miny = dy;}
        if (dy > scan_maxy) { scan_maxy = dy;}
        dx =- x;
        dy =- y;
        mjr = dx;
        ady = dy;
        if (dx < 0) { mjr = -dx; }
        if (dy < 0) { ady = -dy; }
        if (ady > mjr) {
            mjr = ady;
        }
        if (mjr <= 0) {
            continue;
        }
        imjr = iv15[mjr];
        /* Z precision gets added here */
        AT[0] = vA[0] << ZP;
        DT[0] = ((vB[0] - vA[0]) << ZP) * imjr >> 15;
        /* the rest get computed with whatever precision they had */
        for (i = 1; i < rdim; i++) {
            AT[i] = vA[i];
            DT[i] = (vB[i] - vA[i]) * imjr >> 15;
        }
        /* make sure to round! */
        x  = (x  << SP) + SPRND;
        y  = (y  << SP) + SPRND;
        dx = (dx << SP) * imjr >> 15;
        dy = (dy << SP) * imjr >> 15;
        do {
            sx = x >> SP;
            sy = y >> SP;
            if (x_L[sy] > sx) {
                x_L[sy] = sx;
                AS = ABL + (sy << ATTRB);
                for (i = 0; i < rdim; i++) {
                    AS[i << 1] = AT[i];
                }
            }
            if (x_R[sy] < sx) {
                x_R[sy] = sx;
                AS = ABR + (sy << ATTRB);
                for (i = 0; i < rdim; i++) {
                    AS[i << 1] = AT[i];
                }
            }
            x =+ dx;
            y =+ dy;
            for (i = 0; i < rdim; i++) {
                AT[i] =+ DT[i];
            }
        } while (mjr--);
    }
    return(scan_miny >= scan_maxy);
}

pfpoly(stream, len, rgb)
int *stream;
{
    int miny, maxy;
    int pos, beg, pbg;
    register int *vbuf;
    register dty *zbuf;
    register short dz, sz;
    short yt;
    
    if (pscan(stream, PSFLAT, len)) { return; }
    miny = scan_miny;
    maxy = scan_maxy;
    pos  = miny * hres;
    while (miny <= maxy) {
        beg  = x_L[miny];
        pbg  = pos + beg;
        vbuf = pvideo + pbg;
        zbuf = pdepth + pbg;
        len  = x_R[miny] - beg;
        yt   = (miny << ATTRB);
        sz   = abuf[yt];
        dz   = (short)(abuf[yt + 1] - sz) * iv15[len] >> 15;

        do {
            if (*zbuf < sz) {
                *zbuf = sz;
                *vbuf = rgb;
            }
            sz =+ dz;
            vbuf++;
            zbuf++;
        } while (len--);
        /* next scanline */
        miny++;
        pos =+ hres;
    }
}

ptpoly(stream, len, texels)
int *stream;
register int len, *texels;
{
    int miny, maxy;
    int pos, beg, pbg;
    register int *vbuf;
    register dty *zbuf;
    short yt;
    short du, dv, dz;
    register short su, sv, sz;
    register short dlen;
    
    if (pscan(stream, PSTEX, len)) { return; }
    miny = scan_miny;
    maxy = scan_maxy;
    pos  = miny * hres;
    while (miny <= maxy) {
        beg  = x_L[miny];
        pbg  = pos + beg;
        vbuf = pvideo + pbg;
        zbuf = pdepth + pbg;
        len  = x_R[miny] - beg;
        dlen = iv15[len];
        yt   = (miny << ATTRB);
        /* use shorts so we can do a hardware mul */
        sz   = abuf[yt];
        dz   = (short)(abuf[yt + 1] - sz) * dlen >> 15;
        su   = abuf[yt + 2];
        du   = (short)(abuf[yt + 3] - su) * dlen >> 15;
        sv   = abuf[yt + 4];
        dv   = (short)(abuf[yt + 5] - sv) * dlen >> 15;
        while (len-- >= 0) {
            if (*zbuf < sz) {
                *zbuf = sz;
                su =& TXMSK;
                sv =& TXMSK;
                *vbuf = texels[(su >> PTLOG) | (sv & (~(PTDIM - 1)))];
            }
            su =+ du;
            sv =+ dv;
            sz =+ dz;
            vbuf++;
            zbuf++;
        }

        /* next scanline */
        miny++;
        pos =+ hres;
    }
}
