#include "FaceDetKernelsInit.h"
#include "FaceDetKernels.h"
void ResizeImage_1(
		unsigned char * In,
		unsigned char * Out,
		Kernel_Exec_T *Ker)

{
	/* Shared L1: 23936 bytes, L2 buffer: 0 bytes */
	/* Local variables used by this kernel */
	cl_dma_copy_t DmaR_Evt1;
	cl_dma_copy_t DmaW_Evt1;
	int Iter;
	int Last, NextLast, NextNextLast;
	int N_Ti = 0;
	int N_TiIp = 0, InPlane, OutPlane=0;
	KerResizeBilinear_ArgT S_KerArg0, *KerArg0 = &S_KerArg0;

	/* Initialize KerArg, Kernel invariant arguments */
	KerArg0->Win = (unsigned int) (320);
	KerArg0->Hin = (unsigned int) (240);
	KerArg0->Wout = (unsigned int) (64);
	KerArg0->Hout = (unsigned int) (48);
	/* =======================Read First Tile=========================================== */
	/* Initial reads in L2, O_DB or O_BUFF */
	__cl_dma_memcpy((rt_pointerT) In+(0), (rt_pointerT) (FaceDet_L1_Memory + 0)+0, 11520, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	/* ===================End Read First Tile=========================================== */
	/* Kernel Iteration Loop on Iter space */
	for (Iter=0; Iter<7; Iter++) {
		/* Loop Iteration Body on Iter space */
		/* Elaborate Last, Next_Last, Next_Next_Last */
		Last = ((Iter+1) == 7); NextLast = ((Iter+2) == 7); NextNextLast = ((Iter+3) == 7);
		/* =======================Read Next Tile=========================================== */
		cl_dma_wait(&DmaR_Evt1);
		if (!Last) {
			__cl_dma_memcpy((rt_pointerT) In + (((326314*(Iter+1)*7)>>16)*320),
					(rt_pointerT) (FaceDet_L1_Memory + 0) + (11520*((N_Ti+1) % 2)), NextLast?9920:11520, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
		}
		/* ===================End Read Next Tile=========================================== */
		/* Call Kernel LOC_INNER_LOOP */
		KerArg0->In = (unsigned char * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 0) + 11520*((N_Ti) % 2));
		KerArg0->Out = (unsigned char * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 23040) + 448*((N_Ti) % 2));
		KerArg0->HTileOut = (unsigned int) (Last?6:7);
		KerArg0->FirstLineIndex = (unsigned int) ((326314*Iter*7)>>16);
		cl_team_fork(__builtin_pulp_CoreCount(), (void *) KerResizeBilinear, (void *) KerArg0);
		/* =======================Write Tile=========================================== */
		if (Iter) {
			cl_dma_wait(&DmaW_Evt1);
		}
		__cl_dma_memcpy((rt_pointerT) Out + ((Iter)*448),
			(rt_pointerT) (FaceDet_L1_Memory + 23040) + (448*(N_Ti % 2)), Last?384:448, CL_DMA_DIR_LOC2EXT, 0, &DmaW_Evt1);
		/* ===================End Write Tile=========================================== */
		N_Ti++;
		/* End Kernel Iteration Loop on Iter space */
	}
	Iter=7;
	/* =======================Write Last Tile=========================================== */
	cl_dma_wait(&DmaW_Evt1);
	/* ===================End Write Last Tile=========================================== */
}

void ProcessIntegralImage_1(
		Wordu8 *  __restrict__ ImageIn,
		Wordu32 *  __restrict__ IntegralImage,
		Kernel_Exec_T *Ker)

{
	/* Shared L1: 24576 bytes, L2 buffer: 0 bytes */
	/* Local variables used by this kernel */
	cl_dma_copy_t DmaR_Evt1;
	cl_dma_copy_t DmaW_Evt1;
	int Iter;
	int Last, NextLast, NextNextLast;
	int N_Ti = 0;
	int N_TiIp = 0, InPlane, OutPlane=0;
	KerPrimeImage_ArgT S_KerArg0, *KerArg0 = &S_KerArg0;
	KerProcessImage_ArgT S_KerArg1, *KerArg1 = &S_KerArg1;

	/* Initialize KerArg, Kernel invariant arguments */
	KerArg0->KerBuffer = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 4864) + 0);
	KerArg0->W = (Wordu32) (64);
	KerArg1->W = (Wordu32) (64);
	KerArg1->KerBuffer = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 4864) + 0);
	/* =======================Read First Tile=========================================== */
	/* Initial reads in L2, O_DB or O_BUFF */
	__cl_dma_memcpy((rt_pointerT) ImageIn+(0), (rt_pointerT) (FaceDet_L1_Memory + 0)+0, 2432, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	/* ===================End Read First Tile=========================================== */
	/* Call Kernel LOC_INNER_LOOP_PROLOG */
	cl_team_fork(__builtin_pulp_CoreCount(), (void *) KerIntegralImagePrime, (void *) KerArg0);
	/* Kernel Iteration Loop on Iter space */
	for (Iter=0; Iter<2; Iter++) {
		/* Loop Iteration Body on Iter space */
		/* Elaborate Last, Next_Last, Next_Next_Last */
		Last = ((Iter+1) == 2); NextLast = ((Iter+2) == 2); NextNextLast = ((Iter+3) == 2);
		/* =======================Read Next Tile=========================================== */
		cl_dma_wait(&DmaR_Evt1);
		if (!Last) {
			__cl_dma_memcpy((rt_pointerT) ImageIn + ((Iter+1)*2432),
					(rt_pointerT) (FaceDet_L1_Memory + 0) + (2432*((N_Ti+1) % 2)), NextLast?640:2432, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
		}
		/* ===================End Read Next Tile=========================================== */
		/* Call Kernel LOC_INNER_LOOP */
		KerArg1->In = (Wordu8 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 0) + 2432*((N_Ti) % 2));
		KerArg1->H = (Wordu32) (Last?10:38);
		KerArg1->IntegralImage = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 5120) + 9728*((N_Ti) % 2));
		cl_team_fork(__builtin_pulp_CoreCount(), (void *) KerIntegralImageProcess, (void *) KerArg1);
		/* =======================Write Tile=========================================== */
		if (Iter) {
			cl_dma_wait(&DmaW_Evt1);
		}
		__cl_dma_memcpy((rt_pointerT) IntegralImage + ((Iter)*9728),
			(rt_pointerT) (FaceDet_L1_Memory + 5120) + (9728*(N_Ti % 2)), Last?2560:9728, CL_DMA_DIR_LOC2EXT, 0, &DmaW_Evt1);
		/* ===================End Write Tile=========================================== */
		N_Ti++;
		/* End Kernel Iteration Loop on Iter space */
	}
	Iter=2;
	/* =======================Write Last Tile=========================================== */
	cl_dma_wait(&DmaW_Evt1);
	/* ===================End Write Last Tile=========================================== */
}

