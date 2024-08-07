#include "r_ioport.h"
#include "r_mipi_dsi_api.h"

#include "hal_data.h"
#include "dsi_layer.h"

void mipi_dsi0_callback(mipi_dsi_callback_args_t * p_args);
static fsp_err_t dsi_layer_set_peripheral_max_return_msg_size(void);
static bool read_message(uint8_t page, uint8_t id);

static volatile bool g_message_sent = false;
static volatile bool g_message_received = false;
static volatile mipi_dsi_receive_result_t g_rx_result;

uint8_t g_read_data[16];

fsp_err_t dsi_layer_configure_peripheral()
{
    fsp_err_t err = FSP_SUCCESS;
    uint8_t page = 0;
    mipi_dsi_status_t status;

    err = dsi_layer_set_peripheral_max_return_msg_size(); // This must be performed prior to reading from display
    if (FSP_SUCCESS == err)
    {

        LCD_setting_table * p_entry = lcd_init_focuslcd;
        uint32_t counter = 0;
        while(p_entry->cmd_id != REGFLAG_END_OF_TABLE)
        {
            mipi_dsi_cmd_t cmd = { .channel = 0,
                                   .cmd_id = p_entry->cmd_id,
                                   .flags = p_entry->flags,
                                   .tx_len = p_entry->size,
                                   .p_tx_buffer = p_entry->buffer };


            /* Keep track of page for config validation */
            if(cmd.p_tx_buffer[0] == 0xFF)
            {
             page = cmd.p_tx_buffer[5];
            }

            if (p_entry->cmd_id == REGFLAG_DELAY)
            {
                R_BSP_SoftwareDelay(p_entry->size, BSP_DELAY_UNITS_MILLISECONDS);
            }
            else
            {
                g_message_sent = false;
                err = R_MIPI_DSI_Command (&g_mipi_dsi0_ctrl, &cmd);
                if (FSP_SUCCESS == err)
                {

                    while(!g_message_sent);


                    err = R_MIPI_DSI_StatusGet(&g_mipi_dsi0_ctrl, &status);
                    if (FSP_SUCCESS != err)
                    {
                    	__BKPT(0);
                    }
                    while (MIPI_DSI_LINK_STATUS_CH0_RUNNING & status.link_status)
                    {
                        err = R_MIPI_DSI_StatusGet(&g_mipi_dsi0_ctrl, &status);
                        if (FSP_SUCCESS != err)
                        {
                        	__BKPT(0);
                        }
                    }

                    /* Validate commands when possible */
                    if(read_message(page, cmd.p_tx_buffer[0]))
                    {
                        mipi_dsi_cmd_t read_msg = cmd;
                        read_msg.cmd_id = MIPI_DSI_CMD_ID_DCS_READ;
                        read_msg.flags |= MIPI_DSI_CMD_FLAG_BTA;

                        g_message_received = false;
                        err = R_MIPI_DSI_Command (&g_mipi_dsi0_ctrl, &read_msg);
                        if (FSP_SUCCESS != err)
                        {
                        	__BKPT(0);
                        }

                        while(!g_message_received);

                        err = R_MIPI_DSI_StatusGet(&g_mipi_dsi0_ctrl, &status);
                        if (FSP_SUCCESS != err)
                        {
                        	__BKPT(0);
                        }

                        while (MIPI_DSI_LINK_STATUS_CH0_RUNNING & status.link_status)
                        {
                            err = R_MIPI_DSI_StatusGet(&g_mipi_dsi0_ctrl, &status);
                            if (FSP_SUCCESS != err)
                            {
                            	__BKPT(0);
                            }
                        }
                        if((cmd.p_tx_buffer[1] != g_rx_result.data[1]))
                        {
                        	__BKPT(0);
                        }
                    }
                }
                else
                {
                	__BKPT(0);
                    break;
                }

            }
            p_entry++;
            counter++;
        }
    }
    uint8_t g_write_data = 0x0A;
    mipi_dsi_cmd_t read_display_power_mode;
    read_display_power_mode.cmd_id = MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_0_PARAM;
    read_display_power_mode.flags  = MIPI_DSI_CMD_FLAG_LOW_POWER;
    read_display_power_mode.channel = 0;
    read_display_power_mode.p_tx_buffer = &g_write_data;
    read_display_power_mode.tx_len = 1;


    g_message_sent = false;
    err = R_MIPI_DSI_Command (&g_mipi_dsi0_ctrl, &read_display_power_mode);
    if (FSP_SUCCESS != err)
    {
    	__BKPT(0);
    }

    while(!g_message_sent);


    do{
        err = R_MIPI_DSI_StatusGet(&g_mipi_dsi0_ctrl, &status);
        if (FSP_SUCCESS != err)
        {
        	__BKPT(0);
        }
    }while (MIPI_DSI_LINK_STATUS_CH0_RUNNING & status.link_status);

    read_display_power_mode.cmd_id = MIPI_DSI_CMD_ID_DCS_READ;
    read_display_power_mode.flags |= MIPI_DSI_CMD_FLAG_BTA;
    read_display_power_mode.channel = 0;
    read_display_power_mode.p_rx_buffer = g_read_data;
    g_message_received = false;
    err = R_MIPI_DSI_Command (&g_mipi_dsi0_ctrl, &read_display_power_mode);
    if (FSP_SUCCESS != err)
    {
    	__BKPT(0);
    }

    while(!g_message_received);


    do{
        err = R_MIPI_DSI_StatusGet(&g_mipi_dsi0_ctrl, &status);
        if (FSP_SUCCESS != err)
        {
        	__BKPT(0);
        }
    }while (MIPI_DSI_LINK_STATUS_CH0_RUNNING & status.link_status);



    __NOP();


    return err;
}


