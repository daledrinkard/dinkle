/*
 * graphics_settings.h
 *
 *  Created on: Sep 20, 2023
 *      Author: a5138119
 */

#ifndef GRAPHICS_SETTINGS_H_
#define GRAPHICS_SETTINGS_H_


// Possible defines of input format:
// DISPLAY_IN_FORMAT_32BITS_ARGB8888_0
// DISPLAY_IN_FORMAT_32BITS_RGB888_0
// DISPLAY_IN_FORMAT_16BITS_RGB565_0
// DISPLAY_IN_FORMAT_16BITS_ARGB1555_0
// DISPLAY_IN_FORMAT_16BITS_ARGB4444_0
// DISPLAY_IN_FORMAT_CLUT8_0
// DISPLAY_IN_FORMAT_CLUT4_0
// DISPLAY_IN_FORMAT_CLUT1_0

/* START: Generate definitions for colors based on the input format */


/* Color definitions */
#if defined (DISPLAY_IN_FORMAT_32BITS_RGB888_0) || defined (DISPLAY_IN_FORMAT_32BITS_ARGB8888_0 ) \
|| defined (DISPLAY_IN_FORMAT_CLUT8_0 ) || defined( DISPLAY_IN_FORMAT_CLUT4_0 ) || defined( DISPLAY_IN_FORMAT_CLUT1_0 )
    #define BLUE     0x000000FF
    #define CYAN     0x0000FFFF
    #define GREEN    0x0000FF00
    #define YELLOW   0x00FFFF00
    #define RED      0x00FF0000
    #define MAGENTA  0x00FF00FF
    #define BLACK    0x00000000
    #define WHITE    0x00FFFFFF

    #define CLUT_SIZE      8
    #define BLUE_INDEX     0U
    #define CYAN_INDEX     1U
    #define GREEN_INDEX    2U
    #define YELLOW_INDEX   3U
    #define RED_INDEX      4U
    #define MAGENTA_INDEX  5U
    #define BLACK_INDEX    6U
    #define WHITE_INDEX    7U

#elif defined ( DISPLAY_IN_FORMAT_16BITS_RGB565_0 )
    #define BLUE     0x001F
    #define CYAN     0x07FF
    #define GREEN    0x07E0
    #define YELLOW   0xFFE0
    #define RED      0xF800
    #define MAGENTA  0xF81F
    #define BLACK    0x0000
    #define WHITE    0xFFFF

#elif defined ( DISPLAY_IN_FORMAT_16BITS_ARGB1555_0 )
    #define BLUE     0x001F
    #define CYAN     0x03FF
    #define GREEN    0x03E0
    #define YELLOW   0x7FE0
    #define RED      0x7C00
    #define MAGENTA  0x7C1F
    #define BLACK    0x0000
    #define WHITE    0x7FFF

#elif defined ( DISPLAY_IN_FORMAT_16BITS_ARGB4444_0 )
    #define BLUE     0x000F
    #define CYAN     0x00FF
    #define GREEN    0x00F0
    #define YELLOW   0x0FF0
    #define RED      0x0F00
    #define MAGENTA  0x0F0F
    #define BLACK    0x0000
    #define WHITE    0x0FFF

#endif

#if defined (DISPLAY_IN_FORMAT_32BITS_RGB888_0) || defined (DISPLAY_IN_FORMAT_32BITS_ARGB8888_0 )
    #define BITS_PER_PIXEL 32
#elif defined (DISPLAY_IN_FORMAT_CLUT8_0 )
    #define BITS_PER_PIXEL 8
    #define PIXELS_PER_WORD (32/BITS_PER_PIXEL)
#elif defined (DISPLAY_IN_FORMAT_CLUT4_0 )
    #define BITS_PER_PIXEL 4
#endif

/* END: Generate definitions for colors based on the input format  */


#endif /* GRAPHICS_SETTINGS_H_ */
