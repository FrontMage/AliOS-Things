#ifndef SETUP_H
#define SETUP_H

//Cascade stride
#define MAX_NUM_OUT_WINS 15

#if (VERBOSE != 0)
#define PRINTF printf
#else
#define PRINTF(...)
#endif

//#define HAVE_DISPLAY
#define HAVE_CAMERA

#define VERBOSE 1

#if defined(_FOR_GAPOC_)
#define CAMERA_WIDTH    (((640/2)/4)*4)
#define CAMERA_HEIGHT   (((480/2)/4)*4)
#else
#define CAMERA_WIDTH 324
#define CAMERA_HEIGHT 244
#endif

#define WOUT_INIT 64
#define HOUT_INIT 48

#define LCD_OFF_X 40
#define LCD_OFF_Y 60

#define REID_L2_THRESHOLD 1000000

#define FUNCTION_PIN 25 // button pin id for pi_pad_init call
#define BUTTON_PIN_ID 19 // button pin id for pi_gpio_ calls

#endif //SETUP_H
