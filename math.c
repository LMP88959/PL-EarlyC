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

/*  math.c
 * 
 * Integer-only math using fixed point numbers.
 * Implements a basic matrix stack for transformations, among other things.
 * 
 */

int PL_sin[PTRIGMAX] = {
	0x0000,0x0324,0x0647,0x096a,0x0c8b,0x0fab,0x12c8,0x15e2,
	0x18f8,0x1c0b,0x1f19,0x2223,0x2528,0x2826,0x2b1f,0x2e11,
	0x30fb,0x33de,0x36ba,0x398c,0x3c56,0x3f17,0x41ce,0x447a,
	0x471c,0x49b4,0x4c3f,0x4ebf,0x5133,0x539b,0x55f5,0x5842,
	0x5a82,0x5cb4,0x5ed7,0x60ec,0x62f2,0x64e8,0x66cf,0x68a6,
	0x6a6d,0x6c24,0x6dca,0x6f5f,0x70e2,0x7255,0x73b5,0x7504,
	0x7641,0x776c,0x7884,0x798a,0x7a7d,0x7b5d,0x7c29,0x7ce3,
	0x7d8a,0x7e1d,0x7e9d,0x7f09,0x7f62,0x7fa7,0x7fd8,0x7ff6,
	0x8000,0x7ff6,0x7fd8,0x7fa7,0x7f62,0x7f09,0x7e9d,0x7e1d,
	0x7d8a,0x7ce3,0x7c29,0x7b5d,0x7a7d,0x798a,0x7884,0x776c,
	0x7641,0x7504,0x73b5,0x7255,0x70e2,0x6f5f,0x6dca,0x6c24,
	0x6a6d,0x68a6,0x66cf,0x64e8,0x62f2,0x60ec,0x5ed7,0x5cb4,
	0x5a82,0x5842,0x55f5,0x539b,0x5133,0x4ebf,0x4c3f,0x49b4,
	0x471c,0x447a,0x41ce,0x3f17,0x3c56,0x398c,0x36ba,0x33de,
	0x30fb,0x2e11,0x2b1f,0x2826,0x2528,0x2223,0x1f19,0x1c0b,
	0x18f8,0x15e2,0x12c8,0x0fab,0x0c8b,0x096a,0x0647,0x0324
};

int PL_cos[PTRIGMAX];

static struct XF {
    short tx, ty, tz;
	unsigned int rx, ry;
} xf_vw; /* view transform */

static mty idt[16];
static mty mdl[16];

/* maximum matrix stack depth */
#define MSTDEPTH   4
static mty mstack[MSTDEPTH * 16];
static mtop 0;

pminit()
{
    register i;
    
    /* adjust table precision to match the define */
    for (i = 0; i < PTRIGMAX; i++) {
        PL_sin[i] =>> (15 - PP);
    }
    
    /* sine is mirrored over X after PI */
    for (i = 0; i < (PTRIGMAX >> 1); i++) {
        PL_sin[(PTRIGMAX >> 1) + i] = -PL_sin[i];
    }
    /* construct cosine table by copying sine table */
    for (i = 0; i < ((PTRIGMAX >> 1) + (PTRIGMAX >> 2)); i++) {
        PL_cos[i] = PL_sin[i + (PTRIGMAX >> 2)];
    }
    for (i = 0; i < (PTRIGMAX >> 2); i++) {
        PL_cos[i + ((PTRIGMAX >> 1) + (PTRIGMAX >> 2))] = PL_sin[i];
    }
    
    bufset(idt, 0, sizeof(mty) * 16);
    idt[0] = PONE;
    idt[5] = PONE;
    idt[10] = PONE;
    idt[15] = PONE;
    
    bufcpy(mdl, idt, sizeof(mty) * 16);
}

defcam(x, y, z, rx, ry)
{
	xf_vw.tx = -x;
	xf_vw.ty = -y;
	xf_vw.tz = -z;
	
    xf_vw.rx = (unsigned int) (PTRIGMAX - rx & PTRIGMSK);
    xf_vw.ry = (unsigned int) (PTRIGMAX - ry & PTRIGMSK);
}

mstget(out)
mty *out;
{
	mcpy(out, mdl);
}

mstpush()
{
	if ((mtop + 1) >= MSTDEPTH) {
	    uerror(PERR_MISC, "math", "stack overflow");
	}
	mcpy(&mstack[(mtop + 1) * 16], mdl);
	mtop++;
}

mstpop()
{
	if ((mtop - 1) < 0) {
	    uerror(PERR_MISC, "math", "stack underflow");
	}
	mcpy(mdl, &mstack[(mtop--) * 16]);
}

mstidt()
{	
    mcpy(mdl, idt);
}

mstld(m)
mty *m;
{
    mcpy(mdl, m);
}

mstmul(m)
mty *m;
{
    mmul(mdl, m);
}

mscale(x, y, z)
{
	mty mat[16];
	
	bufcpy(mat, idt, sizeof(mat));
	
	mat[0]  = x;
	mat[5]  = y;
	mat[10] = z;
	mstmul(mat);
}

mtrans(x, y, z)
{
    mty mat[16];
    
    bufcpy(mat, idt, sizeof(mat));
    
	mat[12] = x;
	mat[13] = y;
	mat[14] = z;
	mstmul(mat);
}

