/*
 * praatmex_init.cpp — Minimal Praat library initialization.
 *
 * Replaces praat.cpp's praatlib_init() to avoid pulling in GUI symbols.
 * Defines the six global Praat objects and the init function.
 */

#include "praatP.h"
#include <locale.h>

// ---- Six globals from praat.cpp:60-65 ----
structPraatApplication theForegroundPraatApplication;
PraatApplication theCurrentPraatApplication = & theForegroundPraatApplication;
structPraatObjects theForegroundPraatObjects;
PraatObjects theCurrentPraatObjects = & theForegroundPraatObjects;
structPraatPicture theForegroundPraatPicture;
PraatPicture theCurrentPraatPicture = & theForegroundPraatPicture;

// ---- homeDir (static in original praat.cpp) ----
static structMelderFolder homeDir { };

// ---- setThePraatLocale (static in original praat.cpp) ----
static void setThePraatLocale () {
    setlocale (LC_ALL, "C");
}

// ---- installPraatShellPreferences (static in original praat.cpp, NO_GUI path) ----
static void installPraatShellPreferences () {
    praat_statistics_prefs ();
    Melder_audio_prefs ();
    Melder_textEncoding_prefs ();
}

// ---- The init function called by praatmex.cpp ----
#include <mutex>
static std::once_flag praat_inited;

extern "C" void praat_lib_init () {
    std::call_once(praat_inited, []() {
        praatlib_init();
    });
}

// ---- The real init function (replaces praat.cpp) ----
void praatlib_init () {
    setThePraatLocale ();
    Melder_init ();
    Melder_rememberShellDirectory ();
    installPraatShellPreferences ();
    praatP.argc = 0;
    praatP.argv = nullptr;
    praatP.argumentNumber = 1;
    Melder_batch = true;
    praatP.userWantsToOpen = false;
    Melder_setAppName (U"Praatlib");
    theCurrentPraatApplication -> batch = true;
    Melder_getHomeDir (& homeDir);
    Melder_backgrounding = true;
}
