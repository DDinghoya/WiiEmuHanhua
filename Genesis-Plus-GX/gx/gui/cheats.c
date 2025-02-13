/*
 *  cheats.c
 * 
 *  Genesis Plus GX Cheats menu
 *
 *  Copyright Eke-Eke (2010-2019)
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

#include "shared.h"
#include "file_load.h"
#include "cheats.h"
#include "font.h"
#include "gui.h"

#define BG_COLOR_1 {0x49,0x49,0x49,0xff}
#define BG_COLOR_2 {0x66,0x66,0x66,0xff}

#define MAX_CHEATS (350)
#define MAX_DESC_LENGTH (63)

#ifdef HW_RVL
extern const u8 Key_Minus_wii_png[];
extern const u8 Key_Plus_wii_png[];
#else
extern const u8 Key_R_gcn_png[];
extern const u8 Key_L_gcn_png[];
#endif
extern const u8 Key_DPAD_png[];

typedef struct 
{
  char code[12];
  char text[MAX_DESC_LENGTH];
  u8 enable;
  u16 data;
  u16 old;
  u32 address;
  u8 *prev;
} CHEATENTRY;

static int string_offset = 0;
static int selection = 0;
static int offset = 0;
static int type = 0;
static int maxcheats = 0;
static int maxROMcheats = 0;
static int maxRAMcheats = 0;

static CHEATENTRY cheatlist[MAX_CHEATS];
static u8 cheatIndexes[MAX_CHEATS];

static void cheatmenu_cb(void);

static gui_image star;
static gui_image bar_over;
static gui_image key_switch;
static gui_image key_enable;
static gui_image key_delete;

/*****************************************************************************/
/*  GUI Buttons data                                                         */
/*****************************************************************************/
static butn_data arrow_up_data =
{
  {NULL,NULL},
  {Button_up_png,Button_up_over_png}
};

static butn_data arrow_down_data =
{
  {NULL,NULL},
  {Button_down_png,Button_down_over_png}
};
static butn_data button_digit_data =
{
  {NULL,NULL},
  {Button_digit_png,Button_digit_over_png}
};

/*****************************************************************************/
/*  GUI Arrows button                                                        */
/*****************************************************************************/

static gui_butn arrow_up = {&arrow_up_data,BUTTON_VISIBLE|BUTTON_ACTIVE|BUTTON_OVER_SFX,{0,0,0,0},14,76,360,32};
static gui_butn arrow_down = {&arrow_down_data,BUTTON_VISIBLE|BUTTON_ACTIVE|BUTTON_OVER_SFX,{0,0,0,0},14,368,360,32};


/*****************************************************************************/
/*  GUI helpers                                                              */
/*****************************************************************************/
static gui_item action_cancel =
{
  NULL,Key_B_png,"","Exit",10,422,28,28
};

static gui_item action_select =
{
  NULL,Key_A_png,"","Edit Entry",602,422,28,28
};

/*****************************************************************************/
/*  GUI Background images                                                    */
/*****************************************************************************/
static gui_image bg_cheats[7] =
{
  {NULL,Bg_layer_png,IMAGE_VISIBLE|IMAGE_REPEAT,0,0,640,480,255},
  {NULL,Bg_overlay_png,IMAGE_VISIBLE|IMAGE_REPEAT,0,0,640,480,255},
  {NULL,Banner_top_png,IMAGE_VISIBLE|IMAGE_SLIDE_TOP,0,0,640,108,255},
  {NULL,Banner_bottom_png,IMAGE_VISIBLE|IMAGE_SLIDE_BOTTOM,0,380,640,100,255},
  {NULL,Main_logo_png,IMAGE_VISIBLE|IMAGE_SLIDE_TOP,466,40,152,44,255},
  {NULL,Frame_s1_png,IMAGE_VISIBLE,8,70,372,336,152},
  {NULL,Frame_s1_png,IMAGE_SLIDE_RIGHT,411,109,372,296,76},
};

/*****************************************************************************/
/*  Menu Items description                                                   */
/*****************************************************************************/
static gui_item items_cheats[30] =
{
  {NULL,NULL,"","Edit Entry",0,0,0,0},
  {NULL,NULL,"","Edit Entry",0,0,0,0},
  {NULL,NULL,"","Edit Entry",0,0,0,0},
  {NULL,NULL,"","Edit Entry",0,0,0,0},
  {NULL,NULL,"","Edit Entry",0,0,0,0},
  {NULL,NULL,"","Edit Entry",0,0,0,0},
  {NULL,NULL,"","Edit Entry",0,0,0,0},
  {NULL,NULL,"","Edit Entry",0,0,0,0},
  {NULL,NULL,"","Edit Entry",0,0,0,0},
  {NULL,NULL,"","Edit Entry",0,0,0,0},
  {NULL,NULL,"0","Add Character"  ,440,136,40,40},
  {NULL,NULL,"1","Add Character"  ,486,136,40,40},
  {NULL,NULL,"2","Add Character"  ,532,136,40,40},
  {NULL,NULL,"3","Add Character"  ,578,136,40,40},
  {NULL,NULL,"4","Add Character"  ,440,182,40,40},
  {NULL,NULL,"5","Add Character"  ,486,182,40,40},
  {NULL,NULL,"6","Add Character"  ,532,182,40,40},
  {NULL,NULL,"7","Add Character"  ,578,182,40,40},
  {NULL,NULL,"8","Add Character"  ,440,228,40,40},
  {NULL,NULL,"9","Add Character"  ,486,228,40,40},
  {NULL,NULL,"A","Add Character"  ,532,228,40,40},
  {NULL,NULL,"B","Add Character"  ,578,228,40,40},
  {NULL,NULL,"C","Add Character"  ,440,274,40,40},
  {NULL,NULL,"D","Add Character"  ,486,274,40,40},
  {NULL,NULL,"E","Add Character"  ,532,274,40,40},
  {NULL,NULL,"F","Add Character"  ,578,274,40,40},
  {NULL,NULL,"del","Backspace"    ,440,338,40,40},
  {NULL,NULL,":","Add Separator"  ,486,338,40,40},
  {NULL,NULL,"+","Next Characters",532,338,40,40},
  {NULL,NULL,"ok","Save Entry"    ,578,338,40,40}
};

