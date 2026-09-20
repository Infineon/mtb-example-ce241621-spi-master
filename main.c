/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the SPI Master/Slave example
*
* Related Document: See README.md
*
********************************************************************************
* (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "cy_pdl.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Macros
*******************************************************************************/

#define SPI_MODE_MASTER  1
#define SPI_MODE_SLAVE   2
#define SPI_MODE_BOTH    3

#define SPI_MODE SPI_MODE_BOTH

#define CMD_TO_CMD_DELAY           (1000UL)

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* For the Retarget -IO (Debug UART) usage */
static cy_stc_scb_uart_context_t    DEBUG_UART_context;           /** UART context */
static mtb_hal_uart_t               DEBUG_UART_hal_obj;           /** Debug UART HAL object  */

/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function. It initializes the SPI master and/or slave blocks
* and the debug UART for serial communication. Based on the SPI_MODE setting,
* it operates as SPI master, slave, or both. In master mode, it sends commands
* to control slave LEDs. In slave mode, it receives commands and controls the
* user LED accordingly. The received and transmitted data is logged over serial
* terminal.
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    cy_en_scb_uart_status_t init_status;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
     {
          CY_ASSERT(0);
     }

    /* Start UART operation */
    init_status = Cy_SCB_UART_Init(DEBUG_UART_HW, &DEBUG_UART_config, &DEBUG_UART_context);
    if (init_status!=CY_SCB_UART_SUCCESS)
    {
         CY_ASSERT(0);
    }
    Cy_SCB_UART_Enable(DEBUG_UART_HW);

    /* Setup the HAL UART */
    result = mtb_hal_uart_setup(&DEBUG_UART_hal_obj, &DEBUG_UART_hal_config, &DEBUG_UART_context, NULL);

    /* HAL UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
         CY_ASSERT(0);
    }

    result = cy_retarget_io_init(&DEBUG_UART_hal_obj);

    /* HAL retarget_io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
         CY_ASSERT(0);
    }

#if (SPI_MODE == SPI_MODE_MASTER)
    int cmd_send = CYBSP_LED_STATE_OFF;

    /* Configure SPI block */
    Cy_SCB_SPI_Init(mSPI_HW, &mSPI_config, NULL);

    /* Set active slave select to line 0 */
    Cy_SCB_SPI_SetActiveSlaveSelect(mSPI_HW, CY_SCB_SPI_SLAVE_SELECT0);

    /* Enable SPI master block. */
    Cy_SCB_SPI_Enable(mSPI_HW);

    printf("\x1b[2J\x1b[;H");

    printf("************************************************************\r\n");
    printf("PSOC Control C3M/P8: SCB SPI master\r\n");
    printf("************************************************************\r\n\n");

    printf("Configuring SPI master...\r\n");

    for (;;)
    {
        /* Toggle the slave LED state */
        cmd_send = (cmd_send == CYBSP_LED_STATE_OFF) ?
                     CYBSP_LED_STATE_ON : CYBSP_LED_STATE_OFF;
        /* Send the command packet to the slave */
        printf("Data sending from Master %d\r\n", cmd_send);

        /* Send the command packet to the slave */
        result = Cy_SCB_SPI_Write(mSPI_HW, cmd_send);

        /* Give delay between commands */
        Cy_SysLib_Delay(CMD_TO_CMD_DELAY);
    }
#endif

#if (SPI_MODE == SPI_MODE_SLAVE)
    /* Initialize the SPI Slave */
    int rxBuffer;
    cy_stc_scb_spi_context_t sSPI_context;

    /* Configure the SPI block */
    Cy_SCB_SPI_Init(sSPI_HW, &sSPI_config, &sSPI_context);

    /* Set active slave select to line 0 */
    Cy_SCB_SPI_SetActiveSlaveSelect(sSPI_HW, CY_SCB_SPI_SLAVE_SELECT0);

    /* Enable the SPI Slave block */
    Cy_SCB_SPI_Enable(sSPI_HW);
    printf("\x1b[2J\x1b[;H");

    printf("************************************************************\r\n");
    printf("PSOC Control C3M/P8: SCB SPI slave\r\n");
    printf("************************************************************\r\n\n");

    printf("Configuring SPI slave...\r\n");

    for (;;)
    {
         rxBuffer = Cy_SCB_SPI_Read(sSPI_HW);

        if (rxBuffer == CYBSP_LED_STATE_OFF)
        {
            /* Turn off the user LED based on received command */
            printf("Data received from Master %d\r\n", rxBuffer);
             Cy_GPIO_Write(CYBSP_USER_LED2_PORT, CYBSP_USER_LED2_PIN, CYBSP_LED_STATE_OFF);
        }else if (rxBuffer == CYBSP_LED_STATE_ON)
        {
             /* Turn on the user LED based on received command */
            printf("Data received from Master %d\r\n", rxBuffer);
             Cy_GPIO_Write(CYBSP_USER_LED2_PORT, CYBSP_USER_LED2_PIN, CYBSP_LED_STATE_ON);
        }
    }
#endif

#if (SPI_MODE == SPI_MODE_BOTH)
    int cmd_send = CYBSP_LED_STATE_OFF;
    int rxBuffer;
    cy_stc_scb_spi_context_t sSPI_context;

    printf("\x1b[2J\x1b[;H");

    printf("************************************************************\r\n");
    printf("PSOC Control C3M/P8: SCB SPI master and slave\r\n");
    printf("************************************************************\r\n\n");

    printf("Configuring SPI master...\r\n");

    /* Configure SPI block */
    Cy_SCB_SPI_Init(mSPI_HW, &mSPI_config, NULL);

    /* Set active slave select to line 0 */
    Cy_SCB_SPI_SetActiveSlaveSelect(mSPI_HW, CY_SCB_SPI_SLAVE_SELECT0);

    /* Enable SPI master block. */
    Cy_SCB_SPI_Enable(mSPI_HW);

    printf("Configuring SPI slave...\r\n");

    /* Configure the SPI block */
    Cy_SCB_SPI_Init(sSPI_HW, &sSPI_config, &sSPI_context);

    /* Set active slave select to line 0 */
    Cy_SCB_SPI_SetActiveSlaveSelect(sSPI_HW, CY_SCB_SPI_SLAVE_SELECT0);

    /* Enable the SPI Slave block */
    Cy_SCB_SPI_Enable(sSPI_HW);

    for (;;)
    {
        /* Toggle the command value between LED ON and OFF states */
        cmd_send = (cmd_send == CYBSP_LED_STATE_OFF) ?
                     CYBSP_LED_STATE_ON : CYBSP_LED_STATE_OFF;
        /* Log the data being sent from master */
        printf("Data sending from Master %d\r\n", cmd_send);

        /* Transmit the command via SPI master */
        result = Cy_SCB_SPI_Write(mSPI_HW, cmd_send);

        /* Delay between master transmit and slave receive operations */
        Cy_SysLib_Delay(CMD_TO_CMD_DELAY);

        rxBuffer = Cy_SCB_SPI_Read(sSPI_HW);

       if (rxBuffer == CYBSP_LED_STATE_OFF)
       {
            /* Turn off the user LED based on received command */
            printf("Data received from Master %d\r\n", rxBuffer);
            Cy_GPIO_Write(CYBSP_USER_LED2_PORT, CYBSP_USER_LED2_PIN, CYBSP_LED_STATE_OFF);
       }else if (rxBuffer == CYBSP_LED_STATE_ON)
       {
            /* Turn on the user LED based on received command */
            printf("Data received from Master %d\r\n", rxBuffer);
            Cy_GPIO_Write(CYBSP_USER_LED2_PORT, CYBSP_USER_LED2_PIN, CYBSP_LED_STATE_ON);
       }
    }
#endif
}
/* [] END OF FILE */
