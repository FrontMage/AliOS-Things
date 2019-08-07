#include <stdio.h>
#include <aos/kernel.h>

/****************************************************************************/
/* PMSIS includes */
#include "pmsis.h"
#include "pmsis_types.h"

#include "rtos/pmsis_os.h"
#include "rtos/pmsis_driver_core_api/pmsis_driver_core_api.h"

#include "pmsis_api/include/drivers/hyperbus.h"
#include "pmsis_cluster/drivers/delegate/hyperbus/hyperbus_cl_internal.h"

#include "rtos/os_frontend_api/pmsis_task.h"
#include "pmsis_cluster/cluster_sync/fc_to_cl_delegate.h"
#include "pmsis_cluster/cluster_team/cl_team.h"

/* PMSIS BSP includes */
#include "bsp/gapoc_a.h"
#include "bsp/display/ili9341.h"
#include "bsp/camera/mt9v034.h"
#include "bsp/camera/himax.h"
#include "stdio.h"

#include "ImageDraw.h"
#include "setup.h"
#include "FaceDetKernels.h"

#include "faceDet.h"

#ifdef HIMAX
#define CAM_WIDTH    324
#define CAM_HEIGHT   244
#else
#define CAM_WIDTH    320
#define CAM_HEIGHT   240
#endif

#define LCD_WIDTH    320
#define LCD_HEIGHT   240

//#define DEBUG_PRINTF(...)
#define DEBUG_PRINTF printf

static unsigned char *imgBuff0;
static struct pi_device ili;
static pi_buffer_t buffer;
static struct pi_device cam;

L2_MEM unsigned char *ImageOut;
L2_MEM unsigned int *ImageIntegral;
L2_MEM unsigned int *SquaredImageIntegral;
L2_MEM char str_to_lcd[100];

struct pi_device cluster_dev;
struct pi_cluster_task *task;
struct cluster_driver_conf conf;
ArgCluster_T ClusterCall;



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
  struct himax_conf cam_conf;

  himax_conf_init(&cam_conf);

  cam_conf.format = CAMERA_QVGA;

  pi_open_from_conf(device, &cam_conf);
  if (camera_open(device))
    return -1;

  return 0;
}

static int open_camera_mt9v034(struct pi_device *device)
{
  struct mt9v034_conf cam_conf;

  mt9v034_conf_init(&cam_conf);
  cam_conf.format = CAMERA_QVGA;

  pi_open_from_conf(device, &cam_conf);
  if (camera_open(device))
    return -1;

  return 0;
}

static int open_camera(struct pi_device *device)
{
#ifdef USE_CAMERA
#ifdef HIMAX
  return open_camera_himax(device);
#else
  return open_camera_mt9v034(device);
#endif
  return -1;
#else
  return 0;
#endif
}

//int main()
int application_start(int argc, char *argv[])
{
  DEBUG_PRINTF("Entering main controller...\n");

  unsigned int W = CAM_WIDTH, H = CAM_HEIGHT;
  unsigned int Wout = 64, Hout = 48;
  unsigned int ImgSize = W*H;

  imgBuff0 = (unsigned char *)pmsis_l2_malloc((CAM_WIDTH*CAM_HEIGHT)*sizeof(unsigned char));
  if (imgBuff0 == NULL) {
      DEBUG_PRINTF("Failed to allocate Memory for Image \n");
      return 1;
  }

  //This can be moved in init
  ImageOut             = (unsigned char *) pmsis_l2_malloc((Wout*Hout)*sizeof(unsigned char));
  ImageIntegral        = (unsigned int *)  pmsis_l2_malloc((Wout*Hout)*sizeof(unsigned int));
  SquaredImageIntegral = (unsigned int *)  pmsis_l2_malloc((Wout*Hout)*sizeof(unsigned int));

  if (ImageOut==0) {
    DEBUG_PRINTF("Failed to allocate Memory for Image (%d bytes)\n", ImgSize*sizeof(unsigned char));
    return 1;
  }
  if (ImageIntegral==0 || SquaredImageIntegral==0) {
    DEBUG_PRINTF("Failed to allocate Memory for one or both Integral Images (%d bytes)\n", ImgSize*sizeof(unsigned int));
    return 1;
  }
  DEBUG_PRINTF("malloc done\n");

  board_init();

  if (open_display(&ili))
  {
    DEBUG_PRINTF("Failed to open display\n");
    return -1;
  }
  DEBUG_PRINTF("display done\n");

  if (open_camera(&cam))
  {
    DEBUG_PRINTF("Failed to open camera\n");
    return -1;
  }
  DEBUG_PRINTF("Camera open success\n");

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

  ClusterCall.ImageIn              = imgBuff0;
  ClusterCall.Win                  = W;
  ClusterCall.Hin                  = H;
  ClusterCall.Wout                 = Wout;
  ClusterCall.Hout                 = Hout;
  ClusterCall.ImageOut             = ImageOut;
  ClusterCall.ImageIntegral        = ImageIntegral;
  ClusterCall.SquaredImageIntegral = SquaredImageIntegral;

  //Why il faut faire le deux
  pi_cluster_conf_init(&conf);
  pi_open_from_conf(&cluster_dev, (void*)&conf);
  pi_cluster_open(&cluster_dev);

  task = pmsis_l2_malloc(sizeof(struct pi_cluster_task));
  memset(task, 0, sizeof(struct pi_cluster_task));
  task->entry = faceDet_cluster_init;
  task->arg = &ClusterCall;

  pi_cluster_send_task_to_cl(&cluster_dev, task);

  task->entry = faceDet_cluster_main;
  task->arg = &ClusterCall;


#ifdef USE_DISPLAY
  //Setting Screen background to white
  writeFillRect(&ili, 0,0,320,240,0xFFFF);
  setCursor(0,0);
  writeText(&ili,"        Greenwaves \n       Technologies",2);
#endif
  DEBUG_PRINTF("main loop start\n");

  while(1)
  {
#ifdef USE_CAMERA
    camera_control(&cam, CAMERA_CMD_START, 0);
    camera_capture(&cam, imgBuff0, CAM_WIDTH*CAM_HEIGHT);
    camera_control(&cam, CAMERA_CMD_STOP, 0);
#endif

    pi_cluster_send_task_to_cl(&cluster_dev, task);
    DEBUG_PRINTF("end of face detection\n");

#ifdef USE_DISPLAY
  sprintf(str_to_lcd,"1 Image/Sec: \n%d uWatt @ 1.2V   \n%d uWatt @ 1.0V   %c", (int)((float)(1/(50000000.f/ClusterCall.cycles)) * 28000.f),(int)((float)(1/(50000000.f/ClusterCall.cycles)) * 16800.f),'\0');
  //sprintf(out_perf_string,"%d  \n%d  %c", (int)((float)(1/(50000000.f/cycles)) * 28000.f),(int)((float)(1/(50000000.f/cycles)) * 16800.f),'\0');

  setCursor(0,190);
  writeText(&ili,str_to_lcd,2);
  buffer.data = ImageOut;
  display_write(&ili, &buffer, 80, 40, 160, 120);
#endif
  }

  return 0;
}
