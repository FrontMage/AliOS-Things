/*
 * Copyright 2019 GreenWaves Technologies, SAS
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NETWORK_PROCESS_H
#define NETWORK_PROCESS_H

#include "Gap.h"

#include "CNN_BasicKernels.h"
#include "CnnKernels.h"
#include "dnn_utils.h"

// The function return L2 memory address where input image should be loader
// Expected format: 128x128xshort
extern short* network_init();
extern short* network_process(int* activation_size);

extern void network_load(struct pi_device * fs);
extern void network_free();

#endif
