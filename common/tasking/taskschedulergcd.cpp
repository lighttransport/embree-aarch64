
#include <cstddef>

#include "taskschedulergcd.h"

#include <algorithm>

namespace embree
{
    size_t TaskScheduler::GCDNumThreads = 1;
    size_t TaskScheduler::currentThreadIndex = 0;

    void TaskScheduler::create(size_t numThreads, bool set_affinity, bool start_threads)
    {
        numThreads = std::min<size_t>(numThreads, (size_t) getNumberOfLogicalThreads());

        GCDNumThreads = ( numThreads > 1)? 4*numThreads : 1;
    }
}
