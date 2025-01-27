/*****************************************************************************
 * font.c
 *
 *   IPL font engine (using GX rendering)
 *
 *  Copyright Eke-Eke (2009-2016)
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


#include <iostream>

#include "shared.h"
#include "font.h"

#include "gettext.h"
#include "FreeTypeGX.h"
#include "font_zh_ttf.h"

using namespace std;

extern "C" {
    extern FreeTypeGX *fontSystemOne;
}

#define _SHIFTR(v, s, w)	\
    ((u32)(((u32)(v) >> (s)) & ((0x01 << (w)) - 1)))

typedef struct _yay0header {
	unsigned int id ATTRIBUTE_PACKED;
	unsigned int dec_size ATTRIBUTE_PACKED;
	unsigned int links_offset ATTRIBUTE_PACKED;
	unsigned int chunks_offset ATTRIBUTE_PACKED;
} yay0header;

static u8 *fontImage;
static u8 *fontTexture;
static GXTexObj fontTexObj;
static void *ipl_fontarea;
static sys_fontheader *fontHeader;
static u8 font_size[256];
static u16 style = FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE;

#ifndef HW_RVL

/* disable Qoob Modchip before IPL access (emukiddid) */
static void ipl_set_config(unsigned char c)
{
  volatile unsigned long* exi = (volatile unsigned long*)0xCC006800;
  unsigned long val,addr;
  addr=0xc0000000;
  val = c << 24;
  exi[0] = ((((exi[0]) & 0x405) | 256) | 48);     //select IPL
  //write addr of IPL
  exi[0 * 5 + 4] = addr;
  exi[0 * 5 + 3] = ((4 - 1) << 4) | (1 << 2) | 1;
  while (exi[0 * 5 + 3] & 1);
  //write the ipl we want to send
  exi[0 * 5 + 4] = val;
  exi[0 * 5 + 3] = ((4 - 1) << 4) | (1 << 2) | 1;
  while (exi[0 * 5 + 3] & 1);
  exi[0] &= 0x405;        //deselect IPL
}

#endif

static void decode_szp(void *src,void *dest)
{
	u32 i,k,link;
	u8 *dest8,*tmp;
	u32 loff,coff,roff;
	u32 size,cnt,cmask,bcnt;
	yay0header *header;

	dest8 = (u8*)dest;
	header = (yay0header*)src;
	size = header->dec_size;
	loff = header->links_offset;
	coff = header->chunks_offset;

	roff = sizeof(yay0header);
	cmask = 0;
	cnt = 0;
	bcnt = 0;

	do {
		if(!bcnt) {
			cmask = *(u32*)(src+roff);
			roff += 4;
			bcnt = 32;
		}

		if(cmask&0x80000000) {
			dest8[cnt++] = *(u8*)(src+coff);
			coff++;
		} else {
			link = *(u16*)(src+loff);
			loff += 2;

			tmp = dest8+(cnt-(link&0x0fff)-1);
			k = link>>12;
			if(k==0) {
				k = (*(u8*)(src+coff))+18;
				coff++;
			} else k += 2;

			for(i=0;i<k;i++) {
				dest8[cnt++] = tmp[i];
			}
		}
		cmask <<= 1;
		bcnt--;
	} while(cnt<size);
}

static void expand_font(u8 *src,u8 *dest)
{
	s32 cnt;
	u32 idx;
	u8 val1,val2;
  sys_fontheader *sys_fontdata = fontHeader;
	u8 *data = (u8*)sys_fontdata+44;

	if(sys_fontdata->sheet_format==0x0000) {
		cnt = (sys_fontdata->sheet_fullsize/2)-1;

		while(cnt>=0) {
			idx = _SHIFTR(src[cnt],6,2);
			val1 = data[idx];

			idx = _SHIFTR(src[cnt],4,2);
			val2 = data[idx];

			dest[(cnt<<1)+0] =((val1&0xf0)|(val2&0x0f));

			idx = _SHIFTR(src[cnt],2,2);
			val1 = data[idx];

			idx = _SHIFTR(src[cnt],0,2);
			val2 = data[idx];

			dest[(cnt<<1)+1] =((val1&0xf0)|(val2&0x0f));

			cnt--;
		}
	}
	DCStoreRange(dest,sys_fontdata->sheet_fullsize);
}

