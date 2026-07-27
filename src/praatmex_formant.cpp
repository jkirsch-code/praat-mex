/*
 * praatmex_formant.cpp — Formant analysis via Praat C API
 *
 * Usage:
 *   f = praatmex('formant', samples, sr, ...)
 *
 * Parameters (name-value pairs):
 *   'method',            'burg' (default), 'keepall', 'willems', 'robust'
 *   'timeStep',          seconds (default 0.01)
 *   'maxFormants',       double (default 5.0)
 *   'maxFreq',           Hz (default 5500.0)
 *   'windowLength',      seconds (default 0.025)
 *   'preEmphHz',         Hz (default 50.0)
 *   'safetyMargin',      Hz (default 75.0)
 *   'trackFormants',     true/false (default true) — apply Formant_tracker after estimation
 *   'numberOfTracks',    integer (default 3, 1-5)
 *   'refF1',             Hz (default 550)
 *   'refF2',             Hz (default 1650)
 *   'refF3',             Hz (default 2750)
 *   'refF4',             Hz (default 3850)
 *   'refF5',             Hz (default 4950)
 *   'dfCost',            per kHz (default 1.0)
 *   'bfCost',            double (default 1.0)
 *   'octaveJumpCost',    double (default 1.0)
 *   'nrOfStdDev',        double (default 1.5, robust only)
 *   'maxIterations',     integer (default 5, robust only)
 *   'tolerance',         double (default 1e-6, robust only)
 *
 * Returns struct: freq (nFrames x maxFormants), bandwidth (nFrames x maxFormants), time
 */

#include "praat.h"
#include "praatmex_praat_helpers.h"
#include "Formant.h"
#include "Sound_to_Formant.h"
#include "Sound_to_Formant_mt.h"
#include <cmath>