/*****************************************************************************/
/*  Menu Buttons description                                                 */
/*****************************************************************************/
static gui_butn buttons_cheats[30] =
{
  {NULL,  BUTTON_VISIBLE|BUTTON_ACTIVE|BUTTON_SELECT_SFX|BUTTON_OVER_SFX,{1,0,0,0},15,108,358,26},
  {NULL,  BUTTON_VISIBLE|BUTTON_SELECT_SFX|BUTTON_OVER_SFX,{1,0,0,0},15,134,358,26},
  {NULL,  BUTTON_VISIBLE|BUTTON_SELECT_SFX|BUTTON_OVER_SFX,{1,0,0,0},15,160,358,26},
  {NULL,  BUTTON_VISIBLE|BUTTON_SELECT_SFX|BUTTON_OVER_SFX,{1,0,0,0},15,186,358,26},
  {NULL,  BUTTON_VISIBLE|BUTTON_SELECT_SFX|BUTTON_OVER_SFX,{1,0,0,0},15,212,358,26},
  {NULL,  BUTTON_VISIBLE|BUTTON_SELECT_SFX|BUTTON_OVER_SFX,{1,0,0,0},15,238,358,26},
  {NULL,  BUTTON_VISIBLE|BUTTON_SELECT_SFX|BUTTON_OVER_SFX,{1,0,0,0},15,264,358,26},
  {NULL,  BUTTON_VISIBLE|BUTTON_SELECT_SFX|BUTTON_OVER_SFX,{1,0,0,0},15,290,358,26},
  {NULL,  BUTTON_VISIBLE|BUTTON_SELECT_SFX|BUTTON_OVER_SFX,{1,0,0,0},15,316,358,26},
  {NULL,  BUTTON_VISIBLE|BUTTON_SELECT_SFX|BUTTON_OVER_SFX,{1,0,0,0},15,342,358,26},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{0,4,0,1},440,136,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{0,4,1,1},486,136,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{0,4,1,1},532,136,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{0,4,1,0},578,136,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{4,4,0,1},440,182,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{4,4,1,1},486,182,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{4,4,1,1},532,182,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{4,4,1,0},578,182,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{4,4,0,1},440,228,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{4,4,1,1},486,228,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{4,4,1,1},532,228,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{4,4,1,0},578,228,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{4,4,0,1},440,274,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{4,4,1,1},486,274,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{4,4,1,1},532,274,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{4,4,1,0},578,274,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{4,0,0,1},440,338,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{4,0,1,1},486,338,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{4,0,1,1},532,338,40,40},
  {&button_digit_data  ,BUTTON_ACTIVE|BUTTON_SLIDE_RIGHT|BUTTON_OVER_SFX,{4,0,1,0},578,338,40,40}
};

/*****************************************************************************/
/*  Menu description                                                         */
/*****************************************************************************/
static gui_menu menu_cheats =
{
  "Cheats Manager",
  0,0,
  30,30,7,0,
  items_cheats,
  buttons_cheats,
  bg_cheats,
  {&action_cancel, &action_select},
  {&arrow_up,&arrow_down},
  cheatmenu_cb
};

static char ggvalidchars[] = "ABCDEFGHJKLMNPRSTVWXYZ0123456789";

static char arvalidchars[] = "0123456789ABCDEF";

