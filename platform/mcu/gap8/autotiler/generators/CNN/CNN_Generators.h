#ifndef __CNN_GENERATORS_H__
#define __CNN_GENERATORS_H__

extern void LoadCNNLibrary();

/*********************************************************************************************************************************************************************
        Generators for Convolutions, followed by an optional pooling (Max or Average), followed by an optional linear rectification (ReLU).

        Template:
                Name:           Name of the generated user kernel

                Ctrl:           Overide generator default options (TileOrientation, Parallel Features, Use double precision convolution, Use HWCE), Def=(TILE_HOR, 1, 0, 0)

                In_DataSize:    1: byte, 2: half word, 4: word
                Filter_DataSize:1: byte, 2: half word, 4: word
                Bias_DataSize:  1: byte, 2: half word, 4: word
                Out_DataSize:   1: byte, 2: half word, 4: word

                In_InL3:        0: In is in L2, 1: In is in L3 memory
                Filter_InL3:    0: Filter is in L2, 1: Filter is in L3 memory
                Bias_InL3:      0: Bias is in L2, 1: Bias is in L3 memory
                Out_InL3:       0: Out is in L2, 1: Out is in L3 memory

                InFeat:         Number of input feature's maps
                OutFeat:        Number of output feature's maps
                Width:          Number of columns of a given feature map
                Height:         Number of lines of a given feature map

                ConvOper:       Type of convolution, Regular convolution: KOP_CONV, Regular convolution with double precision output: KOP_CONV_DP, Depth wise convolution: KOP_CONV_DW
                Fcx:            Convolution filter x dimension
                Fcy:            Convolution filter y dimension
                Dcx:            Convolution filter dilation factor, x dimension
                Dcy:            Convolution filter dilation factor, y dimension
                Scx:            Convolution filter stride x dimension
                Scy:            Convolution filter stride y dimension
                ConvPad:        0: No padding, 1: Zero padding

                PoolOper:       Type of Pooling, Max Pooling: KOP_MAXPOOL, Average Pooling: KOP_AVGPOOL
                Fpx:            Pooling filter x dimension
                Fpy:            Pooling filter y dimension
                Dpx:            Pooling filter dilation factor, x dimension
                Dpy:            Pooling filter dilation factor, y dimension
                Spx:            Pooling filter stride x dimension
                Spy:            Pooling filter stride y dimension
                PoolPad:        0: No padding, 1: Zero padding

                ReLUOper:       Optional linaer rectification to be performed as a final step, KOP_RELU or KOP_NONE


        CNN_ConvolutionPoolReLU
        
*********************************************************************************************************************************************************************/


extern void CNN_ConvolutionPoolReLU(
	char         *Name,

	CNN_GenControl_T *Ctrl,

	int In_DataSize,
	int Filter_DataSize,
	int Bias_DataSize,
	int Out_DataSize,

	int In_InL3,           // 1 if In comes from L3, 0 if it comes from L2
	int Filter_InL3,
	int Bias_InL3,
	int Out_InL3,

	int InFeat,
	int OutFeat,
	int Width,
	int Height,

	KernelOper_T ConvOper,
	int Fcx,
	int Fcy,
	int Dcx,
	int Dcy,
	int Scx,
	int Scy,
	int          ConvPad,

	KernelOper_T PoolOper,
	int Fpx,
	int Fpy,
	int Dpx,
	int Dpy,
	int Spx,
	int Spy,
	int          PoolPad,

       	KernelOper_T ReLUOper
	);

/*********************************************************************************************************************************************************************
        Generators for Pooling (Max or Average) followed by an optional linear rectification (ReLU) or linear rectification only

        Template:
                Name:           Name of the generated user kernel

                Ctrl:           Overide generator default options (TileOrientation, Parallel Features), Def=(TILE_HOR, 1)

                In_DataSize:    1: byte, 2: half word, 4: word
                Out_DataSize:   1: byte, 2: half word, 4: word

                In_InL3:        0: In is in L2, 1: In is in L3 memory
                Out_InL3:       0: Out is in L2, 1: Out is in L3 memory

                InFeat:         Number of input feature's maps
                OutFeat:        Number of output feature's maps (InFeat has to be equal to OutFeat for these generators
                Width:          Number of columns of a given feature map
                Height:         Number of lines of a given feature map

                PoolOper:       KOP_MAXPOOL or KOP_AVGPOOL or KOP_NONE
                Fpx:            Size of the pooling filter, x dimension
                Fpy:            Size of the pooling filter, y dimension
                Dpx:            Dilation factor, x dimension
                Dpy:            Dilation factor, y dimension
                Spx:            Pooling stride, x dimension
                Spy:            Pooling stride, y dimension

                ReLUOper        optional rectification to be applied after pooling, KOP_RELU or KOP_NONE

        Currently only homegeneous data size are supported (bytes and hald words)

        CNN_PoolReLU
                ParFeat = 0
                        Input feature's maps are evaluated one after the other. A given feature map is tiled and the evaluation of a tile is dispatched on all cores for
                        parallel evaluation.
                ParFeat = 1
                        Input feature's maps are processed as a group, a tile crosses all input feature maps and then each core is given a subset of input feature's maps.
                
*********************************************************************************************************************************************************************/

