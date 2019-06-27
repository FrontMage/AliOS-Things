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

#include "chips/gap8/drivers/pin_names.h"
#include "chips/gap8/drivers/pin_config.h"

#include "pmsis.h"
#include "pmsis_driver/uart_internal.h"

extern int32_t hal_uart_recv_async(uart_dev_t *uart, void *data, uint32_t expect_size, uint32_t timeout, pi_task_t *task_block);

/* ***************************************************************************
// ***  Define DEBUG mode * */
#define DBG_PRINT   printf

// ***  Define Authentification as either ABP or OTAA (uncomment only one)  * */
#define OTAA 1
#define ABP 2

// ***  Define Authentification as either ABP or OTAA (uncomment only one)  * */
#define LORA_JOIN_METHOD    OTAA

/*  ************************************************************************* */

#define  AT_CMD_ARRAY_LENGTH       32   //arbitrary limit
#define  AT_RESP_ARRAY_LENGTH      64   //arbitrary limit

/* Variables used. */
#define BUFFER_SIZE      2048

/****************************************************************************/
/* PMSIS includes */

// ====  Application's Custom Types    ========================================

typedef  enum _AT_Resp_State
{  AT_RESP_NOT_STARTED,
    AT_RESP_IN_PROGRESS,
    AT_RESP_DONE
}   AT_Resp_State_t;


// ==== Application's exported global variables  ==============================
// <None>
#define HAL_WAIT_FOREVER 0xFFFFFFFFU
// need to compute with respect to ticks --- TODO
#define LORA_RESET_LATENCY_MSEC  2000


// ==== Application's own global variables  ====================================

static volatile uint32_t Tx_Index =0;
static volatile AT_Resp_State_t  AT_Resp_State;
char Response[AT_RESP_ARRAY_LENGTH];

GPIO_Type *const gpio_addrs[] = GPIO_BASE_PTRS;

char receive_byte;
static volatile uint8_t Cmd_string[AT_CMD_ARRAY_LENGTH];

static void GAPOC_LORA_AT_Cmd(uart_dev_t *uart, const char* pCmd_Core, char* Response_String)
    // Send AT read command and receive  response, char per char
    //   (!!Beware - means interrupts at rate =baudrate)
{
    int ret, index, real_size;
    AT_Resp_State = AT_RESP_NOT_STARTED;
    // !!! BEWARE -  did'nt (always) without static keywork -- we're passing a pointer to a string that must stay alive in memory

    pi_task_t task_block;
    pi_task_block(&task_block);

    ret = hal_uart_recv_async(uart, &receive_byte, 1, HAL_WAIT_FOREVER, &task_block);
    strcpy((char*)Cmd_string, (char*)"AT");
    strcat((char*)Cmd_string, (char*)pCmd_Core);
    strcat((char*)Cmd_string, (char*)"\r\n");

    // Now send command over UART :
    hal_uart_send(uart, (uint8_t *)Cmd_string, strlen((char *)Cmd_string), HAL_WAIT_FOREVER);

    while(AT_Resp_State != AT_RESP_DONE)
    {
        pi_task_wait_on(&task_block);
        pi_task_block(&task_block);
        //hal_uart_send(uart, &receive_byte, 1, HAL_WAIT_FOREVER);
        ret = hal_uart_recv_async(uart, &receive_byte, 1, HAL_WAIT_FOREVER, &task_block);

        if ((AT_Resp_State == AT_RESP_NOT_STARTED) && (receive_byte == '+'))
        {// looking for '+', start char of any response
            index = 0;
            AT_Resp_State = AT_RESP_IN_PROGRESS;
        }
        else if (AT_Resp_State == AT_RESP_IN_PROGRESS)
        {
            if ((receive_byte == '\r') || (receive_byte == '\n'))
            {//end of response (1st part if multiple)
                Response_String[index]='\0';  // obliterate EOL character and append end of string marker char
                AT_Resp_State = AT_RESP_DONE;
            }
            else
            {
                Response_String[index++] = receive_byte; // Receive chars between leading S3S4 and 2nd S3SA
            }
        }
    }
}

