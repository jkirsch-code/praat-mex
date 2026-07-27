cmake_minimum_required(VERSION 3.16)
project(praatmex C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD 99)

set(PRAAT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/praat)
set(STUBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/stubs)

# ── Build the monolithic praat library ──────────────────────────────────────
add_library(praat STATIC)

# ── Shared definitions ──────────────────────────────────────────────────────
# NO_GUI: disables GUI (gtk/motif/cocoa all set to 0)
# NO_GRAPHICS: implies NO_GUI (extra safety)
# NO_NETWORK: disables network features
# NOMINMAX: prevents windows.h min/max macros
# UNICODE/_UNICODE: use wide-char Win32 APIs (matches conststringW usage)
target_compile_definitions(praat PUBLIC
    NO_GUI NO_GRAPHICS NO_NETWORK NOMINMAX
    UNICODE _UNICODE
)
target_compile_definitions(praat PRIVATE
    _FILE_OFFSET_BITS=64 FPM_64BIT
    __ORDER_BIG_ENDIAN__=4321 __ORDER_LITTLE_ENDIAN__=1234 __BYTE_ORDER__=1234
    NO_OGG_VORBIS
)

# ── Platform-specific definitions and compiler flags ────────────────────────
if (MSVC)
    # Windows-specific: POSIX compat macros, suppress attribute keyword
    target_compile_definitions(praat PRIVATE
        PLATFORM_WINDOWS
        "fseeko=_fseeki64" "ftello=_ftelli64"
        "S_ISDIR(m)=(((m)&0170000)==0040000)"
        "__attribute__(x)="
    )
    # /permissive- enables C++ alternative tokens (and/or/not)
    # /bigobj needed for large translation units (sys/Graphics.cpp)
    # /wd4068 suppress unknown pragma warnings
    # /wd4554 suppress possible operator precedence warning
    target_compile_options(praat PUBLIC /permissive- /EHs /fp:precise /wd4068 /utf-8)
    target_compile_options(praat PRIVATE /wd4554 /bigobj)
elseif(APPLE)
    target_compile_definitions(praat PRIVATE PLATFORM_MACOS)
    target_compile_options(praat PUBLIC -fPIC -Wno-unused-parameter)
    target_link_libraries(praat PRIVATE
        "-framework CoreAudio" "-framework AudioToolbox" "-framework CoreFoundation"
    )
elseif(UNIX)
    target_compile_definitions(praat PRIVATE PLATFORM_LINUX)
    target_compile_options(praat PUBLIC -fPIC -Wno-unused-parameter)
    target_link_libraries(praat PRIVATE pthread dl m)
endif()

# ── Helper: add sources + include dirs to the praat target ──────────────────
function(add_praat_subdir)
    cmake_parse_arguments(ARG "" "" "SOURCES;INCLUDE_DIRS" ${ARGN})
    target_sources(praat PRIVATE ${ARG_SOURCES})
    list(TRANSFORM ARG_INCLUDE_DIRS PREPEND "${PRAAT_SOURCE_DIR}/" OUTPUT_VARIABLE ABSOLUTE_INCLUDE_DIRS)
    target_include_directories(praat PRIVATE ${ABSOLUTE_INCLUDE_DIRS})
endfunction()

# ── Helper: exclude files from a GLOB list ─────────────────────────────────
function(exclude_files LIST_VAR)
    set(_excludes ${ARGN})
    set(_result "")
    foreach(_file IN LISTS ${LIST_VAR})
        get_filename_component(_name "${_file}" NAME)
        set(_skip FALSE)
        foreach(_exc IN LISTS _excludes)
            if("${_name}" STREQUAL "${_exc}")
                set(_skip TRUE)
                break()
            endif()
        endforeach()
        if(NOT _skip)
            list(APPEND _result "${_file}")
        endif()
    endforeach()
    set(${LIST_VAR} "${_result}" PARENT_SCOPE)
endfunction()

# ═══════════════════════════════════════════════════════════════════════════
# EXTERNAL LIBRARIES
# ═══════════════════════════════════════════════════════════════════════════

# ── external/clapack ────────────────────────────────────────────────────────
file(GLOB CLAPACK_SOURCES "${PRAAT_SOURCE_DIR}/external/clapack/*.cpp")
add_praat_subdir(INCLUDE_DIRS melder SOURCES ${CLAPACK_SOURCES})