void ProcessSquaredIntegralImage_1(
		Wordu8 *  __restrict__ ImageIn,
		Wordu32 *  __restrict__ IntegralImage,
		Kernel_Exec_T *Ker)

{
	/* Shared L1: 24576 bytes, L2 buffer: 0 bytes */
	/* Local variables used by this kernel */
	cl_dma_copy_t DmaR_Evt1;
	cl_dma_copy_t DmaW_Evt1;
	int Iter;
	int Last, NextLast, NextNextLast;
	int N_Ti = 0;
	int N_TiIp = 0, InPlane, OutPlane=0;
	KerPrimeImage_ArgT S_KerArg0, *KerArg0 = &S_KerArg0;
	KerProcessImage_ArgT S_KerArg1, *KerArg1 = &S_KerArg1;

	/* Initialize KerArg, Kernel invariant arguments */
	KerArg0->KerBuffer = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 4864) + 0);
	KerArg0->W = (Wordu32) (64);
	KerArg1->W = (Wordu32) (64);
	KerArg1->KerBuffer = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 4864) + 0);
	/* =======================Read First Tile=========================================== */
	/* Initial reads in L2, O_DB or O_BUFF */
	__cl_dma_memcpy((rt_pointerT) ImageIn+(0), (rt_pointerT) (FaceDet_L1_Memory + 0)+0, 2432, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	/* ===================End Read First Tile=========================================== */
	/* Call Kernel LOC_INNER_LOOP_PROLOG */
	cl_team_fork(__builtin_pulp_CoreCount(), (void *) KerIntegralImagePrime, (void *) KerArg0);
	/* Kernel Iteration Loop on Iter space */
	for (Iter=0; Iter<2; Iter++) {
		/* Loop Iteration Body on Iter space */
		/* Elaborate Last, Next_Last, Next_Next_Last */
		Last = ((Iter+1) == 2); NextLast = ((Iter+2) == 2); NextNextLast = ((Iter+3) == 2);
		/* =======================Read Next Tile=========================================== */
		cl_dma_wait(&DmaR_Evt1);
		if (!Last) {
			__cl_dma_memcpy((rt_pointerT) ImageIn + ((Iter+1)*2432),
					(rt_pointerT) (FaceDet_L1_Memory + 0) + (2432*((N_Ti+1) % 2)), NextLast?640:2432, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
		}
		/* ===================End Read Next Tile=========================================== */
		/* Call Kernel LOC_INNER_LOOP */
		KerArg1->In = (Wordu8 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 0) + 2432*((N_Ti) % 2));
		KerArg1->H = (Wordu32) (Last?10:38);
		KerArg1->IntegralImage = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 5120) + 9728*((N_Ti) % 2));
		cl_team_fork(__builtin_pulp_CoreCount(), (void *) KerSquaredIntegralImageProcess, (void *) KerArg1);
		/* =======================Write Tile=========================================== */
		if (Iter) {
			cl_dma_wait(&DmaW_Evt1);
		}
		__cl_dma_memcpy((rt_pointerT) IntegralImage + ((Iter)*9728),
			(rt_pointerT) (FaceDet_L1_Memory + 5120) + (9728*(N_Ti % 2)), Last?2560:9728, CL_DMA_DIR_LOC2EXT, 0, &DmaW_Evt1);
		/* ===================End Write Tile=========================================== */
		N_Ti++;
		/* End Kernel Iteration Loop on Iter space */
	}
	Iter=2;
	/* =======================Write Last Tile=========================================== */
	cl_dma_wait(&DmaW_Evt1);
	/* ===================End Write Last Tile=========================================== */
}

void ProcessCascade_1(
		Wordu32 *  __restrict__ IntegralImage,
		Wordu32 *  __restrict__ SquaredIntegralImage,
		void *  cascade_model,
		Word32  *  __restrict__ CascadeReponse,
		Kernel_Exec_T *Ker)