void mipi_dsi0_callback (mipi_dsi_callback_args_t * p_args)
{
    fsp_err_t err;

    switch (p_args->event)
    {
        case MIPI_DSI_EVENT_SEQUENCE_0:
        {
            g_message_sent |= (p_args->tx_status == MIPI_DSI_SEQUENCE_STATUS_DESCRIPTORS_FINISHED);
            __NOP();
            break;
        }
        case MIPI_DSI_EVENT_SEQUENCE_1:
        {
            __NOP();
            break;
        }
        case MIPI_DSI_EVENT_VIDEO:
        {
        	if (p_args->event == MIPI_DSI_EVENT_VIDEO)
        	{
				if (p_args->video_status | MIPI_DSI_VIDEO_STATUS_OVERFLOW)
				{
					__NOP();
				}
				else if (p_args->video_status | MIPI_DSI_VIDEO_STATUS_UNDERFLOW)
				{
					__NOP();
				}
        	}
            break;
        }
        case MIPI_DSI_EVENT_RECEIVE:
        {
            __NOP();
            g_message_received = true;
            g_rx_result        = *p_args->p_result;
            break;
        }
        case MIPI_DSI_EVENT_FATAL:
        {
            __NOP();
            break;
        }
        case MIPI_DSI_EVENT_PHY:
        {
            __NOP();
            break;
        }
        case MIPI_DSI_EVENT_POST_OPEN:
        {
            /* This case is called from R_DSI_Open(), so not from an interrupt */

            err = dsi_layer_configure_peripheral();
              if (FSP_SUCCESS != err)
              {
                  __BKPT(0);
              }
            break;
        }
        default:
        {
            __NOP();
            break;
        }
    }
}

/* See ILI9806E Datasheet, chapter  3.5.39
 *  1. Set max return packet size
 *  2. Read data be sending appropriate request */
static fsp_err_t  dsi_layer_set_peripheral_max_return_msg_size()
{
    fsp_err_t err;
    uint8_t msg_buffer[] = {0x02, 0x00};
    mipi_dsi_cmd_t return_size_cmd = { .channel = 0,
                                       .cmd_id      = MIPI_DSI_CMD_ID_SET_MAXIMUM_RETURN_PACKET_SIZE,
                                       .flags       = MIPI_DSI_CMD_FLAG_LOW_POWER,
                                       .tx_len      = 2,
                                       .p_tx_buffer = msg_buffer, };
    /* Set Return packet size */
    g_message_sent = false;
    err = R_MIPI_DSI_Command (&g_mipi_dsi0_ctrl, &return_size_cmd);
    if (FSP_SUCCESS == err)
    {
    	while(!g_message_sent);
    }

    return err;
}

/* Return false if command cannot be read back */
static bool read_message(uint8_t page, uint8_t id)
{
    bool validate;
    switch(id)
    {
        /* This is not a complete list */
        case 0x3A:
        case 0x51:
            validate = (page != 0);
            break;
        case 0xA0:
        case 0xA1:
        case 0xA2:
        case 0xA3:
        case 0xA4:
        case 0xA5:
        case 0xA6:
        case 0xA7:
        case 0xA8:
        case 0xA9:
        case 0xAA:
        case 0xAB:
        case 0xAC:
        case 0xAD:
        case 0xAE:
        case 0xAF:
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            validate = (page != 1);
            break;
        case 0x11:
        case 0x29:
        case 0x35:
        case 0xFF:
            validate = false;
            break;
        default:
            validate = true;
            break;
    }
    return validate;
}


