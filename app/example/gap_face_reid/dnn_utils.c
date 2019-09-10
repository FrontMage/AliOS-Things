#include "pmsis.h"
#include "Gap.h"

#include "param_layer_struct.h"
#include "dnn_utils.h"

short memory_pool[MEMORY_POOL_SIZE];

int loadLayerFromFsToL2(struct pi_device *fs, const char* file_name, void* buffer, int size)
{
    PRINTF("Loading layer \"%s\" from FS to L2\n", file_name);
    fs_file_t * file = fs_open(fs, file_name, 0);
    if (file == NULL)
    {
        PRINTF("file open failed\n");
        return 0;
    }

    if((int)file->size > size)
    {
        PRINTF("Provided buffer size %d is smaller than file size %d\n", size, (int) file->size);
        return -1;
    }

    pi_task_t task;
    int size_read = fs_read_async(file, buffer, file->size, pi_task_block(&task));
    pi_task_wait_on(&task);
    printf("Read %d bytes from %s\n", size_read, file_name);

    fs_close(file);

    return size_read;
}

void* loadLayerFromFsToL3(struct pi_device *fs, const char* file_name, struct pi_device *hyper, int* layer_size)
{
    signed char* buff = (signed char*)memory_pool;
    PRINTF("Loading layer \"%s\" from FS to L3\n", file_name);

    fs_file_t * file = fs_open(fs, file_name, 0);
    if (file == NULL)
    {
        PRINTF("file open failed\n");
        return NULL;
    }
    // PRINTF("file open success, size: %d\n", file->size);
    uint32_t hyper_buff;
    ram_alloc(hyper, &hyper_buff, file->size);
    //PRINTF("hyper buffer allocated\n");
    if(hyper_buff == 0)
    {
        PRINTF("HyperRAM allocation failed\n");
        return NULL;
    }

    unsigned int size_total = 0;
    unsigned int size = 0;
    pi_task_t task;
    do
    {
        //PRINTF("Readning data to local bufer\n");
        size = fs_read_async(file, buff, IO_BUFF_SIZE, pi_task_block(&task));
        pi_task_wait_on(&task);
        //PRINTF("Read %d bytes from %s\n", size, file_name);
        size = ((size + 3) & ~3);
        if(size)
        {
            //PRINTF("Writing data to L3\n");
            ram_write(hyper, (uint32_t)(hyper_buff+size_total), buff, size);
            // PRINTF("Writing data to L3 done\n");
        }
        size_total += size;
    } while(size_total < file->size);

    //PRINTF("total read size: %d\n", size_total);

    fs_close(file);

    *layer_size = size_total;

    return (void *)hyper_buff;
}

void loadLayerFromL3ToL2(struct pi_device *hyper, void* hyper_buff, void* base_addr, int layer_size)
{
    cl_ram_req_t req;
    //printf("hyper_buff address: %p\n", hyper_buff);
    //printf("base_addr: %p, size %d\n", base_addr, layer_size);
    cl_ram_read(hyper, (uint32_t)hyper_buff, base_addr, layer_size, &req);
    //printf("after pi_cl_hyper_read\n");
    cl_ram_read_wait(&req);
    //printf("after pi_cl_hyper_read_wait\n");
}

int get_activations_size(int idx)
{
    int out_width = convLayers[idx].win;
    int out_height = convLayers[idx].hin;

    if(!convLayers[idx].conv_padding)
    {
        out_width = out_width - convLayers[idx].kernel_width + 1;
        out_height = out_height - convLayers[idx].kernel_height + 1;
    }

    out_width = out_width / convLayers[idx].conv_stride;
    out_height = out_height / convLayers[idx].conv_stride;

    // see output size formulae at https://pytorch.org/docs/0.4.0/nn.html#torch.nn.MaxPool2d
    // dilation = 1, padding = 0
    if(convLayers[idx].max_pool)
    {
        out_width = (1.f*(out_width-(convLayers[idx].pool_size-1) - 1)) / convLayers[idx].pool_stride + 1;
        out_height = (1.f*(out_height-(convLayers[idx].pool_size-1) - 1)) / convLayers[idx].pool_stride + 1;
    }

    int activation_size = convLayers[idx].nb_of * out_height * out_width;

//     PRINTF("Output size for layer %d: %dx%d\n", idx, out_width, out_height);
//     PRINTF("activation_size %d: %d\n", idx, activation_size);

    return activation_size;
}
