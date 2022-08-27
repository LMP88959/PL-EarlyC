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

/*  pl.c
 * 
 * Handles objects, glues the different modules together.
 * 
 */

vfov  9;
rastm PRFLAT;
cullm PCBACK;

/* temp vertices */
static tv[POMAXV];

static
loadvs(d, s, dim, len, minz, maxz)
register int *d, *s;
int *minz, *maxz;
{
    int z, mn, mx, i;
    
    mn =  0x7ffffff;
    mx = -0x7ffffff;
    
    while (len--) {
        /* index into object vertex array */
        i = s[0] * PVLEN;
        d[0] = tv[i + 0];
        d[1] = tv[i + 1];
        d[2] = tv[i + 2];
        z = d[2];
        if (z > mx) { mx = z; }
        if (z < mn) { mn = z; }

        if (dim == PSTEX) {
            d[3] = s[1] << PTLOG;
            d[4] = s[2] << PTLOG;
        } 
        s =+ 3;
        d =+ dim;
    }
    *minz = mn;
    *maxz = mx;
}

static
rpoly(poly)
struct PL_POLY *poly;
{
    int copy[PPMAXV * PVDIM];
    int clip[PPMAXV * PVDIM];
    int proj[PPMAXV * PVDIM];
    int minz, maxz; /* z extents for frustum testing */
    register int res; /* result of frustum test */
    register int stype = PSFLAT; /* stream type */
    register int nedge;
    int *in;
    
    nedge = poly->nv;
    in = copy;
    
    if ((rastm == PRTEX) && (poly->tex && poly->tex->texdat)) {
        stype = PSTEX;
    }
   
    loadvs(copy, poly->v, stype, nedge + 1, &minz, &maxz);
    res = ctestz(minz, maxz);
    if (res == PCCB) { return; }

    /* test winding order in view space rather than screen space */    
    if ((porder(copy, copy + stype, copy + stype * 2) + 1) & cullm) {
        return;
    }
    
    if (res == PCCX) {
        in = clip;
        nedge = cpolyz(in, copy, stype, nedge);
    }
    
    psproj(in, proj, stype, nedge + 1, vfov);
    
    if (stype == PSTEX) {
        ptpoly(proj, nedge, poly->tex->texdat);
    } else {
        pfpoly(proj, nedge, poly->color);
    }
}

odraw(o)
struct PL_OBJ *o;
{
    extern uerror(), xfvecs();
    register i;

    if (!o) return;

    if (o->nc >= POMAXV) {
        uerror(PERR_MISC, "objmgr", "too many object vertices!");
        return;
    }

    xfvecs(o->c, tv, o->nc);
    for (i = 0; i < o->np; i++) {
        rpoly(&o->p[i]);
    }
}

odel(o)
struct PL_OBJ *o;
{
    extern umemfr();
    register i;
    
    if (!o) return;

    if (o->c) {
        umemfr(o->c);
    }
    o->c  = NULL;
    o->nc = 0;
    
    if (o->p) {
        for (i = 0; i < o->np; i++) {
            o->p[i].color = 0;
            o->p[i].nv    = 0;
            o->p[i].tex   = NULL;
        }
        umemfr(o->p);
    }
    o->p  = NULL;
    o->np = 0;
}

ocpy(d, s)
struct PL_OBJ *d, *s;
{
    extern char *umemgt();
    extern uerror(), bufcpy();
    register i, size;
    
    if (!s) {
        uerror(PERR_MISC, "objmgr", "objcpy null src");
        return;
    }
    if (!d) {
        uerror(PERR_MISC, "objmgr", "objcpy null dst");
        return;
    }
    odel(d);
    if (s->nc > 0) {
        size = s->nc * PVLEN * sizeof(int);
        d->c = umemgt(1, size);
        if (d->c == NULL) {
            uerror(PERR_NO_MEM, "objmgr", "no memory");
            return;
        }
        for (i = 0; i < s->nc * PVLEN; i++) {
            d->c[i] = s->c[i];
        }
        d->nc = s->nc;
    } else {
        d->c  = NULL;
        d->nc = 0;
    }
    if (s->np > 0) {
        size = s->np * sizeof(struct PL_POLY);
        d->p = umemgt(1, size);
        if (d->p == NULL) {
            uerror(PERR_NO_MEM, "objmgr", "no memory");
            return;
        }
        bufcpy(d->p, s->p, size);
        d->np = s->np;
    } else {
        d->p  = NULL;
        d->np = 0;
    }
}

int
c18to24(rgb)
{
    auto r, g, b, lo3;
    
    r = rgb >> 12 & 077;
    g = rgb >>  6 & 077;
    b = rgb >>  0 & 077;
    lo3 = b & 07;
    return(((b << 26) | (g << 18) | (r << 10)) | lo3);
}