static inline void gapoc_any_pin_config(pin_name_e name_of_pin, pin_mode_e pull_en, port_mux_t alternate_fn_id)
{
    // Enable internal pull-up or pull-down if required :
    pin_mode(name_of_pin, (pin_mode_e) pull_en);
    // Function pin_mode() part of pinmap.c API --
    // It accepts both pins axpressed as e.g. A16 or, when it exists, its GPIO name counterpart e.g. GPIO_A31
    // The possible (enumerated) values of the Pull_En parameter in this function are a bit weird as they
    // call PullUp somehting that actually enables in the chip a pull resistor that can pull either low (most cases)
    // or high (for a few pins e.g. UART). But remapping a boolean PullEn value works well and presents a more
    // sensible naming


    // Set pin function (main or alternate functions -- alternate_fn_id=0 for main, 1 for alternate-1, etc.) :
    pin_function(name_of_pin, (int) alternate_fn_id);
    // Function function() part of pinmap.c API -- we make sure the parameter passed as alternate_fn_id is 0 to 3 by using
    // enum type port_mux_t, although pin_function() simply requires an integer (but expects it to be between 0 and 3!)
}


/*! @brief GPIO direction definition */
typedef enum _gpio_pin_direction
{
    uGPIO_DigitalInput = 0U,  /*!< Set current pin as digital input*/
    uGPIO_DigitalOutput = 1U, /*!< Set current pin as digital output*/
} gpio_pin_direction_t;

/*!
 * @brief Sets the output level of the multiple GPIO pins to the logic 1.
 *
 * @param base GPIO peripheral base pointer.
 * @param pin  GPIO pin number
 * @param direction GPIO pin direction
 */
static inline void GPIO_SetPinDirection(GPIO_Type *base, uint32_t pin, gpio_pin_direction_t direction)
{
    base->EN  |= (1U << pin);

    switch (direction) {
        case uGPIO_DigitalInput:
            base->DIR &= ~(1U << pin);
            break;
        case uGPIO_DigitalOutput:
            base->DIR |= (1U << pin);
            break;
    }
}

static inline uint8_t GAPOC_GpioPin_As_Pure_Output(pin_name_e GPIO_Name )
    // NOTE: even if the pad is bidir, GPIO clock is not enabled here (to save power) so pin value cannot be read back
{
    if (GET_IS_GPIO(GPIO_Name))
    {
        GPIO_SetPinDirection(gpio_addrs[GET_GPIO_PORT(GPIO_Name)], GET_GPIO_NUM(GPIO_Name), uGPIO_DigitalOutput);
        // using API from gap_gpio.h

        // Disable the GPIO clock to save power
        gpio_addrs[GET_GPIO_PORT(GPIO_Name)]->EN &= ~(1U << GET_GPIO_NUM(GPIO_Name));
        return 0;
    }
    else
    {
        return -1;
    }
}

/*!
 * @brief Sets the output level of the multiple GPIO pins to the logic 1 or 0.
 *
 * @param base    GPIO peripheral base pointer.
 * @param pin     GPIO pin number
 * @param output  GPIO pin output logic level.
 *        - 0: corresponding pin output low-logic level.
 *        - 1: corresponding pin output high-logic level.
 */
static inline void GPIO_WritePinOutput(GPIO_Type *base, uint32_t pin, uint8_t output)
{
    if (output == 0U)
    {
        base->OUT &= ~(1U << pin);
    }
    else
    {
        base->OUT |= (1U << pin);
    }
}

static inline void GAPOC_GPIO_Set_High(pin_name_e GPIO_Name)
{
    GPIO_WritePinOutput(gpio_addrs[GET_GPIO_PORT(GPIO_Name)], GET_GPIO_NUM(GPIO_Name), 1);
}


static inline void GAPOC_GPIO_Set_Low(pin_name_e GPIO_Name)
{
    GPIO_WritePinOutput(gpio_addrs[GET_GPIO_PORT(GPIO_Name)], GET_GPIO_NUM(GPIO_Name), 0);
}

