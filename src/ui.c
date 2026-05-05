/* ui.c — rendering and key handling for TI-84 Plus CE
   Screen: 320x240, 8px monospace font (8x8 chars = 40 cols x 30 rows)
   Layout:
     y=0-17   : header bar
     y=18-223 : body
     y=207-223: soft-key bar (6 keys)
*/
#include "cas.h"

/* ── font metrics ── */
#define CHAR_W   8
#define CHAR_H   8
#define COLS     40
#define SOFT_Y   207
#define BODY_TOP 20

static const char *OP_NAMES[] = {
    "CURL & DIV",
    "SURFACE AREA",
    "SURF INTEGRAL",
    "FLUX INTEGRAL",
    "VOLUME",
    "TRIPLE INTEGRAL",
    "TANGENT PLANE"
};

/* ── drawing helpers ── */
static void fill(int x,int y,int w,int h,uint8_t c){
    gfx_SetColor(c);
    gfx_FillRectangle(x,y,w,h);
}
static void txt(int x,int y,uint8_t fg,uint8_t bg,const char*s){
    gfx_SetTextFGColor(fg);
    gfx_SetTextBGColor(bg);
    gfx_PrintStringXY(s,x,y);
}
static void header(const char*title){
    fill(0,0,SCR_W,18,C_HEADER);
    gfx_SetTextFGColor(gfx_white);
    gfx_SetTextBGColor(C_HEADER);
    gfx_PrintStringXY(title,4,5);
}
static void softbar(const char*f1,const char*f2,const char*f3,
                    const char*f4,const char*f5,const char*f6){
    fill(0,SOFT_Y,SCR_W,SCR_H-SOFT_Y,C_SOFTBAR);
    const char*lb[]={f1,f2,f3,f4,f5,f6};
    for(int i=0;i<6;i++){
        int x=i*53+2;
        if(i>0){
            gfx_SetColor(gfx_white);
            gfx_VertLine(i*53,SOFT_Y,SCR_H-SOFT_Y);
        }
        gfx_SetTextFGColor(gfx_white);
        gfx_SetTextBGColor(C_SOFTBAR);
        gfx_PrintStringXY(lb[i],x,SOFT_Y+4);
    }
}

/* ════════════════════════════════════════════════════════════════
   MAIN MENU
   ════════════════════════════════════════════════════════════════ */
static void draw_menu(void){
    header("MAT267 CAS  v1.0");
    txt(4,21,C_GRAY,C_BG,"1-7 or arrows + ENTER");
    for(int i=0;i<OP_COUNT;i++){
        int y=36+i*24;
        char buf[32];
        snprintf(buf,32," %d: %s",i+1,OP_NAMES[i]);
        if(i==G.menu_cur){
            fill(0,y-2,SCR_W,20,C_HILIGHT);
            txt(4,y,gfx_white,C_HILIGHT,buf);
        } else {
            txt(4,y,C_FG,C_BG,buf);
        }
    }
    softbar("SEL","","","","","QUIT");
}

/* ════════════════════════════════════════════════════════════════
   ORIENTATION SCREEN
   ════════════════════════════════════════════════════════════════ */
static void draw_orient(void){
    header("FLUX: Orientation");
    txt(4,28,C_FG,C_BG,"Normal direction:");
    int y1=60,y2=90;
    if(G.orient==1){
        fill(0,y1-2,SCR_W,20,C_HILIGHT);
        txt(8,y1,gfx_white,C_HILIGHT,"> UPWARD / OUTWARD");
        txt(8,y2,C_FG,C_BG,"  DOWNWARD / INWARD");
    } else {
        txt(8,y1,C_FG,C_BG,"  UPWARD / OUTWARD");
        fill(0,y2-2,SCR_W,20,C_HILIGHT);
        txt(8,y2,gfx_white,C_HILIGHT,"> DOWNWARD / INWARD");
    }
    softbar("UP","DOWN","","","NEXT","BACK");
}

/* ════════════════════════════════════════════════════════════════
   COORDINATE SCREEN
   ════════════════════════════════════════════════════════════════ */
