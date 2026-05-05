#pragma once
#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

/* ── screen geometry (TI-84 Plus CE: 320×240) ── */
#define SCR_W       320
#define SCR_H       240
#define HEADER_H     18
#define FOOTER_H     16
#define BODY_Y      (HEADER_H + 2)

/* ── colors (gfx palette indices) ── */
#define C_BG        gfx_white
#define C_FG        gfx_black
#define C_HEADER    0x00  /* dark navy  — custom palette slot */
#define C_HILIGHT   0x01  /* highlight blue                   */
#define C_SOFTBAR   0x02  /* soft-key bar                     */
#define C_ACCENT    0x03  /* accent blue for labels           */
#define C_ERR       0x04  /* red for errors                   */
#define C_GRAY      0x05  /* muted gray                       */

/* ── tuning ── */
#define SIMP_N      20
#define SIMP_N3     10
#define DIFF_H      1e-5
#define MAX_EXPR    40
#define MAX_FIELDS  11
#define MAX_RES      6

/* ── operations ── */
typedef enum {
    OP_CURLDIV   = 0,
    OP_SURFAREA  = 1,
    OP_SURFINT   = 2,
    OP_FLUX      = 3,
    OP_VOLUME    = 4,
    OP_TRIPLE    = 5,
    OP_TANGPLANE = 6,
    OP_COUNT     = 7
} Op;

/* ── coordinate systems ── */
typedef enum { COORD_CART=0, COORD_CYL=1, COORD_SPH=2 } Coords;

/* ── screens ── */
typedef enum {
    SCR_MENU   = 0,
    SCR_INPUT  = 1,
    SCR_RESULT = 2,
    SCR_ORIENT = 3,
    SCR_COORDS = 4
} Screen;

/* ── one input field ── */
typedef struct {
    char label[14];
    char val[MAX_EXPR];
    char def[MAX_EXPR];
} Field;

/* ── one result line ── */
typedef struct {
    char label[22];
    char value[52];
} ResLine;

/* ── global state ── */
typedef struct {
    Screen  screen;
    Op      op;
    int     menu_cur;
    int     field_cur;
    int     orient;      /* +1 or -1 */
    Coords  coords;
    Field   f[MAX_FIELDS];
    int     nf;
    ResLine res[MAX_RES];
    int     nres;
    char    err[64];
    double  raw;
} State;

extern State G;

/* ── function declarations ── */
/* eval.c */
double eval(const char *e, double x, double y, double z);
double eval_lim(const char *s);

/* fields.c */
void fields_load(Op op, Coords coords);
void fields_defaults(void);

/* compute.c */
void compute(void);

/* ui.c */
void ui_draw(void);
void ui_handle_key(kb_key_t key, kb_lkey_t lkey);

/* ── inline math helpers ── */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E  2.71828182845904523536
#endif

static inline double nd_x(const char*e,double x,double y,double z)
{ return (eval(e,x+DIFF_H,y,z)-eval(e,x-DIFF_H,y,z))/(2*DIFF_H); }
static inline double nd_y(const char*e,double x,double y,double z)
{ return (eval(e,x,y+DIFF_H,z)-eval(e,x,y-DIFF_H,z))/(2*DIFF_H); }
static inline double nd_z(const char*e,double x,double y,double z)
{ return (eval(e,x,y,z+DIFF_H)-eval(e,x,y,z-DIFF_H))/(2*DIFF_H); }
static inline double nd_u(const char*e,double u,double v)
{ return (eval(e,u+DIFF_H,v,0)-eval(e,u-DIFF_H,v,0))/(2*DIFF_H); }
static inline double nd_v(const char*e,double u,double v)
{ return (eval(e,u,v+DIFF_H,0)-eval(e,u,v-DIFF_H,0))/(2*DIFF_H); }

static inline double cross_mag(double ax,double ay,double az,
                                double bx,double by,double bz){
    double cx=ay*bz-az*by,cy=az*bx-ax*bz,cz=ax*by-ay*bx;
    return sqrt(cx*cx+cy*cy+cz*cz);
}
static inline void cross3(double ax,double ay,double az,
                           double bx,double by,double bz,
                           double*cx,double*cy,double*cz){
    *cx=ay*bz-az*by; *cy=az*bx-ax*bz; *cz=ax*by-ay*bx;
}
static inline void fmt(char*buf,int n,double v){
    double r=round(v*1e6)/1e6;
    long long ri=(long long)r;
    if(r==ri && r>-1e9 && r<1e9) snprintf(buf,n,"%lld",ri);
    else snprintf(buf,n,"%.6g",v);
}
static inline void add_res(const char*lbl,const char*val){
    if(G.nres>=MAX_RES) return;
    strncpy(G.res[G.nres].label,lbl,21);
    strncpy(G.res[G.nres].value,val,51);
    G.nres++;
}