# ── external/gsl ────────────────────────────────────────────────────────────
# GSL C sources don't compile on MSVC (C99 features). We provide minimal
# implementations of the few GSL special functions actually used by Praat
# (in NUMspecfunc.cpp): gsl_sf_lngamma_e, gsl_sf_beta_inc_e,
# gsl_sf_bessel_In_e, gsl_sf_bessel_Kn_e, gsl_sf_erfc_e,
# gsl_sf_gamma_inc_P_e, gsl_sf_gamma_inc_Q_e, and gsl_set_error_handler_off.
# These are implemented in src/praatmex_gsl_stubs.cpp.
file(GLOB GSL_SOURCES "${PRAAT_SOURCE_DIR}/external/gsl/*.c")
list(REMOVE_ITEM GSL_SOURCES)  # Exclude all GSL sources

# ── external/glpk ───────────────────────────────────────────────────────────
file(GLOB GLPK_SOURCES "${PRAAT_SOURCE_DIR}/external/glpk/*.c")
add_praat_subdir(SOURCES ${GLPK_SOURCES})

# ── external/lame ───────────────────────────────────────────────────────────
file(GLOB LAME_SOURCES "${PRAAT_SOURCE_DIR}/external/lame/*.c")
add_praat_subdir(INCLUDE_DIRS melder SOURCES ${LAME_SOURCES})

# ── external/mp3 ────────────────────────────────────────────────────────────
file(GLOB MP3_SOURCES "${PRAAT_SOURCE_DIR}/external/mp3/*.c" "${PRAAT_SOURCE_DIR}/external/mp3/*.cpp")
# Exclude test.c (includes C++ headers)
list(REMOVE_ITEM MP3_SOURCES "${PRAAT_SOURCE_DIR}/external/mp3/test.c")
add_praat_subdir(INCLUDE_DIRS melder SOURCES ${MP3_SOURCES})

# ── external/flac ───────────────────────────────────────────────────────────
file(GLOB FLAC_SOURCES "${PRAAT_SOURCE_DIR}/external/flac/*.c")
add_praat_subdir(INCLUDE_DIRS melder SOURCES ${FLAC_SOURCES})

# ── external/portaudio ──────────────────────────────────────────────────────
file(GLOB PA_SOURCES "${PRAAT_SOURCE_DIR}/external/portaudio/*.c")
# Exclude platform-specific files not for this platform
if(WIN32)
    file(GLOB PA_OTHER "${PRAAT_SOURCE_DIR}/external/portaudio/pa_linux_*.c"
                      "${PRAAT_SOURCE_DIR}/external/portaudio/pa_mac_*.c"
                      "${PRAAT_SOURCE_DIR}/external/portaudio/pa_unix_*.c"
                      "${PRAAT_SOURCE_DIR}/external/portaudio/pa_jack.c")
elseif(APPLE)
    file(GLOB PA_OTHER "${PRAAT_SOURCE_DIR}/external/portaudio/pa_linux_*.c"
                      "${PRAAT_SOURCE_DIR}/external/portaudio/pa_win_*.c"
                      "${PRAAT_SOURCE_DIR}/external/portaudio/pa_unix_*.c"
                      "${PRAAT_SOURCE_DIR}/external/portaudio/pa_jack.c")
else()
    file(GLOB PA_OTHER "${PRAAT_SOURCE_DIR}/external/portaudio/pa_win_*.c"
                      "${PRAAT_SOURCE_DIR}/external/portaudio/pa_mac_*.c"
                      "${PRAAT_SOURCE_DIR}/external/portaudio/pa_unix_*.c"
                      "${PRAAT_SOURCE_DIR}/external/portaudio/pa_jack.c")
endif()
list(REMOVE_ITEM PA_SOURCES ${PA_OTHER})
add_praat_subdir(SOURCES ${PA_SOURCES})

# ═══════════════════════════════════════════════════════════════════════════
# PRAAT CORE MODULES
# ═══════════════════════════════════════════════════════════════════════════

# ── kar ─────────────────────────────────────────────────────────────────────
file(GLOB KAR_SOURCES "${PRAAT_SOURCE_DIR}/kar/*.cpp")
add_praat_subdir(INCLUDE_DIRS melder SOURCES ${KAR_SOURCES})

# ── melder ──────────────────────────────────────────────────────────────────
file(GLOB MELDER_SOURCES "${PRAAT_SOURCE_DIR}/melder/*.cpp")
add_praat_subdir(
    INCLUDE_DIRS melder
    SOURCES ${MELDER_SOURCES}
)
if(MSVC)
    target_include_directories(praat PRIVATE ${STUBS_DIR})
endif()

