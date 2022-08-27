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

/*  clip.c
 * 
 * Code for defining and clipping polygons to the viewport and near plane.
 * 
 */

#define NEARZ    16    /* where near z plane is */

/* special return code specifying the edge was not clipped */
#define NC       0777

#define HI_P     14     /* 2D clipping interpolation precision */
#define HH_P     (HI_P >> 1)

#define CLIP_P   8    /* 3D clipping interpolation precision */

vpminx, vpmaxx, vpminy, vpmaxy;
vpcenx, vpceny;

defvp(minx, miny, maxx, maxy, updcen)
{
	if (minx < 0) { minx = 0; }
	if (miny < 0) { miny = 0; }
	if (maxx >= hres) { maxx = hres - 1; }
	if (maxy >= vres) { maxy = vres - 1; }
	vpminx = minx;
	vpminy = miny;
	vpmaxx = maxx;
	vpmaxy = maxy;
	/* update center of projection */
	if (updcen) {
	    vpcenx = ((minx + maxx) >> 1) + 1;
	    vpceny = ((miny + maxy) >> 1) + 1;
	}
}

static
doclip(L, R, out, len, bound, comp, ocomp)
int *L, *R, *out;
{
    register i, f, fh, fhp;
    
    fhp = ((bound - L[comp]) << 15) / (R[comp] - L[comp]);
    fh = fhp >> (15 - HI_P);
    f = fh >> HH_P;
    /* skip x and y */
    for (i = 3; i < len; i++) {
        out[i] = L[i] + (f * ((R[i] - L[i]) >> HH_P));
    }
    /* z needs extra precision */
    out[2] = L[2] + (fhp * (R[2] - L[2]) >> 15);
    /* explicitly set comp and calculate ocomp with a higher precision */
    out[comp] = bound;
    out[ocomp] = L[ocomp] + (fh * (R[ocomp] - L[ocomp]) >> HI_P);
}

/* 2d line clip */
static int
lclip2(v0, v1, len, min, max, comp)
int **v0, **v1;
{
    static int m0[PVDIM];
    static int m1[PVDIM];
    
    int ooo = 1; /* out of order */
    int ret = 0;
    int *L, *R;
    int **Lp, **Rp;

    Lp = v1;
    Rp = v0;

    if ((*Rp)[comp] < (*Lp)[comp]) {
	    ooo = 0;
	    Lp  = v0;
		Rp  = v1;
	}

    L = *Lp;
    R = *Rp;
    
	if ((L[comp] >= max) || (R[comp] <= min)) {
		return(NC);
	}

    if (L[comp] <= min) {
        ret = !ooo;
        doclip(L, R, m0, len, min, comp, !comp);
        *Lp = m0;
    }

    if (R[comp] >= max) {
        ret |= ooo;
        doclip(L, R, m1, len, max, comp, !comp);
        *Rp = m1;
    }
	return(ret);
}

static int
lclip3(v0, v1, len)
int **v0, **v1;
{
    /* must be static, the memory is used after function execution */
    static int m[PVDIM];
    
    int i, f;
    int ooo = 1; /* out of order */
    int ret = 0;
    int *L, *R;
    int **Lp, **Rp;

    Lp = v1;
    Rp = v0;
    if ((*Rp)[2] < (*Lp)[2]) {
        ooo = 0;
        Lp  = v0;
        Rp  = v1;
    }
    
    L = *Lp;
    R = *Rp;
    
    if (R[2] < NEARZ) {
        return(NC);
    }
    if (L[2] < NEARZ) {
        ret  = !ooo;
        *Lp  = m;
        f    = ((NEARZ - L[2]) << CLIP_P) / (R[2] - L[2]);
        m[0] = L[0] + (f * (R[0] - L[0]) >> CLIP_P);
        m[1] = L[1] + (f * (R[1] - L[1]) >> CLIP_P);
        m[2] = NEARZ;
        for (i = 3; i < len; i++) {
            m[i] = L[i] + (f * (R[i] - L[i]) >> CLIP_P);
        }
    }
    return(ret);
}

/* polygon clip */
static int
pclip(dst, src, len, num, clip, minv, maxv)
register int *dst, *src, len, num;
int (*clip)();
{
    extern bufcpy();
    int nv, nb;
    int *out;
    register int r;
    int *v[2];
    
    nv  = 0;
    out = dst;
    nb  = len * sizeof(int);
    
    while (num--) {
        v[1] = src;
        v[0] = src =+ len;
        r = (*clip)(&v[1], &v[0], len, minv, maxv); /* need to deref func ptr */
        if (r != NC) {
            do {
                bufcpy(out, v[r], nb);
                out =+ len;
                nv++;
            } while (r--);
        }
    }
    bufcpy(out, dst, nb);
    return(nv);
}

static int
lclipx(v0, v1, len, min, max)
int **v0, **v1;
{
    return(lclip2(v0, v1, len, min, max, 0));
}

static int
lclipy(v0, v1, len, min, max)
int **v0, **v1;
{
    return(lclip2(v0, v1, len, min, max, 1));
}

static int
lclipz(v0, v1, len, min, max)
int **v0, **v1;
{
    return(lclip3(v0, v1, len));
}

extern int
clinex(v0, v1, len, min, max)
int **v0, **v1;
{
    return(lclip2(v0, v1, len, min, max, 0) != NC);
}

extern int
cliney(v0, v1, len, min, max)
int **v0, **v1;
{
    return(lclip2(v0, v1, len, min, max, 1) != NC);
}

extern int
cpolyx(dst, src, len, num)
int *dst, *src;
{
    return(pclip(dst, src, len, num, lclipx, vpminx, vpmaxx));
}

extern int
cpolyy(dst, src, len, num)
int *dst, *src;
{
    return(pclip(dst, src, len, num, lclipy, vpminy, vpmaxy));
}

extern int
cpolyz(dst, src, len, num)
int *dst, *src;
{
    return(pclip(dst, src, len, num, lclipz, 0, 0));
}

extern int
ctestz(minz, maxz)
{    
    if (maxz <= NEARZ) { return(PCCB); } /* behind */
    if (minz <  NEARZ) { return(PCCX); } /* crossing */
    return(PCCI); /* in front */
}
