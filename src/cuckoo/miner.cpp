/*
 * Cuckoo Cycle, a memory-hard proof-of-work
 * Copyright (c) 2013-2018 John Tromp
 * Copyright (c) 2017-2018 The Merit Foundation developers
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the The FAIR MINING License and, alternatively,
 * GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.  See src/cuckoo/LICENSE.md for more details.
 **/

#include "miner.h"
#include "cuckoo.h"
#include "mean_cuckoo.h"

#include "consensus/consensus.h"
#include "hash.h"
#include "pow.h"

#include <assert.h>
#include <numeric>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"
#include <vector>

#include <sys/time.h>
#ifndef WIN32
#include <dlfcn.h>
#endif

namespace cuckoo
{

bool VerifyProofOfWorkonlyCycle(
        uint256 hash,
        unsigned int nBits,
        uint8_t edgeBits,
        const std::set<uint32_t>& cycle,
        const Consensus::Params& params)
{

    std::vector<uint32_t> vCycle{cycle.begin(), cycle.end()};

    int res = VerifyCycle(hash, edgeBits, params.nCuckooProofSize, vCycle);

    if (res == verify_code::POW_OK) {
        return true;
    }

    return false;
}

bool VerifyProofOfWork(
        uint256 hash,
        unsigned int nBits,
        uint8_t edgeBits,
        const std::set<uint32_t>& cycle,
        const Consensus::Params& params)
{

    if (cycle.size() != params.nCuckooProofSize) {
//        std::cout << "Fail 1" << std::endl;
        return false;
    }

    if (!params.sEdgeBitsAllowed.count(edgeBits)) {
//        std::cout << "Fail 2" << std::endl;
        return false;
    }

    assert(edgeBits >= MIN_EDGE_BITS && edgeBits <= MAX_EDGE_BITS);

    std::vector<uint32_t> vCycle{cycle.begin(), cycle.end()};

    int res = VerifyCycle(hash, edgeBits, params.nCuckooProofSize, vCycle);

    if (res == verify_code::POW_OK) {
        // check that hash of a cycle is less than a difficulty (old school bitcoin pow)
        return ::CheckProofOfWork(SerializeHash(cycle), nBits, params);
    }

    return false;
}

using Cycle = std::set<uint32_t>;

//#if defined(WIN32) || defined(MAC_OSX)

using Cycles = std::vector<Cycle>;

#ifdef WIN32
//Define the function prototype
typedef bool __cdecl (CALLBACK* FindCyclesOnCudaDeviceType)(
        uint64_t sip_k0, uint64_t sip_k1,
        uint8_t edgebits,
        uint8_t proof_size,
        uint32_t* cycl,
        int device,
	bool* exception,
	char*errormessage
        );

typedef  int __cdecl (CALLBACK* SetupKernelBuffersType)();
#else
#ifdef MAC_OSX
//Define the function prototype
typedef bool __cdecl (* FindCyclesOnCudaDeviceType)(
        uint64_t sip_k0, uint64_t sip_k1,
        uint8_t edgebits,
        uint8_t proof_size,
        uint32_t* cycl,
        int device,
	bool* exception,
	char*errormessage);

typedef  int __cdecl (* SetupKernelBuffersType)();
#else
//Linux
//Define the function prototype
typedef bool (* FindCyclesOnCudaDeviceType)(
        uint64_t sip_k0, uint64_t sip_k1,
        uint8_t edgebits,
        uint8_t proof_size,
        uint32_t* cycl,
        int device,
	bool* exception,
	char*errormessage);

typedef  int (* SetupKernelBuffersType)();

#endif
#endif