static
boxlist(x, y, z, w, h, d, flags)
{
    int v0[3], v1[3], v2[3], v3[3], v4[3], v5[3], v6[3], v7[3];
    int tx0[2], tx1[2], tx2[2], tx3[2];
    int tsz;

    h =>> 1;
    w =>> 1;
    d =>> 1;

    v0[0] = x - h; v0[1] = y - w; v0[2] = z + d;
    v1[0] = x + h; v1[1] = y - w; v1[2] = z + d;
    v2[0] = x + h; v2[1] = y + w; v2[2] = z + d;
    v3[0] = x - h; v3[1] = y + w; v3[2] = z + d;
    v4[0] = x - h; v4[1] = y - w; v4[2] = z - d;
    v5[0] = x - h; v5[1] = y + w; v5[2] = z - d;
    v6[0] = x + h; v6[1] = y + w; v6[2] = z - d;
    v7[0] = x + h; v7[1] = y - w; v7[2] = z - d;
    
    tsz = PTDIM - 1;
    tx0[0] = 0;
    tx0[1] = 0;
    tx1[0] = tsz;
    tx1[1] = 0;
    tx2[0] = tsz;
    tx2[1] = tsz;
    tx3[0] = 0;
    tx3[1] = tsz;

    if (flags & PBACK) {
        imtexc(tx0[0], tx0[1]);
        imvtx(v0[0], v0[1], v0[2]);
        imtexc(tx1[0], tx1[1]);
        imvtx(v1[0], v1[1], v1[2]);
        imtexc(tx2[0], tx2[1]);
        imvtx(v2[0], v2[1], v2[2]);
        imtexc(tx3[0], tx3[1]);
        imvtx(v3[0], v3[1], v3[2]);
    }
    if (flags & PFRONT) {
        imtexc(tx0[0], tx0[1]);
        imvtx(v4[0], v4[1], v4[2]);
        imtexc(tx1[0], tx1[1]);
        imvtx(v5[0], v5[1], v5[2]);
        imtexc(tx2[0], tx2[1]);
        imvtx(v6[0], v6[1], v6[2]);
        imtexc(tx3[0], tx3[1]);
        imvtx(v7[0], v7[1], v7[2]);
    }
    if (flags & PTOP) {
        imtexc(tx0[0], tx0[1]);
        imvtx(v5[0], v5[1], v5[2]);
        imtexc(tx1[0], tx1[1]);
        imvtx(v3[0], v3[1], v3[2]);
        imtexc(tx2[0], tx2[1]);
        imvtx(v2[0], v2[1], v2[2]);
        imtexc(tx3[0], tx3[1]);
        imvtx(v6[0], v6[1], v6[2]);
    }
    if (flags & PBOTTOM) {
        imtexc(tx0[0], tx0[1]);
        imvtx(v4[0], v4[1], v4[2]);
        imtexc(tx1[0], tx1[1]);
        imvtx(v7[0], v7[1], v7[2]);
        imtexc(tx2[0], tx2[1]);
        imvtx(v1[0], v1[1], v1[2]);
        imtexc(tx3[0], tx3[1]);
        imvtx(v0[0], v0[1], v0[2]);
    }
    if (flags & PRIGHT) {
        imtexc(tx0[0], tx0[1]);
        imvtx(v7[0], v7[1], v7[2]);
        imtexc(tx1[0], tx1[1]);
        imvtx(v6[0], v6[1], v6[2]);
        imtexc(tx2[0], tx2[1]);
        imvtx(v2[0], v2[1], v2[2]);
        imtexc(tx3[0], tx3[1]);
        imvtx(v1[0], v1[1], v1[2]);
    }
    if (flags & PLEFT) {
        imtexc(tx0[0], tx0[1]);
        imvtx(v4[0], v4[1], v4[2]);
        imtexc(tx1[0], tx1[1]);
        imvtx(v0[0], v0[1], v0[2]);
        imtexc(tx2[0], tx2[1]);
        imvtx(v3[0], v3[1], v3[2]);
        imtexc(tx3[0], tx3[1]);
        imvtx(v5[0], v5[1], v5[2]);
    }
}

extern struct PL_OBJ *
genbox(w, h, d, flags, r, g, b)
{
    extern char *umemgt();
    extern uerror();
    struct PL_OBJ *t;
    
    if (!(flags & PALL)) {
        return(NULL);
    }

    t = umemgt(1, sizeof(struct PL_OBJ));
    if (t == NULL) {
        uerror(PERR_NO_MEM, "objmgr", "no memory");
    }

    imbeg();
    imtype(PQUADS);
    imcolr(r, g, b);
    boxlist(0, 0, 0, w, h, d, flags);
    imend();
    imexport(t);
    return(t);
}
