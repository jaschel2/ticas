/* compute.c — numerical back-end for all 7 operations */
#include "cas.h"

/* ── Simpson 2-D ── */
typedef double (*F2)(double,double,void*);
static double simp2(F2 f,void*ctx,
                    double a1,double b1,double a2,double b2,int n){
    double hu=(b1-a1)/n,hv=(b2-a2)/n,S=0;
    for(int i=0;i<=n;i++){
        double u=a1+i*hu;
        double wi=(i==0||i==n)?1:(i%2==0?2:4);
        for(int j=0;j<=n;j++){
            double v=a2+j*hv;
            double wj=(j==0||j==n)?1:(j%2==0?2:4);
            S+=wi*wj*f(u,v,ctx);
        }
    }
    return S*(hu*hv/9.0);
}

/* ── Simpson 3-D ── */
typedef double (*F3)(double,double,double,void*);
static double simp3(F3 f,void*ctx,
                    double a1,double b1,double a2,double b2,
                    double a3,double b3,int n){
    double h1=(b1-a1)/n,h2=(b2-a2)/n,h3=(b3-a3)/n,S=0;
    for(int i=0;i<=n;i++){
        double u=a1+i*h1;
        double wi=(i==0||i==n)?1:(i%2==0?2:4);
        for(int j=0;j<=n;j++){
            double v=a2+j*h2;
            double wj=(j==0||j==n)?1:(j%2==0?2:4);
            for(int k=0;k<=n;k++){
                double w=a3+k*h3;
                double wk=(k==0||k==n)?1:(k%2==0?2:4);
                S+=wi*wj*wk*f(u,v,w,ctx);
            }
        }
    }
    return S*(h1*h2*h3/27.0);
}

/* ── surface helpers ── */
typedef struct{const char*xs,*ys,*zs;}Surf;
static void surf_pt(Surf*s,double u,double v,
                    double*px,double*py,double*pz){
    *px=eval(s->xs,u,v,0);
    *py=eval(s->ys,u,v,0);
    *pz=eval(s->zs,u,v,0);
}
static void surf_partials(Surf*s,double u,double v,
                           double*rux,double*ruy,double*ruz,
                           double*rvx,double*rvy,double*rvz){
    *rux=nd_u(s->xs,u,v);*ruy=nd_u(s->ys,u,v);*ruz=nd_u(s->zs,u,v);
    *rvx=nd_v(s->xs,u,v);*rvy=nd_v(s->ys,u,v);*rvz=nd_v(s->zs,u,v);
}

/* ══ 1. CURL & DIV ══════════════════════════════════════════════ */
static void do_curldiv(void){
    const char*P=G.f[0].val,*Q=G.f[1].val,*R=G.f[2].val;
    double x0=eval_lim(G.f[3].val);
    double y0=eval_lim(G.f[4].val);
    double z0=eval_lim(G.f[5].val);
    double Py=nd_y(P,x0,y0,z0),Pz=nd_z(P,x0,y0,z0);
    double Qx=nd_x(Q,x0,y0,z0),Qz=nd_z(Q,x0,y0,z0);
    double Rx=nd_x(R,x0,y0,z0),Ry=nd_y(R,x0,y0,z0);
    double Px=nd_x(P,x0,y0,z0),Qy=nd_y(Q,x0,y0,z0),Rz=nd_z(R,x0,y0,z0);
    double ci=Ry-Qz,cj=Pz-Rx,ck=Qx-Py,dv=Px+Qy+Rz;
    char bi[14],bj[14],bk[14],bd[14],bm[14];
    fmt(bi,14,ci);fmt(bj,14,cj);fmt(bk,14,ck);
    fmt(bd,14,dv);fmt(bm,14,sqrt(ci*ci+cj*cj+ck*ck));
    char curl[52];
    snprintf(curl,52,"<%s,%s,%s>",bi,bj,bk);
    add_res("curl F =",curl);
    add_res("div F =",bd);
    add_res("|curl F| =",bm);
    G.raw=dv;
}

