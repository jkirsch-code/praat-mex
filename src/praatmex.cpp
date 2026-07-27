/*
 * praatmex.cpp — MEX dispatcher
 *
 * Usage:
 *   [out] = praatmex(command, ...)
 *
 * Commands:  init, pitch, formant, intensity, harmonicity,
 *            jitter, shimmer, mfcc, spectrum, pointprocess, textgrid
 */

#include "mex.h"
#include "praat.h"
#include <cstring>
#include <vector>

extern "C" void praat_lib_init();

/* ── Sub-command forward declarations ─────────────────────────────────────── */
void praatmex_pitch(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs);
void praatmex_formant(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs);
void praatmex_intensity(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs);
void praatmex_harmonicity(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs);
void praatmex_jitter(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs);
void praatmex_shimmer(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs);
void praatmex_mfcc(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs);
void praatmex_spectrum(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs);
void praatmex_pointprocess(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs);
void praatmex_cpp(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs);
void praatmex_textgrid(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs);

/* ── Global error capture buffer ──────────────────────────────────────────── */
static char32 theCapturedError[4096];
static bool theHasCapturedError = false;

static void capturingErrorProc(conststring32 message) {
    if (message) {
        str32cpy(theCapturedError, message);
        theHasCapturedError = true;
    }
}

/* ── MEX entry point ─────────────────────────────────────────────────────── */
void mexFunction(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    try {
        if (nrhs < 1)
            mexErrMsgIdAndTxt("praatmex:nrhs", "First argument must be a command string.");

        praat_lib_init();

        /* Install our error capture handler so Praat errors don't go to stderr */
        Melder_setErrorProc(capturingErrorProc);
        theHasCapturedError = false;
        theCapturedError[0] = U'\0';

        char cmd[64];
        mxGetString(prhs[0], cmd, sizeof(cmd));

        /* Shift args by one so sub-commands see argv[1] as the first real argument */
        nrhs--;
        prhs++;

        if (strcmp(cmd, "init") == 0) {
            /* Already initialized above — no-op success */
            if (nlhs >= 1) {
                plhs[0] = mxCreateDoubleScalar(1.0);
            }
            return;
        }
        else if (strcmp(cmd, "pitch") == 0)
            praatmex_pitch(nlhs, plhs, nrhs, prhs);
        else if (strcmp(cmd, "formant") == 0)
            praatmex_formant(nlhs, plhs, nrhs, prhs);
        else if (strcmp(cmd, "intensity") == 0)
            praatmex_intensity(nlhs, plhs, nrhs, prhs);
        else if (strcmp(cmd, "harmonicity") == 0)
            praatmex_harmonicity(nlhs, plhs, nrhs, prhs);
        else if (strcmp(cmd, "jitter") == 0)
            praatmex_jitter(nlhs, plhs, nrhs, prhs);
        else if (strcmp(cmd, "shimmer") == 0)
            praatmex_shimmer(nlhs, plhs, nrhs, prhs);
        else if (strcmp(cmd, "mfcc") == 0)
            praatmex_mfcc(nlhs, plhs, nrhs, prhs);
        else if (strcmp(cmd, "spectrum") == 0)
            praatmex_spectrum(nlhs, plhs, nrhs, prhs);
        else if (strcmp(cmd, "pointprocess") == 0)
            praatmex_pointprocess(nlhs, plhs, nrhs, prhs);
        else if (strcmp(cmd, "cpp") == 0)
            praatmex_cpp(nlhs, plhs, nrhs, prhs);
        else if (strcmp(cmd, "textgrid") == 0)
            praatmex_textgrid(nlhs, plhs, nrhs, prhs);
        else
            mexErrMsgIdAndTxt("praatmex:unknown", "Unknown command: %s", cmd);
    } catch (MelderError &e) {
        /* Copy the error message BEFORE clearing the buffer (pointer becomes invalid) */
        static char32 theMsgBuf[4096];
        theMsgBuf[0] = U'\0';

        conststring32 msg = Melder_getError();
        if (msg && msg[0]) {
            str32cpy(theMsgBuf, msg);
        }
        Melder_clearError();

        /* Fallback: use our captured buffer if Melder_getError was already flushed */
        const char32_t *finalMsg = theMsgBuf;
        if (!finalMsg[0] && theHasCapturedError && theCapturedError[0]) {
            finalMsg = theCapturedError;
        }
        if (!finalMsg[0]) {
            finalMsg = U"Praat error (no message available)";
        }

        /* Convert char32_t to UTF-8 for MATLAB */
        size_t len = 0;
        while (finalMsg[len]) len++;
        std::vector<char> buf;
        buf.reserve(len * 4 + 1);
        for (size_t i = 0; i < len; i++) {
            char32_t c = finalMsg[i];
            if (c < 0x80) {
                buf.push_back((char)c);
            } else if (c < 0x800) {
                buf.push_back((char)(0xC0 | (c >> 6)));
                buf.push_back((char)(0x80 | (c & 0x3F)));
            } else if (c < 0x10000) {
                buf.push_back((char)(0xE0 | (c >> 12)));
                buf.push_back((char)(0x80 | ((c >> 6) & 0x3F)));
                buf.push_back((char)(0x80 | (c & 0x3F)));
            } else {
                buf.push_back((char)(0xF0 | (c >> 18)));
                buf.push_back((char)(0x80 | ((c >> 12) & 0x3F)));
                buf.push_back((char)(0x80 | ((c >> 6) & 0x3F)));
                buf.push_back((char)(0x80 | (c & 0x3F)));
            }
        }
        buf.push_back(0);
        mexErrMsgIdAndTxt("praatmex:praat_error", "%s", buf.data());
    } catch (std::exception &e) {
        const char *msg = e.what();
        /* Skip "C++ error: " prefix if present (from mexErrMsgIdAndTxt re-throw) */
        if (strncmp(msg, "C++ error: ", 11) == 0) msg += 11;
        mexErrMsgIdAndTxt("praatmex:cpp_error", "%s", msg);
    } catch (...) {
        mexErrMsgIdAndTxt("praatmex:unknown_error", "Unknown C++ exception");
    }
}
