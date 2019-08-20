GLOBAL_INCLUDES		+= ./autotiler/include/
GLOBAL_INCLUDES		+= ./autotiler/generators/MatAdd/generator/include
GLOBAL_INCLUDES		+= ./autotiler/generators/MatAdd/kernels/include
GLOBAL_INCLUDES		+= ./autotiler/generators/CNN/generator/include
GLOBAL_INCLUDES		+= ./autotiler/generators/CNN/kernels/include
GLOBAL_INCLUDES		+= ./autotiler/generators/MatMult/generator/include
GLOBAL_INCLUDES		+= ./autotiler/generators/MatMult/kernels/include
GLOBAL_INCLUDES		+= ./autotiler/generators/Fir/generator/include
GLOBAL_INCLUDES		+= ./autotiler/generators/Fir/kernels/include
GLOBAL_INCLUDES		+= ./autotiler/generators/FFT2D/generator/include
GLOBAL_INCLUDES		+= ./autotiler/generators/FFT2D/kernels/include
#GLOBAL_INCLUDES		+= ./autotiler/generators/Resize/generator/include
#GLOBAL_INCLUDES		+= ./autotiler/generators/Resize/kernels/include
GLOBAL_INCLUDES		+= ./autotiler/generators/IntegralImg/generator/include
GLOBAL_INCLUDES		+= ./autotiler/generators/IntegralImg/kernels/include
GLOBAL_INCLUDES		+= ./autotiler/generators/HoG/generator/include
GLOBAL_INCLUDES		+= ./autotiler/generators/HoG/kernels/include

#$(NAME)_SOURCES     += ./autotiler/generators/MatAdd/generator/src/MatAddGenerator.c
#$(NAME)_SOURCES     += ./autotiler/generators/MatAdd/kernels/src/MatAddBasicKernels.c

$(NAME)_SOURCES     += ./autotiler/generators/CNN/generator/src/CNN_Generators.c
$(NAME)_SOURCES     += ./autotiler/generators/CNN/kernels/src/CNN_Pooling_BasicKernels.c
$(NAME)_SOURCES     += ./autotiler/generators/CNN/kernels/src/CNN_SoftMax.c
$(NAME)_SOURCES     += ./autotiler/generators/CNN/kernels/src/CNN_Conv_BasicKernels.c
$(NAME)_SOURCES     += ./autotiler/generators/CNN/kernels/src/CNN_BiasReLULinear_BasicKernels.c

#$(NAME)_SOURCES     += ./autotiler/generators/CNN/kernels/src/CNN_HwCE.c

#$(NAME)_SOURCES     += ./autotiler/generators/MatMult/generator/src/MatMultGenerator.c
#$(NAME)_SOURCES     += ./autotiler/generators/MatMult/kernels/src/MatMultBasicKernels.c

#$(NAME)_SOURCES     += ./autotiler/generators/Fir/generator/src/FirGenerator.c
#$(NAME)_SOURCES     += ./autotiler/generators/Fir/kernels/src/FirBasicKernels.c

#$(NAME)_SOURCES     += ./autotiler/generators/FFT2D/generator/src/FFT2DGenerator.c
#$(NAME)_SOURCES     += ./autotiler/generators/FFT2D/kernels/src/FFTBasicKernels.c
#$(NAME)_SOURCES     += ./autotiler/generators/FFT2D/kernels/src/FFTScalarBasicKernels.c
#$(NAME)_SOURCES     += ./autotiler/generators/FFT2D/twiddles_gen/GenTwid_Swap.c

#$(NAME)_SOURCES     += ./autotiler/generators/Resize/generator/src/ResizeGenerator.c
#$(NAME)_SOURCES     += ./autotiler/generators/Resize/kernels/src/ResizeBasicKernels.c

#$(NAME)_SOURCES     += ./autotiler/generators/IntegralImg/generator/src/IntegralImgGenerator.c
#$(NAME)_SOURCES     += ./autotiler/generators/IntegralImg/kernels/src/IntegralImgBasicKernels.c

#$(NAME)_SOURCES     += ./autotiler/generators/HoG/generator/src/HoGGenerator.c
#$(NAME)_SOURCES     += ./autotiler/generators/HoG/kernels/src/HoGBasicKernels.c
