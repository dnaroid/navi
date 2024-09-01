/*******************************************************************************
 * Size: 20 px
 * Bpp: 4
 * Opts: --bpp 4 --size 20 --no-compress --font /Users/buzz/WORK/icons.ttf --range 0x30-0x3d --format lvgl -o /Users/buzz/Projects/navi_c++/lib/fonts/icons.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef ICONS
#define ICONS 1
#endif

#if ICONS

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0030 "0" */
    0x0, 0x0, 0x0, 0x0, 0x1, 0x10, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x6c, 0xff, 0xff,
    0xc6, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4e, 0xfe,
    0xa8, 0x8a, 0xef, 0xe4, 0x0, 0x0, 0x0, 0x7,
    0xfe, 0x60, 0x0, 0x0, 0x6, 0xef, 0x60, 0x0,
    0x0, 0x4f, 0xd1, 0x0, 0x0, 0x0, 0x0, 0x2d,
    0xf4, 0x0, 0x0, 0xee, 0x10, 0x0, 0x0, 0x0,
    0x0, 0x2, 0xee, 0x0, 0x6, 0xf6, 0x0, 0x0,
    0xb, 0xb0, 0x0, 0x0, 0x6f, 0x60, 0xc, 0xe0,
    0x0, 0x0, 0xd, 0xd0, 0x0, 0x0, 0xe, 0xc0,
    0xf, 0xa0, 0x0, 0x0, 0xd, 0xd0, 0x0, 0x0,
    0xa, 0xf0, 0x1f, 0x80, 0x0, 0xbd, 0xdf, 0xfd,
    0xdb, 0x0, 0x8, 0xf1, 0x1f, 0x80, 0x0, 0xbd,
    0xdf, 0xfd, 0xdb, 0x0, 0x8, 0xf1, 0xf, 0xa0,
    0x0, 0x0, 0xd, 0xd0, 0x0, 0x0, 0xa, 0xf0,
    0xc, 0xe0, 0x0, 0x0, 0xd, 0xd0, 0x0, 0x0,
    0xe, 0xc0, 0x6, 0xf6, 0x0, 0x0, 0xb, 0xb0,
    0x0, 0x0, 0x6f, 0x60, 0x0, 0xee, 0x10, 0x0,
    0x0, 0x0, 0x0, 0x1, 0xee, 0x0, 0x0, 0x4f,
    0xd1, 0x0, 0x0, 0x0, 0x0, 0x1d, 0xf4, 0x0,
    0x0, 0x7, 0xfe, 0x60, 0x0, 0x0, 0x6, 0xef,
    0x70, 0x0, 0x0, 0x0, 0x4e, 0xfe, 0xa8, 0x8a,
    0xef, 0xe4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x6c,
    0xff, 0xff, 0xc6, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x1, 0x10, 0x0, 0x0, 0x0, 0x0,

    /* U+0031 "1" */
    0x0, 0x0, 0x0, 0x0, 0x1, 0x10, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x6c, 0xff, 0xff,
    0xc6, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4e, 0xfe,
    0xa8, 0x8a, 0xef, 0xe4, 0x0, 0x0, 0x0, 0x7,
    0xfe, 0x60, 0x0, 0x0, 0x6, 0xef, 0x60, 0x0,
    0x0, 0x4f, 0xd1, 0x0, 0x0, 0x0, 0x0, 0x2d,
    0xf4, 0x0, 0x0, 0xee, 0x10, 0x0, 0x0, 0x0,
    0x0, 0x2, 0xee, 0x0, 0x6, 0xf6, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x6f, 0x60, 0xc, 0xe0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xe, 0xc0,
    0xf, 0xa0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0xa, 0xf0, 0x1f, 0x80, 0x0, 0xbd, 0xdd, 0xdd,
    0xdb, 0x0, 0x8, 0xf1, 0x1f, 0x80, 0x0, 0xbd,
    0xdd, 0xdd, 0xdb, 0x0, 0x8, 0xf1, 0xf, 0xa0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xa, 0xf0,
    0xc, 0xe0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0xe, 0xc0, 0x6, 0xf6, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x6f, 0x60, 0x0, 0xee, 0x10, 0x0,
    0x0, 0x0, 0x0, 0x1, 0xee, 0x0, 0x0, 0x4f,
    0xd1, 0x0, 0x0, 0x0, 0x0, 0x1d, 0xf4, 0x0,
    0x0, 0x7, 0xfe, 0x60, 0x0, 0x0, 0x6, 0xef,
    0x70, 0x0, 0x0, 0x0, 0x4e, 0xfe, 0xa8, 0x8a,
    0xef, 0xe4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x6c,
    0xff, 0xff, 0xc6, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x1, 0x10, 0x0, 0x0, 0x0, 0x0,

    /* U+0032 "2" */
    0x0, 0x2, 0x88, 0x88, 0x20, 0x0, 0x8a, 0xae,
    0xff, 0xff, 0xea, 0xa8, 0xdf, 0xff, 0xff, 0xff,
    0xff, 0xfd, 0x2, 0x22, 0x22, 0x22, 0x22, 0x20,
    0xf, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xf, 0xd8,
    0x88, 0x88, 0x8d, 0xf0, 0xf, 0xa0, 0x10, 0x1,
    0xa, 0xf0, 0xf, 0xa8, 0xd1, 0x2e, 0x8a, 0xf0,
    0xf, 0xa5, 0xfd, 0xef, 0x5a, 0xf0, 0xf, 0xa0,
    0x7f, 0xf7, 0xa, 0xf0, 0xf, 0xa2, 0xef, 0xfe,
    0x2a, 0xf0, 0xf, 0xaa, 0xf5, 0x5f, 0xaa, 0xf0,
    0xf, 0xa1, 0x40, 0x4, 0x1a, 0xf0, 0xf, 0xb2,
    0x22, 0x22, 0x2b, 0xf0, 0xc, 0xff, 0xff, 0xff,
    0xff, 0xc0, 0x1, 0x78, 0x88, 0x88, 0x87, 0x10,

    /* U+0033 "3" */
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2,
    0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x17,
    0xdf, 0xe2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4a,
    0xff, 0xeb, 0xf5, 0x0, 0x0, 0x0, 0x2, 0x8e,
    0xff, 0xb4, 0xa, 0xf1, 0x0, 0x0, 0x5, 0xbf,
    0xfd, 0x71, 0x0, 0x1f, 0xb0, 0x0, 0x29, 0xef,
    0xfa, 0x30, 0x0, 0x0, 0x6f, 0x50, 0xb, 0xff,
    0xc6, 0x0, 0x0, 0x0, 0x0, 0xde, 0x0, 0x6f,
    0xb2, 0x0, 0x0, 0x0, 0x0, 0x3, 0xf8, 0x0,
    0x3f, 0xfc, 0x50, 0x0, 0x0, 0x0, 0x9, 0xf2,
    0x0, 0x2, 0x9f, 0xfe, 0x70, 0x0, 0x0, 0xf,
    0xc0, 0x0, 0x0, 0x1, 0x7d, 0xfc, 0x0, 0x0,
    0x6f, 0x50, 0x0, 0x0, 0x0, 0x0, 0x9f, 0x90,
    0x0, 0xcf, 0x0, 0x0, 0x0, 0x0, 0x0, 0xc,
    0xf1, 0x2, 0xf9, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x5, 0xf7, 0x8, 0xf3, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0xed, 0xe, 0xd0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x8f, 0x9f, 0x60, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x1f, 0xff, 0x10, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x93, 0x0,
    0x0, 0x0,

    /* U+0034 "4" */
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4,
    0x99, 0x30, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x7, 0xfb, 0xcf, 0x40, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1, 0xf6, 0x0, 0xad, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3f, 0x1b,
    0x75, 0xf0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x1, 0xf6, 0x53, 0xac, 0x0, 0x2, 0x78, 0x60,
    0x0, 0x0, 0x0, 0x9, 0xe0, 0x3f, 0x50, 0x6,
    0xff, 0xdf, 0xe3, 0x0, 0x0, 0x0, 0x1e, 0x9c,
    0xc0, 0x4, 0xf8, 0x0, 0x1b, 0xf1, 0x0, 0x0,
    0x0, 0x5f, 0xf2, 0x0, 0xcb, 0x0, 0x0, 0xe,
    0x80, 0x1b, 0x7a, 0xa7, 0x75, 0x0, 0xf, 0x60,
    0x5e, 0x20, 0xaa, 0x3, 0x50, 0x0, 0x0, 0x0,
    0x0, 0xd8, 0x5, 0xd2, 0xc, 0x90, 0xb4, 0x0,
    0x0, 0x0, 0x0, 0x8, 0xe0, 0x0, 0x2, 0xf4,
    0x0, 0xa7, 0xd4, 0xe3, 0x40, 0x0, 0x1f, 0x60,
    0x0, 0xbc, 0x0, 0x1, 0x2, 0x3, 0x48, 0x0,
    0x0, 0x8e, 0x10, 0x4f, 0x40, 0x0, 0x0, 0x0,
    0x0, 0x6a, 0x0, 0x0, 0xda, 0xe, 0xa0, 0x0,
    0x0, 0x0, 0x0, 0x2, 0x30, 0x0, 0x3, 0xfd,
    0xe1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x79, 0x0,
    0x0, 0x8, 0xf6, 0x73, 0x65, 0x38, 0x28, 0x27,
    0x97, 0x0, 0x0, 0x0, 0x2, 0x18, 0x36, 0x63,
    0x82, 0x82, 0x71, 0x10, 0x0,

    /* U+0035 "5" */
    0x0, 0x0, 0x18, 0xe9, 0x20, 0x0, 0x0, 0x0,
    0x1, 0x8e, 0x0, 0x19, 0xe9, 0x27, 0xec, 0x50,
    0x0, 0x1, 0x8e, 0x8f, 0x18, 0xe8, 0x10, 0xd0,
    0x5, 0xce, 0x72, 0x9e, 0x91, 0xf, 0xf8, 0x10,
    0x0, 0xf0, 0x0, 0x3, 0x9e, 0x81, 0x0, 0xf,
    0xf0, 0x0, 0x0, 0xf0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0xf, 0xf0, 0x0, 0x0, 0xf0, 0x0, 0x0,
    0xd, 0x0, 0x0, 0xf, 0xf0, 0x0, 0x0, 0x34,
    0xcf, 0xfc, 0x43, 0x0, 0x0, 0xf, 0xf0, 0x0,
    0x0, 0x8d, 0x40, 0x4, 0xd8, 0x0, 0x0, 0xf,
    0xf0, 0x0, 0x5, 0xd1, 0x0, 0x0, 0x1d, 0x40,
    0x0, 0xf, 0xf0, 0x0, 0xc, 0x40, 0x0, 0x0,
    0x4, 0xc0, 0x0, 0xf, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0x0, 0x0, 0xf0, 0x0, 0xf, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0x0, 0x0, 0xf0, 0x0, 0xf,
    0xf0, 0x0, 0xc, 0x40, 0x0, 0x0, 0x4, 0xb0,
    0x0, 0xf, 0xf0, 0x0, 0x4, 0xd1, 0x0, 0x0,
    0x1d, 0x40, 0x0, 0xf, 0xf0, 0x0, 0x0, 0x8d,
    0x40, 0x4, 0xdf, 0x40, 0x0, 0xf, 0xf0, 0x0,
    0x0, 0x4, 0xcf, 0xfb, 0x43, 0xe4, 0x0, 0xf,
    0xf0, 0x0, 0x18, 0xe9, 0x20, 0x0, 0x6, 0x3e,
    0x40, 0x8f, 0xf0, 0x18, 0xe9, 0x27, 0xec, 0x50,
    0xd, 0x3, 0xe4, 0x41, 0xf8, 0xe8, 0x10, 0x0,
    0x5, 0xce, 0x72, 0x96, 0x3c, 0x0, 0xe8, 0x10,
    0x0, 0x0, 0x0, 0x3, 0x9e, 0x81, 0x0, 0x0,

    /* U+0036 "6" */
    0x0, 0x0, 0x46, 0x77, 0x77, 0x77, 0x64, 0x0,
    0x0, 0x0, 0x0, 0x7f, 0xfe, 0xdd, 0xdd, 0xdd,
    0xef, 0xf6, 0x0, 0x0, 0x9, 0xf7, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x7f, 0x80, 0x0, 0x4f, 0x40,
    0x0, 0x10, 0x21, 0x2, 0x0, 0x4, 0xf4, 0x0,
    0xba, 0x0, 0x1a, 0x5, 0xa0, 0x7f, 0xa0, 0x0,
    0xab, 0x0, 0xe5, 0x0, 0x60, 0x5a, 0x7, 0xff,
    0xf4, 0x0, 0x6e, 0x0, 0xf5, 0x0, 0x5, 0xf0,
    0x7f, 0xff, 0xf5, 0x0, 0x5f, 0x0, 0xd8, 0x0,
    0x2f, 0xff, 0xff, 0xff, 0xe1, 0x0, 0x4f, 0x20,
    0x8e, 0x10, 0x5, 0xab, 0xbb, 0xb9, 0x20, 0x0,
    0xb, 0xe2, 0xd, 0xc1, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0xbe, 0x2, 0xde, 0x97, 0x66,
    0x66, 0x66, 0x66, 0x10, 0x0, 0x4f, 0x0, 0x7,
    0xbd, 0xee, 0xee, 0xee, 0xee, 0xd1, 0x0, 0x4f,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xcd,
    0x10, 0x4f, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x1c, 0xd1, 0x4f, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x1, 0xcd, 0x6f, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1c, 0xff,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x1, 0xdf, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x15,

    /* U+0037 "7" */
    0x0, 0x0, 0x0, 0x3, 0x44, 0x30, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x18, 0xef, 0xff, 0xfe, 0x81,
    0x0, 0x0, 0x0, 0x3, 0xef, 0xd8, 0x66, 0x8d,
    0xfe, 0x30, 0x0, 0x0, 0x3f, 0xf6, 0x0, 0x0,
    0x0, 0x6f, 0xf3, 0x0, 0x1, 0xef, 0x30, 0x0,
    0x0, 0x0, 0x3, 0xfe, 0x10, 0x8, 0xf5, 0x9,
    0x60, 0x0, 0x6, 0x90, 0x6f, 0x80, 0xe, 0xd0,
    0x6, 0xf6, 0x0, 0x6f, 0x60, 0xd, 0xe0, 0x3f,
    0x80, 0x0, 0x6f, 0x66, 0xf6, 0x0, 0x8, 0xf3,
    0x5f, 0x60, 0x0, 0x6, 0xff, 0x60, 0x0, 0x6,
    0xf4, 0x5f, 0x60, 0x0, 0x6, 0xff, 0x60, 0x0,
    0x6, 0xf4, 0x3f, 0x80, 0x0, 0x6f, 0x66, 0xf6,
    0x0, 0x8, 0xf3, 0xe, 0xd0, 0x6, 0xf6, 0x0,
    0x6f, 0x60, 0xd, 0xe0, 0x8, 0xf5, 0x9, 0x60,
    0x0, 0x6, 0x90, 0x6f, 0x80, 0x1, 0xef, 0x20,
    0x0, 0x0, 0x0, 0x3, 0xfe, 0x10, 0x0, 0x3f,
    0xf5, 0x0, 0x0, 0x0, 0x5f, 0xf3, 0x0, 0x0,
    0x3, 0xef, 0xd8, 0x66, 0x8d, 0xfe, 0x30, 0x0,
    0x0, 0x0, 0x18, 0xef, 0xff, 0xfe, 0x81, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x3, 0x55, 0x30, 0x0,
    0x0, 0x0,

    /* U+0038 "8" */
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1d, 0x70,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x8,
    0xff, 0x80, 0x0, 0x0, 0x0, 0x4f, 0x70, 0x0,
    0x18, 0x9, 0xff, 0x80, 0x0, 0x0, 0x4f, 0xff,
    0x80, 0xd, 0xf9, 0x9, 0xff, 0x80, 0x0, 0x3f,
    0xff, 0xff, 0x80, 0x7f, 0xf9, 0x9, 0xff, 0x70,
    0x3, 0xff, 0xff, 0xff, 0x80, 0x8f, 0xf9, 0x9,
    0xf7, 0x0, 0x4, 0xff, 0xff, 0xff, 0x89, 0xff,
    0xf9, 0x4, 0x0, 0x0, 0x4, 0xff, 0xff, 0xff,
    0xf3, 0x8f, 0xf4, 0x0, 0x0, 0x0, 0x4, 0xff,
    0xff, 0xff, 0x80, 0x77, 0x0, 0x0, 0x0, 0x18,
    0x4, 0xff, 0xff, 0xff, 0x70, 0x0, 0x0, 0x0,
    0xd, 0xfb, 0xb, 0xff, 0xff, 0xf7, 0x0, 0x0,
    0x0, 0x11, 0x6f, 0xfe, 0xe7, 0xff, 0xf7, 0x18,
    0xba, 0x40, 0x1e, 0xc1, 0x6f, 0xfc, 0x4, 0xf7,
    0x2e, 0xff, 0xff, 0x72, 0xff, 0xc1, 0x6f, 0xfb,
    0x0, 0xc, 0xff, 0xff, 0xf4, 0x4, 0xff, 0xc1,
    0x6f, 0xf4, 0x0, 0xff, 0xff, 0xf5, 0x0, 0x4,
    0xff, 0xc1, 0x67, 0x0, 0x1f, 0xff, 0xfe, 0x10,
    0x0, 0x4, 0xff, 0xc0, 0x0, 0x0, 0xdf, 0xf6,
    0xcd, 0x0, 0x0, 0x4, 0xf7, 0x0, 0x0, 0x5,
    0xf5, 0x0, 0x60, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0,

    /* U+0039 "9" */
    0x0, 0x0, 0x0, 0x0, 0x4c, 0xc4, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0xbd, 0x55,
    0xdc, 0x30, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2a,
    0xe6, 0x0, 0x0, 0x5d, 0xb3, 0x0, 0x0, 0x0,
    0x2, 0xae, 0x70, 0x0, 0x0, 0x0, 0x6, 0xea,
    0x20, 0x0, 0x19, 0xe7, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x7e, 0x91, 0xd, 0xe2, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x2e, 0xd0, 0x19,
    0xe7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7e,
    0x91, 0x0, 0x2, 0xae, 0x70, 0x0, 0x0, 0x0,
    0x6, 0xea, 0x20, 0x0, 0xd8, 0x10, 0x2a, 0xe6,
    0x0, 0x0, 0x5d, 0xb2, 0x1, 0x8d, 0x1, 0x9e,
    0x70, 0x3, 0xbd, 0x55, 0xdc, 0x30, 0x7, 0xe9,
    0x10, 0x0, 0x2a, 0xe7, 0x0, 0x4c, 0xc4, 0x0,
    0x6e, 0xa2, 0x0, 0xd, 0x81, 0x2, 0xae, 0x60,
    0x0, 0x5, 0xdb, 0x20, 0x18, 0xd0, 0x19, 0xe7,
    0x0, 0x3b, 0xd5, 0x5d, 0xc3, 0x0, 0x7e, 0x91,
    0x0, 0x2, 0xae, 0x70, 0x4, 0xcc, 0x40, 0x6,
    0xea, 0x20, 0x0, 0x0, 0x0, 0x2a, 0xe6, 0x0,
    0x0, 0x5d, 0xb2, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x3, 0xbd, 0x55, 0xdc, 0x30, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x4c, 0xc4, 0x0, 0x0,
    0x0, 0x0, 0x0,

    /* U+003A ":" */
    0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x72, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xe,
    0xfe, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x1f, 0xff, 0x10, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x17, 0xc6, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x5, 0xfb, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x9f, 0xff,
    0x50, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1d,
    0xfb, 0x2e, 0xfa, 0x70, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x3f, 0xf2, 0x4, 0xff, 0xc0, 0x0, 0x0,
    0x0, 0x0, 0x20, 0x5, 0xfd, 0x10, 0x0, 0x2,
    0x0, 0x0, 0x0, 0x7f, 0xff, 0x90, 0x5f, 0xc0,
    0x9, 0xff, 0xf7, 0x0, 0x7, 0xfd, 0x8c, 0xfa,
    0xd, 0xd0, 0xaf, 0xc8, 0xdf, 0x70, 0xf, 0xd0,
    0x0, 0xbf, 0x2d, 0xd2, 0xfb, 0x0, 0xd, 0xf0,
    0x2f, 0x80, 0x0, 0x6f, 0x5c, 0xc5, 0xf6, 0x0,
    0x8, 0xf2, 0xf, 0xc0, 0x0, 0x9f, 0x32, 0x23,
    0xf9, 0x0, 0xc, 0xf0, 0x9, 0xfb, 0x69, 0xfc,
    0x0, 0x0, 0xcf, 0x96, 0xbf, 0x90, 0x0, 0xaf,
    0xff, 0xc1, 0x0, 0x0, 0x1c, 0xff, 0xfa, 0x0,
    0x0, 0x2, 0x53, 0x0, 0x0, 0x0, 0x0, 0x35,
    0x20, 0x0,

    /* U+003B ";" */
    0x0, 0x3c, 0xff, 0xff, 0xff, 0xf8, 0x0, 0x0,
    0x0, 0x0, 0x0, 0xef, 0xaa, 0xaa, 0xaa, 0xcf,
    0x60, 0x0, 0x0, 0x0, 0x6, 0xf7, 0x0, 0x0,
    0x0, 0x1e, 0xf2, 0x0, 0x0, 0x0, 0xd, 0xe0,
    0x0, 0x0, 0x0, 0x3, 0xfd, 0x74, 0x20, 0x0,
    0xf, 0x90, 0x0, 0x0, 0x0, 0x0, 0x6f, 0xff,
    0xff, 0x80, 0x2f, 0x80, 0x1, 0x0, 0x0, 0x0,
    0x2, 0x35, 0x8d, 0xf1, 0x2f, 0x75, 0xff, 0xe4,
    0x0, 0x2, 0xdf, 0xf7, 0x7, 0xf2, 0x2f, 0xef,
    0xd8, 0xff, 0x97, 0x8f, 0xf9, 0xcf, 0xdb, 0xf2,
    0x1f, 0xff, 0x40, 0xaf, 0xff, 0xff, 0xf0, 0x1f,
    0xff, 0xf1, 0x1, 0x8f, 0x80, 0xaf, 0x52, 0x3f,
    0xc1, 0x5f, 0xa2, 0x10, 0x0, 0xd, 0xff, 0xfa,
    0x0, 0x8, 0xff, 0xfe, 0x10, 0x0, 0x0, 0x0,
    0x79, 0x60, 0x0, 0x0, 0x59, 0x81, 0x0, 0x0,

    /* U+003C "<" */
    0x0, 0x0, 0x0, 0x42, 0x0, 0x0, 0x0, 0x0,
    0x8, 0xff, 0x30, 0x0, 0x0, 0x0, 0xc, 0xff,
    0x70, 0x0, 0x0, 0x0, 0x5, 0xec, 0x10, 0x0,
    0x0, 0x0, 0x28, 0x20, 0x0, 0x0, 0x0, 0x2a,
    0xff, 0xf2, 0x0, 0x0, 0x5, 0xff, 0xff, 0xf2,
    0x0, 0x0, 0xb, 0xf3, 0xaf, 0xf2, 0x61, 0x0,
    0xe, 0xc0, 0xdf, 0xc6, 0xff, 0xa0, 0x1f, 0x90,
    0xff, 0x80, 0x5a, 0xa0, 0x6, 0x10, 0xaf, 0x70,
    0x0, 0x0, 0x0, 0x4, 0x2e, 0xe1, 0x0, 0x0,
    0x0, 0xd, 0xa5, 0xfa, 0x0, 0x0, 0x0, 0x6f,
    0x70, 0xaf, 0x50, 0x0, 0x1, 0xee, 0x0, 0x1e,
    0xe1, 0x0, 0x9, 0xf5, 0x0, 0x5, 0xfa, 0x0,
    0xe, 0xc0, 0x0, 0x0, 0xaf, 0x10, 0x3, 0x10,
    0x0, 0x0, 0x3, 0x0,

    /* U+003D "=" */
    0x0, 0x0, 0x0, 0x3d, 0xff, 0xc3, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0xea, 0x44, 0xae, 0x0,
    0x0, 0x0, 0x0, 0x14, 0x23, 0xf1, 0x0, 0x1f,
    0x32, 0x41, 0x0, 0x2, 0xef, 0xff, 0xb0, 0x0,
    0xb, 0xff, 0xfe, 0x20, 0xb, 0xb0, 0x24, 0x0,
    0x0, 0x0, 0x42, 0xb, 0xb0, 0x4f, 0x20, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x2, 0xf4, 0x7d, 0x0,
    0x0, 0x1a, 0xff, 0xa1, 0x0, 0x0, 0xd7, 0x2f,
    0x80, 0x0, 0xad, 0x33, 0xda, 0x0, 0x8, 0xf2,
    0x4, 0xf3, 0x0, 0xf3, 0x0, 0x3f, 0x0, 0x3f,
    0x40, 0x4, 0xf2, 0x0, 0xf3, 0x0, 0x3f, 0x0,
    0x3f, 0x40, 0x2f, 0x80, 0x0, 0xbd, 0x33, 0xda,
    0x0, 0x8, 0xf2, 0x7d, 0x0, 0x0, 0x1b, 0xff,
    0xa1, 0x0, 0x0, 0xd7, 0x4f, 0x20, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x2, 0xf4, 0xb, 0xb0, 0x24,
    0x0, 0x0, 0x0, 0x42, 0xb, 0xb0, 0x2, 0xef,
    0xff, 0xb0, 0x0, 0xb, 0xff, 0xfe, 0x20, 0x0,
    0x14, 0x23, 0xf1, 0x0, 0x1f, 0x32, 0x41, 0x0,
    0x0, 0x0, 0x0, 0xea, 0x44, 0xae, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x3c, 0xff, 0xd3, 0x0,
    0x0, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 320, .box_w = 20, .box_h = 20, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 200, .adv_w = 320, .box_w = 20, .box_h = 20, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 400, .adv_w = 320, .box_w = 12, .box_h = 16, .ofs_x = 4, .ofs_y = -2},
    {.bitmap_index = 496, .adv_w = 320, .box_w = 18, .box_h = 18, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 658, .adv_w = 320, .box_w = 21, .box_h = 18, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 847, .adv_w = 320, .box_w = 20, .box_h = 20, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 1047, .adv_w = 320, .box_w = 20, .box_h = 18, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1227, .adv_w = 320, .box_w = 18, .box_h = 18, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1389, .adv_w = 320, .box_w = 19, .box_h = 19, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1570, .adv_w = 320, .box_w = 21, .box_h = 17, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1749, .adv_w = 320, .box_w = 20, .box_h = 17, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1919, .adv_w = 320, .box_w = 20, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 2039, .adv_w = 320, .box_w = 12, .box_h = 18, .ofs_x = 4, .ofs_y = -3},
    {.bitmap_index = 2147, .adv_w = 320, .box_w = 18, .box_h = 18, .ofs_x = 1, .ofs_y = -3}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/


/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 48, .range_length = 14, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};


/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 4,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t icons = {
#else
lv_font_t icons = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt, /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt, /*Function pointer to get glyph's bitmap*/
    .line_height = 20, /*The maximum line height required by the font*/
    .base_line = 4, /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc, /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};


#endif /*#if ICONS*/