# ── sys ─────────────────────────────────────────────────────────────────────
file(GLOB SYS_SOURCES "${PRAAT_SOURCE_DIR}/sys/*.cpp")
# Exclude GUI-only files that lack NO_GUI guards, and praat.cpp
# (replaced by our own praatmex_init.cpp which defines praatlib_init)
exclude_files(SYS_SOURCES
    praat.cpp
    motifEmulator.cpp
    DemoEditor.cpp
    Formula.cpp
    GuiTrust.cpp
    Interpreter.cpp
    Manual.cpp
    ManPages.cpp
    ManPages_toHtml.cpp
    Picture.cpp
    Printer.cpp
    praat_picture.cpp
    SoundRecorder.cpp
    HyperPage.cpp
    InfoEditor.cpp
    Notebook.cpp
    NotebookEditor.cpp
    ScriptEditor.cpp
    DataEditor.cpp
    DataGui.cpp
    ButtonEditor.cpp
    StringsEditor.cpp
    TextEditor.cpp
    Editor.cpp
    Ui.cpp
    UiFile.cpp
    Ui_history.cpp
    praat_objectMenus.cpp
    praat_statistics.cpp
    praat_script.cpp
    praat_actions.cpp
    praat_menuCommands.cpp
    praat_logo.cpp
)
add_praat_subdir(
    INCLUDE_DIRS kar melder sys dwsys
    SOURCES ${SYS_SOURCES}
)

# ── dwsys ───────────────────────────────────────────────────────────────────
file(GLOB DWSYS_SOURCES "${PRAAT_SOURCE_DIR}/dwsys/*.cpp")
add_praat_subdir(
    INCLUDE_DIRS melder stat sys fon external/gsl external/clapack kar dwtools external/num
    SOURCES ${DWSYS_SOURCES}
)

# ── stat ────────────────────────────────────────────────────────────────────
file(GLOB STAT_SOURCES "${PRAAT_SOURCE_DIR}/stat/*.cpp")
exclude_files(STAT_SOURCES manual_statistics.cpp)
add_praat_subdir(
    INCLUDE_DIRS kar melder sys dwsys dwtools fon foned stat
    SOURCES ${STAT_SOURCES}
)

# ── fon ─────────────────────────────────────────────────────────────────────
file(GLOB FON_SOURCES "${PRAAT_SOURCE_DIR}/fon/*.cpp")
# Exclude manual_*.cpp, SpeechRecognizer (needs whisper.h), and all praat_*_init.cpp
# (command registration — not needed for library-only use, pulls in GUI deps)
file(GLOB FON_MANUALS "${PRAAT_SOURCE_DIR}/fon/manual_*.cpp")
file(GLOB FON_INITS "${PRAAT_SOURCE_DIR}/fon/praat_*_init.cpp")
list(REMOVE_ITEM FON_SOURCES ${FON_MANUALS} ${FON_INITS})
exclude_files(FON_SOURCES SpeechRecognizer.cpp Photo.cpp TextGrid_Sound.cpp)
add_praat_subdir(
    INCLUDE_DIRS kar melder sys dwsys stat dwtools LPC fon external/portaudio external/flac external/mp3
    SOURCES ${FON_SOURCES}
)

# ── foned (EXCLUDED: GUI editors — not needed for headless MEX) ─────────────
# file(GLOB FONED_SOURCES "${PRAAT_SOURCE_DIR}/foned/*.cpp")
# exclude_files(FONED_SOURCES FunctionEditor.cpp SoundRecorder.cpp)
# add_praat_subdir(
#     INCLUDE_DIRS kar melder sys dwsys stat dwtools LPC fon external/portaudio external/flac external/mp3
#     SOURCES ${FONED_SOURCES}
# )

# ── dwtools ─────────────────────────────────────────────────────────────────
file(GLOB DWTOOLS_SOURCES "${PRAAT_SOURCE_DIR}/dwtools/*.cpp")
# Exclude SpeechSynthesizer (needs espeak_ng.h) and manual_*.cpp
file(GLOB DWTOOLS_MANUALS "${PRAAT_SOURCE_DIR}/dwtools/manual_*.cpp")
list(REMOVE_ITEM DWTOOLS_SOURCES ${DWTOOLS_MANUALS})
exclude_files(DWTOOLS_SOURCES SpeechSynthesizer.cpp Sound_and_TextGrid_extensions.cpp SpeechSynthesizer_and_TextGrid.cpp)
add_praat_subdir(
    INCLUDE_DIRS dwtools kar melder LPC fon foned sys stat dwsys external/portaudio external/clapack EEG
    SOURCES ${DWTOOLS_SOURCES}
)

