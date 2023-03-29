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


void timer_sleep(uint32_t timeMS)
{
    // Timer, sleep
    PerfClock timer;
    PerfClock::Now(&timer, PerfClock::Marker);
    usleep(timeMS * 1000);
    PerfClock::Now(&timer, PerfClock::Elapsed);
    LOG(eWarning, "UNIT_TEST (expected time %lu ms): %s WallClock = %llu, User = %llu, System = %llu\n",
        timeMS, __FUNCTION__,
        timer.GetWallClock(PerfClock::millisecond), timer.GetUserCPU(PerfClock::millisecond), timer.GetSystemCPU(PerfClock::millisecond));

    return;
}

uint64_t do_work(uint32_t timeMS)
{
    uint64_t counter = 0;
    struct timeval timeStamp;
    gettimeofday(&timeStamp, NULL);
    // Convert timestamp to Micro Seconds
    uint64_t inital_time = (uint64_t)(((uint64_t)timeStamp.tv_sec * 1000000) + timeStamp.tv_usec);
    uint64_t elapsed_time = inital_time;
    while(elapsed_time - inital_time < (timeMS * 1000)) {
        counter++;
        gettimeofday(&timeStamp, NULL);
        elapsed_time = (uint64_t)(((uint64_t)timeStamp.tv_sec * 1000000) + timeStamp.tv_usec);
    }

    return counter;
}

void timer_work(uint32_t timeMS)
{
    // Timer, work
    PerfClock timer;
    PerfClock::Now(&timer, PerfClock::Marker);

    uint64_t counter = do_work(timeMS);

    PerfClock::Now(&timer, PerfClock::Elapsed);
    
    uint64_t usCPU = timer.GetUserCPU() + timer.GetSystemCPU();
    double usPerCall = (double)usCPU / (double)counter;

    LOG(eWarning, "UNIT_TEST gettimeofday (expected time %lu ms): %s WallClock = %llu, User = %llu ms, System = %llu, TotalCalls= %llu ms, usPerCall= %0.3lf\n",
        timeMS, __FUNCTION__,
        timer.GetWallClock(PerfClock::millisecond), timer.GetUserCPU(PerfClock::millisecond), timer.GetSystemCPU(PerfClock::millisecond), counter, usPerCall );

    return;
}

uint64_t do_work_getrusage(uint32_t timeMS)
{
    PerfClock timer;
    uint64_t counter = 0;
    struct timeval timeStamp;
    gettimeofday(&timeStamp, NULL);
    // Convert timestamp to Micro Seconds
    uint64_t inital_time = (uint64_t)(((uint64_t)timeStamp.tv_sec * 1000000) + timeStamp.tv_usec);
    uint64_t elapsed_time = inital_time;
    while(elapsed_time - inital_time < (timeMS * 1000)) {
        counter++;
        timer.SetCPU();
        gettimeofday(&timeStamp, NULL);
        elapsed_time = (uint64_t)(((uint64_t)timeStamp.tv_sec * 1000000) + timeStamp.tv_usec);
    }

    return counter;
}

void timer_work_getrusage(uint32_t timeMS)
{
    // Timer, work
    PerfClock timer;
    PerfClock::Now(&timer, PerfClock::Marker);

    uint64_t counter = do_work_getrusage(timeMS);

    PerfClock::Now(&timer, PerfClock::Elapsed);
    
    uint64_t usCPU = timer.GetUserCPU() + timer.GetSystemCPU();
    double usPerCall = (double)usCPU / (double)counter;

    LOG(eWarning, "UNIT_TEST gettimeofday + getrusage (expected time %lu ms): %s WallClock = %llu, User = %llu ms, System = %llu ms, TotalCalls= %llu, usPerCall= %0.3lf\n",
        timeMS, __FUNCTION__,
        timer.GetWallClock(PerfClock::millisecond), timer.GetUserCPU(PerfClock::millisecond), timer.GetSystemCPU(PerfClock::millisecond), counter, usPerCall );

    return;
}

