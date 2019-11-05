#include "pmsis.h"
#include "bsp/bsp.h"
#include "bsp/camera.h"
//#include "bsp/camera/himax.h"
#include "bsp/camera/mt9v034.h"

#ifndef PMSIS_APP_MAIN
#define PMSIS_APP_MAIN int main(void)
#endif

//#define WIDTH    324
//#define HEIGHT   244
#define WIDTH    320
#define HEIGHT   240
#define PPM_HEADER 40
#define BUFF_SIZE ((WIDTH*HEIGHT)+PPM_HEADER)

PI_L2 unsigned char buff[BUFF_SIZE];

typedef struct 
{
  uint32_t host_ready;
  uint32_t gap_ready;
  uint32_t buff_pointer;
  uint32_t buff_size;
  uint32_t gap_init;
} bridge_t;

PI_L2 bridge_t debug_struct = {0};

static void write_ppm_header(char *img_buff, unsigned int W, unsigned int H)
{
  	unsigned int Ind = 0, x, i, L;
  	unsigned char *Buffer = (unsigned char *) img_buff;

  	/* P5<cr>* */
  	Buffer[Ind++] = 0x50; Buffer[Ind++] = 0x35; Buffer[Ind++] = 0xA;

  	/* W <space> */
  	x = W; L=0;
  	while (x>0) { x = x/10; L++; }
  	x = W; i = 1;
  	while (x>0) { Buffer[Ind+L-i] = 0x30 + (x%10); i++; x=x/10; }
  	Ind += L;
  	Buffer[Ind++] = 0x20;

  	/* H <cr> */
  	x = H; L=0;
  	while (x>0) { x = x/10; L++; }
  	x = H; i = 1;
  	while (x>0) { Buffer[Ind+L-i] = 0x30 + (x%10); i++; x=x/10; }
  	Ind += L;
  	Buffer[Ind++] = 0xA;

  	/* 255 <cr> */
  	Buffer[Ind++] = 0x32; Buffer[Ind++] = 0x35; Buffer[Ind++] = 0x35; Buffer[Ind++] = 0xA;
}

static int open_camera(struct pi_device *device)
{
  struct pi_mt9v034_conf cam_conf;

  pi_mt9v034_conf_init(&cam_conf);
  cam_conf.format = PI_CAMERA_QVGA;

  pi_open_from_conf(device, &cam_conf);
  if (pi_camera_open(device))
    return -1;

  return 0;
}

static int test_entry()
{
//#ifdef USE_DEBUG_STRUCT_1c000190
  *(volatile uint32_t *)0x1c000190 = &debug_struct;
//#endif

  struct pi_device camera;
  pi_task_t task;

  if (open_camera(&camera))
  {
    return -1;
  }

  *(volatile uint32_t *)&debug_struct.buff_pointer = (uint32_t)buff;
  *(volatile uint32_t *)&debug_struct.buff_size = (uint32_t)BUFF_SIZE;

  // signal openocd that init is done
  *(volatile uint32_t *)&debug_struct.gap_init = 1;

  while(1)
  { 
    while((*(volatile uint32_t *)&debug_struct.host_ready) == 0)
    {
      pi_time_wait_us(1);
    }
   
    pi_camera_capture_async(&camera, &buff[PPM_HEADER], WIDTH*HEIGHT, pi_task_block(&task));
    
    pi_camera_control(&camera, PI_CAMERA_CMD_START, 0);

    pi_task_wait_on(&task);

    pi_camera_control(&camera, PI_CAMERA_CMD_STOP, 0);

    write_ppm_header(buff,WIDTH,HEIGHT);

    *(volatile uint32_t *)&debug_struct.gap_ready = 1;
    // wait for ACK
    while((*(volatile uint32_t *)&debug_struct.gap_ready) == 1)
    {
      pi_time_wait_us(1);
    }
  }

  return 0;
}

static void test_kickoff(void *arg)
{
  int ret = test_entry();
  pmsis_exit(ret);
}

PMSIS_APP_MAIN
{
  return pmsis_kickoff((void *)test_kickoff);
}