# ── LPC ─────────────────────────────────────────────────────────────────────
file(GLOB LPC_SOURCES "${PRAAT_SOURCE_DIR}/LPC/*.cpp")
file(GLOB LPC_MANUALS "${PRAAT_SOURCE_DIR}/LPC/manual_*.cpp")
list(REMOVE_ITEM LPC_SOURCES ${LPC_MANUALS})
add_praat_subdir(
    INCLUDE_DIRS LPC kar melder fon foned dwtools sys dwsys stat
    SOURCES ${LPC_SOURCES}
)

# ── gram ────────────────────────────────────────────────────────────────────
file(GLOB GRAM_SOURCES "${PRAAT_SOURCE_DIR}/gram/*.cpp")
file(GLOB GRAM_MANUALS "${PRAAT_SOURCE_DIR}/gram/manual_*.cpp")
list(REMOVE_ITEM GRAM_SOURCES ${GRAM_MANUALS})
add_praat_subdir(
    INCLUDE_DIRS kar melder sys dwsys stat dwtools fon foned
    SOURCES ${GRAM_SOURCES}
)

# ── FFNet ───────────────────────────────────────────────────────────────────
file(GLOB FFNET_SOURCES "${PRAAT_SOURCE_DIR}/FFNet/*.cpp")
file(GLOB FFNET_MANUALS "${PRAAT_SOURCE_DIR}/FFNet/manual_*.cpp")
list(REMOVE_ITEM FFNET_SOURCES ${FFNET_MANUALS})
add_praat_subdir(
    INCLUDE_DIRS kar melder sys dwtools fon dwsys stat gram
    SOURCES ${FFNET_SOURCES}
)

# ── artsynth ────────────────────────────────────────────────────────────────
file(GLOB ARTSYNTH_SOURCES "${PRAAT_SOURCE_DIR}/artsynth/*.cpp")
file(GLOB ARTSYNTH_MANUALS "${PRAAT_SOURCE_DIR}/artsynth/manual_*.cpp")
list(REMOVE_ITEM ARTSYNTH_SOURCES ${ARTSYNTH_MANUALS})
add_praat_subdir(
    INCLUDE_DIRS kar melder sys fon stat
    SOURCES ${ARTSYNTH_SOURCES}
)

# ── EEG ─────────────────────────────────────────────────────────────────────
file(GLOB EEG_SOURCES "${PRAAT_SOURCE_DIR}/EEG/*.cpp")
file(GLOB EEG_MANUALS "${PRAAT_SOURCE_DIR}/EEG/manual_*.cpp")
list(REMOVE_ITEM EEG_SOURCES ${EEG_MANUALS})
add_praat_subdir(
    INCLUDE_DIRS kar melder sys dwsys stat dwtools fon foned
    SOURCES ${EEG_SOURCES}
)

# ── sensors ─────────────────────────────────────────────────────────────────
file(GLOB SENSORS_SOURCES "${PRAAT_SOURCE_DIR}/sensors/*.cpp")
add_praat_subdir(
    INCLUDE_DIRS melder stat sys fon external/gsl external/clapack kar dwtools
    SOURCES ${SENSORS_SOURCES}
)

# ═══════════════════════════════════════════════════════════════════════════
# MEX FILES
# ═══════════════════════════════════════════════════════════════════════════

find_package(Matlab REQUIRED)

file(GLOB PRAATEX_MEX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM PRAATEX_MEX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/praatmex.cpp")

add_library(praatmex_modules STATIC ${PRAATEX_MEX_SOURCES})

target_include_directories(praatmex_modules PRIVATE
    src
    ${Matlab_INCLUDE_DIRS}
    ${PRAAT_SOURCE_DIR}/kar
    ${PRAAT_SOURCE_DIR}/melder
    ${PRAAT_SOURCE_DIR}/sys
    ${PRAAT_SOURCE_DIR}/dwsys
    ${PRAAT_SOURCE_DIR}/stat
    ${PRAAT_SOURCE_DIR}/fon
    ${PRAAT_SOURCE_DIR}/dwtools
    ${PRAAT_SOURCE_DIR}/LPC
)

target_link_libraries(praatmex_modules PRIVATE praat)

Matlab_add_mex(NAME praatmex LINK_TO praatmex_modules
    SRC src/praatmex.cpp
)
target_include_directories(praatmex PRIVATE
    src
    ${Matlab_INCLUDE_DIRS}
    ${PRAAT_SOURCE_DIR}/kar
    ${PRAAT_SOURCE_DIR}/melder
    ${PRAAT_SOURCE_DIR}/sys
    ${PRAAT_SOURCE_DIR}/dwsys
    ${PRAAT_SOURCE_DIR}/stat
    ${PRAAT_SOURCE_DIR}/fon
    ${PRAAT_SOURCE_DIR}/dwtools
    ${PRAAT_SOURCE_DIR}/LPC
)