{
	/* Shared L1: 24904 bytes, L2 buffer: 0 bytes */
	/* Local variables used by this kernel */
	cl_dma_copy_t DmaR_Evt1;
	cl_dma_copy_t DmaR_Evt2;
	cl_dma_copy_t DmaW_Evt1;
	int Iter;
	int Last, NextLast, NextNextLast;
	int N_Ti = 0;
	int N_TiIp = 0, InPlane, OutPlane=0;

	/* Initialize KerArg, Kernel invariant arguments */
	/* =======================Read First Tile=========================================== */
	/* Initial reads in L2, O_DB or O_BUFF */
	__cl_dma_memcpy((rt_pointerT) IntegralImage+(0), (rt_pointerT) (FaceDet_L1_Memory + 0)+0, 6144, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	__cl_dma_memcpy((rt_pointerT) SquaredIntegralImage+(0), (rt_pointerT) (FaceDet_L1_Memory + 12288)+0, 6144, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt2);
	/* ===================End Read First Tile=========================================== */
	/* Kernel Iteration Loop on Iter space */
	for (Iter=0; Iter<25; Iter++) {
		/* Loop Iteration Body on Iter space */
		/* Elaborate Last, Next_Last, Next_Next_Last */
		Last = ((Iter+1) == 25); NextLast = ((Iter+2) == 25); NextNextLast = ((Iter+3) == 25);
		/* =======================Read Next Tile=========================================== */
		cl_dma_wait(&DmaR_Evt1);
		cl_dma_wait(&DmaR_Evt2);
		if (!Last) {
			__cl_dma_memcpy((rt_pointerT) IntegralImage + ((Iter+1)*256),
					(rt_pointerT) (FaceDet_L1_Memory + 0) + (6144*((N_Ti+1) % 2)), 6144, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
			__cl_dma_memcpy((rt_pointerT) SquaredIntegralImage + ((Iter+1)*256),
					(rt_pointerT) (FaceDet_L1_Memory + 12288) + (6144*((N_Ti+1) % 2)), 6144, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt2);
		}
		/* ===================End Read Next Tile=========================================== */
		/* Call Kernel LOC_INNER_LOOP */
		KerEvaluateCascade(
			(Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 0) + 6144*((N_Ti) % 2)),
			(Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 12288) + 6144*((N_Ti) % 2)),
			(Wordu32) (64),
			(Wordu32) (24),
			(cascade_model),
			(Wordu8) (24),
			(Wordu8) (24),
			(Word32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 24576) + 164*((N_Ti) % 2))
		);
		/* =======================Write Tile=========================================== */
		if (Iter) {
			cl_dma_wait(&DmaW_Evt1);
		}
		__cl_dma_memcpy((rt_pointerT) CascadeReponse + ((Iter)*164),
			(rt_pointerT) (FaceDet_L1_Memory + 24576) + (164*(N_Ti % 2)), 164, CL_DMA_DIR_LOC2EXT, 0, &DmaW_Evt1);
		/* ===================End Write Tile=========================================== */
		N_Ti++;
		/* End Kernel Iteration Loop on Iter space */
	}
	Iter=25;
	/* =======================Write Last Tile=========================================== */
	cl_dma_wait(&DmaW_Evt1);
	/* ===================End Write Last Tile=========================================== */
}

void ResizeImage_2(
		unsigned char * In,
		unsigned char * Out,
		Kernel_Exec_T *Ker)

{
	/* Shared L1: 21632 bytes, L2 buffer: 0 bytes */
	/* Local variables used by this kernel */
	cl_dma_copy_t DmaR_Evt1;
	cl_dma_copy_t DmaW_Evt1;
	int Iter;
	int Last, NextLast, NextNextLast;
	int N_Ti = 0;
	int N_TiIp = 0, InPlane, OutPlane=0;
	KerResizeBilinear_ArgT S_KerArg0, *KerArg0 = &S_KerArg0;

	/* Initialize KerArg, Kernel invariant arguments */
	KerArg0->Win = (unsigned int) (320);
	KerArg0->Hin = (unsigned int) (240);
	KerArg0->Wout = (unsigned int) (51);
	KerArg0->Hout = (unsigned int) (38);
	/* =======================Read First Tile=========================================== */
	/* Initial reads in L2, O_DB or O_BUFF */
	__cl_dma_memcpy((rt_pointerT) In+(0), (rt_pointerT) (FaceDet_L1_Memory + 0)+0, 10560, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	/* ===================End Read First Tile=========================================== */
	/* Kernel Iteration Loop on Iter space */
	for (Iter=0; Iter<8; Iter++) {
		/* Loop Iteration Body on Iter space */
		/* Elaborate Last, Next_Last, Next_Next_Last */
		Last = ((Iter+1) == 8); NextLast = ((Iter+2) == 8); NextNextLast = ((Iter+3) == 8);
		/* =======================Read Next Tile=========================================== */
		cl_dma_wait(&DmaR_Evt1);
		if (!Last) {
			__cl_dma_memcpy((rt_pointerT) In + (((412186*(Iter+1)*5)>>16)*320),
					(rt_pointerT) (FaceDet_L1_Memory + 0) + (10560*((N_Ti+1) % 2)), NextLast?6400:10560, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
		}
		/* ===================End Read Next Tile=========================================== */
		/* Call Kernel LOC_INNER_LOOP */
		KerArg0->In = (unsigned char * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 0) + 10560*((N_Ti) % 2));
		KerArg0->Out = (unsigned char * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 21120) + 256*((N_Ti) % 2));
		KerArg0->HTileOut = (unsigned int) (Last?3:5);
		KerArg0->FirstLineIndex = (unsigned int) ((412186*Iter*5)>>16);
		cl_team_fork(__builtin_pulp_CoreCount(), (void *) KerResizeBilinear, (void *) KerArg0);
		/* =======================Write Tile=========================================== */
		if (Iter) {
			cl_dma_wait(&DmaW_Evt1);
		}
		__cl_dma_memcpy((rt_pointerT) Out + ((Iter)*255),
			(rt_pointerT) (FaceDet_L1_Memory + 21120) + (256*(N_Ti % 2)), Last?153:255, CL_DMA_DIR_LOC2EXT, 0, &DmaW_Evt1);
		/* ===================End Write Tile=========================================== */
		N_Ti++;
		/* End Kernel Iteration Loop on Iter space */
	}
	Iter=8;
	/* =======================Write Last Tile=========================================== */
	cl_dma_wait(&DmaW_Evt1);
	/* ===================End Write Last Tile=========================================== */
}

