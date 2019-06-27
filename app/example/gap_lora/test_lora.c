// #############################################################################
// ##### DECLARATIONS  ########################################################

// ==  Handy "defines" for application, used locally   =====================


#include "aos/kernel.h"
#include "aos/hal/uart.h"


/* ***************************************************************************
// ***  Define Authentification as either ABP or OTAA (uncomment only one)  * */
//#define LORA_JOIN_METHOD    ABP
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

static void GAPOC_LORA_AT_Cmd(uart_dev_t *uart, const char* pCmd_Core, char* Response_String)
    // Send AT read command and receive  response, char per char
    //   (!!Beware - means interrupts at rate =baudrate)
{
    
    AT_Resp_State = AT_RESP_NOT_STARTED;
    static volatile uint8_t Cmd_string[AT_CMD_ARRAY_LENGTH];  
    // !!! BEWARE -  did'nt (always) without static keywork -- we're passing a pointer to a string that must stay alive in memory

    strcpy((char*)Cmd_string, (char*)"AT");
    strcat((char*)Cmd_string, (char*)pCmd_Core);
    strcat((char*)Cmd_string, (char*)"\r\n");

    // Now senc ommand over UART :
    hal_uart_send(&uart, (uint8_t *)Cmd_string, strlen((char *)Cmd_string),HAL_WAIT_FOREVER);

    int end_of_cmd = 0;
    char receive_byte;
    while(AT_Resp_State != AT_RESP_DONE)
    {
        int ret, index;
        ret = hal_uart_recv(&uart, &receive_byte, 1, HAL_WAIT_FOREVER);
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

// Convert to pure reg access!
static void gapoc_lora_pin_config(void)
{
#if 0
    // to avoid bug with hyper?
    GAPOC_AnyPin_Config( B7, NOPULL, uPORT_MuxAlt0 );  // pin GAP_B7 keeps default function = SPIM0_SCK (output)
#define LORA_UART_AT_BAUDRATE_bps  9600
serial_t hUart;
    GAPOC_AnyPin_Config(UART_TX, NOPULL, uPORT_MuxAlt0);  // Remove pull-up on UART_TX
    GAPOC_AnyPin_Config(UART_RX, NOPULL, uPORT_MuxAlt0);  // Remove pull-up on UART_TX
#endif
}

int application_start(int argc, char *argv[])
{
    gapoc_lora_pin_config();

    // in reality, this conf will be ignored for now... need to implement a few more cleans in uart...
    uart_config_t uartConfig;
    uartConfig.baud_rate = 9600; 
    uartConfig.mode = MODE_TX_RX;
    uart_dev_t uart;
    uart.port = 0;  
    uart.config = uartConfig;
    hal_uart_init(&uart);

    // uart should now be ready for lora!

    aos_msleep(LORA_RESET_LATENCY_MSEC);

    while (1)
    {
    }
    return 0;
}