void gapoc_gpio_init_pure_output_high(pin_name_e GPIO_Name)
{
    gapoc_any_pin_config(GPIO_Name, PULL_NONE, PORT_MUX_GPIO );  // set pin as GPIO with no pull-up/down
    GAPOC_GPIO_Set_High(GPIO_Name);
    GAPOC_GpioPin_As_Pure_Output(GPIO_Name);
}


void gapoc_gpio_init_pure_output_low(pin_name_e GPIO_Name)
{
    gapoc_any_pin_config(GPIO_Name, PULL_NONE, PORT_MUX_GPIO );  // set pin as GPIO with no pull-up/down
    GAPOC_GPIO_Set_Low(GPIO_Name);
    GAPOC_GpioPin_As_Pure_Output(GPIO_Name);
}


typedef struct {
    pin_name_e pin;
    int peripheral;
    int function;
} PinMap;

/************UART***************/
const PinMap PinMap_UART_TX[] = {
    {A7, 0, 0},
    {NC  ,  NC    , 0}
};

const PinMap PinMap_UART_RX[] = {
    {B7, 0, 0},
    {NC  ,  NC    , 0}
};

#define LORA_UART_AT_BAUDRATE_bps  9600
// Convert to pure reg access!
static void gapoc_lora_pin_config(void)
{
    //pinmap_pinout(UART_TX, PinMap_UART_TX);
    //pinmap_pinout(UART_RX, PinMap_UART_RX);

    gapoc_any_pin_config(UART_TX, PULL_NONE, PORT_MUX_ALT0);
    gapoc_any_pin_config(UART_RX, PULL_NONE, PORT_MUX_ALT0);
}

static inline uint8_t GAPOC_GpioPin_As_HighZ(pin_name_e GPIO_Name )
    // This is putting pin in input mode, but with GPIO clock disabled
    // to save power (pin can't actually be read back then)
{
    if (GET_IS_GPIO(GPIO_Name))
    {
        GPIO_SetPinDirection(gpio_addrs[GET_GPIO_PORT(GPIO_Name)], GET_GPIO_NUM(GPIO_Name), uGPIO_DigitalInput);
        // using API from gap_gpio.h

        // disconnect any pull-up/down:
        pin_mode(GPIO_Name, (pin_mode_e) PULL_NONE);

        // Disable the GPIO clock
        gpio_addrs[GET_GPIO_PORT(GPIO_Name)]->EN &= ~(1U << GET_GPIO_NUM(GPIO_Name));
        return 0;
    }
    else
    {
        return -1;
    }
}

// This is same as High-Z mode (GPIO clock off to save power, but pin is pulled rather than left floating
void GAPOC_GPIO_Init_JustPull(pin_name_e GPIO_Name)
{
    gapoc_any_pin_config(GPIO_Name, PULL_NONE, PORT_MUX_GPIO );  // set pin as GPIO with no pull-up/down
    GAPOC_GpioPin_As_HighZ(GPIO_Name);
}

