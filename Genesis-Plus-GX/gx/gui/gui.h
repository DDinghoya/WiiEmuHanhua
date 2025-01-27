/****************************************************************************
 *  gui.c
 *
 *  generic GUI Engine (using GX rendering)
 *
 *  Copyright Eke-Eke (2009-2019)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/

#ifndef _GUI_H
#define _GUI_H

#define BG_COLOR_MAX 15

/*****************************************************************************/
/*  GUI Buttons state                                                        */
/*****************************************************************************/
#define BUTTON_VISIBLE       0x01
#define BUTTON_ACTIVE        0x02
#define BUTTON_SELECTED      0x04
#define BUTTON_OVER_SFX      0x08
#define BUTTON_SELECT_SFX    0x10
#define BUTTON_FADE          0x20
#define BUTTON_SLIDE_LEFT    0x40
#define BUTTON_SLIDE_RIGHT   0x80
#define BUTTON_SLIDE_TOP    0x100
#define BUTTON_SLIDE_BOTTOM 0x200

/*****************************************************************************/
/*  GUI Image state                                                        */
/*****************************************************************************/
#define IMAGE_VISIBLE       0x01
#define IMAGE_REPEAT        0x02
#define IMAGE_FADE          0x04
#define IMAGE_SLIDE_LEFT    0x08
#define IMAGE_SLIDE_RIGHT   0x10
#define IMAGE_SLIDE_TOP     0x20
#define IMAGE_SLIDE_BOTTOM  0x40

/*****************************************************************************/
/*  Generic GUI structures                                                   */
/*****************************************************************************/

/* Item descriptor*/
typedef struct
{
  gx_texture *texture;  /* temporary texture data                             */
  const u8 *data;       /* pointer to png image data (items icon only)        */
  char text[64];        /* item string (items list only)                      */
  char comment[64];     /* item comment                                       */
  u16 x;                /* item image or text X position (upper left corner)  */
  u16 y;                /* item image or text Y position (upper left corner)  */
  u16 w;                /* item image or text width                           */
  u16 h;                /* item image or text height                          */
} gui_item;

/* Button Data descriptor */
typedef struct
{
  gx_texture *texture[2];  /* temporary texture datas               */
  const u8 *image[2];       /* pointer to png image datas (default) */
} butn_data;

/* Button descriptor */
typedef struct
{
  butn_data *data;          /* pointer to button image/texture data         */
  u16 state;                /* button state (ACTIVE,VISIBLE,SELECTED...)    */
  u8 shift[4];              /* direction offsets                            */
  u16 x;                    /* button image X position (upper left corner)  */
  u16 y;                    /* button image Y position (upper left corner)  */
  u16 w;                    /* button image pixels width                    */
  u16 h;                    /* button image pixels height                   */
} gui_butn;

/* Image descriptor */
typedef struct
{
  gx_texture *texture;  /* temporary texture data                 */
  const u8 *data;       /* pointer to png image data              */
  u8 state;             /* image state (VISIBLE)                  */
  u16 x;                /* image X position (upper left corner)   */
  u16 y;                /* image Y position (upper left corner)   */
  u16 w;                /* image width                            */
  u16 h;                /* image height                           */
  u8 alpha;             /* alpha transparency                     */
} gui_image;

/* Menu descriptor */
typedef struct
{
  char title[64];             /* menu title                         */
  int selected;               /* index of selected item             */
  int offset;                 /* items list offset                  */
  int max_items;              /* total number of items              */
  int max_buttons;            /* total number of buttons            */
  int max_images;             /* total number of background images  */
  u8 screenshot;              /* game screen background             */
  gui_item *items;            /* menu items                         */
  gui_butn *buttons;          /* menu buttons                       */
  gui_image *bg_images;       /* background images                  */
  gui_item *helpers[2];       /* left & right key comments          */
  gui_butn *arrows[2];        /* arrows buttons                     */
  void (*cb)(void);           /* specific draw callback */
} gui_menu;

