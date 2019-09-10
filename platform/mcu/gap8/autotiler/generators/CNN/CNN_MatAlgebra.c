#include <stdio.h>
#include <math.h>
#include "CNN_BasicKernels.h"

#define Minu(a, b)              (( ((unsigned int)a)<((unsigned int)b) )?((unsigned int)a):((unsigned int)b) )

#ifdef __pulp__
#define Abs(a)			__builtin_pulp_abs((a))
#define Min(a, b)               __builtin_pulp_minsi((a), (b))
#define Max(a, b)               __builtin_pulp_maxsi((a), (b))
#else
#define Abs(a)			(((int)(a)<0)?(-(a)):(a))
#define Min(a, b)               (((a)<(b))?(a):(b))
#define Max(a, b)               (((a)>(b))?(a):(b))
#endif

static int CoreCountDynamic = 1;
static int ActiveCore = gap_ncore();

static inline unsigned int __attribute__((always_inline)) ChunkSize(unsigned int X)

{
        unsigned int NCore;
        unsigned int Log2Core;
        unsigned int Chunk;

        if (CoreCountDynamic) NCore = ActiveCore; else NCore = gap_ncore();
        Log2Core = gap_fl1(NCore);
        Chunk = (X>>Log2Core) + ((X&(NCore-1))!=0);
        return Chunk;
}

void KerParMatAdd_fp(KerMat3_fp_T *Arg)

{
	short int * __restrict__ In1	= Arg->In1;
	short int * __restrict__ In2	= Arg->In2;
	short int * __restrict__ Out	= Arg->Out;
	int W				= Arg->W;
	int H				= Arg->H;

        unsigned int CoreId = gap_coreid();
        unsigned int Chunk = ChunkSize(Arg->N);
        unsigned int First = Chunk*CoreId;
        unsigned int Last = Min(First+Chunk, Arg->N);
	int i, j;

	for (i=First; i<Last; i++) {
		short int * __restrict__ I1 = In1 + i*W*H;
		short int * __restrict__ I2 = In2 + i*W*H;
		short int * __restrict__ O  = Out + i*W*H;
		for (j=0; j<((W*H)/2); j++) {
			O[2*j] = gap_clip(I1[2*j]+I2[2*j], 15); O[2*j+1] = gap_clip(I1[2*j+1]+I2[2*j+1], 15);
		}
		O[W*H-1] = gap_clip(I1[W*H-1] + I2[W*H-1], 15);
	}
        gap_waitbarrier(0);
}

void KerParMatAdd_fps(KerMat3_fps_T *Arg)

{
	signed char * __restrict__ In1	= Arg->In1;
	signed char * __restrict__ In2	= Arg->In2;
	signed char * __restrict__ Out	= Arg->Out;
	int W				= Arg->W;
	int H				= Arg->H;

        unsigned int CoreId = gap_coreid();
        unsigned int Chunk = ChunkSize(Arg->N);
        unsigned int First = Chunk*CoreId;
        unsigned int Last = Min(First+Chunk, Arg->N);
	int i, j;

	for (i=First; i<Last; i++) {
		signed char * __restrict__ I1 = In1 + i*W*H;
		signed char * __restrict__ I2 = In2 + i*W*H;
		signed char * __restrict__ O  = Out + i*W*H;
		for (j=0; j<((W*H)/2); j++) {
			O[2*j] = gap_clip(I1[2*j]+I2[2*j], 7); O[2*j+1] = gap_clip(I1[2*j+1]+I2[2*j+1], 7);
		}
		O[W*H-1] = gap_clip(I1[W*H-1] + I2[W*H-1], 7);
	}
        gap_waitbarrier(0);
}

void KerParMatAddReLU_fp(KerMat3_fp_T *Arg)

{
	short int * __restrict__ In1	= Arg->In1;
	short int * __restrict__ In2	= Arg->In2;
	short int * __restrict__ Out	= Arg->Out;
	int W				= Arg->W;
	int H				= Arg->H;

        unsigned int CoreId = gap_coreid();
        unsigned int Chunk = ChunkSize(Arg->N);
        unsigned int First = Chunk*CoreId;
        unsigned int Last = Min(First+Chunk, Arg->N);
	int i, j;

	for (i=First; i<Last; i++) {
		short int * __restrict__ I1 = In1 + i*W*H;
		short int * __restrict__ I2 = In2 + i*W*H;
		short int * __restrict__ O  = Out + i*W*H;
		for (j=0; j<((W*H)/2); j++) {
			O[2*j] = Max(gap_clip(I1[2*j]+I2[2*j], 15), 0); O[2*j+1] = Max(gap_clip(I1[2*j+1]+I2[2*j+1], 15), 0);
		}
		O[W*H-1] = Max(gap_clip(I1[W*H-1] + I2[W*H-1], 15), 0);
	}
        gap_waitbarrier(0);
}

void KerParMatAddReLU_fps(KerMat3_fps_T *Arg)

{
	signed char * __restrict__ In1	= Arg->In1;
	signed char * __restrict__ In2	= Arg->In2;
	signed char * __restrict__ Out	= Arg->Out;
	int W				= Arg->W;
	int H				= Arg->H;

        unsigned int CoreId = gap_coreid();
        unsigned int Chunk = ChunkSize(Arg->N);
        unsigned int First = Chunk*CoreId;
        unsigned int Last = Min(First+Chunk, Arg->N);
	int i, j;

	for (i=First; i<Last; i++) {
		signed char * __restrict__ I1 = In1 + i*W*H;
		signed char * __restrict__ I2 = In2 + i*W*H;
		signed char * __restrict__ O  = Out + i*W*H;
		for (j=0; j<((W*H)/2); j++) {
			O[2*j] = Max(gap_clip(I1[2*j]+I2[2*j], 7), 0); O[2*j+1] = Max(gap_clip(I1[2*j+1]+I2[2*j+1], 7), 0);
		}
		O[W*H-1] = Max(gap_clip(I1[W*H-1] + I2[W*H-1], 7), 0);
	}
        gap_waitbarrier(0);
}
