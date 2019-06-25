## Contents

```sh
gap_lora/
├── aos.mk
├── Config.in
├── lora_config.h
├── README.md
└── test_lora.c
```

## Introduction

The gap_lora example will control lora modules through uart interfact, to communicate with lora gateway in both EU868 and CN470.

### Supported Boards

- GAPoc_A

### Build

```sh
# generate gap_lora@mgapuino8 default config
aos make gap_lora@mgapuino8 -c config

# or customize config manually
aos make menuconfig

# build
aos make
```

### Configure

In file lora_config.h, we can configure:
 - Different freqency: CN470 or EU868
 - All device IDs
 - Power level
 - Class mode
 - Join mode

### Result

```sh
Start execution on GAP8
Application is starting on gapoc
Going to init uart
uart->priv=0x1c07ffb8
CMD RESET: +RESET
0: Got Resp.:  RESET: OK
Going to send command
0: Got Resp.:  AT: OK
0: Got Resp.:  ID: DevAddr, 26:01:2B:84
0: Got Resp.:  ID: DevEui, 8C:F9:57:20:00:00:FF:99
0: Got Resp.:  ID: AppEui, 70:B3:D5:7E:D0:01:E1:3B
0: Got Resp.:  CLASS: A
0: Got Resp.:  DR: EU868
0: Got Resp.:  POWER: 6
0: Got Resp.:  LW: DC, OFF, 0
0: Got Resp.:  LW: JDC, OFF
OTAA Join Mode
0: Got Resp.:  KEY: APPKEY 2832613EB313F5F659D12EC1999EC597
0: Got Resp.:  MODE: LWOTAA
0: Got Resp.:  JOIN: Join failed
*** PROBLEM - LoRa Join Failed, retry ***
0: Got Resp.:  JOIN: Network joined
Got Resp.:  MSG: Start
```

The result on EU868 Server:
Payload:
4869212048656C6C6F2066726F6D20416C694F53212121
ASCII text:
Hi! Hello from AliOS!!!