static void GetFontTexel(s32 c,void *image,s32 pos,s32 stride)
{
	u32 sheets,rem;
	u32 xoff,yoff;
	u32 xpos,ypos;
	u8 *img_start;
	u8 *ptr1,*ptr2;
  sys_fontheader *sys_fontdata = fontHeader;

	if(c<sys_fontdata->first_char || c>sys_fontdata->last_char) c = sys_fontdata->inval_char;
	else c -= sys_fontdata->first_char;

	sheets = sys_fontdata->sheet_column*sys_fontdata->sheet_row;
	rem = c%sheets;
  sheets = c/sheets;
	xoff = (rem%sys_fontdata->sheet_column)*sys_fontdata->cell_width;
	yoff = (rem/sys_fontdata->sheet_column)*sys_fontdata->cell_height;
	img_start = fontImage+(sys_fontdata->sheet_size*sheets);

	ypos = 0;
	while(ypos<sys_fontdata->cell_height) {
		xpos = 0;
		while(xpos<sys_fontdata->cell_width) {
			ptr1 = img_start+(((sys_fontdata->sheet_width/8)<<5)*((ypos+yoff)/8));
			ptr1 = ptr1+(((xpos+xoff)/8)<<5);
			ptr1 = ptr1+(((ypos+yoff)%8)<<2);
			ptr1 = ptr1+(((xpos+xoff)%8)/2);

			ptr2 = (u8 *)(image+((ypos/8)*(((stride<<1)/8)<<5)));
			ptr2 = ptr2+(((xpos+pos)/8)<<5);
			ptr2 = ptr2+(((xpos+pos)%8)/2);
			ptr2 = ptr2+((ypos%8)<<2);

			*ptr2 = *ptr1;

			xpos += 2;
		}
		ypos++;
	}
}

void DrawChar(unsigned char c, int xpos, int ypos, int size, GXColor color)
{
  /* get font texture data */
  memset(fontTexture,0,fontHeader->cell_width * fontHeader->cell_height / 2);
  GetFontTexel(c,fontTexture,0,fontHeader->cell_width/2);
  DCStoreRange(fontTexture, fontHeader->cell_width * fontHeader->cell_height / 2);
  GX_InvalidateTexAll();

  /* adjust texture width */
  s32 width = (fontHeader->cell_width * size * vmode->fbWidth) / (fontHeader->cell_height * vmode->viWidth);

  /* adjust texture height */
  size = (size * vmode->efbHeight) / 480;

  /* GX rendering */
  GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
  GX_Position2s16(xpos, ypos - size);
  GX_Color4u8(color.r, color.g, color.b, 0xff);
  GX_TexCoord2f32(0.0, 0.0);
  GX_Position2s16(xpos + width, ypos - size);
  GX_Color4u8(color.r, color.g, color.b, 0xff);
  GX_TexCoord2f32(1.0, 0.0);
  GX_Position2s16(xpos + width, ypos);
  GX_Color4u8(color.r, color.g, color.b, 0xff);
  GX_TexCoord2f32(1.0, 1.0);
  GX_Position2s16(xpos, ypos);
  GX_Color4u8(color.r, color.g, color.b, 0xff);
  GX_TexCoord2f32(0.0, 1.0);
  GX_End();
  GX_DrawDone();
}

/****************************************************************************
 *  IPL font support
 *
 ****************************************************************************/
extern "C" void __SYS_ReadROM(void *buf,u32 len,u32 offset);