int GAPOC_BSP_Board_Init(void)
{
    GPIO_Type *const gpio_addrs[] = GPIO_BASE_PTRS;
    PORT_Type *const port_addrs[] = PORT_BASE_PTRS;
#if 0
    int voltage_mV = VOLTAGE_mV;
    int fqcy_Hz = CORE_FQCY_MHZ * 1000000;

    // ---------------------------------------------

    // -- Set voltage   ----------
    if (PMU_SetVoltage(voltage_mV, 1)) {
        DBG_PRINT("Error when changing voltage\n");
        return -1;
    }

    // -- Set frequency  ----------
    if (FLL_SetFrequency(uFLL_SOC, fqcy_Hz, 1) == -1) {
        DBG_PRINT("Error of changing freqency, check Voltage value!\n");
        return -1;
    }
    DBG_PRINT("Frequency = %d , Voltage = %d mv\n", FLL_GetFrequency(uFLL_SOC), voltage_mV);
#endif


    // -- Default init for GPIOs -------------------------
    // NB:  GPIO_xx_yy refers to GPIO #xx available on physical pin #yy of GAP8
    // !!! #yy is omitted if only 1 physical pin matches  !!!

    // -- SPECIAL CASES:  5x2 pins that share same GPIO source

    // NB - The same GPIOA0 can be routed to GAP_A4 (=GAPA4_CONN on GAPOC) or GAP_A3 (= GAPA3_CONN on GAPOC) -- beware of conflicts !
    //  PLUS: if DIP switch pos. 6 is closed, GAP_A3 also drives Green "heatbeat" LED
    // *** GAP_A4
    gapoc_any_pin_config( A4, PULL_NONE, PORT_MUX_ALT0 );  // pin GAP_A4 keeps default function = SPIM1_MISO (input)
    // enable pull, as by default nothing on connector, so pin not driven
    // *** GAP_A3  : Green LED ON  (GPIO_A0_A3) (if DIP switch S6 set to allow Hearbeat LED) / GAPA3_CONN driven high from GAP
    gapoc_gpio_init_pure_output_high(GPIO_A0_A3);



    // NB - The same GPIOA1 can be routed to GAP_B2 (= GPIO_LED_G/NINA_SW1 on GAPOC) or GAP_B3 (= GAPB3_CONN on GAPOC) -- beware of conflicts !
    // *** GAP_B3
    GAPOC_GPIO_Init_JustPull(GPIO_A1_B3);
    // *** GAP_B2
    GAPOC_GPIO_Init_JustPull(GPIO_A1_B2);  // = GPIO_LED_G/NINA_SW1, normally driven by NINA but can drive from GAP at Nina start-up (Nina boot mode)

    // NB - The same GPIOA2 can be routed to GAP_A2(=GPIO_NINA_RST#) or GAP_A5 (=GAP_A5_CONN)
    //     ==> !! *** DO NOT USE GAP_A5 as GPIO (any other alt function ok) ***
    //     (unless NINA is not use -- even in that case, beware of NINA power consumption if toggling its RST#)
    // *** GAP_A2
    gapoc_gpio_init_pure_output_low(GPIO_A2_A2);      // = GPIO_NINA_NRST  // TBD - possible conflcit with GAP_A5_CONN above ?? (also pertains to GPIO A2)
    // *** GAP_A5
    gapoc_any_pin_config( A5, PULL_NONE, PORT_MUX_ALT0 );  // pin GAP_A5 routed to GAPA5_CONN keeps default function = SPIM1_CS0 (output)


    // NB - The same GPIOA3 can be routed to GAP_B1(=GPIO_CIS_EXP) or GAP_B4(=GAP_B4_CONN) -- beware of conflicts !
    //  ==> !! *** DO NOT USE GAP_B4 as GPIO if GPIO_CIS_EXP is required by CIS (single shot mode)
    // *** GAP_B1
    gapoc_gpio_init_pure_output_low(GPIO_A3_B1);      // = GPIO_CIS_EXP
    // *** GAP_B4
    gapoc_any_pin_config( B4, PULL_NONE, PORT_MUX_ALT0 );  // pin GAP_B4 routed to GAPB4_CONN keeps default function = SPIM1_SCK (output)


    // NB - The same GPIOA4 can be routed to GAP_A44 (GPIO_1V8_EN) or GAP_A43 (=CIS_PCLK)
    //      No conflict here since GAP_A43 dedicated to CPI interface, not available for use as GPIO
    // *** GAP_A44
#if (GAPOC_START_HYPERBUS_1V8==0)
    gapoc_gpio_init_pure_output_low (GPIO_A4_A44);     // = GPIO_1V8_EN  -->  1V8 not supplied to HyperMem after init
#else
    gapoc_gpio_init_pure_output_high(GPIO_A4_A44);     // = GPIO_1V8_EN  -->  1V8 stil supplied to HyperMem after init (in line w/ weak Pull-up on this signal on-board)
#endif
    // *** GAP_A43
    gapoc_any_pin_config( A43, PULL_NONE, PORT_MUX_ALT0 );  // pin GAP_A43 keeps default function = CAM_PCLK (input)
    // If CIS is not powered then PCLK=0V (sure??), if powered then PCLK is driven to 0 or 1 ==> won't be floating, no need to enable Pull Resistor


    // NB - The same GPIOA5 can be routed to GAP_B40 (=GPIO_CIS_PWRON) or GAP_A37 (=CIS_HSYNC)
    //      No conflict here since GAP_A37 dedicated to CPI interface, not available for use as GPIO
    // *** GAP_B40
    gapoc_gpio_init_pure_output_low(GPIO_A5_B40);     // = GPIO_CIS_PWRON
    // *** GAP_A37
    gapoc_any_pin_config( A37, PULL_NONE, PORT_MUX_ALT0 );  // pin GAP_A37 keeps default function = CAM_HSYNC (input)
    // If CIS is not powered then HSYNC=0V (sure??), if powered then HSYNC is driven to 0 or 1 ==> won't be floating, no need to enable Pull Resistor


    // -- REMAINING GPIOs (no possible GPIO conflict issues) :

    // *** GAP_B12
    GAPOC_GPIO_Init_JustPull(GPIO_A19);   // = GAPB12_CONN
    // *** GAP_A13
    GAPOC_GPIO_Init_JustPull(GPIO_A18);   // = GAPA13_CONN


    // *** GAP_B13
    gapoc_gpio_init_pure_output_high(GPIO_A21);  // = GPIO_A21 =GPIO_NINA17, can be used as UART_DSR input on Nina (high= de-asserted)



    // TODO - TBD - Also initalize non-GPIO pins here ?

    return 0;
}