void praatmex_formant(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    if (nrhs < 2)
        mexErrMsgIdAndTxt("praatmex:formant:args",
            "Usage: f = praatmex('formant', samples, sr, ...)");

    validateInputIsVector(prhs[0], "formant");
    const double *samples = mxGetPr(prhs[0]);
    size_t nSamples = mxGetNumberOfElements(prhs[0]);
    double fs = mxGetScalar(prhs[1]);

    char methodBuf[64] = "burg";
    double timeStep = 0.01;
    double maxFormants = 5.0;
    double maxFreq = 5500.0;
    double windowLength = 0.025;
    double preEmphHz = 50.0;
    double safetyMargin = 75.0;
    /* Tracking params */
    bool trackFormants = true;
    integer numberOfTracks = 3;
    double refF1 = 550.0, refF2 = 1650.0, refF3 = 2750.0, refF4 = 3850.0, refF5 = 4950.0;
    double dfCost = 1.0;
    double bfCost = 1.0;
    double octaveJumpCost = 1.0;
    /* Robust params */
    double nrOfStdDev = 1.5;
    integer maxIterations = 5;
    double tolerance = 1e-6;

    static const char *known[] = {"method","timeStep","maxFormants","maxFreq","windowLength",
        "preEmphHz","safetyMargin","trackFormants","numberOfTracks",
        "refF1","refF2","refF3","refF4","refF5","dfCost","bfCost","octaveJumpCost",
        "nrOfStdDev","maxIterations","tolerance"};
    checkUnknownParams(prhs, nrhs, 2, "formant", known, sizeof(known)/sizeof(known[0]));

    int idx = 2;
    while (idx + 1 < nrhs) {
        const char *name = mxArrayToString(prhs[idx]);
        if (!name) { idx += 2; continue; }
        const mxArray *val = prhs[idx + 1];
        if (strcmp(name, "method") == 0) mxGetString(val, methodBuf, sizeof(methodBuf));
        else if (strcmp(name, "timeStep") == 0) timeStep = mxGetScalar(val);
        else if (strcmp(name, "maxFormants") == 0) maxFormants = mxGetScalar(val);
        else if (strcmp(name, "maxFreq") == 0) maxFreq = mxGetScalar(val);
        else if (strcmp(name, "windowLength") == 0) windowLength = mxGetScalar(val);
        else if (strcmp(name, "preEmphHz") == 0) preEmphHz = mxGetScalar(val);
        else if (strcmp(name, "safetyMargin") == 0) safetyMargin = mxGetScalar(val);
        else if (strcmp(name, "trackFormants") == 0) trackFormants = (mxGetScalar(val) != 0.0);
        else if (strcmp(name, "numberOfTracks") == 0) numberOfTracks = (integer)mxGetScalar(val);
        else if (strcmp(name, "refF1") == 0) refF1 = mxGetScalar(val);
        else if (strcmp(name, "refF2") == 0) refF2 = mxGetScalar(val);
        else if (strcmp(name, "refF3") == 0) refF3 = mxGetScalar(val);
        else if (strcmp(name, "refF4") == 0) refF4 = mxGetScalar(val);
        else if (strcmp(name, "refF5") == 0) refF5 = mxGetScalar(val);
        else if (strcmp(name, "dfCost") == 0) dfCost = mxGetScalar(val);
        else if (strcmp(name, "bfCost") == 0) bfCost = mxGetScalar(val);
        else if (strcmp(name, "octaveJumpCost") == 0) octaveJumpCost = mxGetScalar(val);
        else if (strcmp(name, "nrOfStdDev") == 0) nrOfStdDev = mxGetScalar(val);
        else if (strcmp(name, "maxIterations") == 0) maxIterations = (integer)mxGetScalar(val);
        else if (strcmp(name, "tolerance") == 0) tolerance = mxGetScalar(val);
        mxFree((void *)name);
        idx += 2;
    }

    autoSound sound = createSoundFromSamples(samples, nSamples, fs);

    autoFormant formant;
    if (strcmp(methodBuf, "burg") == 0) {
        formant = Sound_to_Formant_burg(sound.get(), timeStep,
            maxFormants, maxFreq, windowLength, preEmphHz);
    } else if (strcmp(methodBuf, "keepall") == 0) {
        formant = Sound_to_Formant_keepAll(sound.get(), timeStep,
            maxFormants, maxFreq, windowLength, preEmphHz);
    } else if (strcmp(methodBuf, "willems") == 0) {
        formant = Sound_to_Formant_willems(sound.get(), timeStep,
            maxFormants, maxFreq, windowLength, preEmphHz);
    } else if (strcmp(methodBuf, "robust") == 0) {
        formant = Sound_to_Formant_robust(sound.get(), timeStep,
            maxFormants, maxFreq, windowLength, preEmphHz, safetyMargin,
            nrOfStdDev, maxIterations, tolerance, 0.0, false);
    } else {
        mexErrMsgIdAndTxt("praatmex:formant:method",
            "Unknown method '%s'. Use 'burg', 'keepall', 'willems', or 'robust'.", methodBuf);
    }

    if (!formant)
        mexErrMsgIdAndTxt("praatmex:formant:fail", "Formant analysis failed.");

    if (trackFormants) {
        try {
            formant = Formant_tracker(formant.get(), numberOfTracks,
                refF1, refF2, refF3, refF4, refF5,
                dfCost, bfCost, octaveJumpCost);
        } catch (MelderError &) {
            Melder_clearError();
            /* Fall back to untracked formants */
        }
    }

    long nFrames = formant->nx;
    integer nF = (integer)maxFormants;
    std::vector<double> freq(nFrames * nF, 0.0);
    std::vector<double> bw(nFrames * nF, 0.0);
    std::vector<double> time(nFrames, 0.0);

    for (long i = 1; i <= nFrames; i++) {
        double t = formant->x1 + (i - 1) * formant->dx;
        time[i - 1] = t;
        Formant_Frame frame = &formant->frames[i];
        integer nAvail = frame->numberOfFormants;
        for (integer f = 1; f <= nF; f++) {
            if (f <= nAvail) {
                double v = frame->formant[f].frequency;
                double b = frame->formant[f].bandwidth;
                freq[(i - 1) * nF + (f - 1)] = (std::isfinite(v) && v > 0.0) ? v : 0.0;
                bw[(i - 1) * nF + (f - 1)] = (std::isfinite(b) && b > 0.0) ? b : 0.0;
            }
        }
    }

    /* Return freq/bandwidth as nF x nFrames matrices (MATLAB column-major) */
    const char *fields[] = {"freq", "bandwidth", "time"};
    mxArray *out = mxCreateStructMatrix(1, 1, 3, fields);

    mxArray *freqArr = mxCreateDoubleMatrix(nF, nFrames, mxREAL);
    mxArray *bwArr = mxCreateDoubleMatrix(nF, nFrames, mxREAL);
    double *fp = mxGetPr(freqArr);
    double *bp = mxGetPr(bwArr);
    for (integer i = 0; i < nFrames; i++) {
        for (integer f = 0; f < nF; f++) {
            fp[i * nF + f] = freq[i * nF + f];
            bp[i * nF + f] = bw[i * nF + f];
        }
    }

    mxSetFieldByNumber(out, 0, 0, freqArr);
    mxSetFieldByNumber(out, 0, 1, bwArr);
    mxSetFieldByNumber(out, 0, 2, doublesToMxArray(time));
    plhs[0] = out;
}