extern "C" int  FONT_Init(void);
int FONT_Init(void)
{
#ifndef HW_RVL
  /* --- Game Cube --- disable Qoob before accessing IPL */
  ipl_set_config(6);
#endif

  /* read IPL font (ASCII) from Mask ROM */
  ipl_fontarea = memalign(32,131360);
  if (!ipl_fontarea)
    return 0;
  memset(ipl_fontarea,0,131360);
  __SYS_ReadROM(ipl_fontarea+119072,12288,0x1FCF00);

  /* YAY0 decompression */
  decode_szp(ipl_fontarea+119072,ipl_fontarea);

	/* retrieve IPL font data */
  fontHeader = (sys_fontheader*)ipl_fontarea;
  fontImage = (u8*)((((u32)ipl_fontarea+fontHeader->sheet_image)+31)&~31);

  /* expand to I4 format */
  expand_font((u8*)ipl_fontarea+fontHeader->sheet_image,fontImage);

  /* character width table */
  int i,c;
  for (i=0; i<256; ++i)
  {
    if ((i < fontHeader->first_char) || (i > fontHeader->last_char))
      c = fontHeader->inval_char;
    else
      c = i - fontHeader->first_char;

    font_size[i] = ((u8*)fontHeader)[fontHeader->width_table + c];
  }

  /* initialize texture data */
  fontTexture = (u8 *)(memalign(32, fontHeader->cell_width * fontHeader->cell_height / 2));
  if (!fontTexture)
  {
    free(ipl_fontarea);
    return 0;
  }

  /* initialize texture object */
  GX_InitTexObj(&fontTexObj, fontTexture, fontHeader->cell_width, fontHeader->cell_height, GX_TF_I4, GX_CLAMP, GX_CLAMP, GX_FALSE);

  return 1;
}

extern "C" void FONT_Shutdown(void);
void FONT_Shutdown(void)
{
  if (fontHeader)
    free(ipl_fontarea);
  if (fontTexture)
    free(fontTexture);

    if (fontSystemOne)
    {
        delete fontSystemOne;
    }
}

extern "C" int  FONT_writeW(wchar_t *string, int size, int x, int y, int max_width, GXColor color);
int FONT_writeW(wchar_t *string, int size, int x, int y, int max_width, GXColor color)
{
    x -= (vmode->fbWidth / 2);
    y -= (vmode->efbHeight / 2);
    if (!fontSystemOne)
    {
        fontSystemOne = new FreeTypeGX();
        fontSystemOne->loadFont(font_zh_ttf, font_zh_ttf_size, size, false);
    }
    else if (size != fontSystemOne->getFontSize())
    {
        fontSystemOne->loadFont(font_zh_ttf, font_zh_ttf_size, size, false);
    }

    if (max_width > 0)
    {
        fontSystemOne->setMaxVideoWidth(x + max_width);
    }
    else
    {
        fontSystemOne->setMaxVideoWidth(0);
    }

    return fontSystemOne->drawText(x, y, string, color, (u16)(FTGX_ALIGN_MASK | FTGX_JUSTIFY_MASK));
}

extern "C" int  FONT_write(char *string, int size, int x, int y, int max_width, GXColor color);
int FONT_write(char *string, int size, int x, int y, int max_width, GXColor color)
{
    x -= (vmode->fbWidth / 2);
    y -= (vmode->efbHeight / 2);
    if (!fontSystemOne)
    {
        fontSystemOne = new FreeTypeGX();
        fontSystemOne->loadFont(font_zh_ttf, font_zh_ttf_size, size, false);
    }
    else if (size != fontSystemOne->getFontSize())
    {
        fontSystemOne->loadFont(font_zh_ttf, font_zh_ttf_size, size, false);
    }

    if (max_width > 0)
    {
        fontSystemOne->setMaxVideoWidth(x + max_width);
    }
    else
    {
        fontSystemOne->setMaxVideoWidth(0);
    }

    char *utf8Txt = (char *)(gettext(string));
    char delims[] = "\n";
    char *result = NULL;
    result = strtok(utf8Txt, delims);

    while (result != NULL )
    {
        wchar_t* strWChar = fontSystemOne->charToWideChar(result);
        int notPrintLen = fontSystemOne->drawText(x, y, strWChar, color, (u16)(FTGX_ALIGN_MASK | FTGX_JUSTIFY_MASK));
        fontSystemOne->freeWideCharBuf(strWChar);
        //int notPrintLen = FONT_writeW(fontSystemOne->charToWideChar(result), size, x, y, max_width, color);
        if (notPrintLen > 0)
        {
            return notPrintLen;
        }
        result = strtok(NULL, delims);
        y += size;
    }

//  int w, ox;
//
//  GX_LoadTexObj(&fontTexObj, GX_TEXMAP0);
//
//  x -= (vmode->fbWidth / 2);
//  y -= (vmode->efbHeight / 2);
//
//  ox = x;
//
//  while (*string)
//  {
//    if (*string == '\n')
//    {
//      x = ox;
//      y += size;
//    }
//    else
//    {
//      w = (font_size[(u8)*string] * size * vmode->fbWidth) / (fontHeader->cell_height * vmode->viWidth);
//      if ((x + w) > (ox + max_width)) return strlen(string);
//      DrawChar(*string, x, y, size,color);
//      x += w;
//    }
//    string++;
//  }

  return 0;
}