static u32 decode_cheat(char *string, int index)
{
  char *p;
  int i,n;
  u32 len = 0;
  u32 address = 0;
  u16 data = 0;
  u8 ref = 0;

  /* 16-bit Game Genie code (ABCD-EFGH) */
  if ((strlen(string) >= 9) && (string[4] == '-'))
  {
    /* 16-bit system only */
    if ((system_hw & SYSTEM_PBC) != SYSTEM_MD)
    {
      return 0;
    }

    for (i = 0; i < 8; i++)
    {
      if (i == 4) string++;
      p = strchr (ggvalidchars, *string++);
      if (p == NULL) return 0;
      n = p - ggvalidchars;

      switch (i)
      {
        case 0:
        data |= n << 3;
        break;

        case 1:
        data |= n >> 2;
        address |= (n & 3) << 14;
        break;

        case 2:
        address |= n << 9;
        break;

        case 3:
        address |= (n & 0xF) << 20 | (n >> 4) << 8;
        break;
    
        case 4:
        data |= (n & 1) << 12;
        address |= (n >> 1) << 16;
        break;

        case 5:
        data |= (n & 1) << 15 | (n >> 1) << 8;
        break;

        case 6:
        data |= (n >> 3) << 13;
        address |= (n & 7) << 5;
        break;

        case 7:
        address |= n;
        break;
      }
    }

    /* code length */
    len = 9;
  }

  /* 8-bit Game Genie code (DDA-AAA-XXX) */
  else if ((strlen(string) >= 11) && (string[3] == '-') && (string[7] == '-'))
  {
    /* 8-bit system only */
    if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
    {
      return 0;
    }

    /* decode 8-bit data */
    for (i=0; i<2; i++)
    {
      p = strchr (arvalidchars, *string++);
      if (p == NULL) return 0;
      n = (p - arvalidchars) & 0xF;
      data |= (n  << ((1 - i) * 4));
    }

    /* decode 16-bit address (low 12-bits) */
    for (i=0; i<3; i++)
    {
      if (i==1) string++; /* skip separator */
      p = strchr (arvalidchars, *string++);
      if (p == NULL) return 0;
      n = (p - arvalidchars) & 0xF;
      address |= (n  << ((2 - i) * 4));
    }

    /* decode 16-bit address (high 4-bits) */
    p = strchr (arvalidchars, *string++);
    if (p == NULL) return 0;
    n = (p - arvalidchars) & 0xF;
    n ^= 0xF; /* bits inversion */
    address |= (n  << 12);

    /* RAM address are also supported */
    if (address >= 0xC000)
    {
      /* convert to 24-bit Work RAM address */
      address = 0xFF0000 | (address & 0x1FFF);
    }

    /* decode reference 8-bit data */
    for (i=0; i<2; i++)
    {
      string++; /* skip separator and 2nd digit */
      p = strchr (arvalidchars, *string++);
      if (p == NULL) return 0;
      n = (p - arvalidchars) & 0xF;
      ref |= (n  << ((1 - i) * 4));
    }
    ref = (ref >> 2) | ((ref & 0x03) << 6);  /* 2-bit right rotation */
    ref ^= 0xBA;  /* XOR */

    /* update old data value */
    cheatlist[index].old = ref;

    /* code length */
    len = 11;
  }

  /* Action Replay code */
  else if (string[6] == ':')
  {
    if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
    {
      /* 16-bit code (AAAAAA:DDDD) */
      if (strlen(string) < 11) return 0;

      /* decode 24-bit address */
      for (i=0; i<6; i++)
      {
        p = strchr (arvalidchars, *string++);
        if (p == NULL) return 0;
        n = (p - arvalidchars) & 0xF;
        address |= (n << ((5 - i) * 4));
      }

      /* decode 16-bit data */
      string++;
      for (i=0; i<4; i++)
      {
        p = strchr (arvalidchars, *string++);
        if (p == NULL) return 0;
        n = (p - arvalidchars) & 0xF;
        data |= (n << ((3 - i) * 4));
      }

      /* code length */
      len = 11;
    }
    else
    {
      /* 8-bit code (xxAAAA:DD) */
      if (strlen(string) < 9) return 0;

      /* decode 16-bit address */
      string+=2;
      for (i=0; i<4; i++)
      {
        p = strchr (arvalidchars, *string++);
        if (p == NULL) return 0;
        n = (p - arvalidchars) & 0xF;
        address |= (n << ((3 - i) * 4));
      }

      /* ROM addresses are not supported */
      if (address < 0xC000) return 0;

      /* convert to 24-bit Work RAM address */
      address = 0xFF0000 | (address & 0x1FFF);

      /* decode 8-bit data */
      string++;
      for (i=0; i<2; i++)
      {
        p = strchr (arvalidchars, *string++);
        if (p == NULL) return 0;
        n = (p - arvalidchars) & 0xF;
        data |= (n  << ((1 - i) * 4));
      }

      /* code length */
      len = 9;
    }
  }

  /* Valid code found ? */
  if (len)
  {
    /* update cheat address & data values */
    cheatlist[index].address = address;
    cheatlist[index].data = data;
  }

  /* return code length (0 = invalid) */
  return len;
}

static void apply_cheats(void)
{
  u8 *ptr;
  
  /* clear ROM&RAM patches counter */
  maxROMcheats = maxRAMcheats = 0;

  int i;
  for (i = 0; i < maxcheats; i++)
  {
    if (cheatlist[i].enable)
    {
      if (cheatlist[i].address < cart.romsize)
      {
        if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
        {
          /* patch ROM data */
          cheatlist[i].old = *(u16 *)(cart.rom + (cheatlist[i].address & 0xFFFFFE));
          *(u16 *)(cart.rom + (cheatlist[i].address & 0xFFFFFE)) = cheatlist[i].data;
        }
        else
        {
          /* add ROM patch */
          maxROMcheats++;
          cheatIndexes[MAX_CHEATS - maxROMcheats] = i;

          /* get current banked ROM address */
          ptr = &z80_readmap[(cheatlist[i].address) >> 10][cheatlist[i].address & 0x03FF];

          /* check if reference matches original ROM data */
          if (((u8)cheatlist[i].old) == *ptr)
          {
            /* patch data */
            *ptr = cheatlist[i].data;

            /* save patched ROM address */
            cheatlist[i].prev = ptr;
          }
          else
          {
            /* no patched ROM address yet */
            cheatlist[i].prev = NULL;
          }
        }
      }
      else if (cheatlist[i].address >= 0xFF0000)
      {
        /* add RAM patch */
        cheatIndexes[maxRAMcheats++] = i;
      }
    }
  }
}

static void clear_cheats(void)
{
  int i = maxcheats;

  /* disable cheats in reversed order in case the same address is used by multiple patches */
  while (i > 0)
  {
    if (cheatlist[i-1].enable)
    {
      if (cheatlist[i-1].address < cart.romsize)
      {
        if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
        {
          /* restore original ROM data */
          *(u16 *)(cart.rom + (cheatlist[i-1].address & 0xFFFFFE)) = cheatlist[i-1].old;
        }
        else
        {
          /* check if previous banked ROM address has been patched */
          if (cheatlist[i-1].prev != NULL)
          {
            /* restore original data */
            *cheatlist[i-1].prev = cheatlist[i-1].old;

            /* no more patched ROM address */
            cheatlist[i-1].prev = NULL;
          }
        }
      }
    }

    i--;
  }
}

