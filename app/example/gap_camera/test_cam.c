#include <stdio.h>
#include <aos/kernel.h>

#include "pmsis.h"
#include "bsp/display/ili9341.h"
#include "bsp/camera/himax.h"
#include "bsp/camera/mt9v034.h"

#define QVGA 1
//#define QQVGA 1

#define USE_DISPLAY 1

#ifdef QVGA
#ifdef HIMAX
#define CAM_WIDTH    324
#define CAM_HEIGHT   244
#else
#define CAM_WIDTH    320
#define CAM_HEIGHT   240
#endif
#else
#define CAM_WIDTH    160
#define CAM_HEIGHT   120
#endif

#define LCD_WIDTH    320
#define LCD_HEIGHT   240

static unsigned char *imgBuff0;
static struct pi_device ili;
static pi_buffer_t buffer;
static struct pi_device device;

static void lcd_handler(void *arg);
static void cam_handler(void *arg);


static int open_display(struct pi_device *device)
{
#ifdef USE_DISPLAY
  struct ili9341_conf ili_conf;

  ili9341_conf_init(&ili_conf);

  pi_open_from_conf(device, &ili_conf);

  if (display_open(device))
    return -1;
#endif

  return 0;
}


static int open_camera_himax(struct pi_device *device)
{
#ifdef HIMAX
  struct himax_conf cam_conf;

  himax_conf_init(&cam_conf);

#ifdef QVGA
  cam_conf.format = CAMERA_QVGA;
#endif

  pi_open_from_conf(device, &cam_conf);
  if (camera_open(device))
    return -1;

#endif
  return 0;
}

#define MT9V034_BLACK_LEVEL_CTRL  0x47
#define MT9V034_BLACK_LEVEL_AUTO  (0 << 0)
#define MT9V034_AEC_AGC_ENABLE    0xaf
#define MT9V034_AEC_ENABLE_A      (1 << 0)
#define MT9V034_AGC_ENABLE_A      (1 << 1)

static int open_camera_mt9v034(struct pi_device *device)
{
  struct mt9v034_conf cam_conf;

  mt9v034_conf_init(&cam_conf);

#ifdef QVGA
  cam_conf.format = CAMERA_QVGA;
#endif
#ifdef QQVGA
  cam_conf.format = CAMERA_QQVGA;
#endif

  pi_open_from_conf(device, &cam_conf);
  if (camera_open(device))
    return -1;
  printf("camera opened\n");

  uint16_t val = MT9V034_BLACK_LEVEL_AUTO;
  camera_reg_set(device, MT9V034_BLACK_LEVEL_CTRL, (uint8_t *) &val);
  val = MT9V034_AEC_ENABLE_A|MT9V034_AGC_ENABLE_A;
  camera_reg_set(device, MT9V034_AEC_AGC_ENABLE, (uint8_t *) &val);

  return 0;
}

static int open_camera(struct pi_device *device)
{
#ifdef HIMAX
  return open_camera_himax(device);
#else
  return open_camera_mt9v034(device);
#endif
  return -1;
}

int application_start(int argc, char *argv[])
{
    printf("Entering main controller...\n");

#ifdef __PULP_OS__
  rt_freq_set(__RT_FREQ_DOMAIN_FC, 250000000);
#endif

  imgBuff0 = (unsigned char *)pmsis_l2_malloc((CAM_WIDTH*CAM_HEIGHT)*sizeof(unsigned char));
  if (imgBuff0 == NULL) {
      printf("Failed to allocate Memory for Image \n");
      pmsis_exit(1);
  }

  if (open_display(&ili))
  {
    printf("Failed to open display\n");
    pmsis_exit(-1);
  }

  if (open_camera(&device))
  {
    printf("Failed to open camera\n");
    pmsis_exit(-2);
  }

#ifdef HIMAX
  buffer.data = imgBuff0+CAM_WIDTH*2+2;
  buffer.stride = 4;

  // WIth Himax, propertly configure the buffer to skip boarder pixels
  pi_buffer_init(&buffer, PI_BUFFER_TYPE_L2, imgBuff0+CAM_WIDTH*2+2);
  pi_buffer_set_stride(&buffer, 4);
#else
  buffer.data = imgBuff0;
  pi_buffer_init(&buffer, PI_BUFFER_TYPE_L2, imgBuff0);
#endif
  pi_buffer_set_format(&buffer, CAM_WIDTH, CAM_HEIGHT, 1, PI_BUFFER_FORMAT_GRAY);

  printf("Camera start.\n");
  int i=0;
  while (i<200)
  {
      printf("going to control\n");
      camera_control(&device, CAMERA_CMD_START, 0);
      camera_capture(&device, imgBuff0, CAM_WIDTH*CAM_HEIGHT);
      camera_control(&device, CAMERA_CMD_STOP, 0);
      i++;
  }
  printf("Camera stop, loop %d times\n", i);

  return 0;
}