extern "C" int  FONT_writeCenter(char *string, int size, int x1, int x2, int y, GXColor color);
int FONT_writeCenter(char *string, int size, int x1, int x2, int y, GXColor color)
{
    int x;
    int w = 0;
    y  -= (vmode->efbHeight / 2);

    if (!fontSystemOne)
    {
        fontSystemOne = new FreeTypeGX();
        fontSystemOne->loadFont(font_zh_ttf, font_zh_ttf_size, size, false);
    }
    else if (size != fontSystemOne->getFontSize())
    {
        fontSystemOne->loadFont(font_zh_ttf, font_zh_ttf_size, size, false);
    }

    char *utf8Txt = (char *)(gettext(string));
    char delims[] = "\n";
    char *result = NULL;
    result = strtok(utf8Txt, delims);

    fontSystemOne->setMaxVideoWidth(0);

    while (result != NULL )
    {
        wchar_t *text = fontSystemOne->charToWideChar(result);
        w = fontSystemOne->getWidth(text);

        if ((x1 + w) > x2)
        {
            w = x2 - x1;
        }
        x = x1 + (x2 - x1 - w - vmode->fbWidth) / 2;

        fontSystemOne->drawText(x, y, text, color, (u16)(FTGX_ALIGN_MASK | FTGX_JUSTIFY_MASK));
        fontSystemOne->freeWideCharBuf(text);

        result = strtok(NULL, delims);
        y += size;
    }

//  int x;
//  int i = 0;
//  int w = 0;
//
//  GX_LoadTexObj(&fontTexObj, GX_TEXMAP0);
//
//  while (string[i] && (string[i] != '\n'))
//  {
//    w += (font_size[(u8)string[i++]] * size * vmode->fbWidth) / (fontHeader->cell_height * vmode->viWidth);
//  }
//
//  if ((x1 + w) > x2) w = x2 - x1;
//  x = x1 + (x2 - x1 - w - vmode->fbWidth) / 2;
//  x2 -= (vmode->fbWidth / 2);
//  y  -= (vmode->efbHeight / 2);
//
//  while (*string && (*string != '\n'))
//  {
//    w = (font_size[(u8)*string] * size * vmode->fbWidth) / (fontHeader->cell_height * vmode->viWidth);
//    if ((x + w) > x2) return strlen(string);
//    DrawChar(*string, x, y, size,color);
//    x += w;
//    string++;
//  }
//
//  if (*string == '\n')
//  {
//    string++;
//    return FONT_writeCenter(string, size, x1, x2 + (vmode->fbWidth / 2), y + size + (vmode->efbHeight / 2), color);
//  }

  return 0;
}