static void switch_chars(void)
{
  int i;
  gui_menu *m = &menu_cheats;

  if (m->items[10].text[0] == '0')
  {
    m->items[10].text[0] = 'G';
    m->items[11].text[0] = 'H';
    m->items[12].text[0] = 'J';
    m->items[13].text[0] = 'K';
    m->items[14].text[0] = 'L';
    m->items[15].text[0] = 'M';
    m->items[16].text[0] = 'N';
    m->items[17].text[0] = 'P';
    m->items[18].text[0] = 'R';
    m->items[19].text[0] = 'S';
    m->items[20].text[0] = 'T';
    m->items[21].text[0] = 'V';
    m->items[22].text[0] = 'W';
    m->items[23].text[0] = 'X';
    m->items[24].text[0] = 'Y';
    m->items[25].text[0] = 'Z';
  }
  else if (m->items[10].text[0] == 'G')
  {
    m->items[10].text[0] = '0';
    m->items[11].text[0] = '1';
    m->items[12].text[0] = '2';
    m->items[13].text[0] = '3';
    m->items[14].text[0] = '4';
    m->items[15].text[0] = '5';
    m->items[16].text[0] = '6';
    m->items[17].text[0] = '7';
    m->items[18].text[0] = '8';
    m->items[19].text[0] = '9';
    m->items[20].text[0] = 'A';
    m->items[21].text[0] = 'B';
    m->items[22].text[0] = 'C';
    m->items[23].text[0] = 'D';
    m->items[24].text[0] = 'E';
    m->items[25].text[0] = 'F';
  }
  else if (m->items[10].text[0] == 'A')
  {
    m->items[10].text[0] = 'Q';
    m->items[11].text[0] = 'R';
    m->items[12].text[0] = 'S';
    m->items[13].text[0] = 'T';
    m->items[14].text[0] = 'U';
    m->items[15].text[0] = 'V';
    m->items[16].text[0] = 'W';
    m->items[17].text[0] = 'X';
    m->items[18].text[0] = 'Y';
    m->items[19].text[0] = 'Z';
    m->items[20].text[0] = '0';
    m->items[21].text[0] = '1';
    m->items[22].text[0] = '2';
    m->items[23].text[0] = '3';
    m->items[24].text[0] = '4';
    m->items[25].text[0] = '5';
  }
  else if (m->items[10].text[0] == 'Q')
  {
    m->items[10].text[0] = '6';
    m->items[11].text[0] = '7';
    m->items[12].text[0] = '8';
    m->items[13].text[0] = '9';

    /* hide unused buttons */
    for (i=14; i<26; i++)
    {
      m->buttons[i].state &= ~BUTTON_VISIBLE;
    }
    m->buttons[10].shift[1] = 16;
    m->buttons[11].shift[1] = 16;
    m->buttons[12].shift[1] = 16;
    m->buttons[13].shift[1] = 16;
    m->buttons[26].shift[0] = 16;
    m->buttons[27].shift[0] = 16;
    m->buttons[28].shift[0] = 16;
    m->buttons[29].shift[0] = 16;
  }
  else if (m->items[10].text[0] == '6')
  {
    m->items[10].text[0] = 'A';
    m->items[11].text[0] = 'B';
    m->items[12].text[0] = 'C';
    m->items[13].text[0] = 'D';
    m->items[14].text[0] = 'E';
    m->items[15].text[0] = 'F';
    m->items[16].text[0] = 'G';
    m->items[17].text[0] = 'H';
    m->items[18].text[0] = 'I';
    m->items[19].text[0] = 'J';
    m->items[20].text[0] = 'K';
    m->items[21].text[0] = 'L';
    m->items[22].text[0] = 'M';
    m->items[23].text[0] = 'N';
    m->items[24].text[0] = 'O';
    m->items[25].text[0] = 'P';

    /* show previously unused buttons */
    for (i=14; i<26; i++)
    {
      m->buttons[i].state |= BUTTON_VISIBLE;
    }
    m->buttons[10].shift[1] = 4;
    m->buttons[11].shift[1] = 4;
    m->buttons[12].shift[1] = 4;
    m->buttons[13].shift[1] = 4;
    m->buttons[26].shift[0] = 4;
    m->buttons[27].shift[0] = 4;
    m->buttons[28].shift[0] = 4;
    m->buttons[29].shift[0] = 4;
  }
}