void ProcessIntegralImage_2(
		Wordu8 *  __restrict__ ImageIn,
		Wordu32 *  __restrict__ IntegralImage,
		Kernel_Exec_T *Ker)

{
	/* Shared L1: 19588 bytes, L2 buffer: 0 bytes */
	/* Local variables used by this kernel */
	cl_dma_copy_t DmaR_Evt1;
	cl_dma_copy_t DmaW_Evt1;
	int Iter;
	int Last, NextLast, NextNextLast;
	int N_Ti = 0;
	int N_TiIp = 0, InPlane, OutPlane=0;
	KerPrimeImage_ArgT S_KerArg0, *KerArg0 = &S_KerArg0;
	KerProcessImage_ArgT S_KerArg1, *KerArg1 = &S_KerArg1;

	/* Initialize KerArg, Kernel invariant arguments */
	KerArg0->KerBuffer = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 3880) + 0);
	KerArg0->W = (Wordu32) (51);
	KerArg1->W = (Wordu32) (51);
	KerArg1->H = (Wordu32) (38);
	KerArg1->KerBuffer = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 3880) + 0);
	/* =======================Read First Tile=========================================== */
	/* Initial reads in L2, O_DB or O_BUFF */
	__cl_dma_memcpy((rt_pointerT) ImageIn+(0), (rt_pointerT) (FaceDet_L1_Memory + 0)+0, 1938, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	/* ===================End Read First Tile=========================================== */
	/* Call Kernel LOC_INNER_LOOP_PROLOG */
	cl_team_fork(__builtin_pulp_CoreCount(), (void *) KerIntegralImagePrime, (void *) KerArg0);
	/* Kernel Iteration Loop on Iter space */
	Iter=0; {
		/* Loop Iteration Body on Iter space */
		/* Elaborate Last, Next_Last, Next_Next_Last */
		Last = ((Iter+1) == 1); NextLast = ((Iter+2) == 1); NextNextLast = ((Iter+3) == 1);
		/* =======================Read Next Tile=========================================== */
		cl_dma_wait(&DmaR_Evt1);
		if (!Last) {
			__cl_dma_memcpy((rt_pointerT) ImageIn + ((0+1)*1938),
					(rt_pointerT) (FaceDet_L1_Memory + 0) + (1940*((N_Ti+1) % 2)), 1938, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
		}
		/* ===================End Read Next Tile=========================================== */
		/* Call Kernel LOC_INNER_LOOP */
		KerArg1->In = (Wordu8 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 0) + 1940*((N_Ti) % 2));
		KerArg1->IntegralImage = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 4084) + 7752*((N_Ti) % 2));
		cl_team_fork(__builtin_pulp_CoreCount(), (void *) KerIntegralImageProcess, (void *) KerArg1);
		/* =======================Write Tile=========================================== */
		if (0) {
			cl_dma_wait(&DmaW_Evt1);
		}
		__cl_dma_memcpy((rt_pointerT) IntegralImage + ((0)*7752),
			(rt_pointerT) (FaceDet_L1_Memory + 4084) + (7752*(N_Ti % 2)), 7752, CL_DMA_DIR_LOC2EXT, 0, &DmaW_Evt1);
		/* ===================End Write Tile=========================================== */
		N_Ti++;
		/* End Kernel Iteration Loop on Iter space */
	}
	Iter=1;
	/* =======================Write Last Tile=========================================== */
	cl_dma_wait(&DmaW_Evt1);
	/* ===================End Write Last Tile=========================================== */
}

void ProcessSquaredIntegralImage_2(
		Wordu8 *  __restrict__ ImageIn,
		Wordu32 *  __restrict__ IntegralImage,
		Kernel_Exec_T *Ker)