/* ══ 2. SURFACE AREA ════════════════════════════════════════════ */
typedef struct{Surf s;}SACtx;
static double sa_f(double u,double v,void*p){
    SACtx*c=(SACtx*)p;
    double rux,ruy,ruz,rvx,rvy,rvz;
    surf_partials(&c->s,u,v,&rux,&ruy,&ruz,&rvx,&rvy,&rvz);
    return cross_mag(rux,ruy,ruz,rvx,rvy,rvz);
}
static void do_surfarea(void){
    SACtx ctx;
    ctx.s.xs=G.f[0].val;ctx.s.ys=G.f[1].val;ctx.s.zs=G.f[2].val;
    double u0=eval_lim(G.f[3].val),u1=eval_lim(G.f[4].val);
    double v0=eval_lim(G.f[5].val),v1=eval_lim(G.f[6].val);
    double r=simp2(sa_f,&ctx,u0,u1,v0,v1,SIMP_N);
    char b[20],bp[20]; fmt(b,20,r); fmt(bp,20,r/M_PI);
    char tmp[28]; snprintf(tmp,28,"%s*pi",bp);
    add_res("SA =",b); add_res("  =",tmp); G.raw=r;
}

/* ══ 3. SURFACE INTEGRAL ════════════════════════════════════════ */
typedef struct{Surf s;const char*fs;}SICtx;
static double si_f(double u,double v,void*p){
    SICtx*c=(SICtx*)p;
    double rux,ruy,ruz,rvx,rvy,rvz,px,py,pz;
    surf_partials(&c->s,u,v,&rux,&ruy,&ruz,&rvx,&rvy,&rvz);
    surf_pt(&c->s,u,v,&px,&py,&pz);
    return eval(c->fs,px,py,pz)*cross_mag(rux,ruy,ruz,rvx,rvy,rvz);
}
static void do_surfint(void){
    SICtx ctx;
    ctx.s.xs=G.f[0].val;ctx.s.ys=G.f[1].val;ctx.s.zs=G.f[2].val;
    ctx.fs=G.f[3].val;
    double u0=eval_lim(G.f[4].val),u1=eval_lim(G.f[5].val);
    double v0=eval_lim(G.f[6].val),v1=eval_lim(G.f[7].val);
    double r=simp2(si_f,&ctx,u0,u1,v0,v1,SIMP_N);
    char b[20],bp[20]; fmt(b,20,r); fmt(bp,20,r/M_PI);
    char tmp[28]; snprintf(tmp,28,"%s*pi",bp);
    add_res("Integral =",b); add_res("       =",tmp); G.raw=r;
}

/* ══ 4. FLUX ════════════════════════════════════════════════════ */
typedef struct{Surf s;const char*Ps,*Qs,*Rs;int ori;}FLCtx;
static double fl_f(double u,double v,void*p){
    FLCtx*c=(FLCtx*)p;
    double rux,ruy,ruz,rvx,rvy,rvz,px,py,pz,cx,cy,cz;
    surf_partials(&c->s,u,v,&rux,&ruy,&ruz,&rvx,&rvy,&rvz);
    surf_pt(&c->s,u,v,&px,&py,&pz);
    cross3(rux,ruy,ruz,rvx,rvy,rvz,&cx,&cy,&cz);
    double dot=eval(c->Ps,px,py,pz)*cx
              +eval(c->Qs,px,py,pz)*cy
              +eval(c->Rs,px,py,pz)*cz;
    return dot*c->ori;
}
static void do_flux(void){
    FLCtx ctx;
    ctx.s.xs=G.f[0].val;ctx.s.ys=G.f[1].val;ctx.s.zs=G.f[2].val;
    ctx.Ps=G.f[3].val;ctx.Qs=G.f[4].val;ctx.Rs=G.f[5].val;
    ctx.ori=G.orient;
    double u0=eval_lim(G.f[6].val),u1=eval_lim(G.f[7].val);
    double v0=eval_lim(G.f[8].val),v1=eval_lim(G.f[9].val);
    double r=simp2(fl_f,&ctx,u0,u1,v0,v1,SIMP_N);
    char b[20],bp[20]; fmt(b,20,r); fmt(bp,20,r/M_PI);
    char tmp[28]; snprintf(tmp,28,"%s*pi",bp);
    add_res("Flux =",b); add_res("    =",tmp);
    add_res("Orient:",G.orient==1?"up/out":"down/in");
    G.raw=r;
}