static void cheatmenu_cb(void)
{
  int i;
  int yoffset = 108;
  char temp[MAX_DESC_LENGTH];

  /* Draw browser array */
  gxDrawRectangle(15, 108, 358, 26, 127, (GXColor)BG_COLOR_1);
  gxDrawRectangle(15, 134, 358, 26, 127, (GXColor)BG_COLOR_2);
  gxDrawRectangle(15, 160, 358, 26, 127, (GXColor)BG_COLOR_1);
  gxDrawRectangle(15, 186, 358, 26, 127, (GXColor)BG_COLOR_2);
  gxDrawRectangle(15, 212, 358, 26, 127, (GXColor)BG_COLOR_1);
  gxDrawRectangle(15, 238, 358, 26, 127, (GXColor)BG_COLOR_2);
  gxDrawRectangle(15, 264, 358, 26, 127, (GXColor)BG_COLOR_1);
  gxDrawRectangle(15, 290, 358, 26, 127, (GXColor)BG_COLOR_2);
  gxDrawRectangle(15, 316, 358, 26, 127, (GXColor)BG_COLOR_1);
  gxDrawRectangle(15, 342, 358, 26, 127, (GXColor)BG_COLOR_2);

  /* Draw Cheat list */
  for (i=0; ((offset + i) < maxcheats) && (i < 10); i++)
  {
    if (i == selection)
    {
      /* selection bar */
      gxDrawTexture(bar_over.texture,16,yoffset+1,356,24,255);

      /* cheat description need to be specifically handled */
      if (type)
      {
        /* check if text is being edited */
        if (menu_cheats.bg_images[6].state & IMAGE_VISIBLE)
        {
          /* adjust offset so that last characters are visible */
          string_offset += FONT_writeCenter(cheatlist[offset + i].text+string_offset,16,40,366,yoffset+21,(GXColor)WHITE);
        }
        else
        {
          /* scroll text (speed = 1/10 frame) */
          if ((string_offset/10) >= strlen(cheatlist[offset + i].text))
          {
            string_offset = 0;
          }

          if (string_offset)
          {
            sprintf(temp,"%s ",cheatlist[offset + i].text+string_offset/10);
            strncat(temp, cheatlist[offset + i].text, string_offset/10);
          }
          else
          {
            strcpy(temp, cheatlist[offset + i].text);
          }

          if (FONT_writeCenter(temp,16,40,366,yoffset+21,(GXColor)WHITE))
          {
            /* scroll text if string does not fit */
            string_offset ++;
          }
        }
      }
      else
      {
        FONT_writeCenter(cheatlist[offset + i].code,18,40,366,yoffset+22,(GXColor)WHITE);
      }
    }
    else
    {
      if (type)
      {
        FONT_writeCenter(cheatlist[offset + i].text,16,40,366,yoffset+21,(GXColor)WHITE);
      }
      else
      {
        FONT_writeCenter(cheatlist[offset + i].code,18,40,366,yoffset+22,(GXColor)WHITE);
      }
    }

    /* draw cheat enable mark */
    if (cheatlist[offset + i].enable)
    {
      gxDrawTexture(star.texture,20,yoffset+5,16,16,255);
    }

    yoffset += 26;
  }

  /* New Entry */
  if (i < 10)
  {
    if (i == selection)
    {
      /* selection bar */
      gxDrawTexture(bar_over.texture,16,yoffset+1,356,24,255);

      /* check if new code is being edited */
      if (menu_cheats.bg_images[6].state & IMAGE_VISIBLE)
      {
        FONT_writeCenter(cheatlist[offset + selection].code,18,40,366,yoffset+22,(GXColor)WHITE);
      }
      else
      {
        FONT_writeCenter("New Code",18,40,366,yoffset+22,(GXColor)WHITE);
      }
    }
    else
    {
      FONT_writeCenter("New Code",18,40,366,yoffset+22,(GXColor)WHITE);
    }
  }

  /* Extra helpers */
  if (maxcheats && !(menu_cheats.bg_images[6].state & IMAGE_VISIBLE))
  {
    /* switch between cheat code & description preview */
#ifdef HW_RVL
    gxDrawTexture(key_switch.texture,268,424,24,24,255);
    FONT_write(type ? "View\nCode":"View\nText",16,300,436,640,(GXColor)WHITE);
#else
    gxDrawTexture(key_switch.texture,272,424,24,24,255);
    FONT_write(type ? "View\nCode":"View\nText",16,304,436,640,(GXColor)WHITE);
#endif

    /* delete & enable cheats */
    if ((offset + selection) < maxcheats)
    {
  #ifdef HW_RVL
      gxDrawTexture(key_enable.texture,152,424,24,24,255);
      gxDrawTexture(key_delete.texture,372,424,24,24,255);
      FONT_write(cheatlist[offset + selection].enable ? "Disable\nCheat":"Enable\nCheat",16,184,436,640,(GXColor)WHITE);
      FONT_write("Delete\nCheat",16,404,436,640,(GXColor)WHITE);
  #else
      gxDrawTexture(key_enable.texture,136,426,44,20,255);
      gxDrawTexture(key_delete.texture,368,426,44,20,255);
      FONT_write(cheatlist[offset + selection].enable ? "Disable\nCheat":"Enable\nCheat",16,188,436,640,(GXColor)WHITE);
      FONT_write("Delete\nCheat",16,420,436,640,(GXColor)WHITE);
  #endif
    }
  }
}


/****************************************************************************
 * CheatMenu
 *
 * Manage cheats for the currently loaded game
 *
 ****************************************************************************/ 
