/**
* Copyright 2021 Comcast Cable Communications Management, LLC
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* SPDX-License-Identifier: Apache-2.0
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#include "rdk_perf.h"
#include "rdk_perf_logging.h"
#include "rdk_perf_msgqueue.h"

// Uint Tests prototype
void unit_tests();
void unit_tests_c();

static int32_t s_systemMaxMsg = 0;

void Func4()
{
    RDKPerf perf(__FUNCTION__);
}

#define SLEEP_US_PER_MESSAGE 2

void MeasureRDKperf()
{
    uint64_t totalTime_us = 0;
    uint64_t totalCalls = 0;

    uint32_t loopTimes = ( 1000000 / ( s_systemMaxMsg / 2 ) );

    for(uint32_t nIdx = 0; nIdx < loopTimes; nIdx++) {

        uint64_t startTime_us = PerfRecord::TimeStamp();

        // /2 because Entry+Exit
        // -2 because the Func3 Entry+Exit
        uint32_t nCount = ( s_systemMaxMsg / 2 );

        totalCalls += nCount;

        while(nCount > 0) {
            nCount--;
            Func4();
        }

        totalTime_us += ( PerfRecord::TimeStamp() - startTime_us );

        // Sleep so the queue can be handled
        usleep( s_systemMaxMsg * SLEEP_US_PER_MESSAGE );
    }

    LOG(eWarning, "MeasureRDKperf, QueueDepth: %u, TotalRDKperfCalls: %lu, us/call: %1.2lf\n",
                    s_systemMaxMsg,
                    totalCalls,
                    (double)totalTime_us / (double)totalCalls );
}

void Func3(uint32_t nCount)
{
    RDKPerf perf(__FUNCTION__);

    // /2 because Entry+Exit
    // -2 because the Func3 Entry+Exit
    nCount = ( s_systemMaxMsg / 2 );
    while(nCount > 0) {
        nCount--;
        Func4();
    }
}

void Func2()
{
    RDKPerf perf(__FUNCTION__);
    int loopTimes = ( 1000000 / ( s_systemMaxMsg / 2 ) );
    for(int nIdx = 0; nIdx < loopTimes; nIdx++) {
        Func3(nIdx);
        usleep(s_systemMaxMsg * SLEEP_US_PER_MESSAGE);
    }
}

void Func1()
{
    // RDKPerfRemote perfRemote(__FUNCTION__);
    RDKPerf pref(__FUNCTION__);
    sleep(2);
    Func2();
}

//#define MAX_LOOP 1024 * 1024 * 1
#define MAX_LOOP 1
void* task1(void* pData)
{
    pthread_setname_np(pthread_self(), __FUNCTION__);
    RDKPerf perf(__FUNCTION__);
    Func2();
/*
    sleep(4);

    RDKPerfHandle hPerf = RDKPerfStart("Func3_Wrapper");
    int nCount = 0;
    while(nCount < MAX_LOOP) {
        nCount = Func3(nCount);   
    }
    RDKPerfStop(hPerf);*/
    return NULL;
}

void* task2(void* pData)
{
    pthread_setname_np(pthread_self(), __FUNCTION__);
    RDKPerf perf(__FUNCTION__);
    Func1();
    Func2();

    RDKPerfHandle hPerf = RDKPerfStart("test_c");
    sleep(1);
    RDKPerfStop(hPerf);
    
    return NULL;
}

void test_inline()
{
    uint32_t sleep_interval = 5;

    FUNC_METRICS_START(100);

    while(sleep_interval != 0) {
        sleep_interval--;
    }

    FUNC_METRICS_END();

    return;
}

static int GetSystemMaxMsg(void)
{
    static const char* msgmax_filename = "/proc/sys/fs/mqueue/msg_max";

    FILE* fp = fopen(msgmax_filename, "r");

    if(fp == NULL) {
        LOG(eError, "Can't open \"%s\"\n", msgmax_filename);
        return 0;
    }

    int msg_max = 0;
    char buffer[12];

    if(fgets(buffer, 12, fp) != NULL) {
        msg_max = atoi(buffer);
    }else{
        LOG(eError, "Can't parse content of \"%s\"\n", msgmax_filename);
    }

    return msg_max;
}


int main(int argc, char *argv[])
{    
    LOG(eWarning, "Enter test app %s\n", __DATE__);

    pid_t child_pid;

    s_systemMaxMsg = GetSystemMaxMsg();

#ifdef PERF_REMOTE
    // child_pid = fork();
    // if(child_pid == 0) {
    //     /* This is done by the child process. */

    //     const char* command     = "./build/perfservice";
    //     const char* args[]      = { "./build/perfservice", NULL };
    //     const char* env[]       = { "LD_LIBRARY_PATH=./build", NULL };

    //     execvpe(command, args, env);

    //     /* If execv returns, it must have failed. */

    //     printf("Unknown command %s\n", command);
    //     exit(0);
    // }
    // sleep(1);
#endif
    // Perform Unit tests
    //unit_tests();
    //unit_tests_c();

    MeasureRDKperf();
    return;

#ifdef DO_THREAD_TESTS
    pthread_t threadId1;
    pthread_t threadId2;

    LOG(eWarning, "Creating Test threads\n");

    pthread_create(&threadId1, NULL, &task1, NULL);
    //pthread_create(&threadId2, NULL, &task2, NULL);
 
    pthread_join(threadId1, NULL);
    //pthread_join(threadId2, NULL);
#endif

#ifdef DO_INLINE_TESTS
    for(int idx = 0; idx < 1000; idx++) {
        test_inline();
    }
#endif
    // Don't need to make this call as the process terminate handler will 
    // call the RDKPerf_ReportProcess() function
    // RDKPerf_ReportProcess(getpid());
}