{
	/* Shared L1: 19588 bytes, L2 buffer: 0 bytes */
	/* Local variables used by this kernel */
	cl_dma_copy_t DmaR_Evt1;
	cl_dma_copy_t DmaW_Evt1;
	int Iter;
	int Last, NextLast, NextNextLast;
	int N_Ti = 0;
	int N_TiIp = 0, InPlane, OutPlane=0;
	KerPrimeImage_ArgT S_KerArg0, *KerArg0 = &S_KerArg0;
	KerProcessImage_ArgT S_KerArg1, *KerArg1 = &S_KerArg1;

	/* Initialize KerArg, Kernel invariant arguments */
	KerArg0->KerBuffer = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 3880) + 0);
	KerArg0->W = (Wordu32) (51);
	KerArg1->W = (Wordu32) (51);
	KerArg1->H = (Wordu32) (38);
	KerArg1->KerBuffer = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 3880) + 0);
	/* =======================Read First Tile=========================================== */
	/* Initial reads in L2, O_DB or O_BUFF */
	__cl_dma_memcpy((rt_pointerT) ImageIn+(0), (rt_pointerT) (FaceDet_L1_Memory + 0)+0, 1938, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	/* ===================End Read First Tile=========================================== */
	/* Call Kernel LOC_INNER_LOOP_PROLOG */
	cl_team_fork(__builtin_pulp_CoreCount(), (void *) KerIntegralImagePrime, (void *) KerArg0);
	/* Kernel Iteration Loop on Iter space */
	Iter=0; {
		/* Loop Iteration Body on Iter space */
		/* Elaborate Last, Next_Last, Next_Next_Last */
		Last = ((Iter+1) == 1); NextLast = ((Iter+2) == 1); NextNextLast = ((Iter+3) == 1);
		/* =======================Read Next Tile=========================================== */
		cl_dma_wait(&DmaR_Evt1);
		if (!Last) {
			__cl_dma_memcpy((rt_pointerT) ImageIn + ((0+1)*1938),
					(rt_pointerT) (FaceDet_L1_Memory + 0) + (1940*((N_Ti+1) % 2)), 1938, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
		}
		/* ===================End Read Next Tile=========================================== */
		/* Call Kernel LOC_INNER_LOOP */
		KerArg1->In = (Wordu8 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 0) + 1940*((N_Ti) % 2));
		KerArg1->IntegralImage = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 4084) + 7752*((N_Ti) % 2));
		cl_team_fork(__builtin_pulp_CoreCount(), (void *) KerSquaredIntegralImageProcess, (void *) KerArg1);
		/* =======================Write Tile=========================================== */
		if (0) {
			cl_dma_wait(&DmaW_Evt1);
		}
		__cl_dma_memcpy((rt_pointerT) IntegralImage + ((0)*7752),
			(rt_pointerT) (FaceDet_L1_Memory + 4084) + (7752*(N_Ti % 2)), 7752, CL_DMA_DIR_LOC2EXT, 0, &DmaW_Evt1);
		/* ===================End Write Tile=========================================== */
		N_Ti++;
		/* End Kernel Iteration Loop on Iter space */
	}
	Iter=1;
	/* =======================Write Last Tile=========================================== */
	cl_dma_wait(&DmaW_Evt1);
	/* ===================End Write Last Tile=========================================== */
}

void ProcessCascade_2(
		Wordu32 *  __restrict__ IntegralImage,
		Wordu32 *  __restrict__ SquaredIntegralImage,
		void *  cascade_model,
		Word32  *  __restrict__ CascadeReponse,
		Kernel_Exec_T *Ker)

{
	/* Shared L1: 23968 bytes, L2 buffer: 0 bytes */
	/* Local variables used by this kernel */
	cl_dma_copy_t DmaR_Evt1;
	cl_dma_copy_t DmaR_Evt2;
	cl_dma_copy_t DmaW_Evt1;
	int Iter;
	int Last, NextLast, NextNextLast;
	int N_Ti = 0;
	int N_TiIp = 0, InPlane, OutPlane=0;

	/* Initialize KerArg, Kernel invariant arguments */
	/* =======================Read First Tile=========================================== */
	/* Initial reads in L2, O_DB or O_BUFF */
	__cl_dma_memcpy((rt_pointerT) IntegralImage+(0), (rt_pointerT) (FaceDet_L1_Memory + 0)+0, 5712, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	__cl_dma_memcpy((rt_pointerT) SquaredIntegralImage+(0), (rt_pointerT) (FaceDet_L1_Memory + 11424)+0, 5712, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt2);
	/* ===================End Read First Tile=========================================== */
	/* Kernel Iteration Loop on Iter space */
	for (Iter=0; Iter<3; Iter++) {
		/* Loop Iteration Body on Iter space */
		/* Elaborate Last, Next_Last, Next_Next_Last */
		Last = ((Iter+1) == 3); NextLast = ((Iter+2) == 3); NextNextLast = ((Iter+3) == 3);
		/* =======================Read Next Tile=========================================== */
		cl_dma_wait(&DmaR_Evt1);
		cl_dma_wait(&DmaR_Evt2);
		if (!Last) {
			__cl_dma_memcpy((rt_pointerT) IntegralImage + ((Iter+1)*1020),
					(rt_pointerT) (FaceDet_L1_Memory + 0) + (5712*((N_Ti+1) % 2)), 5712, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
			__cl_dma_memcpy((rt_pointerT) SquaredIntegralImage + ((Iter+1)*1020),
					(rt_pointerT) (FaceDet_L1_Memory + 11424) + (5712*((N_Ti+1) % 2)), 5712, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt2);
		}
		/* ===================End Read Next Tile=========================================== */
		/* Call Kernel LOC_INNER_LOOP */
		KerEvaluateCascade(
			(Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 0) + 5712*((N_Ti) % 2)),
			(Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 11424) + 5712*((N_Ti) % 2)),
			(Wordu32) (51),
			(Wordu32) (28),
			(cascade_model),
			(Wordu8) (24),
			(Wordu8) (24),
			(Word32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 22848) + 560*((N_Ti) % 2))
		);
		/* =======================Write Tile=========================================== */
		if (Iter) {
			cl_dma_wait(&DmaW_Evt1);
		}
		__cl_dma_memcpy((rt_pointerT) CascadeReponse + ((Iter)*560),
			(rt_pointerT) (FaceDet_L1_Memory + 22848) + (560*(N_Ti % 2)), 560, CL_DMA_DIR_LOC2EXT, 0, &DmaW_Evt1);
		/* ===================End Write Tile=========================================== */
		N_Ti++;
		/* End Kernel Iteration Loop on Iter space */
	}
	Iter=3;
	/* =======================Write Last Tile=========================================== */
	cl_dma_wait(&DmaW_Evt1);
	/* ===================End Write Last Tile=========================================== */
}

void ResizeImage_3(
		unsigned char * In,
		unsigned char * Out,
		Kernel_Exec_T *Ker)

