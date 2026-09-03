#ifndef PROBES_H
#define PROBES_H

#ifdef ENABLE_USDT_PROBES

#include "atlas_probes.h"

#else

#define ATLAS_TASK_ENQUEUE_ENABLED() 0
#define ATLAS_TASK_ENQUEUE(arg1) \
	do { \
	} while (0)

#define ATLAS_TASK_DEQUEUE_ENABLED() 0
#define ATLAS_TASK_DEQUEUE(arg1) \
	do { \
	} while (0)

#define ATLAS_TASK_EXECUTION_START_ENABLED() 0
#define ATLAS_TASK_EXECUTION_START(arg1, arg2, arg3) \
	do { \
	} while (0)

#define ATLAS_TASK_EXECUTION_END_ENABLED() 0
#define ATLAS_TASK_EXECUTION_END(arg1, arg2, arg3) \
	do { \
	} while (0)

#endif

#endif