int application_start(int argc, char *argv[])
{
    DBG_PRINT("Application is starting on gapoc\n");
    /*
    // In this version of release, we are not using other devices, therefore, keep the configure as default is enough for now. This part need to be activated later.

    GAPOC_BSP_Board_Init();

    gapoc_gpio_init_pure_output_low(GPIO_A2_A2);

    gapoc_any_pin_config(B7, PULL_NONE, PORT_MUX_ALT0);
    */

    // in reality, this conf will be ignored for now... need to implement a few more cleans in uart...
    uart_config_t uartConfig;
    uartConfig.baud_rate = 9600;
    uartConfig.mode = MODE_TX_RX;
    uartConfig.stop_bits = STOP_BITS_1;

    uart_dev_t uart;
    uart.port = 0;
    uart.config = uartConfig;

    DBG_PRINT("Going to init uart\n");
    hal_uart_init(&uart);
    DBG_PRINT("uart->priv=%p\n",uart.priv);

    // gapoc_lora_pin_config();
    // uart should now be ready for lora!

    // --    Reset LoRa modem (optional) --------------

    // led
    //GAPOC_GPIO_Set_High(GPIO_A0_A3);

    GAPOC_LORA_AT_Cmd(&uart, "+RESET", Response);
    DBG_PRINT("%d: Got Resp.:  %s\n", AT_Resp_State, Response);
    aos_msleep(LORA_RESET_LATENCY_MSEC);
    //GAPOC_GPIO_Set_Low(GAPOC_HEARTBEAT_LED);

    // --    Send AT Commands --------------

    // First, Just send "AT" to check status OK  --
    DBG_PRINT("Going to send command\n");
    GAPOC_LORA_AT_Cmd(&uart, "", Response);
    DBG_PRINT("%d: Got Resp.:  %s\n", AT_Resp_State, Response);
    if (strncmp( Response, "AT: OK", 6) != 0)
    {
        DBG_PRINT("*** PROBLEM - LoRa Modem not responding ok ***\n");
    }
    else
    {

        // BEWARE -- At present, GAPOC_LORA_AT_Cmd is designed to handle a single line (ended by CR/LF) of response
        //  so take care not to send "combined" AT commands/queries that return multiple responses

        GAPOC_LORA_AT_Cmd(&uart, "+ID=DevAddr,\"26 01 15 B4\"", Response);             // Device Address: 4 bytes
        DBG_PRINT("%d: Got Resp.:  %s\n", AT_Resp_State, Response);

        GAPOC_LORA_AT_Cmd(&uart, "+ID=DevEui,\"d8 96 e0 ff ff 01 17 fb\"", Response);  // Device Extended Unique ID: 8 bytes
        DBG_PRINT("%d: Got Resp.:  %s\n", AT_Resp_State, Response);

        GAPOC_LORA_AT_Cmd(&uart, "+ID=AppEui,\"d8 96 e0 ff ff 00 00 00\"", Response);  // App Extended Unique ID: 8 bytes
        DBG_PRINT("%d: Got Resp.:  %s\n", AT_Resp_State, Response);

        GAPOC_LORA_AT_Cmd(&uart, "+CLASS=A", Response);
        DBG_PRINT("%d: Got Resp.:  %s\n", AT_Resp_State, Response);

        GAPOC_LORA_AT_Cmd(&uart, "+DR=CN470", Response);     // Band scheme: eg. CN470 or EU868
        DBG_PRINT("%d: Got Resp.:  %s\n", AT_Resp_State, Response);

        GAPOC_LORA_AT_Cmd(&uart, "+POWER=6", Response);  // Will be rounded to closest supported value (depending on band used)
        DBG_PRINT("%d: Got Resp.:  %s\n", AT_Resp_State, Response);

        GAPOC_LORA_AT_Cmd(&uart, "+LW=DC, OFF", Response);   // Duty Cycle Limitation On/Off
        DBG_PRINT("%d: Got Resp.:  %s\n", AT_Resp_State, Response);

        GAPOC_LORA_AT_Cmd(&uart, "+LW=JDC,OFF", Response);   // Join Duty Cycle Limitation On/Off
        DBG_PRINT("%d: Got Resp.:  %s\n", AT_Resp_State, Response);


#if LORA_JOIN_METHOD==ABP
        DBG_PRINT("ABP Join Mode\n");
        GAPOC_LORA_AT_Cmd(&uart, "+KEY=NwkSKey,\"01 23 45 67 89 AB CD EF 01 23 45 67 89 AB CD EF\"", Response);    // Network Session Key: 16 bytes
        DBG_PRINT("%d: Got Resp.:  %s\n", AT_Resp_State, Response);

        GAPOC_LORA_AT_Cmd(&uart, "+KEY=AppSKey,\"01 23 45 67 89 AB CD EF 01 23 45 67 89 AB CD EF\"", Response);    // App Session Key: 16 bytes
        DBG_PRINT("%d: Got Resp.:  %s\n", AT_Resp_State, Response);

        GAPOC_LORA_AT_Cmd(&uart, "+MODE=LWABP",Response);
        DBG_PRINT("%d: Got Resp.:  %s\n", AT_Resp_State, Response);

#elif LORA_JOIN_METHOD==OTAA
        DBG_PRINT("OTAA Join Mode\n");
        GAPOC_LORA_AT_Cmd(&uart, "+KEY=AppKey,\"76 5F C6 CC F0 CF 1F E2 D3 3E 2F B7 17 EC F6 E4\"", Response);    // Applicatin Key: 16 bytes
        DBG_PRINT("%d: Got Resp.:  %s\n", AT_Resp_State, Response);

        GAPOC_LORA_AT_Cmd(&uart, "+MODE=LWOTAA",Response);
        DBG_PRINT("%d: Got Resp.:  %s\n", AT_Resp_State, Response);

        GAPOC_LORA_AT_Cmd(&uart, "+JOIN", Response);
        DBG_PRINT("%d: Got Resp.:  %s\n", AT_Resp_State, Response);
#else
#error "You didn't select a valid join Method"
#endif

    }
    aos_msleep(LORA_RESET_LATENCY_MSEC);

    while (1)
    {
    }
    return 0;
}