static void draw_coords(void){
    header("TRIPLE: Coordinates");
    txt(4,28,C_FG,C_BG,"Choose coord system:");
    const char*names[]={"CARTESIAN  (x,y,z)",
                        "CYLINDRICAL(r,th,z)",
                        "SPHERICAL  (p,ph,th)"};
    for(int i=0;i<3;i++){
        int y=56+i*30;
        if(G.coords==i){
            fill(0,y-2,SCR_W,22,C_HILIGHT);
            txt(8,y,gfx_white,C_HILIGHT,names[i]);
        } else {
            txt(8,y,C_FG,C_BG,names[i]);
        }
    }
    softbar("CART","CYL","SPH","","NEXT","BACK");
}

/* ════════════════════════════════════════════════════════════════
   INPUT SCREEN
   ════════════════════════════════════════════════════════════════ */
#define VISIBLE 7
static void draw_input(void){
    header(OP_NAMES[G.op]);

    /* orientation reminder */
    if(G.op==OP_FLUX){
        char ob[20];
        snprintf(ob,20,"%s",G.orient==1?"[UP/OUT]":"[DOWN/IN]");
        gfx_SetTextFGColor(C_ACCENT);
        gfx_SetTextBGColor(C_HEADER);
        gfx_PrintStringXY(ob,220,5);
    }

    int scroll=0;
    if(G.field_cur>=VISIBLE) scroll=G.field_cur-VISIBLE+1;

    for(int i=0;i<VISIBLE&&(i+scroll)<G.nf;i++){
        int fi=i+scroll;
        int y=BODY_TOP+i*26;
        int active=(fi==G.field_cur);

        /* label */
        gfx_SetTextFGColor(active?C_ACCENT:C_GRAY);
        gfx_SetTextBGColor(C_BG);
        gfx_PrintStringXY(G.f[fi].label,4,y);

        /* value box */
        int bx=112,bw=200,bh=18;
        if(active){
            fill(bx,y-1,bw,bh,C_HILIGHT);
            char buf[MAX_EXPR+2];
            snprintf(buf,sizeof buf,"%s|",G.f[fi].val);
            gfx_SetTextFGColor(gfx_white);
            gfx_SetTextBGColor(C_HILIGHT);
            gfx_PrintStringXY(buf,bx+3,y);
        } else {
            gfx_SetColor(C_ACCENT);
            gfx_Rectangle(bx,y-1,bw,bh);
            gfx_SetTextFGColor(C_FG);
            gfx_SetTextBGColor(C_BG);
            gfx_PrintStringXY(G.f[fi].val,bx+3,y);
        }
    }

    /* field counter */
    if(G.nf>VISIBLE){
        char si[10]; snprintf(si,10,"%d/%d",G.field_cur+1,G.nf);
        gfx_SetTextFGColor(C_GRAY);
        gfx_SetTextBGColor(C_BG);
        gfx_PrintStringXY(si,288,5);
    }

    softbar("DEL","CLR","PI","NEG","NEXT","CALC");
}

/* ════════════════════════════════════════════════════════════════
   RESULT SCREEN
   ════════════════════════════════════════════════════════════════ */
static void draw_result(void){
    header(OP_NAMES[G.op]);
    if(G.err[0]){
        gfx_SetTextFGColor(C_ERR);
        gfx_SetTextBGColor(C_BG);
        gfx_PrintStringXY("ERROR:",4,28);
        gfx_PrintStringXY(G.err,4,44);
    } else {
        int y=24;
        for(int i=0;i<G.nres;i++){
            gfx_SetTextFGColor(C_ACCENT);
            gfx_SetTextBGColor(C_BG);
            gfx_PrintStringXY(G.res[i].label,4,y);
            y+=13;
            gfx_SetTextFGColor(C_FG);
            gfx_PrintStringXY(G.res[i].value,12,y);
            y+=16;
        }
        /* raw value footer */
        gfx_SetColor(C_GRAY);
        gfx_HorizLine(0,y+2,SCR_W);
        char raw[32]; snprintf(raw,32,"raw=%.8g",G.raw);
        gfx_SetTextFGColor(C_GRAY);
        gfx_PrintStringXY(raw,4,y+6);
    }
    softbar("","","","","REDO","MENU");
}

/* ════════════════════════════════════════════════════════════════
   DRAW DISPATCHER
   ════════════════════════════════════════════════════════════════ */
void ui_draw(void){
    switch(G.screen){
    case SCR_MENU:   draw_menu();   break;
    case SCR_INPUT:  draw_input();  break;
    case SCR_RESULT: draw_result(); break;
    case SCR_ORIENT: draw_orient(); break;
    case SCR_COORDS: draw_coords(); break;
    }
}

