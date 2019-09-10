#include "pmsis.h"
#include "Gap.h"

#include "facedet_pipeline.h"
#include "FaceDetKernels.h"
#include "setup.h"

#include "ImageDraw.h"

static inline unsigned int __attribute__((always_inline)) ChunkSize(unsigned int X)
{
    unsigned int NCore;
    unsigned int Log2Core;
    unsigned int Chunk;

    NCore = pi_nb_cluster_cores();
    Log2Core = gap_fl1(NCore);
    Chunk = (X>>Log2Core) + ((X&(NCore-1))!=0);
    return Chunk;
}

void detection_cluster_init(ArgCluster_T *ArgC)
{
    // printf ("Cluster Init start\n");
    FaceDet_L1_Memory = (char *) pmsis_l1_malloc(_FaceDet_L1_Memory_SIZE);
    if (FaceDet_L1_Memory == 0)
    {
        printf("Failed to allocate %d bytes for L1_memory\n", _FaceDet_L1_Memory_SIZE);
        return ;
    }

    //Get Cascade Model
    ArgC->model = getFaceCascade();

    #ifdef PERF_COUNT
    // initialize the performance clock
    rt_perf_init(ArgC->perf);
    // Configure performance counters for counting the cycles
    rt_perf_conf(ArgC->perf, (1<<RT_PERF_CYCLES));
    //printf("Cluster core %d Launched, %d cores configuration\n", 1, pi_nb_cluster_cores());
    #endif
}

static void prepare_to_render(ArgCluster_T *ArgC)
{
    unsigned int width = ArgC->Win;
    unsigned int height = ArgC->Hin;

    unsigned int CoreId = pi_core_id();
    unsigned int ChunkCell = ChunkSize(height);
    unsigned int First = CoreId*ChunkCell, Last = Min(height, First+ChunkCell);

    for(unsigned int i = First/2; i < Last; i++)
    {
        for(unsigned int j = 0; j < width/2; j++)
        {
            ArgC->ImageRender[((i/2)*(width/2))+j] = ArgC->ImageIn[((i)*width)+2*j];
        }
    }
}

static void draw_responses(unsigned char* ImageIn, int Win, int Hin, cascade_reponse_t* reponses, int num_reponse)
{
    for(int i = 0; i < num_reponse; i++)
    {
        if(reponses[i].x!=-1)
        {
            #ifndef DUMP_SUCCESSFUL_FRAME
            DrawRectangle(ImageIn, Hin, Win, reponses[i].x, reponses[i].y, reponses[i].w, reponses[i].h, 0);
            DrawRectangle(ImageIn, Hin, Win, reponses[i].x-1, reponses[i].y-1, reponses[i].w+2, reponses[i].h+2, 255);
            DrawRectangle(ImageIn, Hin, Win, reponses[i].x-2, reponses[i].y-2, reponses[i].w+4, reponses[i].h+4, 255);
            DrawRectangle(ImageIn, Hin, Win, reponses[i].x-3, reponses[i].y-3, reponses[i].w+6, reponses[i].h+6, 0);
            #endif

            printf("Found face at (%d,%d) with size (%d,%d) at scale %d\n", reponses[i].x, reponses[i].y, reponses[i].w, reponses[i].h, reponses[i].layer_idx);
        }
    }
}

void detection_cluster_main(ArgCluster_T *ArgC)
{
    unsigned int i, MaxCore = pi_nb_cluster_cores();

    #ifdef PERF_COUNT
    //gap8_resethwtimer();
    unsigned int Ta = 0;//gap8_readhwtimer();
    #endif

    cascade_detect(ArgC);

    #ifdef PERF_COUNT
    unsigned int Ti = 0;//gap8_readhwtimer();
    ArgC->cycles = Ti-Ta;
    #endif

    draw_responses(ArgC->ImageIn, ArgC->Win, ArgC->Hin, ArgC->reponses, ArgC->num_reponse);

    //Converting image to RGB 565 for LCD screen and binning image to half the size
    cl_team_fork(__builtin_pulp_CoreCount(), (void *)prepare_to_render, (void *) ArgC);
}
