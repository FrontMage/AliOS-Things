/*
 * Copyright (c) 2019, GreenWaves Technologies
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of GreenWaves Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// #############################################################################
// ##### DECLARATIONS  ########################################################

// ==  Handy "defines" for application, used locally   =====================

#include "aos/kernel.h"
#include "aos/hal/uart.h"

#include "pmsis_driver/pin_names.h"
#include "pmsis_driver/pin_config.h"

#include "pmsis.h"
#include "pmsis_driver/uart/uart_internal.h"

#include "bsp/ble/nina_b112/nina_b112.h"
#include "bsp/gapoc_a.h"
#include "ble_protocol.h"


#define QUEUE_SIZE 4
#define AT_RESP_ARRAY_LENGTH 64

#define MIN(a, b) (((a)<(b))?(a):(b))

unsigned char face_image_buffer[QUEUE_SIZE][128*128];
short face_descriptor[QUEUE_SIZE][512];
char name[QUEUE_SIZE][16] = {"Lena", "Francesco"};

unsigned int queue_head = 0;
unsigned int queue_tail = 2;

char string_buffer[127];
char current_name[17];
short current_descriptor[512];

nina_ble_t ble;
pi_task_t name_task;
pi_task_t teskriptor_task;

char empty_response = '\0';
unsigned char ack = BLE_ACK;
unsigned char action = 0;
char* ptr = NULL;

static uint8_t rx_buffer[AT_RESP_ARRAY_LENGTH];

static void name_ready()
{
    printf("name_ready called!\n");
    current_name[16] = '\0';
    printf("Name %s got\n", current_name);
    nina_b112_send_data(&ble, &ack, 1, NULL);
    printf("BLE_ACK responded\n");
}

int application_start(int argc, char *argv[])
{
    char Resp_String[AT_RESP_ARRAY_LENGTH];

    if (nina_b112_open(&ble))
    {
        printf("Failed to open nina ble\n");
        return ;
    }
    printf("BLE UART init done\n");
    // --  Start NINA-B1 BLE module   -----

    // Init GPIO that will control NINA DSR in deasserted position
    pi_gpio_pin_write(0, GPIOA21_NINA17, 0);

    // Enable BLE (release reset)
    pi_gpio_pin_write(0, GPIOA2_NINA_RST, 1);

    pi_time_wait_us(1*1000*1000); // some waiting needed after BLE reset...

    printf("Sending cmd using pmsis bsp\n");
    nina_b112_AT_send(&ble, "E0");
    printf("Echo disabled\n");
    nina_b112_AT_send(&ble, "+UFACTORY");
    printf("Factory configuration restored\n");
    nina_b112_AT_send(&ble, "+UBTUB=FFFFFFFFFFFF");
    printf("Set UBTUB\n");
    nina_b112_AT_send(&ble, "+UBTLE=2");
    printf("Set UBTLE\n");
    nina_b112_AT_send(&ble, "+UBTLN=GreenWaves-GAPOC");
    printf("Set UBTLN\n");
    nina_b112_AT_query(&ble, "+UMRS?", (char *) rx_buffer);
    printf("BLE configuration : %s\n", rx_buffer);
    nina_b112_AT_send(&ble, "+UMRS=115200,2,8,1,1");
    nina_b112_AT_query(&ble, "+UMRS?", (char *) rx_buffer);
    printf("BLE configuration : %s\n", rx_buffer);
    nina_b112_AT_query(&ble, "+UBTLN?", (char *) rx_buffer);
    printf("BLE name : %s\n", rx_buffer);
    //nina_b112_close(&ble);

    printf("AT Config Done\n");

    // After Reboot of NINA,  central connects to NINA and NINA will provide
    // unsollicited AT event: +UUBTACLC:<peer handle,0,<remote BT address>)
    // (...but sometimes just provides empty event instead !?)

    // Just make sure NINA sends something as AT unsolicited response, therefore is ready :
    nina_b112_wait_for_event(&ble, Resp_String);
    printf("Received Event after reboot: %s\n", Resp_String);

    /*
    GAPOC_GPIO_Set_High(GAPOC_HEARTBEAT_LED);
    #ifdef __FREERTOS__
    vTaskDelay( 100 / portTICK_PERIOD_MS );
    #else
    wait(0.1);
    #endif

    GAPOC_GPIO_Set_Low(GAPOC_HEARTBEAT_LED);
    #ifdef __FREERTOS__
    vTaskDelay( 100 / portTICK_PERIOD_MS );
    #else
    wait(0.1);
    #endif
    */

    // Enter Data Mode
    nina_b112_AT_send(&ble, "O");
    printf("Data Mode Entered!\n");

    pi_time_wait_us(1*1000*1000); // leave some time for Central to be properly configured

    while(1)
    {
        nina_b112_get_data_blocking(&ble, &action, 1);
        nina_b112_get_data_blocking(&ble, (uint8_t *) current_name, 16);
        current_name[16] = '\0';
        int action_id = action;
        printf("Action: %d\n", action_id);
        printf("Text got: %s\n",current_name);
    }

    printf("Exiting BLE test\n");

    return 0;
}