/* ════════════════════════════════════════════════════════════════
   KEY HANDLING
   TI-84 CE key rows:
     kb_Data[1]=kb_KeyGraph/Trace/Zoom/Window/Yeq/2nd/Mode/Del
     kb_Data[2]=kb_KeyStore/.../Alpha/X,T,O,n/Stat
     kb_Data[3]=kb_KeyMath/Apps/Prgm/Vars/Clear
     kb_Data[4]=kb_KeyRecip/Sin/Cos/Tan/Power/Sqrt/Sq/Comma/LParen/RParen
     kb_Data[5]=kb_Key7/8/9/Mul/Div
     kb_Data[6]=kb_Key4/5/6/Add/Sub
     kb_Data[7]=kb_Key1/2/3/0/Dot/Neg/Enter
   We use kb_IsDown() after kb_Scan()
   ════════════════════════════════════════════════════════════════ */

/* append char to active field */
static void append(char c){
    Field*f=&G.f[G.field_cur];
    int len=strlen(f->val);
    if(len<MAX_EXPR-1){ f->val[len]=c; f->val[len+1]='\0'; }
}
static void append_str(const char*s){
    Field*f=&G.f[G.field_cur];
    int len=strlen(f->val);
    strncat(f->val,s,MAX_EXPR-len-1);
}
static void backspace(void){
    Field*f=&G.f[G.field_cur];
    int len=strlen(f->val);
    if(len>0) f->val[len-1]='\0';
}
static void clear_field(void){
    G.f[G.field_cur].val[0]='\0';
}
static void neg_field(void){
    Field*f=&G.f[G.field_cur];
    if(f->val[0]=='-') memmove(f->val,f->val+1,strlen(f->val));
    else {
        int len=strlen(f->val);
        if(len<MAX_EXPR-1){ memmove(f->val+1,f->val,len+1); f->val[0]='-'; }
    }
}
static void next_field(void){
    if(G.field_cur<G.nf-1) G.field_cur++;
    else { compute(); G.screen=SCR_RESULT; }
}
static void enter_op(Op op){
    G.op=op; G.nres=0; G.err[0]='\0';
    if(op==OP_FLUX)      { G.screen=SCR_ORIENT; return; }
    if(op==OP_TRIPLE)    { G.screen=SCR_COORDS; return; }
    fields_load(op,G.coords); fields_defaults();
    G.field_cur=0; G.screen=SCR_INPUT;
}

