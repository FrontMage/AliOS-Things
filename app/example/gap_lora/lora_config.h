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

#ifndef __GAP_LORA_CONFIG__
#define __GAP_LORA_CONFIG__

// ***  Define Authentification as either ABP or OTAA (uncomment only one)  * */
#define OTAA 1
#define ABP 2
#define LORA_JOIN_METHOD    OTAA

#define CN              1
#define EU              2
#define ZONE            EU

#define LORA_UART_AT_BAUDRATE_bps  9600

#if  ZONE==CN
#define DEVADDR         "\"26 01 15 B4\""
#define DEVEUI          "\"d8 96 e0 ff ff 01 17 fb\""
#define APPEUI          "\"d8 96 e0 ff ff 00 00 00\""
#define FREQ            "+DR=CN470"
#else
#define DEVADDR         "\"26 01 2B 84\""
#define DEVEUI          "\"8C F9 57 20 00 00 FF 99\""
#define APPEUI          "\"70 B3 D5 7E D0 01 E1 3B\""
#define FREQ            "+DR=EU868"
#endif

#define POWER_LEVEL     "6"
#define CLASS_MODE      "A"

#if LORA_JOIN_METHOD==ABP
#define NWKSKEY         "\"AC 7A 0F 6E 38 54 F2 11 2E 25 76 21 2B 67 C1 C3\""
#define APPSKEY         "\"8A 96 E8 B9 E9 D4 2D 85 23 2C 1F 81 C1 8B 87 27\""
#define MODE            "LWABP"
#elif LORA_JOIN_METHOD==OTAA
#define APPKEY          "\"28 32 61 3E B3 13 F5 F6 59 D1 2E C1 99 9E C5 97\""
#define MODE            "LWOTAA"
#endif

#define FORCE_JOIN      //"=FORCE"

#define MSG             "+MSG=\"Hi! Hello from AliOS!!!\""
#define LOOP            3
#define LATENCY         10

// *** Define the AT Commands **/
//
#define AT              ""
#define AT_RESET        "+RESET"
#define AT_DEVADDR      "+ID=DevAddr,"DEVADDR
#define AT_DEVEUI       "+ID=DevEui,"DEVEUI
#define AT_APPEUI       "+ID=AppEui,"APPEUI

#define AT_CLASS        "+CLASS="CLASS_MODE

#define POWER           "+POWER="POWER_LEVEL

#define DCL_ON          "+LW=DC, ON"
#define DCL_OFF         "+LW=DC, OFF"

#define JOIN_DCL_ON     "+LW=JDC,ON"
#define JOIN_DCL_OFF    "+LW=JDC,OFF"

#define AT_NWKSKEY      "+KEY=NwkSKey,"NWKSKEY
#define AT_APPSKEY      "+KEY=AppSKey,"APPSKEY
#define AT_APPKEY       "+KEY=AppKey,"APPKEY
#define JOIN_MODE       "+MODE="MODE
#define JOIN_CMD        "+JOIN"FORCE_JOIN

#endif