{
	/* Shared L1: 21440 bytes, L2 buffer: 0 bytes */
	/* Local variables used by this kernel */
	cl_dma_copy_t DmaR_Evt1;
	cl_dma_copy_t DmaW_Evt1;
	int Iter;
	int Last, NextLast, NextNextLast;
	int N_Ti = 0;
	int N_TiIp = 0, InPlane, OutPlane=0;
	KerResizeBilinear_ArgT S_KerArg0, *KerArg0 = &S_KerArg0;

	/* Initialize KerArg, Kernel invariant arguments */
	KerArg0->Win = (unsigned int) (320);
	KerArg0->Hin = (unsigned int) (240);
	KerArg0->Wout = (unsigned int) (40);
	KerArg0->Hout = (unsigned int) (30);
	/* =======================Read First Tile=========================================== */
	/* Initial reads in L2, O_DB or O_BUFF */
	__cl_dma_memcpy((rt_pointerT) In+(0), (rt_pointerT) (FaceDet_L1_Memory + 0)+0, 10560, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	/* ===================End Read First Tile=========================================== */
	/* Kernel Iteration Loop on Iter space */
	for (Iter=0; Iter<8; Iter++) {
		/* Loop Iteration Body on Iter space */
		/* Elaborate Last, Next_Last, Next_Next_Last */
		Last = ((Iter+1) == 8); NextLast = ((Iter+2) == 8); NextNextLast = ((Iter+3) == 8);
		/* =======================Read Next Tile=========================================== */
		cl_dma_wait(&DmaR_Evt1);
		if (!Last) {
			__cl_dma_memcpy((rt_pointerT) In + (((522103*(Iter+1)*4)>>16)*320),
					(rt_pointerT) (FaceDet_L1_Memory + 0) + (10560*((N_Ti+1) % 2)), NextLast?5440:10560, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
		}
		/* ===================End Read Next Tile=========================================== */
		/* Call Kernel LOC_INNER_LOOP */
		KerArg0->In = (unsigned char * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 0) + 10560*((N_Ti) % 2));
		KerArg0->Out = (unsigned char * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 21120) + 160*((N_Ti) % 2));
		KerArg0->HTileOut = (unsigned int) (Last?2:4);
		KerArg0->FirstLineIndex = (unsigned int) ((522103*Iter*4)>>16);
		cl_team_fork(__builtin_pulp_CoreCount(), (void *) KerResizeBilinear, (void *) KerArg0);
		/* =======================Write Tile=========================================== */
		if (Iter) {
			cl_dma_wait(&DmaW_Evt1);
		}
		__cl_dma_memcpy((rt_pointerT) Out + ((Iter)*160),
			(rt_pointerT) (FaceDet_L1_Memory + 21120) + (160*(N_Ti % 2)), Last?80:160, CL_DMA_DIR_LOC2EXT, 0, &DmaW_Evt1);
		/* ===================End Write Tile=========================================== */
		N_Ti++;
		/* End Kernel Iteration Loop on Iter space */
	}
	Iter=8;
	/* =======================Write Last Tile=========================================== */
	cl_dma_wait(&DmaW_Evt1);
	/* ===================End Write Last Tile=========================================== */
}

void ProcessIntegralImage_3(
		Wordu8 *  __restrict__ ImageIn,
		Wordu32 *  __restrict__ IntegralImage,
		Kernel_Exec_T *Ker)

{
	/* Shared L1: 12160 bytes, L2 buffer: 0 bytes */
	/* Local variables used by this kernel */
	cl_dma_copy_t DmaR_Evt1;
	cl_dma_copy_t DmaW_Evt1;
	int Iter;
	int Last, NextLast, NextNextLast;
	int N_Ti = 0;
	int N_TiIp = 0, InPlane, OutPlane=0;
	KerPrimeImage_ArgT S_KerArg0, *KerArg0 = &S_KerArg0;
	KerProcessImage_ArgT S_KerArg1, *KerArg1 = &S_KerArg1;

	/* Initialize KerArg, Kernel invariant arguments */
	KerArg0->KerBuffer = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 2400) + 0);
	KerArg0->W = (Wordu32) (40);
	KerArg1->W = (Wordu32) (40);
	KerArg1->H = (Wordu32) (30);
	KerArg1->KerBuffer = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 2400) + 0);
	/* =======================Read First Tile=========================================== */
	/* Initial reads in L2, O_DB or O_BUFF */
	__cl_dma_memcpy((rt_pointerT) ImageIn+(0), (rt_pointerT) (FaceDet_L1_Memory + 0)+0, 1200, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	/* ===================End Read First Tile=========================================== */
	/* Call Kernel LOC_INNER_LOOP_PROLOG */
	cl_team_fork(__builtin_pulp_CoreCount(), (void *) KerIntegralImagePrime, (void *) KerArg0);
	/* Kernel Iteration Loop on Iter space */
	Iter=0; {
		/* Loop Iteration Body on Iter space */
		/* Elaborate Last, Next_Last, Next_Next_Last */
		Last = ((Iter+1) == 1); NextLast = ((Iter+2) == 1); NextNextLast = ((Iter+3) == 1);
		/* =======================Read Next Tile=========================================== */
		cl_dma_wait(&DmaR_Evt1);
		if (!Last) {
			__cl_dma_memcpy((rt_pointerT) ImageIn + ((0+1)*1200),
					(rt_pointerT) (FaceDet_L1_Memory + 0) + (1200*((N_Ti+1) % 2)), 1200, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
		}
		/* ===================End Read Next Tile=========================================== */
		/* Call Kernel LOC_INNER_LOOP */
		KerArg1->In = (Wordu8 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 0) + 1200*((N_Ti) % 2));
		KerArg1->IntegralImage = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 2560) + 4800*((N_Ti) % 2));
		cl_team_fork(__builtin_pulp_CoreCount(), (void *) KerIntegralImageProcess, (void *) KerArg1);
		/* =======================Write Tile=========================================== */
		if (0) {
			cl_dma_wait(&DmaW_Evt1);
		}
		__cl_dma_memcpy((rt_pointerT) IntegralImage + ((0)*4800),
			(rt_pointerT) (FaceDet_L1_Memory + 2560) + (4800*(N_Ti % 2)), 4800, CL_DMA_DIR_LOC2EXT, 0, &DmaW_Evt1);
		/* ===================End Write Tile=========================================== */
		N_Ti++;
		/* End Kernel Iteration Loop on Iter space */
	}
	Iter=1;
	/* =======================Write Last Tile=========================================== */
	cl_dma_wait(&DmaW_Evt1);
	/* ===================End Write Last Tile=========================================== */
}

