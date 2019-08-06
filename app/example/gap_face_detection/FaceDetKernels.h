#ifndef __FACEDETKERNEL_H__
#define __FACEDETKERNEL_H__

#include "AutoTilerLibTypes.h"
#include "FaceDetKernelsInit.h"
#include "FaceDetBasicKernels.h"
#include "Gap8.h"
#include "pmsis_tiling.h"
#define _FaceDet_L1_Memory_SIZE 24904
#define _FaceDet_L2_Memory_SIZE 0
extern char *FaceDet_L1_Memory; /* Size given for generation: 25000 bytes, used: 24904 bytes */
extern char *FaceDet_L2_Memory; /* Size used for generation: 0 bytes */
extern void ResizeImage_1(
		unsigned char * In,
		unsigned char * Out,
		Kernel_Exec_T *Ker);
extern void ProcessIntegralImage_1(
		Wordu8 *  __restrict__ ImageIn,
		Wordu32 *  __restrict__ IntegralImage,
		Kernel_Exec_T *Ker);
extern void ProcessSquaredIntegralImage_1(
		Wordu8 *  __restrict__ ImageIn,
		Wordu32 *  __restrict__ IntegralImage,
		Kernel_Exec_T *Ker);
extern void ProcessCascade_1(
		Wordu32 *  __restrict__ IntegralImage,
		Wordu32 *  __restrict__ SquaredIntegralImage,
		void *  cascade_model,
		Word32  *  __restrict__ CascadeReponse,
		Kernel_Exec_T *Ker);
extern void ResizeImage_2(
		unsigned char * In,
		unsigned char * Out,
		Kernel_Exec_T *Ker);
extern void ProcessIntegralImage_2(
		Wordu8 *  __restrict__ ImageIn,
		Wordu32 *  __restrict__ IntegralImage,
		Kernel_Exec_T *Ker);
extern void ProcessSquaredIntegralImage_2(
		Wordu8 *  __restrict__ ImageIn,
		Wordu32 *  __restrict__ IntegralImage,
		Kernel_Exec_T *Ker);
extern void ProcessCascade_2(
		Wordu32 *  __restrict__ IntegralImage,
		Wordu32 *  __restrict__ SquaredIntegralImage,
		void *  cascade_model,
		Word32  *  __restrict__ CascadeReponse,
		Kernel_Exec_T *Ker);
extern void ResizeImage_3(
		unsigned char * In,
		unsigned char * Out,
		Kernel_Exec_T *Ker);
extern void ProcessIntegralImage_3(
		Wordu8 *  __restrict__ ImageIn,
		Wordu32 *  __restrict__ IntegralImage,
		Kernel_Exec_T *Ker);
extern void ProcessSquaredIntegralImage_3(
		Wordu8 *  __restrict__ ImageIn,
		Wordu32 *  __restrict__ IntegralImage,
		Kernel_Exec_T *Ker);
extern void ProcessCascade_3(
		Wordu32 *  __restrict__ IntegralImage,
		Wordu32 *  __restrict__ SquaredIntegralImage,
		void *  cascade_model,
		Word32  *  __restrict__ CascadeReponse,
		Kernel_Exec_T *Ker);
extern void final_resize(
		unsigned char * In,
		unsigned char * Out,
		Kernel_Exec_T *Ker);
#endif
