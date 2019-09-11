#include <stdio.h>
#include <aos/kernel.h>

/* PMSIS includes */
#include "pmsis.h"
#include "pmsis/rtos/os_frontend_api/pmsis_time.h"

#include "Gap.h"

/* PMSIS BSP includes */
#include "bsp/bsp.h"
#include "bsp/buffer.h"
#include "bsp/display/ili9341.h"
#include "bsp/ram/hyperram.h"
#if defined(GAPOC)
#include "bsp/gapoc_a.h"
#include "bsp/camera/mt9v034.h"
#else
#include "bsp/gapuino.h"
#include "bsp/camera/himax.h"
#endif  /* GAPOC */

/* App includes */
//#include "stdio.h"
#include "setup.h"
#include "cascade.h"
#include "ImgIO.h"

#include "network_process_manual.h"
#include "dnn_utils.h"
#include "face_db.h"

#include "CnnKernels.h"
#include "ExtraKernels.h"
#include "reid_pipeline.h"
#include "facedet_pipeline.h"

#define MT9V034_BLACK_LEVEL_CTRL  0x47
#define MT9V034_BLACK_LEVEL_AUTO  (0 << 0)
#define MT9V034_AEC_AGC_ENABLE    0xaf
#define MT9V034_AEC_ENABLE_A      (1 << 0)
#define MT9V034_AGC_ENABLE_A      (1 << 1)

#if defined(HAVE_DISPLAY)
void setCursor(struct pi_device *device,signed short x, signed short y);
void writeFillRect(struct pi_device *device, unsigned short x, unsigned short y, unsigned short w, unsigned short h, unsigned short color);
void writeText(struct pi_device *device,char* str,int fontsize);
#endif  /* HAVE_DISPLAY */

cascade_reponse_t responses[MAX_NUM_OUT_WINS];

struct pi_device HyperRam;

static void my_copy(short* in, unsigned char* out, int Wout, int Hout)
{
    for(int i = 0; i < Hout; i++)
    {
        for(int j = 0; j < Hout; j++)
        {
            out[i*Wout + j] = (unsigned char)in[i*Wout + j];
        }
    }
}

static int open_display(struct pi_device *device)
{
    #if defined(HAVE_DISPLAY)
    struct ili9341_conf ili_conf;

    ili9341_conf_init(&ili_conf);

    pi_open_from_conf(device, &ili_conf);

    if (display_open(device))
        return -1;
    #endif  /* HAVE_DISPLAY */
    return 0;
}

#if defined(HAVE_CAMERA)
#if defined(GAPOC)
static int open_camera_mt9v034(struct pi_device *device)
{
    struct mt9v034_conf cam_conf;

    mt9v034_conf_init(&cam_conf);
    cam_conf.format = CAMERA_QVGA;

    pi_open_from_conf(device, &cam_conf);
    if (camera_open(device))
        return -1;

    uint8_t val = MT9V034_BLACK_LEVEL_AUTO;
    camera_reg_set(device, MT9V034_BLACK_LEVEL_CTRL, &val);
    val = MT9V034_AEC_ENABLE_A|MT9V034_AGC_ENABLE_A;
    camera_reg_set(device, MT9V034_AEC_AGC_ENABLE, &val);

    return 0;
}
#else
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
#endif  /* GAPOC */
#endif  /* HAVE_CAMERA */

static int open_camera(struct pi_device *device)
{
#if defined(HAVE_CAMERA)
#if defined(GAPOC)
    return open_camera_mt9v034(device);
#else
    return open_camera_himax(device);
#endif  /* GAPOC */
#else
    return 0;
#endif  /* HAVE_CAMERA */
}

#define IMAGE_SIZE CAMERA_WIDTH*CAMERA_HEIGHT

