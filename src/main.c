/* main.c — MAT267 CAS for TI-84 Plus CE */
#include "cas.h"

State G;

/* custom palette: 6 colors starting at index 0 */
static const gfx_color_t PALETTE[] = {
    gfx_RGBTo1555(0,  30,  80),   /* 0: dark navy  — header/highlight bg */
    gfx_RGBTo1555(30, 80,  180),  /* 1: blue       — selected row        */
    gfx_RGBTo1555(40, 40,  40),   /* 2: dark gray  — soft-key bar        */
    gfx_RGBTo1555(0,  80,  160),  /* 3: accent blue — labels             */
    gfx_RGBTo1555(180,0,   0),    /* 4: red        — errors              */
    gfx_RGBTo1555(120,120, 120),  /* 5: gray       — muted text          */
};

int main(void) {
    /* ── init graphics ── */
    gfx_Begin();
    gfx_SetPalette(PALETTE, sizeof PALETTE, 0);
    gfx_SetDrawBuffer();
    gfx_SetTextFGColor(C_FG);
    gfx_SetTextBGColor(C_BG);
    gfx_SetMonospaceFont(8);

    /* ── init state ── */
    memset(&G, 0, sizeof G);
    G.screen   = SCR_MENU;
    G.menu_cur = 0;
    G.orient   = 1;
    G.coords   = COORD_SPH;

    while (true) {
        /* draw */
        gfx_FillScreen(C_BG);
        ui_draw();
        gfx_SwapDraw();

        /* keys */
        kb_Scan();

        /* check for exit: [2nd]+[Mode] = quit to OS */
        if (kb_IsDown(kb_Key2nd) && kb_IsDown(kb_KeyMode)) break;

        /* find first pressed key */
        kb_key_t  key  = 0;
        kb_lkey_t lkey = 0;
        for (int g = 1; g <= 7; g++) {
            kb_key_t row = kb_Data[g];
            if (row) { key = (kb_key_t)(g << 8); lkey = row; break; }
        }
        if (key || lkey) {
            ui_handle_key(key, lkey);
            /* debounce */
            while (kb_AnyKey()) kb_Scan();
        }
    }

    gfx_End();
    return 0;
}