mrotx(rx)
{
    register cx, sx;
    mty mat[16];
      
    bufcpy(mat, idt, sizeof(mat));
    
    cx = PL_cos[rx & PTRIGMSK];
    sx = PL_sin[rx & PTRIGMSK];

    mat[(1 << 2) + 1] = cx;
    mat[(2 << 2) + 1] = -sx;
    mat[(1 << 2) + 2] = sx;
    mat[(2 << 2) + 2] = cx;
    
    mstmul(mat);
}

mroty(ry)
{
    register cy, sy;
    mty mat[16];
      
    bufcpy(mat, idt, sizeof(mat));

    cy = PL_cos[ry & PTRIGMSK];
    sy = PL_sin[ry & PTRIGMSK];
    
    mat[(0 << 2) + 0] = cy;
    mat[(2 << 2) + 0] = sy;
    mat[(0 << 2) + 2] = -sy;
    mat[(2 << 2) + 2] = cy;
    
    mstmul(mat);
}

mrotz(rz)
{
    register cz, sz;
    mty mat[16];
    
    bufcpy(mat, idt, sizeof(mat));
      
    cz = PL_cos[rz & PTRIGMSK];
    sz = PL_sin[rz & PTRIGMSK];
    
    mat[(0 << 2) + 0] = cz;
    mat[(1 << 2) + 0] = sz;
    mat[(0 << 2) + 1] = -sz;
    mat[(1 << 2) + 1] = cz;
    
    mstmul(mat);
}

xfvecs(v, out, len)
int *v, *out;
{
    register short sx, sy, cx, cy;
    register short x, y, z, w;
    register short xx, yy, zz;
    short tx, ty, tz;
    mty *m;

    cx = PL_cos[xf_vw.rx];
    sx = PL_sin[xf_vw.rx];
    cy = PL_cos[xf_vw.ry];
    sy = PL_sin[xf_vw.ry];

    m = mdl;
    
	tx = xf_vw.tx + m[12];
	ty = xf_vw.ty + m[13];
	tz = xf_vw.tz + m[14];

    while ((len--) > 0) {
        x = v[0];
        y = v[1];
        z = v[2];

        xx = ((x * m[0] + y * m[4] + z * m[8])  >> PP) + tx;
        yy = ((x * m[1] + y * m[5] + z * m[9])  >> PP) + ty;
        zz = ((x * m[2] + y * m[6] + z * m[10]) >> PP) + tz;

        /* yaw */
        w  = (zz * sy + xx * cy) >> PP;
        zz = (zz * cy - xx * sy) >> PP;
        xx = w;

        /* pitch */
        w  = (yy * cx - zz * sx) >> PP;
        zz = (yy * sx + zz * cx) >> PP;
        yy = w;

        out[0] = xx;
        out[1] = yy;
        out[2] = zz;
        v   =+ PVLEN;
        out =+ PVLEN;
    }
}

mmul(a, b)
register mty *a, *b;
{
    register i;
    mty m[16];
	
	bufcpy(m, a, sizeof(m));
    
    for (i = 0; i < 16; i =+ 4) {
        a[i + 0] = ((b[i + 0] * m[0]) >> PP) + ((b[i + 1] * m[4]) >> PP)
                 + ((b[i + 2] * m[8]) >> PP) + ((b[i + 3] * m[12])>> PP);
        
        a[i + 1] = ((b[i + 0] * m[1]) >> PP) + ((b[i + 1] * m[5]) >> PP)
                 + ((b[i + 2] * m[9]) >> PP) + ((b[i + 3] * m[13])>> PP);
        
        a[i + 2] = ((b[i + 0] * m[2]) >> PP) + ((b[i + 1] * m[6]) >> PP)
                 + ((b[i + 2] * m[10])>> PP) + ((b[i + 3] * m[14])>> PP);
        
        a[i + 3] = ((b[i + 0] * m[3]) >> PP) + ((b[i + 1] * m[7]) >> PP)
                 + ((b[i + 2] * m[11])>> PP) + ((b[i + 3] * m[15])>> PP);
    }
}

mcpy(dst, src)
register mty *dst, *src;
{
	bufcpy(dst, src, sizeof(mty) * 16);
}

int
porder(a, b, c)
register int *a, *b, *c;
{
    int nc[3];
    
    nc[0] = (a[2] * b[1]) - (a[1] * b[2]);
    nc[1] = (a[0] * b[2]) - (a[2] * b[0]);
    nc[2] = (a[1] * b[0]) - (a[0] * b[1]);
    vshort(nc);
    return(((c[0] * nc[0]) + (c[1] * nc[1]) + (c[2] * nc[2])) < 0);
}

vshort(v)
register int *v;
{
    while (v[0] > 32767 || v[0] < -32768 ||
           v[1] > 32767 || v[1] < -32768 ||
           v[2] > 32767 || v[2] < -32768) {
        v[0] =>> 1;
        v[1] =>> 1;
        v[2] =>> 1;
    }
}

psproj(s, d, len, n, fov)
register int *s, *d;
register int len, n, fov;
{
    int ffac, nb, shift;
    
    ffac = (1 << (fov + 12));
    shift = fov - 2; /* to fit into short */
    
    len =- 3;
    nb = len * sizeof(int);
    while (n--) {
        fov = ffac / s[2];
        /* rounding is necessary */
        *d++ = ((s[0] * fov + (1 << 11)) >> 12) + vpcenx;
        *d++ = vpceny - ((s[1] * fov + (1 << 11)) >> 12);
        *d++ = fov >> shift;
        bufcpy(d, s =+ 3, nb);
        s =+ len;
        d =+ len;
    }
}