void CheatMenu(void)
{
  int i, update = 0;
  int digit_cnt = 0;
  int max = 0;
  char temp[MAXPATHLEN];
  char *str = NULL;
  gui_menu *m = &menu_cheats;

  /* clear existing ROM patches */
  clear_cheats();

  /* reset scrolling */
  string_offset = 0;

  /* background overlay */
  if (config.bg_overlay)
  {
    bg_cheats[1].state |= IMAGE_VISIBLE;
  }
  else
  {
    bg_cheats[1].state &= ~IMAGE_VISIBLE;
  }
  
  /* additional textures */
  star.texture = gxTextureOpenPNG(Star_full_png,0);
  bar_over.texture = gxTextureOpenPNG(Overlay_bar_png,0);
  key_switch.texture = gxTextureOpenPNG(Key_DPAD_png,0);
#ifdef HW_RVL
  key_enable.texture = gxTextureOpenPNG(Key_Plus_wii_png,0);
  key_delete.texture = gxTextureOpenPNG(Key_Minus_wii_png,0);
#else
  key_enable.texture = gxTextureOpenPNG(Key_L_gcn_png,0);
  key_delete.texture = gxTextureOpenPNG(Key_R_gcn_png,0);
#endif

  /* selected item */
  m->selected = selection;

  /* slide-in menu */
  GUI_InitMenu(m);
  GUI_DrawMenuFX(m,30,0);

  /* lock background elements */
  m->bg_images[2].state &= ~IMAGE_SLIDE_TOP;
  m->bg_images[3].state &= ~IMAGE_SLIDE_BOTTOM;
  m->bg_images[4].state &= ~IMAGE_SLIDE_TOP;

  while (update != -1)
  {
    /* update arrows buttons */
    if (offset > 0)
      m->arrows[0]->state |= BUTTON_VISIBLE;
    else
      m->arrows[0]->state &= ~BUTTON_VISIBLE;
    if (((offset + 10) < (maxcheats + 1)) && ((offset + 10) < MAX_CHEATS))
      m->arrows[1]->state |= BUTTON_VISIBLE;
    else
      m->arrows[1]->state &= ~BUTTON_VISIBLE;

    /* draw menu */
    GUI_DrawMenu(m);

    /* check if browsing cheats list */
    if (!(m->bg_images[6].state & IMAGE_VISIBLE))
    {
      /* restore scrolling list settings */
      m->offset = offset;
      m->max_items = (maxcheats < MAX_CHEATS) ? (maxcheats + 1) : MAX_CHEATS;
      m->max_buttons = 10;
      m->helpers[1] = NULL;

      /* check if arrow button is high-lighted */
      if (m->selected >= 30)
      {
        /* adjust selected button index */
        m->selected -= 20;
      }
    }

#ifdef HW_RVL
    if (Shutdown)
    {
      /* close additional textures */
      gxTextureClose(&star.texture);
      gxTextureClose(&bar_over.texture);
      gxTextureClose(&key_switch.texture);
      gxTextureClose(&key_enable.texture);
      gxTextureClose(&key_delete.texture);

      /* restore default GUI settings */
      m->max_items = m->max_buttons = 30;
      m->helpers[1] = &action_select;
    }
#endif

    /* update menu */
    update = GUI_UpdateMenu(m);

    /* check if browsing cheats list */
    if (!(m->bg_images[6].state & IMAGE_VISIBLE))
    {
      if (m->selected < 10)
      {
        /* update selected cheat */
        if (selection != m->selected)
        {
          selection = m->selected;
          string_offset = 0;
        }
      }
      else
      {
        /* arrow button is selected */
        m->selected += 20;
      }

      /* save cheats list offset */
      if (offset != m->offset)
      {
        offset = m->offset;
        string_offset = 0;
      }

      /* restore default GUI settings */
      m->offset = 0;
      m->max_items = m->max_buttons = 30;
      m->helpers[1] = &action_select;
    }


    /* handle pressed buttons */
    if (update > 0)
    {
      switch (m->selected)
      {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9: /* Edit cheat */
        {
          if (type && ((selection + offset) != maxcheats))
          {
            /* cheat description */
            str = cheatlist[offset + selection].text;
            strcpy(temp, str);
            max = MAX_DESC_LENGTH - 2;
            digit_cnt = strlen(str);
            if (digit_cnt <= max)
            {
              str[digit_cnt] = '*';
              str[digit_cnt+1] = 0;
            }

            /* init specific characters */
            m->items[10].text[0] = '6';
            m->items[27].text[0] = ' ';
            strcpy(m->items[27].comment,"Add White Space");
            switch_chars();
          }
          else
          {
            /* cheat code */
            str = cheatlist[offset + selection].code;
            strcpy(temp, str);
            if ((offset + selection) == maxcheats)
            {
              /* initialize code */
              max = 0;
              digit_cnt = 0;
              str[0] = '*';
              str[1] = 0;
            }
            else
            {
              /* code type */
              if (str[6] == ':')
              {
                /* Action Replay code */
                if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
                {
                  /* 16-bit code */
                  max = 10;
                }
                else
                {
                  /* 8-bit code */
                  max = 8;
                }
              }
              else if (str[4] == '-')
              {
                /* 16-bit Game Genie code */
                max = 8;
              }
              else
              {
                /* 8-bit Game Genie code */
                max = 10;
              }

              /* set cursor to end of code */
              digit_cnt = max + 1;
            }

            /* init specific characters */
            m->items[10].text[0] = 'G';
            m->items[27].text[0] = ':';
            strcpy(m->items[27].comment,"Add Code Separator");
            switch_chars();
          }

          /* show digit buttons */
          for (i=10; i<30; i++)
            m->buttons[i].state |= BUTTON_VISIBLE;

          /* show right window */
          m->bg_images[6].state |= IMAGE_VISIBLE;

          /* disable left buttons */
          for (i=0; i<10; i++)
            m->buttons[i].state &= ~BUTTON_ACTIVE;

          /* disable arrow buttons */
          m->arrows[0]->state &= ~BUTTON_ACTIVE;
          m->arrows[1]->state &= ~BUTTON_ACTIVE;

          /* slide in right window */
          GUI_DrawMenuFX(m,20,0);

          /* update helper */
          strcpy(action_cancel.comment,"Cancel");

          /* select first digit */
          m->selected = 10;
          
          /* reset scrolling */
          string_offset = 0;
          break;
        }

        case 26:  /* Backspace */
        {
          if (digit_cnt > 0)
          {
            /* delete last character */
            str[digit_cnt--] = 0;

            /* code separator is being deleted */
            if ((str[digit_cnt] == ':') || (str[digit_cnt] == '-'))
            {
              /* reset detected code type (except 8-bit Game Genie code using 2 separators) */
              if (((system_hw & SYSTEM_PBC) == SYSTEM_MD) || (digit_cnt != 7))
              {
                max = 0;
              }
            }

            /* edit mark */
            str[digit_cnt] = '*';

            /* update scroll value if necessary */
            if (string_offset > 0)
              string_offset--;
          }
          break;
        }

        case 27:
        {
          if (type && ((offset + selection) != maxcheats))
          {
            /* SPACE character */
            if (digit_cnt <= max)
            {
              str[digit_cnt++] = ' ';
              str[digit_cnt] = 0;
              if (digit_cnt <= max)
              {
                str[digit_cnt] = '*';
                str[digit_cnt+1] = 0;
              }
            }
          }
          else
          {
            /* Separator character */
            if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
            {
              /* 16-bit codes */
              if (digit_cnt == 4)
              {
                /* Game Genie code */
                max = 8;
                str[4] = '-';
              }
              else if ((digit_cnt == 6) && (max != 8))
              {
                /* Action Replay code */
                max = 10;
                str[6] = ':';
              }
              else
              {
                break;
              }
            }
            else
            {
              /* 8-bit codes */
              if (digit_cnt == 3)
              {
                /* Game Genie code */
                max = 10;
                str[3] = '-';
              }
              else if ((digit_cnt == 7) && (max == 10))
              {
                /* Game Genie code (last part) */
                str[7] = '-';
              }
              else if ((digit_cnt == 6) && (max != 10))
              {
                /* Action Replay code */
                max = 8;
                str[6] = ':';
              }
              else
              {
                break;
              }
            }

            digit_cnt++;
            str[digit_cnt] = '*';
            str[digit_cnt+1] = 0;
          }
          break;
        }

        case 28:  /* Next character set */
        {
          GUI_DrawMenuFX(m,40,1);
          switch_chars();
          GUI_DrawMenuFX(m,40,0);
          break;
        }

        case 29:  /* Validate entry */
        {
          /* finalize cheat description */  
          if (type && ((offset + selection) != maxcheats))
          {
            str[digit_cnt] = 0;
            update = -1;
          }
          
          /* finalize cheat code edition */
          else if (max && (digit_cnt > max))
          {
            /* check if cheat code is valid */
            if (decode_cheat(cheatlist[offset + selection].code, offset + selection))
            {
              /* new cheat ? */
              if ((offset + selection) == maxcheats)
              {
                /* increase cheat count */
                maxcheats++;

                /* enable cheat by default */
                cheatlist[offset + selection].enable = 1;

                /* no description by default */
                strcpy(cheatlist[offset + selection].text,"No Description");
              }

              /* return to cheat selection */
              update = -1;
            }
            else
            {
              GUI_WaitPrompt("Error", "Invalid Cheat Code");
            }
          }
          break;
        }

        default:  /* Add Character */
        {
          /* force code separator if none has been set yet */
          if ((max == 0) && (digit_cnt == 6))
            break;

          /* force 8-bit Game Genie code last separator */
          if (((system_hw & SYSTEM_PBC) != SYSTEM_MD) && (max == 10) && (digit_cnt == 7))
            break;

          /* add character */
          if ((digit_cnt <= max) || (max == 0))
          {
            str[digit_cnt++] = m->items[m->selected].text[0];
            str[digit_cnt] = 0;
            if ((digit_cnt <= max) || (max == 0))
            {
              str[digit_cnt] = '*';
              str[digit_cnt+1] = 0;
            }

            if (string_offset > 0)
              string_offset ++;
          }
          break;
        }
      }
    }
    else if (update < 0)
    {
      /* cancel */
      if (m->bg_images[6].state & IMAGE_VISIBLE)
      {
        /* Restore old entry */
        strcpy(str, temp);
      }
    }
    else
    {
      /* check other buttons pressed while browsing cheats list */
      if (maxcheats && !(m->bg_images[6].state & IMAGE_VISIBLE))
      {
        if ((m_input.keys & PAD_BUTTON_LEFT) || (m_input.keys & PAD_BUTTON_RIGHT))
        {
          /* Switch between cheat code & description */
          type ^= 1;

          /* reset scrolling */
          string_offset = 0;
        }
        else if ((offset + selection) < maxcheats)
        {
          /* Delete selected cheat code*/
          if (m_input.keys & PAD_TRIGGER_R)
          {
            if (GUI_WaitConfirm("Warning","Delete Cheat Entry ?"))
            {
              /* shift cheat list up to selected entry */
              for (i = offset + selection + 1; i < maxcheats; i++)
              {
                strcpy(cheatlist[i-1].text,cheatlist[i].text);
                strcpy(cheatlist[i-1].code,cheatlist[i].code);
                cheatlist[i-1].address = cheatlist[i].address;
                cheatlist[i-1].data = cheatlist[i].data;
                cheatlist[i-1].enable = cheatlist[i].enable;
              }

              /* clear last cheat */
              cheatlist[maxcheats-1].text[0] = 0;
              cheatlist[maxcheats-1].code[0] = 0;
              cheatlist[maxcheats-1].address = 0;
              cheatlist[maxcheats-1].data = 0;
              cheatlist[maxcheats-1].enable = 0;

              /* adjust scrolling list */
              if (maxcheats < 10)
              {
                /* disable next button */
                m->buttons[maxcheats].state &= ~BUTTON_ACTIVE;
                m->buttons[maxcheats-1].shift[1] = 0;
              }
              else 
              {
                /* scroll down cheat list if there is less than 10 visible entries */
                if ((maxcheats < (offset + 10)) && (maxcheats < MAX_CHEATS))
                {
                  offset--;
                }
              }

              /* decrease cheat count */
              maxcheats--;

              /* reset scrolling */
              string_offset = 0;
            }
          }
          else if (m_input.keys & PAD_TRIGGER_L)
          {
            /* Enable/Disable selected cheat code */
            cheatlist[offset + selection].enable ^= 1;
          }
        }
      }
    }

    if (update < 0)
    {
      if (m->bg_images[6].state & IMAGE_VISIBLE)
      {
        /* slide out right window */
        GUI_DrawMenuFX(m,20,1);

        /* hide digit buttons */
        for (i=10; i<30; i++)
          m->buttons[i].state &= ~BUTTON_VISIBLE;

        /* hide right window */
        m->bg_images[6].state &= ~IMAGE_VISIBLE;

        /* enable left buttons */
        for (i=0; i<10; i++)
        {
          if (i < maxcheats)
          {
            menu_cheats.buttons[i].state |= BUTTON_ACTIVE;
            menu_cheats.buttons[i].shift[1] = 1;
          }
          else if (i == maxcheats)
          {
            menu_cheats.buttons[i].state |= BUTTON_ACTIVE;
            menu_cheats.buttons[i].shift[1] = 0;
          }
          else
          {
            menu_cheats.buttons[i].state &= ~BUTTON_ACTIVE;
            menu_cheats.buttons[i].shift[1] = 0;
          }
        }

        /* enable arrow buttons */
        m->arrows[0]->state |= BUTTON_ACTIVE;
        m->arrows[1]->state |= BUTTON_ACTIVE;

        /* restore helper */
        strcpy(action_cancel.comment,"Back");

        /* select current cheat */
        m->selected = selection;

        /* stay in menu */
        update = 0;
      }
    }
  }

  /* apply ROM patches */
  apply_cheats();

  /* save cheats to file */
  snprintf(temp, MAXPATHLEN, "%s/cheats/%s.pat", DEFAULT_PATH, rom_filename);

  if (maxcheats)
  {
    /* open file */
    FILE *f = fopen(temp, "w");

    /* write cheats */
    if (f)
    {
      for (i=0; i<maxcheats; i++)
      {
        fprintf(f, "%s\t%s\n", cheatlist[i].code, cheatlist[i].text);
        fprintf(f, "%s\n", cheatlist[i].enable ? "ON" : "OFF");
      }
      fclose(f);
    }
  }

  /* unlock background elements */
  m->bg_images[2].state |= IMAGE_SLIDE_TOP;
  m->bg_images[3].state |= IMAGE_SLIDE_BOTTOM;
  m->bg_images[4].state |= IMAGE_SLIDE_TOP;
  
  /* leave menu */
  GUI_DrawMenuFX(m,30,1);
  GUI_DeleteMenu(m);

  /* close additional textures */
  gxTextureClose(&star.texture);
  gxTextureClose(&bar_over.texture);
  gxTextureClose(&key_switch.texture);
  gxTextureClose(&key_enable.texture);
  gxTextureClose(&key_delete.texture);
}


