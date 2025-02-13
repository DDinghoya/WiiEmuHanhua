#define EMPTY_PALETTE_1 {0,0,0},
#define EMPTY_PALETTE_4 EMPTY_PALETTE_1 EMPTY_PALETTE_1 EMPTY_PALETTE_1 EMPTY_PALETTE_1
#define EMPTY_PALETTE_16 EMPTY_PALETTE_4 EMPTY_PALETTE_4 EMPTY_PALETTE_4 EMPTY_PALETTE_4
#define EMPTY_PALETTE_64 EMPTY_PALETTE_16 EMPTY_PALETTE_16 EMPTY_PALETTE_16 EMPTY_PALETTE_16
#define EMPTY_PALETTE_DEEMPH_X_7 EMPTY_PALETTE_64 EMPTY_PALETTE_64 EMPTY_PALETTE_64 EMPTY_PALETTE_64 EMPTY_PALETTE_64 EMPTY_PALETTE_64 EMPTY_PALETTE_64

pal rp2c04001[512] = {
 #include "rp2c04001.h"
	EMPTY_PALETTE_DEEMPH_X_7
};

pal rp2c04002[512] = {
 #include "rp2c04002.h"
	EMPTY_PALETTE_DEEMPH_X_7
};

pal rp2c04003[512] = {
 #include "rp2c04003.h"
	EMPTY_PALETTE_DEEMPH_X_7
};
pal rp2c05004[512] = {
 #include "rp2c05004.h"
	EMPTY_PALETTE_DEEMPH_X_7
};

// Fixed palette entries used by the GUI
pal palette_unvarying[] = {
{ 0x00<<2,0x00<<2,0x00<<2}, //  0 = Black
{ 0x3F<<2,0x3F<<2,0x34<<2}, //  1 = White
{ 0x00<<2,0x00<<2,0x00<<2}, //  2 = Black
{ 0x1d<<2,0x1d<<2,0x24<<2}, //  3 = Greyish
{ 190,  0,  0}, //  4 = Reddish
{  51,255, 51}, //  5 = Bright green
{  49, 14,200}, //  6 = Ultramarine Blue
// 16 saturated colors useful for Lua scripts
{   0,  0,  0}, //  7 = Black
{   0,  0,128}, //  8 = Dark Blue
{   0,128,  0}, //  9 = Dark Green
{   0,128,128}, // 10 = Dark Cyan
{ 128,  0,  0}, // 11 = Dark Red
{ 128,  0,128}, // 12 = Dark Magenta
{ 128,128,  0}, // 13 = Dark Yellow
{ 128,128,128}, // 14 = Light Grey
{  64, 64, 64}, // 15 = Dark Grey
{   0,  0,255}, // 16 = Bright Blue
{   0,255,  0}, // 17 = Bright Green
{   0,255,255}, // 18 = Bright Cyan
{ 255,  0,  0}, // 19 = Bright Red
{ 255,  0,255}, // 20 = Bright Magenta
{ 255,255,  0}, // 21 = Bright Yellow
{ 255,255,255}, // 22 = Bright White
};


