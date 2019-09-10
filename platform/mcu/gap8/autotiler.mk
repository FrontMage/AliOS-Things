GLOBAL_INCLUDES		+= ./autotiler/include/
GLOBAL_INCLUDES		+= ./autotiler/generators/CNN
GLOBAL_INCLUDES		+= ./autotiler/generators/FFT2D
GLOBAL_INCLUDES		+= ./autotiler/generators/IntegralImg
GLOBAL_INCLUDES		+= ./autotiler/generators/MatAdd
#GLOBAL_INCLUDES		+= ./autotiler/generators/Resize

$(NAME)_SOURCES     += ./autotiler/generators/CNN/CNN_BiasReLULinear_BasicKernels.c
$(NAME)_SOURCES     += ./autotiler/generators/CNN/CNN_Conv_BasicKernels.c
$(NAME)_SOURCES     += ./autotiler/generators/CNN/CNN_Generators.c
$(NAME)_SOURCES     += ./autotiler/generators/CNN/CNN_Pooling_BasicKernels.c
$(NAME)_SOURCES     += ./autotiler/generators/CNN/CNN_SoftMax.c

#$(NAME)_SOURCES     += ./autotiler/generators/FFT2D/generator/FFT2DGenerator.c
#$(NAME)_SOURCES     += ./autotiler/generators/FFT2D/kernels/FFTBasicKernels.c
#$(NAME)_SOURCES     += ./autotiler/generators/FFT2D/kernels/FFTScalarBasicKernels.c
#$(NAME)_SOURCES     += ./autotiler/generators/FFT2D/twiddles_gen/GenTwid_Swap.c

#$(NAME)_SOURCES     += ./autotiler/generators/IntegralImg/generator/IntegralImgGenerator.c
#$(NAME)_SOURCES     += ./autotiler/generators/IntegralImg/kernels/IntegralImgBasicKernels.c

#$(NAME)_SOURCES     += ./autotiler/generators/MatAdd/MatAddGenerator.c
#$(NAME)_SOURCES     += ./autotiler/generators/MatAdd/MatAddBasicKernels.c

#$(NAME)_SOURCES     += ./autotiler/generators/Resize/generator/ResizeGenerator.c
#$(NAME)_SOURCES     += ./autotiler/generators/Resize/kernels/ResizeBasicKernels.c