typedef struct 
{
  u32 progress;           /* progress counter */
  bool refresh;           /* messagebox current state */
  gui_menu *parent;       /* parent menu  */
  char title[64];         /* box title    */
  char msg[64];           /* box message  */
  gx_texture *window;     /* pointer to box texture */
  gx_texture *top;        /* pointer to box title texture */
  gx_texture *buttonA;    /* pointer to button A texture */
  gx_texture *buttonB;    /* pointer to button A texture */
  gx_texture *throbber;   /* pointer to throbber texture */
} gui_message;

/* Menu inputs */
struct t_input_menu
{
  u16 keys;
#ifdef HW_RVL
  struct ir_t ir;
#endif
} m_input;

/* Optionbox callback */
typedef void (*optioncallback)(void);

/* Generic textures*/
#ifdef HW_RVL
extern gx_texture *w_pointer;
#endif

/* Generic backgrounds */
extern const u8 Bg_layer_png[];
extern const u8 Bg_overlay_png[];
extern const u8 Banner_main_png[];
extern const u8 Banner_bottom_png[];
extern const u8 Banner_top_png[];
extern const u8 Main_logo_png[];

/* Generic frames */
extern const u8 Frame_s1_png[];
extern const u8 Frame_s2_png[];
extern const u8 Frame_s3_png[];
extern const u8 Frame_s1_title_png[];
extern const u8 Frame_s2_title_png[];
extern const u8 Frame_throbber_png[];

/* Generic Buttons */
extern const u8 Button_text_png[];
extern const u8 Button_text_over_png[];
extern const u8 Button_icon_png[];
extern const u8 Button_icon_over_png[];
extern const u8 Button_icon_sm_png[];
extern const u8 Button_icon_sm_over_png[];
extern const u8 Button_up_png[];
extern const u8 Button_up_over_png[];
extern const u8 Button_down_png[];
extern const u8 Button_down_over_png[];
extern const u8 Button_arrow_png[];
extern const u8 Button_arrow_over_png[];
extern const u8 Button_digit_png[];
extern const u8 Button_digit_over_png[];

/* Generic images*/
#ifdef HW_RVL
#define Key_A_png Key_A_wii_png
#define Key_B_png Key_B_wii_png
extern const u8 generic_point_png[];
extern const u8 Key_A_wii_png[];
extern const u8 Key_B_wii_png[];
#else
#define Key_A_png Key_A_gcn_png
#define Key_B_png Key_B_gcn_png
extern const u8 Key_A_gcn_png[];
extern const u8 Key_B_gcn_png[];
#endif
extern const u8 Star_full_png[];
extern const u8 Star_empty_png[];
extern const u8 Overlay_bar_png[];

/* Generic Sounds */
extern const u8 button_over_pcm[];
extern const u8 button_select_pcm[];
extern const u8 intro_pcm[];
extern const u32 button_select_pcm_size;
extern const u32 button_over_pcm_size;
extern const u32 intro_pcm_size;

extern u8 SILENT;

extern void GUI_InitMenu(gui_menu *menu);
extern void GUI_DeleteMenu(gui_menu *menu);
extern void GUI_DrawMenu(gui_menu *menu);
extern void GUI_DrawMenuFX(gui_menu *menu, u8 speed, u8 out);
extern void GUI_SlideMenuTitle(gui_menu *m, int title_offset);
extern int GUI_UpdateMenu(gui_menu *menu);
extern int GUI_RunMenu(gui_menu *menu);
extern void GUI_TextWindow(gui_menu *parent, char *title, char items[][64], u8 nb_items, u8 fontsize);
extern int GUI_OptionWindow(gui_menu *parent, char *title, char *infos, char *items[], u8 nb_items);
extern void GUI_OptionBox(gui_menu *parent, optioncallback cb, char *title, void *option, float step, float min, float max, u8 type);
extern void GUI_OptionBox2(gui_menu *parent, char *text_1, char *text_2, s16 *option_1, s16 *option_2, s16 step, s16 min, s16 max);
extern void GUI_MsgBoxOpen(char *title, char *msg, bool throbber);
extern void GUI_MsgBoxUpdate(char *title, char *msg);
extern void GUI_MsgBoxClose(void);
extern void GUI_WaitPrompt(char *title, char *msg);
extern int GUI_WaitConfirm(char *title, char *msg);
extern void GUI_FadeOut();
extern GXColor *GUI_GetBgColor(void);
extern void GUI_SetBgColor(u8 color);

#endif