// Default palette 
pal palette[512] = {

        { 0x1D<<2, 0x1D<<2, 0x1D<<2 }, /* Value 0 */
        { 0x09<<2, 0x06<<2, 0x23<<2 }, /* Value 1 */
        { 0x00<<2, 0x00<<2, 0x2A<<2 }, /* Value 2 */
        { 0x11<<2, 0x00<<2, 0x27<<2 }, /* Value 3 */
        { 0x23<<2, 0x00<<2, 0x1D<<2 }, /* Value 4 */
        { 0x2A<<2, 0x00<<2, 0x04<<2 }, /* Value 5 */
        { 0x29<<2, 0x00<<2, 0x00<<2 }, /* Value 6 */
        { 0x1F<<2, 0x02<<2, 0x00<<2 }, /* Value 7 */
        { 0x10<<2, 0x0B<<2, 0x00<<2 }, /* Value 8 */
        { 0x00<<2, 0x11<<2, 0x00<<2 }, /* Value 9 */
        { 0x00<<2, 0x14<<2, 0x00<<2 }, /* Value 10 */
        { 0x00<<2, 0x0F<<2, 0x05<<2 }, /* Value 11 */
        { 0x06<<2, 0x0F<<2, 0x17<<2 }, /* Value 12 */
        { 0x00<<2, 0x00<<2, 0x00<<2 }, /* Value 13 */
        { 0x00<<2, 0x00<<2, 0x00<<2 }, /* Value 14 */
        { 0x00<<2, 0x00<<2, 0x00<<2 }, /* Value 15 */
        { 0x2F<<2, 0x2F<<2, 0x2F<<2 }, /* Value 16 */
        { 0x00<<2, 0x1C<<2, 0x3B<<2 }, /* Value 17 */
        { 0x08<<2, 0x0E<<2, 0x3B<<2 }, /* Value 18 */
        { 0x20<<2, 0x00<<2, 0x3C<<2 }, /* Value 19 */
        { 0x2F<<2, 0x00<<2, 0x2F<<2 }, /* Value 20 */
        { 0x39<<2, 0x00<<2, 0x16<<2 }, /* Value 21 */
        { 0x36<<2, 0x0A<<2, 0x00<<2 }, /* Value 22 */
        { 0x32<<2, 0x13<<2, 0x03<<2 }, /* Value 23 */
        { 0x22<<2, 0x1C<<2, 0x00<<2 }, /* Value 24 */
        { 0x00<<2, 0x25<<2, 0x00<<2 }, /* Value 25 */
        { 0x00<<2, 0x2A<<2, 0x00<<2 }, /* Value 26 */
        { 0x00<<2, 0x24<<2, 0x0E<<2 }, /* Value 27 */
        { 0x00<<2, 0x20<<2, 0x22<<2 }, /* Value 28 */
        { 0x00<<2, 0x00<<2, 0x00<<2 }, /* Value 29 */
        { 0x00<<2, 0x00<<2, 0x00<<2 }, /* Value 30 */
        { 0x00<<2, 0x00<<2, 0x00<<2 }, /* Value 31 */
        { 0x3F<<2, 0x3F<<2, 0x3F<<2 }, /* Value 32 */
        { 0x0F<<2, 0x2F<<2, 0x3F<<2 }, /* Value 33 */
        { 0x17<<2, 0x25<<2, 0x3F<<2 }, /* Value 34 */
        { 0x33<<2, 0x22<<2, 0x3F<<2 }, /* Value 35 */
        { 0x3D<<2, 0x1E<<2, 0x3F<<2 }, /* Value 36 */
        { 0x3F<<2, 0x1D<<2, 0x2D<<2 }, /* Value 37 */
        { 0x3F<<2, 0x1D<<2, 0x18<<2 }, /* Value 38 */
        { 0x3F<<2, 0x26<<2, 0x0E<<2 }, /* Value 39 */
        { 0x3C<<2, 0x2F<<2, 0x0F<<2 }, /* Value 40 */
        { 0x20<<2, 0x34<<2, 0x04<<2 }, /* Value 41 */
        { 0x13<<2, 0x37<<2, 0x12<<2 }, /* Value 42 */
        { 0x16<<2, 0x3E<<2, 0x26<<2 }, /* Value 43 */
        { 0x00<<2, 0x3A<<2, 0x36<<2 }, /* Value 44 */
        { 0x1E<<2, 0x1E<<2, 0x1E<<2 }, /* Value 45 */
        { 0x00<<2, 0x00<<2, 0x00<<2 }, /* Value 46 */
        { 0x00<<2, 0x00<<2, 0x00<<2 }, /* Value 47 */
        { 0x3F<<2, 0x3F<<2, 0x3F<<2 }, /* Value 48 */
        { 0x2A<<2, 0x39<<2, 0x3F<<2 }, /* Value 49 */
        { 0x31<<2, 0x35<<2, 0x3F<<2 }, /* Value 50 */
        { 0x35<<2, 0x32<<2, 0x3F<<2 }, /* Value 51 */
        { 0x3F<<2, 0x31<<2, 0x3F<<2 }, /* Value 52 */
        { 0x3F<<2, 0x31<<2, 0x36<<2 }, /* Value 53 */
        { 0x3F<<2, 0x2F<<2, 0x2C<<2 }, /* Value 54 */
        { 0x3F<<2, 0x36<<2, 0x2A<<2 }, /* Value 55 */
        { 0x3F<<2, 0x39<<2, 0x28<<2 }, /* Value 56 */
        { 0x38<<2, 0x3F<<2, 0x28<<2 }, /* Value 57 */
        { 0x2A<<2, 0x3C<<2, 0x2F<<2 }, /* Value 58 */
        { 0x2C<<2, 0x3F<<2, 0x33<<2 }, /* Value 59 */
        { 0x27<<2, 0x3F<<2, 0x3C<<2 }, /* Value 60 */
        { 0x31<<2, 0x31<<2, 0x31<<2 }, /* Value 61 */
        { 0x00<<2, 0x00<<2, 0x00<<2 }, /* Value 62 */
        { 0x00<<2, 0x00<<2, 0x00<<2 }, /* Value 63 */

		//luke's .16+ palette
        //{0x60, 0x60, 0x60}, /* Value 0 */
        //{0x00, 0x00, 0x70}, /* Value 1 */
        //{0x14, 0x00, 0x80}, /* Value 2 */
        //{0x2C, 0x00, 0x6E}, /* Value 3 */
        //{0x4A, 0x00, 0x4E}, /* Value 4 */
        //{0x6C, 0x00, 0x18}, /* Value 5 */
        //{0x5A, 0x03, 0x02}, /* Value 6 */
        //{0x51, 0x18, 0x00}, /* Value 7 */
        //{0x34, 0x24, 0x00}, /* Value 8 */
        //{0x00, 0x34, 0x00}, /* Value 9 */
        //{0x00, 0x32, 0x00}, /* Value 10 */
        //{0x00, 0x34, 0x20}, /* Value 11 */
        //{0x00, 0x2C, 0x78}, /* Value 12 */
        //{0x00, 0x00, 0x00}, /* Value 13 */
        //{0x02, 0x02, 0x02}, /* Value 14 */
        //{0x02, 0x02, 0x02}, /* Value 15 */
        //{0xC4, 0xC4, 0xC4}, /* Value 16 */
        //{0x00, 0x58, 0xDE}, /* Value 17 */
        //{0x30, 0x1F, 0xFC}, /* Value 18 */
        //{0x7F, 0x14, 0xE0}, /* Value 19 */
        //{0xA8, 0x00, 0xB0}, /* Value 20 */
        //{0xC0, 0x06, 0x5C}, /* Value 21 */
        //{0xC0, 0x2B, 0x0E}, /* Value 22 */
        //{0xA6, 0x40, 0x10}, /* Value 23 */
        //{0x6F, 0x61, 0x00}, /* Value 24 */
        //{0x30, 0x80, 0x00}, /* Value 25 */
        //{0x00, 0x7C, 0x00}, /* Value 26 */
        //{0x00, 0x7C, 0x3C}, /* Value 27 */
        //{0x00, 0x6E, 0x84}, /* Value 28 */
        //{0x14, 0x14, 0x14}, /* Value 29 */
        //{0x04, 0x04, 0x04}, /* Value 30 */
        //{0x04, 0x04, 0x04}, /* Value 31 */
        //{0xF0, 0xF0, 0xF0}, /* Value 32 */
        //{0x4C, 0xAA, 0xFF}, /* Value 33 */
        //{0x6F, 0x73, 0xF5}, /* Value 34 */
        //{0xB0, 0x70, 0xFF}, /* Value 35 */
        //{0xDA, 0x5A, 0xFF}, /* Value 36 */
        //{0xF0, 0x60, 0xC0}, /* Value 37 */
        //{0xF8, 0x83, 0x6D}, /* Value 38 */
        //{0xD0, 0x90, 0x30}, /* Value 39 */
        //{0xD4, 0xC0, 0x30}, /* Value 40 */
        //{0x66, 0xD0, 0x00}, /* Value 41 */
        //{0x26, 0xDD, 0x1A}, /* Value 42 */
        //{0x2E, 0xC8, 0x66}, /* Value 43 */
        //{0x34, 0xC2, 0xBE}, /* Value 44 */
        //{0x54, 0x54, 0x54}, /* Value 45 */
        //{0x06, 0x06, 0x06}, /* Value 46 */
        //{0x06, 0x06, 0x06}, /* Value 47 */
        //{0xFF, 0xFF, 0xFF}, /* Value 48 */
        //{0xB6, 0xDA, 0xFF}, /* Value 49 */
        //{0xC8, 0xCA, 0xFF}, /* Value 50 */
        //{0xDA, 0xC2, 0xFF}, /* Value 51 */
        //{0xF0, 0xBE, 0xFF}, /* Value 52 */
        //{0xFC, 0xBC, 0xEE}, /* Value 53 */
        //{0xFF, 0xD0, 0xB4}, /* Value 54 */
        //{0xFF, 0xDA, 0x90}, /* Value 55 */
        //{0xEC, 0xEC, 0x92}, /* Value 56 */
        //{0xDC, 0xF6, 0x9E}, /* Value 57 */
        //{0xB8, 0xFF, 0xA2}, /* Value 58 */
        //{0xAE, 0xEA, 0xBE}, /* Value 59 */
        //{0x9E, 0xEF, 0xEF}, /* Value 60 */
        //{0xBE, 0xBE, 0xBE}, /* Value 61 */
        //{0x08, 0x08, 0x08}, /* Value 62 */
        //{0x08, 0x08, 0x08}, /* Value 63 */
/*
// old palette:
74, 74, 74
24, 18, 8C
00, 00, A8
44, 00, 9C
8C, 00, 74
A8, 00, 10
A4, 00, 00
7C, 08, 00
40, 2C, 00
00, 44, 00
00, 50, 00
00, 3C, 14
18, 3C, 5C
00, 00, 00
00, 00, 00
00, 00, 00
BC, BC, BC
00, 70, EC
20, 38, EC
80, 00, F0
BC, 00, BC
E4, 00, 58
D8, 28, 00
C8, 4C, 0C
88, 70, 00
00, 94, 00
00, A8, 00
00, 90, 38
00, 80, 88
00, 00, 00
00, 00, 00
00, 00, 00
FC, FC, FC
3C, BC, FC
5C, 94, FC
40, 88, FC
F4, 78, FC
FC, 74, B4
FC, 74, 60
FC, 98, 38
F0, BC, 3C
80, D0, 10
4C, DC, 48
58, F8, 98
00, E8, D8
78, 78, 78
00, 00, 00
00, 00, 00
FC, FC, FC
A8, E4, FC
C4, D4, FC
D4, C8, FC
FC, C4, FC
FC, C4, D8
FC, BC, B0
FC, D8, A8
FC, E4, A0
E0, FC, A0
A8, F0, BC
B0, FC, CC
9C, FC, F0
C4, C4, C4
00, 00, 00
00, 00, 00

slightly better but too dark:
0x5C, 0x5C, 0x5C
0x00, 0x20, 0x74
0x00, 0x00, 0x98
0x30, 0x00, 0x84
0x58, 0x00, 0x68
0x68, 0x00, 0x30
0x60, 0x00, 0x00
0x48, 0x10, 0x00
0x28, 0x18, 0x00
0x20, 0x28, 0x00
0x00, 0x40, 0x00
0x00, 0x38, 0x30
0x00, 0x38, 0x48
0x00, 0x00, 0x00
0x10, 0x10, 0x10
0x10, 0x10, 0x10
0xAA, 0xAA, 0xAA
0x18, 0x54, 0xBC
0x28, 0x30, 0xF8
0x68, 0x20, 0xD8
0xA8, 0x00, 0xB8
0xB8, 0x14, 0x5C
0xB8, 0x18, 0x10
0x8C, 0x38, 0x0C
0x60, 0x50, 0x00
0x3C, 0x60, 0x00
0x00, 0x78, 0x00
0x00, 0x70, 0x4C
0x00, 0x70, 0x80
0xFF, 0xFF, 0x10
0x10, 0xFF, 0xFF
0x10, 0x10, 0xFF
0xFF, 0xFF, 0xFF
0x4C, 0x9C, 0xEC
0x78, 0x80, 0xFF
0xAC, 0x70, 0xFF
0xE0, 0x60, 0xF0
0xFC, 0x68, 0xA4
0xFF, 0x70, 0x68
0xE2, 0x92, 0x38
0xBB, 0xAB, 0x00
0x7E, 0xC2, 0x1C
0x40, 0xD0, 0x30
0x28, 0xC4, 0x80
0x20, 0xB8, 0xC0
0x48, 0x48, 0x48
0x10, 0x10, 0x10
0x10, 0x10, 0x10
0xFF, 0xFF, 0xFF
0xB4, 0xD4, 0xFF
0xC0, 0xC8, 0xFF
0xD8, 0xC4, 0xFF
0xF0, 0xC0, 0xFF
0xFF, 0xC4, 0xD8
0xFF, 0xC8, 0xC0
0xF8, 0xD4, 0xAC
0xE8, 0xE0, 0x98
0xD0, 0xE8, 0xA0
0xB8, 0xF0, 0xA8
0xA8, 0xE8, 0xD0
0xA8, 0xE0, 0xE8
0xB2, 0xB2, 0xB2
0x10, 0xFF, 0x10
0xFF, 0x10, 0x10

*/

EMPTY_PALETTE_DEEMPH_X_7

};
