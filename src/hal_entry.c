/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include "hal_data.h"
#include "graphics_settings.h"


#define APP_ERR_TRAP(err)               ({\
    if(err)\
    {\
        __asm("BKPT #0\n"); /* trap upon the error  */ \
    }\
})

void R_BSP_WarmStart(bsp_warm_start_event_t event);

extern bsp_leds_t g_bsp_leds;

void rbg_render565(void);

void rbg_render565(void)
{
    /* generate the rendered image */

    //Variable to hold display configuration
    display_input_cfg_t const *p_input = &g_display0.p_cfg->input[0];

    //pointer to frame buffer
    uint16_t * buf_ptr = (uint16_t *)fb_background;

    uint16_t bar_width = p_input->vsize/7;

    //for loop across vertical, then horizontal pixels
    for(uint32_t y = 0; y < p_input->vsize; y++)
    {
        for(uint32_t x = 0; x < p_input->hsize; x++)
        {
            if(y < bar_width)
            {
                buf_ptr[x] = BLUE;
            }
            else if(y < (bar_width*2) )
            {
                buf_ptr[x] = CYAN;
            }
            else if(y < (bar_width*3) )
            {
                buf_ptr[x] = GREEN;
            }
            else if(y < (bar_width*4) )
            {
                buf_ptr[x] = WHITE;
            }
            else if(y < (bar_width*5) )
            {
                buf_ptr[x] = MAGENTA;
            }
            else if(y < (bar_width*6) )
            {
                buf_ptr[x] = BLACK;
            }
            else
            {
                buf_ptr[x] = YELLOW;
            }
        }
        buf_ptr += p_input->hstride;
    }
}

/*******************************************************************************************************************//**
 * @brief  Blinky example application
 *
 * Blinks all leds at a rate of 1 second using the software delay function provided by the BSP.
 *
 **********************************************************************************************************************/
void hal_entry (void)
{
#if BSP_TZ_SECURE_BUILD

    /* Enter non-secure code */
    R_BSP_NonSecureEnter();
#endif

    volatile fsp_err_t           err     = FSP_SUCCESS;

    /* Initialize SDRAM */
    bsp_sdram_init();

    /* Define the units to be used with the software delay function */
    const bsp_delay_units_t bsp_delay_units = BSP_DELAY_UNITS_MILLISECONDS;

    /* Set the blink frequency (must be <= bsp_delay_units */
    const uint32_t freq_in_hz = 2;

    /* Calculate the delay in terms of bsp_delay_units */
    const uint32_t delay = bsp_delay_units / freq_in_hz;

    /* LED type structure */
    bsp_leds_t leds = g_bsp_leds;

    /* If this board has no LEDs then trap here */
    if (0 == leds.led_count)
    {
        while (1)
        {
            ;                          // There are no LEDs on this board
        }
    }

    /* Holds level to set for pins */
    bsp_io_level_t pin_level = BSP_IO_LEVEL_LOW;

    rbg_render565();

    //set up LCD display
    /* Configure display GPIO used by this application */
    R_IOPORT_PinCfg(&g_ioport_ctrl, DISP_RST, IOPORT_PERIPHERAL_IO | IOPORT_CFG_PORT_DIRECTION_OUTPUT);
    R_IOPORT_PinCfg(&g_ioport_ctrl, DISP_BLEN, IOPORT_PERIPHERAL_IO | IOPORT_CFG_PORT_DIRECTION_OUTPUT);

    /* Reset Display - active low
     * NOTE: Wake may not be issued for 120 ms after HW reset */
    R_IOPORT_PinWrite(&g_ioport_ctrl, DISP_RST, BSP_IO_LEVEL_LOW);
    R_BSP_SoftwareDelay(7, BSP_DELAY_UNITS_MICROSECONDS); // Set active for 5-9 us
    R_IOPORT_PinWrite(&g_ioport_ctrl, DISP_RST, BSP_IO_LEVEL_HIGH);

    /* open MIPI DSI Interface */
    err = R_GLCDC_Open(&g_display0_ctrl, &g_display0_cfg);
    APP_ERR_TRAP(err);

    /* NOTE: cannot send commands for 5 ms after HW reset */
    R_BSP_SoftwareDelay(5, BSP_DELAY_UNITS_MILLISECONDS);

    /* Note: When video is started, timing of any further messages must carefully controlled to not interfere with transmission. */
    err = R_GLCDC_Start(&g_display0_ctrl);
    APP_ERR_TRAP(err);

    /* Wait for the LCD to display valid data, so no white flash on screen */
    R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);

    /* Enable the backlight */
    err = R_IOPORT_PinWrite(&g_ioport_ctrl, DISP_BLEN, BSP_IO_LEVEL_HIGH);
    APP_ERR_TRAP(err);

    while (1)
    {
        /* Enable access to the PFS registers. If using r_ioport module then register protection is automatically
         * handled. This code uses BSP IO functions to show how it is used.
         */
        R_BSP_PinAccessEnable();

        /* Update all board LEDs */
        for (uint32_t i = 0; i < leds.led_count; i++)
        {
            /* Get pin to toggle */
            uint32_t pin = leds.p_leds[i];

            /* Write to this pin */
            R_BSP_PinWrite((bsp_io_port_pin_t) pin, pin_level);
        }

        /* Protect PFS registers */
        R_BSP_PinAccessDisable();

        /* Toggle level for next write */
        if (BSP_IO_LEVEL_LOW == pin_level)
        {
            pin_level = BSP_IO_LEVEL_HIGH;
        }
        else
        {
            pin_level = BSP_IO_LEVEL_LOW;
        }

        /* Delay */
        R_BSP_SoftwareDelay(delay, bsp_delay_units);
    }
}

/*******************************************************************************************************************//**
 * This function is called at various points during the startup process.  This implementation uses the event that is
 * called right before main() to set up the pins.
 *
 * @param[in]  event    Where at in the start up process the code is currently at
 **********************************************************************************************************************/
void R_BSP_WarmStart (bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_RESET == event)
    {
#if BSP_FEATURE_FLASH_LP_VERSION != 0

        /* Enable reading from data flash. */
        R_FACI_LP->DFLCTL = 1U;

        /* Would normally have to wait tDSTOP(6us) for data flash recovery. Placing the enable here, before clock and
         * C runtime initialization, should negate the need for a delay since the initialization will typically take more than 6us. */
#endif
    }

    if (BSP_WARM_START_POST_C == event)
    {
        /* C runtime environment and system clocks are setup. */

        /* Configure pins. */
        R_IOPORT_Open(&g_ioport_ctrl, g_ioport.p_cfg);
    }
}