void ProcessSquaredIntegralImage_3(
		Wordu8 *  __restrict__ ImageIn,
		Wordu32 *  __restrict__ IntegralImage,
		Kernel_Exec_T *Ker)

{
	/* Shared L1: 12160 bytes, L2 buffer: 0 bytes */
	/* Local variables used by this kernel */
	cl_dma_copy_t DmaR_Evt1;
	cl_dma_copy_t DmaW_Evt1;
	int Iter;
	int Last, NextLast, NextNextLast;
	int N_Ti = 0;
	int N_TiIp = 0, InPlane, OutPlane=0;
	KerPrimeImage_ArgT S_KerArg0, *KerArg0 = &S_KerArg0;
	KerProcessImage_ArgT S_KerArg1, *KerArg1 = &S_KerArg1;

	/* Initialize KerArg, Kernel invariant arguments */
	KerArg0->KerBuffer = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 2400) + 0);
	KerArg0->W = (Wordu32) (40);
	KerArg1->W = (Wordu32) (40);
	KerArg1->H = (Wordu32) (30);
	KerArg1->KerBuffer = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 2400) + 0);
	/* =======================Read First Tile=========================================== */
	/* Initial reads in L2, O_DB or O_BUFF */
	__cl_dma_memcpy((rt_pointerT) ImageIn+(0), (rt_pointerT) (FaceDet_L1_Memory + 0)+0, 1200, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	/* ===================End Read First Tile=========================================== */
	/* Call Kernel LOC_INNER_LOOP_PROLOG */
	cl_team_fork(__builtin_pulp_CoreCount(), (void *) KerIntegralImagePrime, (void *) KerArg0);
	/* Kernel Iteration Loop on Iter space */
	Iter=0; {
		/* Loop Iteration Body on Iter space */
		/* Elaborate Last, Next_Last, Next_Next_Last */
		Last = ((Iter+1) == 1); NextLast = ((Iter+2) == 1); NextNextLast = ((Iter+3) == 1);
		/* =======================Read Next Tile=========================================== */
		cl_dma_wait(&DmaR_Evt1);
		if (!Last) {
			__cl_dma_memcpy((rt_pointerT) ImageIn + ((0+1)*1200),
					(rt_pointerT) (FaceDet_L1_Memory + 0) + (1200*((N_Ti+1) % 2)), 1200, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
		}
		/* ===================End Read Next Tile=========================================== */
		/* Call Kernel LOC_INNER_LOOP */
		KerArg1->In = (Wordu8 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 0) + 1200*((N_Ti) % 2));
		KerArg1->IntegralImage = (Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 2560) + 4800*((N_Ti) % 2));
		cl_team_fork(__builtin_pulp_CoreCount(), (void *) KerSquaredIntegralImageProcess, (void *) KerArg1);
		/* =======================Write Tile=========================================== */
		if (0) {
			cl_dma_wait(&DmaW_Evt1);
		}
		__cl_dma_memcpy((rt_pointerT) IntegralImage + ((0)*4800),
			(rt_pointerT) (FaceDet_L1_Memory + 2560) + (4800*(N_Ti % 2)), 4800, CL_DMA_DIR_LOC2EXT, 0, &DmaW_Evt1);
		/* ===================End Write Tile=========================================== */
		N_Ti++;
		/* End Kernel Iteration Loop on Iter space */
	}
	Iter=1;
	/* =======================Write Last Tile=========================================== */
	cl_dma_wait(&DmaW_Evt1);
	/* ===================End Write Last Tile=========================================== */
}

void ProcessCascade_3(
		Wordu32 *  __restrict__ IntegralImage,
		Wordu32 *  __restrict__ SquaredIntegralImage,
		void *  cascade_model,
		Word32  *  __restrict__ CascadeReponse,
		Kernel_Exec_T *Ker)

