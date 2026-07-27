/*
 * macos_carbon_compat.h
 *
 * Stubs for deprecated/removed Carbon types and functions.
 * When building headless (NO_GUI) with macintosh defined, Praat's Gui.h and
 * melder_audio.cpp reference Carbon MLTE and Carbon Event Manager types that
 * have been removed from modern macOS SDK headers (Xcode 26+ / macOS 15+).
 *
 * These types are never used at runtime in a headless MEX build; they exist
 * only to satisfy the compiler for struct members and dead code paths.
 */
#ifndef MACOS_CARBON_COMPAT_H
#define MACOS_CARBON_COMPAT_H

#ifdef __APPLE__

/*
 * Carbon MLTE types (removed from macOS SDK headers).
 * Referenced by Gui.h:1110-1111 in the GuiText struct.
 */
#ifndef TXNObject
typedef void * TXNObject;
#endif

#ifndef TXNFrameID
typedef long TXNFrameID;
#endif

/*
 * Carbon Event Manager types (removed from macOS SDK headers).
 * Referenced by melder_audio.cpp:1173.
 */
#ifndef EventRecord
typedef struct {
	short what;
	unsigned long message;
	unsigned long when;
	struct { short v; short h; } where;
	unsigned short modifiers;
} EventRecord;
#endif

/* Carbon Event Manager constants */
#ifndef keyDownMask
enum {
	keyDownMask   = 0x0008,
	charCodeMask  = 0x00FF,
	cmdKey        = 0x0100
};
#endif

/*
 * Carbon Event Manager functions (declared here, implemented in macos_carbon_compat.cpp).
 * These are only called in the macintosh branch of melder_audio.cpp, which checks
 * for the Escape key during audio playback. In a headless MEX build, audio playback
 * uses PortAudio, so these are safe no-ops.
 */
#ifdef __cplusplus
extern "C" {
#endif

short EventAvail(short mask, EventRecord *event);
void  FlushEvents(short whichMask, short stopMask);

#ifdef __cplusplus
}
#endif

#endif /* __APPLE__ */

#endif /* MACOS_CARBON_COMPAT_H */
