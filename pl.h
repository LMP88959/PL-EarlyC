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

/*  pl.h
 * 
 * Main header file for the PL library.
 * 
 */

/* maximum possible horizontal or vertical resolution */
#define PMAXDIM 640

/*****************************************************************************/
/********************************* CLIPPING **********************************/
/*****************************************************************************/

#define PCCI  00   /* in front of z plane */
#define PCCX  01   /* crossing z plane */
#define PCCB  02   /* completely behind z plane */

extern vpminx, vpmaxx, vpminy, vpmaxy;
extern vpcenx, vpceny;

/* define viewport
 * 
 * update_center - updates what the engine considers to be the perspective
 *                 focal point of the image
 * 
 */
extern defvp();

/* clip lines and polygons to 2D viewport */
extern int clinex(); /* clip line x */
extern int cliney(); /* clip line y */
extern int cpolyx(); /* clip poly x */
extern int cpolyy(); /* clip poly y */

/* test z bounds to determine its position relative to near plane */
extern int ctestz();
/* clip polygon to near plane */
extern int cpolyz();

/*****************************************************************************/
/********************************** ENGINE ***********************************/
/*****************************************************************************/

/* maximum number of vertices in object */
#define POMAXV  128

#define PRFLAT  1
#define PRTEX   0

#define PCNONE  0
#define PCFRNT  1
#define PCBACK  2

/* for storage size definition */
#define PVDIM       5  /* X Y Z U V */
#define PPOLY_VLEN  3  /* Idx U V */

extern vfov; /* min valid value = 8 */
extern rastm; /* PRFLAT or PRTEX */
extern cullm; /* cull mode */

struct PL_POLY {
    struct PL_TEX *tex;
    
    /* a user defined polygon may only have 3 or 4 vertices.  */
    
    /* [index, U, V] array of indices into obj verts array */
    int v[6 * PPOLY_VLEN]; 
    int color;
    int nv;
};

struct PL_OBJ {
    struct PL_POLY *p; /* list of polygons in the object */
    int *c;  /* coords: array of [x, y, z, 0] values */
    int  np; /* num polys */
    int  nc; /* num coords */
};

extern odraw(); /* draw object */
extern odel(); /* delete object */
extern ocpy(); /* copy object */

/*****************************************************************************/
/*********************************** IMODE ***********************************/
/*****************************************************************************/

#define PTRIS   0
#define PQUADS  1

extern imbeg(); /* begin primitive */
/* type is PTRIS or PQUADS */
extern imtype();

/* applies to the next polygon made. */
extern imtex(); /* texture */
/* last color defined before the poly is finished is used as the poly's color */
extern imcolr(); /* color */
extern imtexc(); /* texture coordinate */
extern imvtx(); /* vertex */

/* doesn't delete the previous object once called */
extern imend(); /* end primitive */

extern iminit(); /* initialize (only needed if not exporting) */
extern imdraw(); /* render (only needed if not exporting) */

/* save current model that has been defined in immediate mode */
extern imexport();

/*****************************************************************************/
/********************************* GRAPHICS **********************************/
/*****************************************************************************/

/* textures must be square with a dimension of PTDIM */
#define PTLOG   7
#define PTDIM   (1 << PTLOG)

#define PPMAXV  8  /* max verts in a polygon (post-clip) */

#define PSFLAT  3  /* X Y Z */
#define PSTEX   5  /* X Y Z U V */

extern hres;       /* horizontal resolution */
extern vres;       /* vertical resolution */

typedef short dty; /* depth buffer type */
extern int *pvideo;
extern dty *pdepth;

/* only square textures with dimensions of PTDIM */
struct PL_TEX {
    int *texdat;
};

/* Call this to initialize PL
 * 
 * video - pointer to target image (4 byte-per-pixel true color X8R8G8B8)
 * hres - horizontal resolution of image
 * vres - vertical resolution of image
 * 
 */
extern pinit();

/* 18-bit RGB to video compatible 24-bit RGB */
extern int c18to24();

/* clear entire screen color and depth */
extern pc();

/* Solid color polygon fill.
 * Expecting input stream of 3 values [X,Y,Z] */
extern pfpoly();

/* Affine (linear) texture mapped polygon fill.
 * Expecting input stream of 5 values [X,Y,Z,U,V] */
extern ptpoly();

/*****************************************************************************/
/*********************************** MATH ************************************/
/*****************************************************************************/

/* number of elements in PL_sin and PL_cos */
#define PTRIGMAX       256
#define PTRIGMSK       (PTRIGMAX - 1)

/* number of elements in a vector */
#define PVLEN          4

/* precision for fixed point math */
#define PP             7
#define PONE           (1 << PP)

extern int PL_sin[PTRIGMAX];
extern int PL_cos[PTRIGMAX];

typedef short mty; /* matrix type */

/* vectors are assumed to be integer arrays of length PL_VLEN */
/* matrices are assumed to be integer arrays of length 16 */

extern int porder();
extern vshort(); /* shorten vector to fit in 15 bits */
extern psproj(); /* perspective project */

/* matrix stack (mst) */
extern mstget(); /* get current top of mst */
extern mstpush(); /* push matrix onto mst */
extern mstpop(); /* pop matrix from mst */
extern mstidt(); /* load identity matrix */
extern mstld (); /* load specified matrix to mst */
extern mstmul(); /* multiply given matrix to mst */
extern mscale(); /* scale by x, y, z */
extern mtrans(); /* translate by x, y, z */
extern mrotx(); /* rotate x */
extern mroty(); /* y */
extern mrotz(); /* z */
extern defcam(); /* define camera */

/* transform a stream of vertices by the current model+view */
extern xfvecs();

/* result is stored in 'a' */
extern mmul(); /* matrix mul */
extern mcpy(); /* copy matrix */

/*****************************************************************************/
/************************************ GEN ************************************/
/*****************************************************************************/

/* flags to specify the faces of the box to generate */
#define PTOP       001
#define PBOTTOM    002
#define PBACK      004
#define PFRONT     010
#define PLEFT      020
#define PRIGHT     040
#define PALL       077

/* generate a box */
extern struct PL_OBJ *genbox();

/*****************************************************************************/
/******************************* USER DEFINED ********************************/
/*****************************************************************************/

#define NULL    0

#define PERR_NO_MEM   0
#define PERR_MISC     1
/* error function (PL expects program to halt after calling this) */
extern uerror();

/* user memory allocation function, ideally a calloc or something similar */
extern char *umemgt();
/* user memory freeing function */
extern umemfr();

extern bufcpy(); /* (*to, *from, size) */
extern bufset(); /* (*to, val, size) */

