/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <dlfcn.h>
#include <assert.h>

#include <include/NeuralNetworks.h>

int (*ANeuralNetworksInitialize)();
void (*ANeuralNetworksShutdown)();
int (*ANeuralNetworksMemory_createShared)(size_t size, ANeuralNetworksMemory** memory);

int (*ANeuralNetworksMemory_getPointer)(ANeuralNetworksMemory* memory, uint8_t** buffer);
void (*ANeuralNetworksMemory_free)(ANeuralNetworksMemory* memory);
int (*ANeuralNetworksModel_create)(ANeuralNetworksModel** model);
void (*ANeuralNetworksModel_free)(ANeuralNetworksModel* model);
int (*ANeuralNetworksModel_addOperand)(ANeuralNetworksModel* model,
                                    const ANeuralNetworksOperandType* type);
int (*ANeuralNetworksModel_setOperandValue)(ANeuralNetworksModel* model, int32_t index,
                                         const void* buffer, size_t length);
int (*ANeuralNetworksModel_setOperandValueFromMemory)(ANeuralNetworksModel* model, int32_t index,
                                                   const ANeuralNetworksMemory* memory,
                                                   uint32_t offset, size_t length);
int (*ANeuralNetworksModel_addOperation)(ANeuralNetworksModel* model,
                                      ANeuralNetworksOperationType type,
                                      ANeuralNetworksIntList* inputs,
                                      ANeuralNetworksIntList* outputs);
int (*ANeuralNetworksModel_setInputsAndOutputs)(ANeuralNetworksModel* model,
                                             ANeuralNetworksIntList* inputs,
                                             ANeuralNetworksIntList* outputs);
int (*ANeuralNetworksRequest_create)(ANeuralNetworksModel* model, ANeuralNetworksRequest** request);
void (*ANeuralNetworksRequest_free)(ANeuralNetworksRequest* request);
int (*ANeuralNetworksRequest_setPreference)(ANeuralNetworksRequest* request, uint32_t preference);
int (*ANeuralNetworksRequest_setInput)(ANeuralNetworksRequest* request, int32_t index,
                                    const ANeuralNetworksOperandType* type, const void* buffer,
                                    size_t length);
int (*ANeuralNetworksRequest_setInputFromMemory)(ANeuralNetworksRequest* request, int32_t index,
                                              const ANeuralNetworksOperandType* type,
                                              const ANeuralNetworksMemory* memory, uint32_t offset,
                                              uint32_t length);
int (*ANeuralNetworksRequest_setOutput)(ANeuralNetworksRequest* request, int32_t index,
                                     const ANeuralNetworksOperandType* type, void* buffer,
                                     size_t length);
int (*ANeuralNetworksRequest_setOutputFromMemory)(ANeuralNetworksRequest* request, int32_t index,
                                               const ANeuralNetworksOperandType* type,
                                               const ANeuralNetworksMemory* memory, uint32_t offset,
                                               uint32_t length);
int (*ANeuralNetworksRequest_startCompute)(ANeuralNetworksRequest* request,
                                        ANeuralNetworksEvent** event);
int (*ANeuralNetworksEvent_wait)(ANeuralNetworksEvent* event);

void (*ANeuralNetworksEvent_free)(ANeuralNetworksEvent* event);

int32_t initNNAPI(void) {
    static int32_t initCompleted = 0;
    static int32_t initStatus = -1;

    if (initCompleted) {
        return initStatus;
    }
    void *handle = dlopen("libneuralnetworks.so", RTLD_NOW);
    assert(handle);

#   define GET_PROC(s) s=dlsym(handle,#s)
    GET_PROC(ANeuralNetworksInitialize);
    GET_PROC(ANeuralNetworksShutdown);
    GET_PROC(ANeuralNetworksMemory_createShared);

    GET_PROC(ANeuralNetworksMemory_getPointer);
    GET_PROC(ANeuralNetworksMemory_free);
    GET_PROC(ANeuralNetworksModel_create);
    GET_PROC(ANeuralNetworksModel_free);
    GET_PROC(ANeuralNetworksModel_addOperand);
    GET_PROC(ANeuralNetworksModel_setOperandValue);
    GET_PROC(ANeuralNetworksModel_setOperandValueFromMemory);
    GET_PROC(ANeuralNetworksModel_addOperation);
    GET_PROC(ANeuralNetworksModel_setInputsAndOutputs);
    GET_PROC(ANeuralNetworksRequest_create);
    GET_PROC(ANeuralNetworksRequest_free);
    GET_PROC(ANeuralNetworksRequest_setPreference);
    GET_PROC(ANeuralNetworksRequest_setInput);
    GET_PROC(ANeuralNetworksRequest_setInputFromMemory);
    GET_PROC(ANeuralNetworksRequest_setOutput);
    GET_PROC(ANeuralNetworksRequest_setOutputFromMemory);
    GET_PROC(ANeuralNetworksRequest_startCompute);

    // event itself could only pass to app from Request::stateCompute()
    GET_PROC(ANeuralNetworksEvent_wait);
    GET_PROC(ANeuralNetworksEvent_free);

#   undef GET_PROC

    initCompleted = 1;
    if (ANeuralNetworksInitialize &&
        ANeuralNetworksShutdown  &&
        ANeuralNetworksMemory_createShared &&
        ANeuralNetworksMemory_getPointer &&
        ANeuralNetworksMemory_free &&
        ANeuralNetworksModel_create &&
        ANeuralNetworksModel_free &&
        ANeuralNetworksModel_addOperand &&
        ANeuralNetworksModel_setOperandValue &&
        ANeuralNetworksModel_setOperandValueFromMemory &&
        ANeuralNetworksModel_addOperation &&
        ANeuralNetworksModel_setInputsAndOutputs &&
        ANeuralNetworksRequest_create &&
        ANeuralNetworksRequest_free &&
        ANeuralNetworksRequest_setPreference &&
        ANeuralNetworksRequest_setInput &&
        ANeuralNetworksRequest_setInputFromMemory &&
        ANeuralNetworksRequest_setOutput &&
        ANeuralNetworksRequest_setOutputFromMemory &&
        ANeuralNetworksRequest_startCompute &&
        ANeuralNetworksEvent_wait &&
        ANeuralNetworksEvent_free) {
        initStatus = 0;
    }

    return initStatus;
}
