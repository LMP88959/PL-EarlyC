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

/*  imode.c
 * 
 * Simple immediate mode geometry interface.
 * 
 */

static struct PL_OBJ p; /* product */
static struct PL_OBJ wc;

static vtx[POMAXV];             /* temp storage for vertices */
static struct PL_POLY ply[POMAXV / 4]; /* temp storage for polygons */

static ptype PTRIS;
static nv 0;     /* entered so far */
static np 0;     /* entered so far */
static struct PL_TEX *curtex NULL;
static cverts[4];
static vnum 0;
static ctexc[2 * 4];
static tnum 0;
static cr 077;
static cg 077;
static cb 077;
static cu 0;
static cv 0;

static int
addvtx(x, y, z)
{
	register i, v;

	for (i = 0; i < nv; i++) {
	    v = i * PVLEN;
        if ((vtx[v    ] == x) &&
            (vtx[v + 1] == y) &&
            (vtx[v + 2] == z)) {
            return(i);
        }
	}
	v = nv * PVLEN;
    vtx[v    ] = x;
    vtx[v + 1] = y;
    vtx[v + 2] = z;
	return(nv++);
}

static
addply()
{
    extern bufset();
	struct PL_POLY *t;
    register i, b, edges;

	t = &ply[np];
	bufset(t, 0, sizeof(struct PL_POLY));
	t->tex = curtex;
	edges = 3;
	
	if (ptype == PQUADS) {
	    edges = 4;
        /* check for a quad with two identical vertices,
         * if it has any, turn it into a triangle
         */
        if (cverts[0] == cverts[1]) {
            edges            = 3;
            cverts[1]        = cverts[2];
            ctexc[2 * 1    ] = ctexc[2 * 2];
            ctexc[2 * 1 + 1] = ctexc[2 * 2 + 1];

            cverts[2]        = cverts[3];
            ctexc[2 * 2    ] = ctexc[2 * 3];
            ctexc[2 * 2 + 1] = ctexc[2 * 3 + 1];
        }
        if (cverts[2] == cverts[3]) {
            edges = 3;
        }
    }
	
    t->color = c18to24(cr << 12 | cg << 6 | cb << 0);
    
	for (i = 0; i < edges; i++) {
		b = i * PPOLY_VLEN;
		t->v[b    ] = cverts[i];
		t->v[b + 1] = ctexc[2 * i];
		t->v[b + 2] = ctexc[2 * i + 1];
	}
	b = i * PPOLY_VLEN;
	t->v[b    ] = cverts[0];
	t->v[b + 1] = ctexc[0];
	t->v[b + 2] = ctexc[1];
	t->nv = edges;
	
	np++;
}

imbeg()
{
    extern umemfr();
    register i;
    
    if (p.c) {
        umemfr(p.c);
    }
    p.c  = NULL;
    p.nc = 0;

    if (p.p) {
        for (i = 0; i < p.np; i++) {
            p.p[i].color = 0;
            p.p[i].nv    = 0;
            p.p[i].tex   = NULL;
        }
        umemfr(p.p);
    }
    p.p  = NULL;
    p.np = 0;

	nv = 0;
	np = 0;
	vnum = 0;
	tnum = 0;
}

imtype(type)
{
	/* reset when primitive type is changed */
	if (type != ptype) {
		vnum = 0;
		tnum = 0;
	}
	ptype = type;
}

imtex(tex)
struct PL_TEX *tex;
{
	curtex = tex;
}

imcolr(r, g, b)
{
	cr = r;
	cg = g;
	cb = b;
}

imtexc(u, v)
{
	cu = u;
	cv = v;
}

imvtx(x, y, z)
{
	cverts[vnum++] = addvtx(x, y, z);
	ctexc[tnum++]  = cu;
	ctexc[tnum++]  = cv;
	switch (ptype) {
		case PTRIS:
			if (vnum == 3) {
				addply();
				vnum = 0;
				tnum = 0;
			}
			break;
		case PQUADS:
			if (vnum == 4) {
			    addply();
				vnum = 0;
				tnum = 0;
			}
			break;
		default:
			vnum = 0;
			tnum = 0;
			break;
	}
}

imend()
{
    extern char *umemgt();
    extern bufcpy(), uerror();
    register i;

	if ((nv == 0) || (np == 0)) {
	    return;
	}
    
    if (p.c) {
        uerror(PERR_MISC, "imode", "end without beg v");
        return;
    }
    p.c = umemgt(nv * PVLEN, sizeof(int));
    if (p.c == NULL) { goto nomem; }
    for (i = 0; i < (nv * PVLEN); i++) {
        p.c[i] = vtx[i];
    }
    p.nc = nv;
    if (p.p) {
        uerror(PERR_MISC, "imode", "end without beg p");
        return;
    }
    p.p = umemgt(np + 1, sizeof(struct PL_POLY));
    if (p.p == NULL) { goto nomem; }
    for (i = 0; i < np; i++) {
        bufcpy(&p.p[i], &ply[i], sizeof(struct PL_POLY));
    }
    p.np = np;
    return;
nomem:
    uerror(PERR_NO_MEM, "imode", "no memory");  
}

iminit()
{
    extern ocpy();
    
	if (nv && np) {
	    ocpy(&wc, &p);
	}
}

imdraw()
{
    extern odraw();
    
	if (nv && np) {
	    odraw(&wc);
	}
}

imexport(dst)
struct PL_OBJ *dst;
{
    extern char *umemgt();
    extern bufcpy(), uerror();
    register i;
	
	dst->c = umemgt(p.nc * PVLEN, sizeof(int));
	if (dst->c == NULL) { goto nomem; }
	for (i = 0; i < (p.nc * PVLEN); i++) {
		dst->c[i] = p.c[i];
	}
	dst->nc = p.nc;
	dst->p = umemgt(p.np, sizeof(struct PL_POLY));
	if (dst->p == NULL) { goto nomem; }
	for (i = 0; i < np; i++) {
		bufcpy(&dst->p[i], &p.p[i], sizeof(struct PL_POLY));
	}
	dst->np = p.np;
	return;
nomem:
    uerror(PERR_NO_MEM, "imode", "no memory");	
}