extern void CNN_PoolReLU(
	char         *Name,

	CNN_GenControl_T *Ctrl,

	int In_DataSize,
	int Out_DataSize,

	int In_InL3,           // 1 if In comes from L3, 0 if it comes from L2
	int Out_InL3,

	int InFeat,
	int OutFeat,
	int Width,
	int Height,

	KernelOper_T PoolOper,
	int Fpx,
	int Fpy,
	int Dpx,
	int Dpy,
	int Spx,
	int Spy,
	int          PoolPad,

	KernelOper_T ReLUOper
	);

/*********************************************************************************************************************************************************************
        Generators for Linear layers followed by an optional linear rectification (ReLU)

        Template:
                Name:           Name of the generated user kernel

                Ctrl:           Overide generator default options (TileOrientation, Parallel Features), Def=(TILE_HOR, 0)

                In_DataSize:    1: byte, 2: half word, 4: word
                Filter_DataSize:1: byte, 2: half word, 4: word
                Bias_DataSize:  1: byte, 2: half word, 4: word
                Out_DataSize:   1: byte, 2: half word, 4: word

                In_InL3:        0: In is in L2, 1: In is in L3 memory
                Filter_InL3:    0: Filter is in L2, 1: Filter is in L3 memory
                Bias_InL3:      0: Bias is in L2, 1: Bias is in L3 memory
                Out_InL3:       0: Out is in L2, 1: Out is in L3 memory

                InDim:          Number of inputs
                OutDim:         Number of outputs

                KernelOper_T    LinearOper      Should always be KOP_LINEAR
                KernelOper_T    ReLUOper        KOP_RELU if ReLU has to be applied after Linear, KOP_NONE otherwise

        CNN_LinearReLU
                Input, Bias and Output are assumed to fit into shared L1 (Buffer In or Out) and Filter is tiled.
        
*********************************************************************************************************************************************************************/

extern void CNN_LinearReLU(
        char *Name,

	CNN_GenControl_T *Ctrl,

        int In_DataSize,
        int Filter_DataSize,
        int Bias_DataSize,
        int Out_DataSize,

        int In_InL3,
        int Filter_InL3,
        int Bias_InL3,
        int Out_InL3,

        int InDim,
        int OutDim,

        KernelOper_T LinearOper,
        KernelOper_T ReLUOper
        );

/*********************************************************************************************************************************************************************
        Generators for SoftMax layers 

        Template:
                Name:           Name of the generated user kernel

                Ctrl:           Overide generator default options (TileOrientation), Def=(TILE_HOR)

                In_DataSize:    1: byte, 2: half word,
                Out_DataSize:   2: half word (Q15 format)

                In_InL3:        0: In is in L2, 1: In is in L3 memory
                Out_InL3:       0: Out is in L2, 1: Out is in L3 memory

                Dim:            Number of inputs

                KernelOper_T    SoftMaxOper     Should always be KOP_SOFTMAX

        CNN_SoftMax
                Input and output are assumed to fit within given shared L1 memory. Dim is partitionned into subsets of inputs and each subset is given to
                a different code. By definition Output contains value is the [0.0 .. 1.0] range with sum(Output)=1.0. Results are always represented in Q15
                if DataSize is half word or in Q7 is DataSize is byte while for Input the point position must be provided (Norm)
        
*********************************************************************************************************************************************************************/

extern void CNN_SoftMax(
        char *Name,

	CNN_GenControl_T *Ctrl,

        int In_DataSize,
        int Out_DataSize,

        int In_InL3,
        int Out_InL3,

        int Dim,
        KernelOper_T SoftMaxOper
        );

/*********************************************************************************************************************************************************************
        Generators for Matrix Addition layers 

        Template:
                Name:           Name of the generated user kernel

                Ctrl:           Overide generator default options (TileOrientation, Parallel Features), Def=(TILE_HOR, 1)

                In1_DataSize:   1: byte, 2: half word,
                In2_DataSize:   1: byte, 2: half word,
                Out_DataSize:   1: byte, 2: half word

                In1_InL3:       0: In is in L2, 1: In is in L3 memory
                In2_InL3:       0: In is in L2, 1: In is in L3 memory
                Out_InL3:       0: Out is in L2, 1: Out is in L3 memory

                InFeat:         Number of input features
                OutFeat:        Number of input features, should always be equal to InFeat
                Width:          Width of a given feature
                Height:         Height of a given feature

                KernelOper_T    AddMatOper      Should always be KOP_MATADD

        CNN_MatAdd
        
*********************************************************************************************************************************************************************/

extern void CNN_MatAdd(
        char *Name,

	CNN_GenControl_T *Ctrl,

        int In1_DataSize,
        int In2_DataSize,
        int Out_DataSize,

        int In1_InL3,
        int In2_InL3,
        int Out_InL3,

        int InFeat,
        int OutFeat,
        int Width,
        int Height,

        KernelOper_T AddMatOper
	);
#endif