void ui_handle_key(kb_key_t key, kb_lkey_t lkey){
    (void)key; /* we use kb_IsDown directly */

    /* ── MAIN MENU ── */
    if(G.screen==SCR_MENU){
        if(kb_IsDown(kb_KeyUp)   && G.menu_cur>0)          G.menu_cur--;
        if(kb_IsDown(kb_KeyDown) && G.menu_cur<OP_COUNT-1) G.menu_cur++;
        if(kb_IsDown(kb_Key1))   enter_op(OP_CURLDIV);
        if(kb_IsDown(kb_Key2))   enter_op(OP_SURFAREA);
        if(kb_IsDown(kb_Key3))   enter_op(OP_SURFINT);
        if(kb_IsDown(kb_Key4))   enter_op(OP_FLUX);
        if(kb_IsDown(kb_Key5))   enter_op(OP_VOLUME);
        if(kb_IsDown(kb_Key6))   enter_op(OP_TRIPLE);
        if(kb_IsDown(kb_Key7))   enter_op(OP_TANGPLANE);
        if(kb_IsDown(kb_KeyEnter)||kb_IsDown(kb_KeyTrace))
            enter_op((Op)G.menu_cur);
        return;
    }

    /* ── ORIENT SCREEN ── */
    if(G.screen==SCR_ORIENT){
        if(kb_IsDown(kb_KeyUp)||kb_IsDown(kb_KeyYeq))  G.orient= 1;
        if(kb_IsDown(kb_KeyDown)||kb_IsDown(kb_KeyWindow)) G.orient=-1;
        if(kb_IsDown(kb_KeyEnter)||kb_IsDown(kb_KeyGraph)){
            fields_load(G.op,G.coords); fields_defaults();
            G.field_cur=0; G.screen=SCR_INPUT;
        }
        if(kb_IsDown(kb_KeyClear)) G.screen=SCR_MENU;
        return;
    }

    /* ── COORDS SCREEN ── */
    if(G.screen==SCR_COORDS){
        if(kb_IsDown(kb_KeyUp)  && G.coords>0) G.coords--;
        if(kb_IsDown(kb_KeyDown)&& G.coords<2) G.coords++;
        if(kb_IsDown(kb_KeyYeq))    G.coords=COORD_CART;
        if(kb_IsDown(kb_KeyWindow)) G.coords=COORD_CYL;
        if(kb_IsDown(kb_KeyZoom))   G.coords=COORD_SPH;
        if(kb_IsDown(kb_KeyEnter)||kb_IsDown(kb_KeyGraph)){
            fields_load(G.op,G.coords); fields_defaults();
            G.field_cur=0; G.screen=SCR_INPUT;
        }
        if(kb_IsDown(kb_KeyClear)) G.screen=SCR_MENU;
        return;
    }

    /* ── RESULT SCREEN ── */
    if(G.screen==SCR_RESULT){
        if(kb_IsDown(kb_KeyClear)||kb_IsDown(kb_KeyGraph))
            G.screen=SCR_INPUT;
        if(kb_IsDown(kb_KeyTrace))
            { G.screen=SCR_MENU; G.menu_cur=(int)G.op; }
        return;
    }

    /* ── INPUT SCREEN ── */
    if(G.screen==SCR_INPUT){
        /* navigation */
        if(kb_IsDown(kb_KeyUp)){ if(G.field_cur>0) G.field_cur--; return; }
        if(kb_IsDown(kb_KeyDown)||kb_IsDown(kb_KeyEnter)){ next_field(); return; }
        if(kb_IsDown(kb_KeyClear)){ G.screen=SCR_MENU; return; }

        /* F1-F5 soft-key row = Y=/Window/Zoom/Trace/Graph */
        if(kb_IsDown(kb_KeyYeq))   { backspace(); return; }   /* DEL */
        if(kb_IsDown(kb_KeyWindow)){ clear_field(); return; }  /* CLR */
        if(kb_IsDown(kb_KeyZoom))  { append_str("pi"); return;}/* PI  */
        if(kb_IsDown(kb_KeyTrace)) { neg_field(); return; }    /* NEG */
        if(kb_IsDown(kb_KeyGraph)) { compute(); G.screen=SCR_RESULT; return; } /* CALC */

        /* digits */
        if(kb_IsDown(kb_Key0)){ append('0'); return; }
        if(kb_IsDown(kb_Key1)){ append('1'); return; }
        if(kb_IsDown(kb_Key2)){ append('2'); return; }
        if(kb_IsDown(kb_Key3)){ append('3'); return; }
        if(kb_IsDown(kb_Key4)){ append('4'); return; }
        if(kb_IsDown(kb_Key5)){ append('5'); return; }
        if(kb_IsDown(kb_Key6)){ append('6'); return; }
        if(kb_IsDown(kb_Key7)){ append('7'); return; }
        if(kb_IsDown(kb_Key8)){ append('8'); return; }
        if(kb_IsDown(kb_Key9)){ append('9'); return; }

        /* operators */
        if(kb_IsDown(kb_KeyAdd)) { append('+'); return; }
        if(kb_IsDown(kb_KeySub)) { append('-'); return; }
        if(kb_IsDown(kb_KeyMul)) { append('*'); return; }
        if(kb_IsDown(kb_KeyDiv)) { append('/'); return; }
        if(kb_IsDown(kb_KeyPower)){ append('^'); return; }
        if(kb_IsDown(kb_KeyDecPt)){ append('.'); return; }
        if(kb_IsDown(kb_KeyLParen)){ append('('); return; }
        if(kb_IsDown(kb_KeyRParen)){ append(')'); return; }

        /* trig — MATH key cycles, use dedicated keys */
        if(kb_IsDown(kb_KeySin)) { append_str("sin("); return; }
        if(kb_IsDown(kb_KeyCos)) { append_str("cos("); return; }
        if(kb_IsDown(kb_KeyTan)) { append_str("tan("); return; }
        if(kb_IsDown(kb_KeySqrt)){ append_str("sqrt("); return; }
        if(kb_IsDown(kb_KeySquare)){ append_str("^2"); return; }
        if(kb_IsDown(kb_KeyRecip)) { append_str("^-1"); return; }

        /* variables */
        if(kb_IsDown(kb_KeyXnt)) { append('x'); return; } /* X,T,θ,n key */
        if(kb_IsDown(kb_KeyAlpha)&& kb_IsDown(kb_KeyAdd)){ append('y'); return; }
    }
}