/* ══ 5. VOLUME ══════════════════════════════════════════════════ */
typedef struct{const char*zt,*zb;}VOLCtx;
static double vol_f(double r,double th,void*p){
    VOLCtx*c=(VOLCtx*)p;
    double top=eval(c->zt,r,th,0),bot=eval(c->zb,r,th,0);
    double h=top-bot; if(h<0)h=0;
    return h*r;
}
static void do_volume(void){
    VOLCtx ctx; ctx.zt=G.f[0].val; ctx.zb=G.f[1].val;
    double r0=eval_lim(G.f[2].val),r1=eval_lim(G.f[3].val);
    double t0=eval_lim(G.f[4].val),t1=eval_lim(G.f[5].val);
    double r=simp2(vol_f,&ctx,r0,r1,t0,t1,SIMP_N);
    char b[20],bp[20]; fmt(b,20,r); fmt(bp,20,r/M_PI);
    char tmp[28]; snprintf(tmp,28,"%s*pi",bp);
    add_res("Volume =",b); add_res("      =",tmp); G.raw=r;
}

/* ══ 6. TRIPLE INTEGRAL ═════════════════════════════════════════ */
typedef struct{const char*fs;Coords coords;}TICtx;
static double ti_f(double a,double b,double c_,void*p){
    TICtx*c=(TICtx*)p;
    double x,y,z,jac;
    switch(c->coords){
    case COORD_CART: x=a;y=b;z=c_;jac=1;break;
    case COORD_CYL:  x=a*cos(b);y=a*sin(b);z=c_;jac=a;break;
    default:         /* SPH */
        x=a*sin(b)*cos(c_);y=a*sin(b)*sin(c_);z=a*cos(b);
        jac=a*a*sin(b);break;
    }
    return eval(c->fs,x,y,z)*jac;
}
static void do_triple(void){
    TICtx ctx; ctx.fs=G.f[0].val; ctx.coords=G.coords;
    double a0=eval_lim(G.f[1].val),a1=eval_lim(G.f[2].val);
    double b0=eval_lim(G.f[3].val),b1=eval_lim(G.f[4].val);
    double c0=eval_lim(G.f[5].val),c1=eval_lim(G.f[6].val);
    double r=simp3(ti_f,&ctx,a0,a1,b0,b1,c0,c1,SIMP_N3);
    char b[20],bp[20]; fmt(b,20,r); fmt(bp,20,r/M_PI);
    char tmp[28]; snprintf(tmp,28,"%s*pi",bp);
    add_res("Integral =",b); add_res("       =",tmp);
    const char*cn[]={"Cartesian","Cylindrical","Spherical"};
    add_res("Coords:",cn[G.coords]);
    G.raw=r;
}

/* ══ 7. TANGENT PLANE ═══════════════════════════════════════════ */
static void do_tangplane(void){
    const char*f=G.f[0].val;
    double x0=eval_lim(G.f[1].val),y0=eval_lim(G.f[2].val);
    double f0=eval(f,x0,y0,0);
    double fx=nd_x(f,x0,y0,0),fy=nd_y(f,x0,y0,0);
    double c=f0-fx*x0-fy*y0;
    char bfx[14],bfy[14],bc[14],bf0[14];
    fmt(bfx,14,fx);fmt(bfy,14,fy);fmt(bc,14,c);fmt(bf0,14,f0);
    char line[52];
    snprintf(line,52,"z=%sx+%sy+%s",bfx,bfy,bc);
    add_res("Tangent plane:",line);
    add_res("f(x0,y0) =",bf0);
    add_res("fx =",bfx); add_res("fy =",bfy);
    G.raw=f0;
}

/* ══ DISPATCHER ═════════════════════════════════════════════════ */
void compute(void){
    G.nres=0; G.err[0]='\0'; G.raw=0;
    switch(G.op){
    case OP_CURLDIV:   do_curldiv();   break;
    case OP_SURFAREA:  do_surfarea();  break;
    case OP_SURFINT:   do_surfint();   break;
    case OP_FLUX:      do_flux();      break;
    case OP_VOLUME:    do_volume();    break;
    case OP_TRIPLE:    do_triple();    break;
    case OP_TANGPLANE: do_tangplane(); break;
    default: snprintf(G.err,64,"Unknown op"); break;
    }
}