/****************************************************************************
 * CheatLoad
 *
 * Load cheats from associated .pat file, called when loading a new game
 * ROM patches are automatically applied.
 * RAM patches are applied once per frame.
 *
 ****************************************************************************/ 
void CheatLoad(void)
{
  int len;
  int cnt = 0;
  char temp[MAXPATHLEN];

  /* reset cheat count */
  maxcheats = 0;
  
  /* make cheat filename */
  snprintf(temp, MAXPATHLEN, "%s/cheats/%s.pat", DEFAULT_PATH, rom_filename);

  /* open file */
  FILE *f = fopen(temp, "r");
  if (f)
  {
    /* clear string */
    memset(temp, 0, 256);

    /* read cheats from file (one line per cheat) */
    while (fgets(temp, 256, f) && (maxcheats < MAX_CHEATS) && (cnt < MAX_CHEATS))
    {
      /* remove CR & EOL chars */
      if ((temp[strlen(temp) - 2] == 0x0d) || (temp[strlen(temp) - 2] == 0x0a))
        temp[strlen(temp) - 2] = 0;
      else
        temp[strlen(temp) - 1] = 0;

      /* check cheat validty */
      len = decode_cheat(temp, maxcheats);

      if (len)
      {
        /* copy cheat code */
        strncpy(cheatlist[maxcheats].code, temp, len);
        cheatlist[maxcheats].code[len] = 0;
        len++;

        /* jump TAB and SPACE characters */
        while ((temp[len] == 0x20) || (temp[len] == 0x09))
         len++;

        /* copy cheat description */
        strncpy(cheatlist[maxcheats].text, &temp[len], MAX_DESC_LENGTH - 1);
        cheatlist[maxcheats].text[MAX_DESC_LENGTH - 1] = 0;

        /* increment cheat count */
        maxcheats++;
      }
      else if (!strcmp(temp,"ON") && config.autocheat)
      {
        /* enable flag */
        cheatlist[cnt++].enable = 1;
      }
      else if (!strcmp(temp,"OFF") && config.autocheat)
      {
        /* disable flag */
        cheatlist[cnt++].enable = 0;
      }
    }

    /* by default, disable cheats that were not flagged */
    while (cnt < maxcheats)
     cheatlist[cnt++].enable = 0;

    /* close file */
    fclose(f);
  }

  /* apply ROM patches */
  apply_cheats();

  /* adjust menu buttons */
  for (cnt=0; cnt<10; cnt++)
  {
    if (cnt < maxcheats)
    {
      menu_cheats.buttons[cnt].state |= BUTTON_ACTIVE;
      menu_cheats.buttons[cnt].shift[1] = 1;
    }
    else if (cnt == maxcheats)
    {
      menu_cheats.buttons[cnt].state |= BUTTON_ACTIVE;
      menu_cheats.buttons[cnt].shift[1] = 0;
    }
    else
    {
      menu_cheats.buttons[cnt].shift[1] = 0;
      menu_cheats.buttons[cnt].state &= ~BUTTON_ACTIVE;
    }
  }

  /* reset menu */
  selection = offset = 0;
}

