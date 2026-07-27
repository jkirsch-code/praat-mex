/*
 * melder_carbon_safe.h — Safe inclusion of Carbon/Carbon.h for Praat.
 *
 * Praat's macport_on.h renames Collection (and other Carbon-clashing names)
 * to MacintoshCollection before Carbon.h is processed, then macport_off.h
 * undefines them.  This prevents Carbon's `typedef OpaqueCollection* Collection`
 * from colliding with Praat's own Collection.h.
 *
 * Without this wrapper, a plain `-include Carbon/Carbon.h` causes:
 *   error: typedef redefinition with different types
 *          ('CollectionOf<structThing> *' vs 'struct OpaqueCollection *')
 */

#ifndef _MELDER_CARBON_SAFE_H_
#define _MELDER_CARBON_SAFE_H_

#include "macport_on.h"
#include <Carbon/Carbon.h>
#include "macport_off.h"

/*
 * Stubs for Carbon Event Manager APIs removed from macOS 15+ SDKs.
 * Praat's melder_audio.cpp uses these for Escape key detection during
 * audio playback.  Since we build a headless MEX (NO_GUI), these are
 * harmless no-ops.
 */
#if ! defined(EventAvail)
	static inline bool EventAvail (unsigned short /* mask */, void * /* event */) { return false; }
#endif

#endif /* _MELDER_CARBON_SAFE_H_ */