extern "C" int  FONT_writeCenterOld(char *string, int size, int x1, int x2, int y, GXColor color);
int FONT_writeCenterOld(char *string, int size, int x1, int x2, int y, GXColor color)
{
  int x;
  int i = 0;
  int w = 0;

  GX_LoadTexObj(&fontTexObj, GX_TEXMAP0);

  while (string[i] && (string[i] != '\n'))
  {
    w += (font_size[(u8)string[i++]] * size * vmode->fbWidth) / (fontHeader->cell_height * vmode->viWidth);
  }

  if ((x1 + w) > x2) w = x2 - x1;
  x = x1 + (x2 - x1 - w - vmode->fbWidth) / 2;
  x2 -= (vmode->fbWidth / 2);
  y  -= (vmode->efbHeight / 2);

  while (*string && (*string != '\n'))
  {
    w = (font_size[(u8)*string] * size * vmode->fbWidth) / (fontHeader->cell_height * vmode->viWidth);
    if ((x + w) > x2) return strlen(string);
    DrawChar(*string, x, y, size,color);
    x += w;
    string++;
  }

  if (*string == '\n')
  {
    string++;
    return FONT_writeCenterOld(string, size, x1, x2 + (vmode->fbWidth / 2), y + size + (vmode->efbHeight / 2), color);
  }

  return 0;
}

extern "C" int  FONT_alignRight(char *string, int size, int x, int y, GXColor color);
int FONT_alignRight(char *string, int size, int x, int y, GXColor color)
{
    int w = 0;
    y -= (vmode->efbHeight / 2);

    if (!fontSystemOne)
    {
        fontSystemOne = new FreeTypeGX();
        fontSystemOne->loadFont(font_zh_ttf, font_zh_ttf_size, size, false);
    }
    else if (size != fontSystemOne->getFontSize())
    {
        fontSystemOne->loadFont(font_zh_ttf, font_zh_ttf_size, size, false);
    }

    const char *utf8Txt = gettext(string);
    wchar_t *text = fontSystemOne->charToWideChar(utf8Txt);
    w = fontSystemOne->getWidth(text);
    x -= (vmode->fbWidth / 2);
    x -= w;

    fontSystemOne->setMaxVideoWidth(0);

    fontSystemOne->drawText(x, y, text, color, (u16)(FTGX_ALIGN_MASK | FTGX_JUSTIFY_MASK));

    fontSystemOne->freeWideCharBuf(text);

//    try
//    {
//        if (!fontSystem[size])
//        {
//            fontSystem[size] = new FreeTypeGX(GX_TF_I4);
//            fontSystem[size]->loadFont(font_zh_ttf, font_zh_ttf_size, size, false);
//        }
//        fontSystem[size]->drawText(x, y, fontSystem[size]->charToWideChar(string), color, (u16)(FTGX_ALIGN_MASK | FTGX_JUSTIFY_MASK));
//    }
//    catch (const char* &e)
//    {
//        cout<<"init falure!"<<e<<endl;
//    }
//  int ox;
//  int i = 0;
//  int w = 0;
//
//  GX_LoadTexObj(&fontTexObj, GX_TEXMAP0);
//
//  while (string[i] && (string[i] != '\n'))
//  {
//    w += (font_size[(u8)string[i++]] * size * vmode->fbWidth) / (fontHeader->cell_height * vmode->viWidth);
//  }
//
//  x -= (vmode->fbWidth / 2);
//  y -= (vmode->efbHeight / 2);
//
//  ox = x;
//  x -= w;
//
//  while (*string && (*string != '\n'))
//  {
//    w = (font_size[(u8)*string] * size * vmode->fbWidth) / (fontHeader->cell_height * vmode->viWidth);
//    if ((x + w) > ox) return strlen(string);
//    DrawChar(*string, x, y, size,color);
//    x += w;
//    string++;
//  }
//
//  if (*string == '\n')
//  {
//    string++;
//    return FONT_alignRight(string, size, ox + (vmode->fbWidth / 2), y + size + (vmode->efbHeight / 2), color);
//  }

  return 0;
}

extern "C" char * JoinString(char *s1, char *s2);
char joinStr[100];
char * JoinString(char *s1, char *s2)
{
    joinStr[0] = '\0';
    const char *utf8Txt = gettext(s1);

    strcpy(joinStr, utf8Txt);
    strcat(joinStr, s2);

    return joinStr;
}