   bool dllinited = false;
   bool freeResult, runTimeLinkSuccess = false; 
   FindCyclesOnCudaDeviceType FindCyclesOnCudaDevicePtr = NULL;
   SetupKernelBuffersType SetupKernelBuffersPtr = NULL;
//#endif

#ifdef WIN32
   HINSTANCE dllHandle = NULL;              
void intcudadll()
{
    dllinited = true;
    //Load the dll and keep the handle to it
    dllHandle = LoadLibrary("bitcashcuda.dll");

    // If the handle is valid, try to get the function address. 
    if (NULL != dllHandle) 
    { 
        //Get pointer to our function using GetProcAddress:
        SetupKernelBuffersPtr = (SetupKernelBuffersType)GetProcAddress(dllHandle,
         "SetupKernelBuffers");

        FindCyclesOnCudaDevicePtr = (FindCyclesOnCudaDeviceType)GetProcAddress(dllHandle,
         "FindCyclesOnCudaDevice");
       

        // If the function address is valid, call the function. 
        if (runTimeLinkSuccess = (NULL != FindCyclesOnCudaDevicePtr && NULL!=SetupKernelBuffersPtr))
        {
            SetupKernelBuffersPtr();
        }
    }


    //If unable to call the DLL function, use an alternative. 
    if(!runTimeLinkSuccess)
      LogPrintf("Unable to load bitcashcuda.dll\n");
}

void freecudadll()
{
    dllinited = false;
    if (NULL != dllHandle) 
    {   
       //Free the library:
       freeResult = FreeLibrary(dllHandle);       
    } 
}

#else

#ifdef MAC_OSX

void* dllHandle = NULL;              

void intcudadll()
{
    dllinited = true;
    //Load the dll and keep the handle to it
    dllHandle = dlopen("@executable_path/../Frameworks/bitcashcuda.dylib", RTLD_NOW);

    // If the handle is valid, try to get the function address. 
    if (dllHandle!=NULL) 
    { 
        //Get pointer to our function using GetProcAddress:
        SetupKernelBuffersPtr = (SetupKernelBuffersType)dlsym(dllHandle,
         "SetupKernelBuffers");

        FindCyclesOnCudaDevicePtr = (FindCyclesOnCudaDeviceType)dlsym(dllHandle,
         "FindCyclesOnCudaDevice");
       

        // If the function address is valid, call the function. 
        if ((runTimeLinkSuccess = (NULL != FindCyclesOnCudaDevicePtr && NULL!=SetupKernelBuffersPtr)))
        {
            SetupKernelBuffersPtr();
        }
    }
  /*
    char *errstr;
    errstr = dlerror();
    if (errstr != NULL)
    LogPrintf ("A dynamic linking error occurred: (%s)\n", errstr);else
    LogPrintf ("A dynamic linking NO error occurred\n");
*/

    //If unable to call the DLL function, use an alternative. 
    if(!runTimeLinkSuccess)
      LogPrintf("Unable to load bitcashcuda.dylib\n");
}

void freecudadll()
{
    dllinited = false;
    if (dllHandle!=NULL) 
    {   
        //Free the library:
        freeResult = dlclose(dllHandle);       
    } 
}

#else
//Linux
void* dllHandle = NULL;              

void intcudadll()
{
    dllinited = true;
    //Load the dll and keep the handle to it
    dllHandle = dlopen("/usr/lib/bitcashcuda.so.1.0.0", RTLD_NOW);

    // If the handle is valid, try to get the function address. 
    if (dllHandle!=NULL) 
    { 
        //Get pointer to our function using GetProcAddress:
        SetupKernelBuffersPtr = (SetupKernelBuffersType)dlsym(dllHandle,
         "SetupKernelBuffers");

        FindCyclesOnCudaDevicePtr = (FindCyclesOnCudaDeviceType)dlsym(dllHandle,
         "FindCyclesOnCudaDevice");
       

        // If the function address is valid, call the function. 
        if ((runTimeLinkSuccess = (NULL != FindCyclesOnCudaDevicePtr && NULL!=SetupKernelBuffersPtr)))
        {
            SetupKernelBuffersPtr();
        }
    }
  /*
    char *errstr;
    errstr = dlerror();
    if (errstr != NULL)
    LogPrintf ("A dynamic linking error occurred: (%s)\n", errstr);else
    LogPrintf ("A dynamic linking NO error occurred\n");
*/
    //If unable to call the DLL function, use an alternative. 
    if(!runTimeLinkSuccess)
      LogPrintf("Unable to load /usr/lib/bitcashcuda.so.1.0.0\n");
}

void freecudadll()
{
    dllinited = false;
    if (dllHandle!=NULL) 
    {   
        //Free the library:
        freeResult = dlclose(dllHandle);       
    } 
}


#endif
#endif

bool FindProofOfWorkAdvanced(
    const uint256 hash,
    unsigned int nBits,
    uint8_t edgeBits,
    Cycle& cycle,
    const Consensus::Params& params,
    size_t nThreads,
    bool& cycleFound,
    ctpl::thread_pool* pool,
    bool& trygpumining,
    bool& gpuminingfailed,
    int gpuid,
    bool selectgpucpu,
    bool gpumining
)
{
    assert(cycle.empty());

    if  (selectgpucpu) trygpumining=gpumining;
    if (trygpumining)
    {
        //GPU mining (Nvidia)

        bool exception;
        char errormessage[256];

        if (!dllinited) intcudadll();

        siphash_keys keys;
        Cycles cycles;
        uint32_t cycl[1000];
        auto hashStr = hash.GetHex();    
        setKeys(hashStr.c_str(), hashStr.size(), &keys);

        try {
       
            int _id=gpuid;
            if (runTimeLinkSuccess){
                cycleFound = FindCyclesOnCudaDevicePtr(
                            keys.k0, keys.k1,
                            edgeBits,
                            params.nCuckooProofSize,
                            (uint32_t*)&cycl,
                            _id,&exception,(char*)&errormessage);
                if (exception) {
                    LogPrintf("GPU mining error: %s\n", errormessage);
                    sleep(3000);//sleep 3 seconds, so that we do not get 100% cpu usage when something fails
                    if  (!selectgpucpu) {
                      trygpumining = false;
                      gpuminingfailed = true;
                    }
                }
            } else
            {
                LogPrintf("Could not load bitcashcuda.dll or bitcashcuda.dylib or bitcashcuda.so.1.0.0.\n");
                if  (!selectgpucpu) {
                    trygpumining = false;
                    gpuminingfailed = true;
                }
            }


        }
        catch (const std::exception &exc)
        {	
            if (!selectgpucpu) {
                trygpumining = false;
                gpuminingfailed = true;
            }
            LogPrintf("GPU mining error: %s\n", exc.what());
        }		


        if (cycleFound)
        {
            int i;
            for (i=0;i<42;i++)
            {
                cycle.insert(cycl[i]);
            }
        }

    } 
    if (!trygpumining) {
        //CPU mining
        cycleFound =
            FindCycleAdvanced(hash, edgeBits, params.nCuckooProofSize, cycle, nThreads, pool);
    }
    
    if (cycleFound && ::CheckProofOfWork(SerializeHash(cycle), nBits, params)) {
        return true;
    }

    cycle.clear();
 
    return false;
}

}