void body(void *parameters)
{
    PRINTF("Start ReID Demo Application\n");

    board_init();
    pi_pad_set_function(25, 1);    // Need to take a look why

    static pi_buffer_t ImageInBuffer;
    static pi_buffer_t RenderBuffer;
    char* person_name;
    struct pi_device cluster_dev;
    struct pi_device camera;
    struct pi_device display;
    struct cluster_driver_conf cluster_conf;
    struct pi_cluster_task cluster_task;
    cluster_task.stack_size = (1024*2);

    unsigned char* ImageRender;
    unsigned char* ImageIn;
    unsigned char* ImageOut;
    unsigned int* ImageIntegral;
    unsigned int* SquaredImageIntegral;
    int* output_map;

    char string_buffer[120];
    ArgCluster_T ClusterDetectionCall;
    ArgClusterDnn_T ClusterDnnCall;

    int width = CAMERA_WIDTH;
    int height = CAMERA_HEIGHT;

    PRINTF("Camera resolution: %dx%d\n", width, height);

    PRINTF("Configuring Hyperram..\n");
    struct hyperram_conf hyper_conf;

    hyperram_conf_init(&hyper_conf);
    pi_open_from_conf(&HyperRam, &hyper_conf);

    int32_t error = ram_open(&HyperRam);
    if (error)
    {
        PRINTF("Error %d: cannot open Hyperram!\n", error);
        pmsis_exit(-2);
    }
    // The hyper chip need to wait a bit.
    // TODO: find out need to wait how many times.
    pi_time_wait_us(1*1000*1000);

    PRINTF("HyperRAM config done\n");

    uint32_t preview_hyper, descriptor_hyper;

    ram_alloc(&HyperRam, &preview_hyper, 128*128);
    ram_alloc(&HyperRam, &descriptor_hyper, 512*sizeof(short));
    PRINTF("HyperRam alloc done\n");

    PRINTF("Configuring Hyperflash and FS..\n");
    struct pi_device fs;
    struct pi_device flash;
    struct fs_conf conf;
    struct hyperflash_conf flash_conf;

    fs_conf_init(&conf);
    hyperflash_conf_init(&flash_conf);
    pi_open_from_conf(&flash, &flash_conf);
    if (flash_open(&flash))
    {
        PRINTF("Error: Flash open failed\n");
        pmsis_exit(-3);
    }

    conf.flash = &flash;
    pi_open_from_conf(&fs, &conf);
    error = fs_mount(&fs);
    if (error)
    {
        PRINTF("Error: FS mount failed with error %d\n", error);
        pmsis_exit(-4);
    }
    PRINTF("FS mounted.\n");

    PRINTF("Loading layers to HyperRAM\n");
    network_load(&fs);

    PRINTF("Unmount FS as it's not needed any more\n");
    fs_unmount(&fs);
    PRINTF("Network load done, unmount FS\n");

    PRINTF("Initializing display\n");
    if (open_display(&display))
    {
        PRINTF("Failed to open display\n");
        pmsis_exit(-5);
    }
    PRINTF("Display done\n");

    #if defined(HAVE_DISPLAY)
    //Setting Screen background to white
    writeFillRect(&display, 0, 0, 240, 320, 0xFFFF);
    setCursor(&display, 0, 250);
    writeText(&display,"        Greenwaves \n       Technologies", 2);
    #endif  /* HAVE_DISPLAY */

    #if defined(HAVE_CAMERA)
    PRINTF("Initializing camera\n");
    if (open_camera(&camera))
    {
        PRINTF("Failed to open camera\n");
        pmsis_exit(-6);
    }
    PRINTF("Camera done\n");
    #endif  /* HAVE_CAMERA */

    // put camera frame to memory pool tail, it does not intersect with the first DNN layer data
    ImageIn = ((unsigned char*)(memory_pool + MEMORY_POOL_SIZE)) - IMAGE_SIZE;
    ImageOut = ImageIn - WOUT_INIT*HOUT_INIT;
    ImageIntegral = ((unsigned int*)ImageOut) - WOUT_INIT*HOUT_INIT;
    SquaredImageIntegral = ImageIntegral - WOUT_INIT*HOUT_INIT;
    output_map = (int *)SquaredImageIntegral - (HOUT_INIT-24+1)*(WOUT_INIT-24+1);
    ImageRender = (unsigned char *) memory_pool;

    ImageInBuffer.data = ImageIn;
    pi_buffer_init(&ImageInBuffer, PI_BUFFER_TYPE_L2, ImageIn);
    pi_buffer_set_format(&ImageInBuffer, CAMERA_WIDTH, CAMERA_HEIGHT, 1, PI_BUFFER_FORMAT_GRAY);

    pi_buffer_init(&RenderBuffer, PI_BUFFER_TYPE_L2, ImageOut);
    pi_buffer_set_format(&ImageInBuffer, CAMERA_WIDTH/2, CAMERA_HEIGHT/2, 1, PI_BUFFER_FORMAT_GRAY);

    //Why il faut faire le deux
    PRINTF("Init cluster...\n");
    pi_cluster_conf_init(&cluster_conf);
    cluster_conf.id = 0;
    pi_open_from_conf(&cluster_dev, &cluster_conf);
    pi_cluster_open(&cluster_dev);
    PRINTF("Init cluster...done\n");

    ClusterDetectionCall.ImageIn              = ImageIn;
    ClusterDetectionCall.Win                  = CAMERA_WIDTH;
    ClusterDetectionCall.Hin                  = CAMERA_HEIGHT;
    ClusterDetectionCall.Wout                 = WOUT_INIT;
    ClusterDetectionCall.Hout                 = HOUT_INIT;
    ClusterDetectionCall.ImageOut             = ImageOut;
    ClusterDetectionCall.ImageIntegral        = ImageIntegral;
    ClusterDetectionCall.SquaredImageIntegral = SquaredImageIntegral;
    ClusterDetectionCall.ImageRender          = ImageRender;
    ClusterDetectionCall.output_map           = output_map;
    ClusterDetectionCall.reponses             = responses;

    //Cluster Init
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cluster_task, (void (*)(void *))detection_cluster_init, &ClusterDetectionCall));

    PRINTF("main loop start\n");

    int saved_index = 0;

    while (1)
    {
        #if defined(HAVE_CAMERA)
        camera_control(&camera, CAMERA_CMD_START, 0);
        camera_capture(&camera, ImageIn, IMAGE_SIZE);
        camera_control(&camera, CAMERA_CMD_STOP, 0);
        #endif  /* HAVE_CAMERA */

        pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cluster_task, (void (*)(void *))detection_cluster_main, &ClusterDetectionCall));

        //Printing cycles to screen
        #if defined(HAVE_DISPLAY)
        sprintf(string_buffer, "%d  \n%d  ", (int)((float)(1/(50000000.f/ClusterDetectionCall.cycles)) * 28000.f),(int)((float)(1/(50000000.f/ClusterDetectionCall.cycles)) * 16800.f));
        setCursor(&display, 0, 250+2*8);
        writeText(&display, string_buffer, 2);

        RenderBuffer.data = ImageRender;
        display_write(&display, &RenderBuffer, LCD_OFF_X, LCD_OFF_Y, CAMERA_WIDTH/2,CAMERA_HEIGHT/2);
        #endif  /* HAVE_DISPLAY */

        printf("num_reponse: %d\n", ClusterDetectionCall.num_reponse);
        if (ClusterDetectionCall.num_reponse)
        {
            PRINTF("Faces detected!\n");
            //pi_cluster_close(&cluster_dev);
            int optimal_detection_id = -1;
            int optimal_score = -1;
            for (int i=0; i<ClusterDetectionCall.num_reponse; i++)
            {
                if (responses[i].score > optimal_score)
                {
                    optimal_detection_id = i;
                    optimal_score = responses[i].score;
                }
            }
            //pi_cluster_open(&cluster_dev);
            PRINTF("Score: %X, optimal_detection_id: %d\n", optimal_score, optimal_detection_id);

            ClusterDnnCall.roi         = &responses[optimal_detection_id];
            ClusterDnnCall.frame       = ImageIn;
            ClusterDnnCall.face        = ((unsigned char*)output_map) - (194*194); // Largest possible face after Cascade
            ClusterDnnCall.scaled_face = network_init();
            if (!ClusterDnnCall.scaled_face)
            {
                PRINTF("Failed to initialize ReID network!\n");
                pmsis_exit(-7);
            }

            ExtraKernels_L1_Memory = L1_Memory;

            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cluster_task, (void (*)(void *))reid_prepare_cluster, &ClusterDnnCall));
            PRINTF("prepare done\n");

            my_copy(ClusterDnnCall.scaled_face, ClusterDnnCall.face, 128, 128);
            int iterations = 128*128 / 1024;
            for (int i=0; i<iterations; i++)
            {
                ram_write(&HyperRam, preview_hyper+i*1024, ((char*)ClusterDnnCall.face) + i*1024, 1024);
            }

            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cluster_task, (void (*)(void *))reid_inference_cluster, &ClusterDnnCall));
            PRINTF("inference done\n");

            int id_l2 = identify_by_db(ClusterDnnCall.output, &person_name);
            sprintf(string_buffer, "ReID L2: %d\n", id_l2);
            PRINTF(string_buffer);

            /* Release all memory for reid network */
            network_deinit();
        }
    }
    PRINTF("Face ReID Demo done!\n");
    pmsis_exit(0);
}


int application_start(int argc, char *argv[])
{
    PRINTF("Entering main controller...\n");
    body(NULL);
    return 0;
}