/****************************************************************************
 * RAMCheatUpdate
 *
 * Apply RAM patches (this should be called once per frame)
 *
 ****************************************************************************/ 
void RAMCheatUpdate(void)
{
  int index, cnt = maxRAMcheats;
  
  while (cnt)
  {
    /* get cheat index */
    index = cheatIndexes[--cnt];

    /* apply RAM patch */
    if (cheatlist[index].data & 0xFF00)
    {
      /* word patch */
      *(u16 *)(work_ram + (cheatlist[index].address & 0xFFFE)) = cheatlist[index].data;
    }
    else
    {
      /* byte patch */
      work_ram[cheatlist[index].address & 0xFFFF] = cheatlist[index].data;
    }
  }
}


/****************************************************************************
 * ROMCheatUpdate
 *
 * Apply ROM patches (this should be called each time banking is changed)
 *
 ****************************************************************************/ 
void ROMCheatUpdate(void)
{
  int index, cnt = maxROMcheats;
  u8 *ptr;
  
  while (cnt)
  {
    /* get cheat index */
    index = cheatIndexes[MAX_CHEATS - cnt];

    /* check if previous banked ROM address was patched */
    if (cheatlist[index].prev != NULL)
    {
      /* restore original data */
      *cheatlist[index].prev = cheatlist[index].old;

      /* no more patched ROM address */
      cheatlist[index].prev = NULL;
    }

    /* get current banked ROM address */
    ptr = &z80_readmap[(cheatlist[index].address) >> 10][cheatlist[index].address & 0x03FF];

    /* check if reference matches original ROM data */
    if (((u8)cheatlist[index].old) == *ptr)
    {
      /* patch data */
      *ptr = cheatlist[index].data;

      /* save patched ROM address */
      cheatlist[index].prev = ptr;
    }

    /* next ROM patch */
    cnt--;
  }
}
