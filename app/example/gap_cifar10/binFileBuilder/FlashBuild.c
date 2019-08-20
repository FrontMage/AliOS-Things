#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "CifarCoeff.h"

void writeBinFile(char *binFileName, void* Buffer, unsigned int Size)
{
        FILE *Fi = fopen(binFileName, "w");
        fwrite(Buffer, 1, Size, Fi);
        fclose(Fi);
}

int main()

{
	writeBinFile("Cifar10_Filter0.dat", (void *) Filter_Layer0, sizeof(Filter_Layer0));
	writeBinFile("Cifar10_Bias0.dat", (void *) Bias_Layer0, sizeof(Bias_Layer0));

	writeBinFile("Cifar10_Filter1.dat", (void *) Filter_Layer1, sizeof(Filter_Layer1));
	writeBinFile("Cifar10_Bias1.dat", (void *) Bias_Layer1, sizeof(Bias_Layer1));

	writeBinFile("Cifar10_Filter2.dat", (void *) Filter_Layer2, sizeof(Filter_Layer2));
	writeBinFile("Cifar10_Bias2.dat", (void *) Bias_Layer2, sizeof(Bias_Layer2));
    return 0;
}

