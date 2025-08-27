#include <windows.h>
#include <psapi.h>
#include "mem_api.h"

size_t getCurrentMemSize(){
    PROCESS_MEMORY_COUNTERS pmc;
    // store initial mem size
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }else{
        return 0;
    }
}