{
	/* Shared L1: 20152 bytes, L2 buffer: 0 bytes */
	/* Local variables used by this kernel */
	cl_dma_copy_t DmaR_Evt1;
	cl_dma_copy_t DmaR_Evt2;
	cl_dma_copy_t DmaW_Evt1;
	int Iter;
	int Last, NextLast, NextNextLast;
	int N_Ti = 0;
	int N_TiIp = 0, InPlane, OutPlane=0;

	/* Initialize KerArg, Kernel invariant arguments */
	/* =======================Read First Tile=========================================== */
	/* Initial reads in L2, O_DB or O_BUFF */
	__cl_dma_memcpy((rt_pointerT) IntegralImage+(0), (rt_pointerT) (FaceDet_L1_Memory + 0)+0, 4800, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	__cl_dma_memcpy((rt_pointerT) SquaredIntegralImage+(0), (rt_pointerT) (FaceDet_L1_Memory + 9600)+0, 4800, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt2);
	/* ===================End Read First Tile=========================================== */
	/* Kernel Iteration Loop on Iter space */
	Iter=0; {
		/* Loop Iteration Body on Iter space */
		/* Elaborate Last, Next_Last, Next_Next_Last */
		Last = ((Iter+1) == 1); NextLast = ((Iter+2) == 1); NextNextLast = ((Iter+3) == 1);
		/* =======================Read Next Tile=========================================== */
		cl_dma_wait(&DmaR_Evt1);
		cl_dma_wait(&DmaR_Evt2);
		if (!Last) {
			__cl_dma_memcpy((rt_pointerT) IntegralImage + ((0+1)*1120),
					(rt_pointerT) (FaceDet_L1_Memory + 0) + (4800*((N_Ti+1) % 2)), 4800, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
			__cl_dma_memcpy((rt_pointerT) SquaredIntegralImage + ((0+1)*1120),
					(rt_pointerT) (FaceDet_L1_Memory + 9600) + (4800*((N_Ti+1) % 2)), 4800, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt2);
		}
		/* ===================End Read Next Tile=========================================== */
		/* Call Kernel LOC_INNER_LOOP */
		KerEvaluateCascade(
			(Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 0) + 4800*((N_Ti) % 2)),
			(Wordu32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 9600) + 4800*((N_Ti) % 2)),
			(Wordu32) (40),
			(Wordu32) (30),
			(cascade_model),
			(Wordu8) (24),
			(Wordu8) (24),
			(Word32 * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 19200) + 476*((N_Ti) % 2))
		);
		/* =======================Write Tile=========================================== */
		if (0) {
			cl_dma_wait(&DmaW_Evt1);
		}
		__cl_dma_memcpy((rt_pointerT) CascadeReponse + ((0)*476),
			(rt_pointerT) (FaceDet_L1_Memory + 19200) + (476*(N_Ti % 2)), 476, CL_DMA_DIR_LOC2EXT, 0, &DmaW_Evt1);
		/* ===================End Write Tile=========================================== */
		N_Ti++;
		/* End Kernel Iteration Loop on Iter space */
	}
	Iter=1;
	/* =======================Write Last Tile=========================================== */
	cl_dma_wait(&DmaW_Evt1);
	/* ===================End Write Last Tile=========================================== */
}

void final_resize(
		unsigned char * In,
		unsigned char * Out,
		Kernel_Exec_T *Ker)

{
	/* Shared L1: 24640 bytes, L2 buffer: 0 bytes */
	/* Local variables used by this kernel */
	cl_dma_copy_t DmaR_Evt1;
	cl_dma_copy_t DmaW_Evt1;
	int Iter;
	int Last, NextLast, NextNextLast;
	int N_Ti = 0;
	int N_TiIp = 0, InPlane, OutPlane=0;
	KerResizeBilinear_ArgT S_KerArg0, *KerArg0 = &S_KerArg0;

	/* Initialize KerArg, Kernel invariant arguments */
	KerArg0->Win = (unsigned int) (320);
	KerArg0->Hin = (unsigned int) (240);
	KerArg0->Wout = (unsigned int) (160);
	KerArg0->Hout = (unsigned int) (120);
	KerArg0->HTileOut = (unsigned int) (15);
	/* =======================Read First Tile=========================================== */
	/* Initial reads in L2, O_DB or O_BUFF */
	__cl_dma_memcpy((rt_pointerT) In+(0), (rt_pointerT) (FaceDet_L1_Memory + 0)+0, 9920, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
	/* ===================End Read First Tile=========================================== */
	/* Kernel Iteration Loop on Iter space */
	for (Iter=0; Iter<8; Iter++) {
		/* Loop Iteration Body on Iter space */
		/* Elaborate Last, Next_Last, Next_Next_Last */
		Last = ((Iter+1) == 8); NextLast = ((Iter+2) == 8); NextNextLast = ((Iter+3) == 8);
		/* =======================Read Next Tile=========================================== */
		cl_dma_wait(&DmaR_Evt1);
		if (!Last) {
			__cl_dma_memcpy((rt_pointerT) In + (((130525*(Iter+1)*15)>>16)*320),
					(rt_pointerT) (FaceDet_L1_Memory + 0) + (9920*((N_Ti+1) % 2)), 9920, CL_DMA_DIR_EXT2LOC, 0, &DmaR_Evt1);
		}
		/* ===================End Read Next Tile=========================================== */
		/* Call Kernel LOC_INNER_LOOP */
		KerArg0->In = (unsigned char * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 0) + 9920*((N_Ti) % 2));
		KerArg0->Out = (unsigned char * __restrict__) ((rt_pointerT) (FaceDet_L1_Memory + 19840) + 2400*((N_Ti) % 2));
		KerArg0->FirstLineIndex = (unsigned int) ((130525*Iter*15)>>16);
		cl_team_fork(__builtin_pulp_CoreCount(), (void *) KerResizeBilinear, (void *) KerArg0);
		/* =======================Write Tile=========================================== */
		if (Iter) {
			cl_dma_wait(&DmaW_Evt1);
		}
		__cl_dma_memcpy((rt_pointerT) Out + ((Iter)*2400),
			(rt_pointerT) (FaceDet_L1_Memory + 19840) + (2400*(N_Ti % 2)), 2400, CL_DMA_DIR_LOC2EXT, 0, &DmaW_Evt1);
		/* ===================End Write Tile=========================================== */
		N_Ti++;
		/* End Kernel Iteration Loop on Iter space */
	}
	Iter=8;
	/* =======================Write Last Tile=========================================== */
	cl_dma_wait(&DmaW_Evt1);
	/* ===================End Write Last Tile=========================================== */
}