uint64_t do_work_PerfClockElapsed(uint32_t timeMS)
{
    PerfClock timer;
    uint64_t counter = 0;
    struct timeval timeStamp;
    gettimeofday(&timeStamp, NULL);
    // Convert timestamp to Micro Seconds
    uint64_t inital_time = (uint64_t)(((uint64_t)timeStamp.tv_sec * 1000000) + timeStamp.tv_usec);
    uint64_t elapsed_time = inital_time;
    while(elapsed_time - inital_time < (timeMS * 1000)) {
        counter++;
        PerfClock::Now(&timer, PerfClock::Elapsed);
        PerfClock::Now(&timer, PerfClock::Marker);
        gettimeofday(&timeStamp, NULL);
        elapsed_time = (uint64_t)(((uint64_t)timeStamp.tv_sec * 1000000) + timeStamp.tv_usec);
    }

    return counter;
}

void timer_work_PerfClockElapsed(uint32_t timeMS)
{
    // Timer, work
    PerfClock timer;
    PerfClock::Now(&timer, PerfClock::Marker);

    uint64_t counter = do_work_PerfClockElapsed(timeMS);

    PerfClock::Now(&timer, PerfClock::Elapsed);
    
    uint64_t usCPU = timer.GetUserCPU() + timer.GetSystemCPU();
    double usPerCall = (double)usCPU / (double)counter;

    LOG(eWarning, "UNIT_TEST gettimeofday + PerfClock Marker + Elapsed (expected time %lu ms): %s WallClock = %llu, User = %llu ms, System = %llu ms, TotalCalls= %llu, usPerCall= %0.3lf\n",
        timeMS, __FUNCTION__,
        timer.GetWallClock(PerfClock::millisecond), timer.GetUserCPU(PerfClock::millisecond), timer.GetSystemCPU(PerfClock::millisecond), counter, usPerCall );

    return;
}

void record_with_work(uint32_t timeMS)
{
    int idx = 0;
    while(idx < 1) {
        RDKPerf perf (__FUNCTION__);

        usleep((timeMS / 2) * 1000);
        do_work(timeMS / 2);

        idx++;
    }

    return;
}

void record_with_threshold(uint32_t timeMS)
{
    int idx = 0;
    while(idx < 1) {
        RDKPerf perf (__FUNCTION__, timeMS/2);

        do_work(timeMS);

        idx++;
    }

    return;    
}

void benchmark_simple( uint32_t loopsize )
{
    PerfClock clock;

    RDKPerf* a = new RDKPerf("Elapsed Marker");
    for( uint32_t i = 0; i < loopsize; i++)
    {
        PerfClock::Now(&clock, PerfClock::Elapsed);
        PerfClock::Now(&clock, PerfClock::Marker);
    }
    delete a;

    RDKPerf* b = new RDKPerf("SetCPU");
    for( uint32_t i = 0; i < loopsize; i++)
    {
        clock.SetCPU();
    }
    delete b;

    RDKPerf* c = new RDKPerf("ConsDestructor pair");
    PerfClock::Now(&clock, PerfClock::Marker);
    for( uint32_t i = 0; i < loopsize;i++)
    {
        RDKPerf* dummy = new RDKPerf("dummy");
        delete dummy;
    }
    PerfClock::Now(&clock, PerfClock::Elapsed);
    delete c;

    uint64_t usCPU = clock.GetUserCPU() + clock.GetSystemCPU();
    double usPerCall = (double)usCPU / (double)loopsize;

    LOG(eWarning, "ConsDestructor pair usPerCall= %0.3lf\n", usPerCall );
}

// Unit Tests entry point
#define DELAY_SHORT 2 * 1000 // 2s
#define DELAY_LONG 10 * 1000 // 2s

void unit_tests()
{
    LOG(eWarning, "---------------------- Unit Tests START --------------------\n");

    timer_sleep(DELAY_SHORT);

    timer_work(DELAY_SHORT);

    timer_work_getrusage(DELAY_SHORT);

    timer_work_PerfClockElapsed(DELAY_SHORT);

    record_with_work(DELAY_SHORT);

    record_with_threshold(DELAY_SHORT);

    benchmark_simple(1000000);

    RDKPerf_ReportThread(pthread_self());
     
    LOG(eWarning, "---------------------- Unit Tests END --------------------\n");
    return;
}

