/*
 * macos_carbon_compat.cpp
 *
 * Stub implementations for Carbon Event Manager functions.
 * See macos_carbon_compat.h for details.
 */
#ifdef __APPLE__

#include "macos_carbon_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

short EventAvail(short /*mask*/, EventRecord * /*event*/) {
	return 0;
}

void FlushEvents(short /*whichMask*/, short /*stopMask*/) {
}

#ifdef __cplusplus
}
#endif

#endif /* __APPLE__ */
