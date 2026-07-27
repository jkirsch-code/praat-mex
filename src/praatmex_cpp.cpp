/*
 * praatmex_cpp.cpp — CPP / CPPs analysis via Praat LPC C API
 *
 * Usage:
 *   r = praatmex('cpp', samples, sr, ...)
 *
 * Parameters (name-value pairs):
 *   'pitchFloor',              Hz (default 60)
 *   'pitchCeiling',            Hz (default 330)
 *   'timeStep',                seconds (default 0.002)
 *   'maximumFrequency',        Hz (default 5000)
 *   'preEmphasisFrom',         Hz (default 50)
 *   'computeCPPs',             true/false (default true) — also compute smoothed CPPs
 *   'timeAveragingWindow',     seconds (default 0.02)
 *   'quefrencyAveragingWindow', seconds (default 0.0005)
 *   'peakInterpolation',       'none','parabolic'(default),'cubic','sinc70','sinc700'
 *   'trendLowerBound',         seconds (default 0.001)
 *   'trendUpperBound',         seconds (default 0.05)
 *   'trendType',               'exponential'(default), 'linear'
 *   'fitMethod',               'robust_slow'(default), 'robust', 'lsq'
 *
 * Returns struct:
 *   .time   — 1×N frame center times
 *   .cpp    — 1×N CPP values
 *   .cpps   — 1×N CPPs values (if computeCPPs=true)
 */

#include "praat.h"
#include "praatmex_helpers.h"
#include "Sound.h"
#include "Sound_to_PowerCepstrogram.h"
#include "PowerCepstrogram.h"
#include "PowerCepstrum.h"
#include <vector>

void praatmex_cpp(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    if (nrhs < 2)
        mexErrMsgIdAndTxt("praatmex:cpp:args",
            "Usage: r = praatmex('cpp', samples, sr, ...)");

    validateInputIsVector(prhs[0], "cpp");
    const double *samples = mxGetPr(prhs[0]);
    size_t nSamples = mxGetNumberOfElements(prhs[0]);
    double fs = mxGetScalar(prhs[1]);

    double pitchFloor = 60.0;
    double pitchCeiling = 330.0;
    double timeStep = 0.002;
    double maximumFrequency = 5000.0;
    double preEmphasisFrom = 50.0;
    bool computeCPPs = true;
    double timeAveragingWindow = 0.02;
    double quefrencyAveragingWindow = 0.0005;
    char peakInterpolationStr[64] = "parabolic";
    double trendLowerBound = 0.001;
    double trendUpperBound = 0.05;
    char trendTypeStr[64] = "exponential";
    char fitMethodStr[64] = "robust_slow";

    static const char *known[] = {"pitchFloor","pitchCeiling","timeStep","maximumFrequency",
        "preEmphasisFrom","computeCPPs","timeAveragingWindow","quefrencyAveragingWindow",
        "peakInterpolation","trendLowerBound","trendUpperBound","trendType","fitMethod"};
    checkUnknownParams(prhs, nrhs, 2, "cpp", known, sizeof(known)/sizeof(known[0]));

    int idx = 2;
    while (idx + 1 < nrhs) {
        const char *name = mxArrayToString(prhs[idx]);
        if (!name) { idx += 2; continue; }
        const mxArray *val = prhs[idx + 1];
        if (strcmp(name, "pitchFloor") == 0) pitchFloor = mxGetScalar(val);
        else if (strcmp(name, "pitchCeiling") == 0) pitchCeiling = mxGetScalar(val);
        else if (strcmp(name, "timeStep") == 0) timeStep = mxGetScalar(val);
        else if (strcmp(name, "maximumFrequency") == 0) maximumFrequency = mxGetScalar(val);
        else if (strcmp(name, "preEmphasisFrom") == 0) preEmphasisFrom = mxGetScalar(val);
        else if (strcmp(name, "computeCPPs") == 0) computeCPPs = mxGetScalar(val) != 0.0;
        else if (strcmp(name, "timeAveragingWindow") == 0) timeAveragingWindow = mxGetScalar(val);
        else if (strcmp(name, "quefrencyAveragingWindow") == 0) quefrencyAveragingWindow = mxGetScalar(val);
        else if (strcmp(name, "peakInterpolation") == 0) mxGetString(val, peakInterpolationStr, sizeof(peakInterpolationStr));
        else if (strcmp(name, "trendLowerBound") == 0) trendLowerBound = mxGetScalar(val);
        else if (strcmp(name, "trendUpperBound") == 0) trendUpperBound = mxGetScalar(val);
        else if (strcmp(name, "trendType") == 0) mxGetString(val, trendTypeStr, sizeof(trendTypeStr));
        else if (strcmp(name, "fitMethod") == 0) mxGetString(val, fitMethodStr, sizeof(fitMethodStr));
        mxFree((void *)name);
        idx += 2;
    }

    /* Map string enums to Praat enums */
    kVector_peakInterpolation peakInterp = kVector_peakInterpolation::PARABOLIC;
    if (strcmp(peakInterpolationStr, "none") == 0) peakInterp = kVector_peakInterpolation::NONE;
    else if (strcmp(peakInterpolationStr, "cubic") == 0) peakInterp = kVector_peakInterpolation::CUBIC;
    else if (strcmp(peakInterpolationStr, "sinc70") == 0) peakInterp = kVector_peakInterpolation::SINC70;
    else if (strcmp(peakInterpolationStr, "sinc700") == 0) peakInterp = kVector_peakInterpolation::SINC700;

    kCepstrum_trendType trendType = kCepstrum_trendType::EXPONENTIAL_DECAY;
    if (strcmp(trendTypeStr, "linear") == 0)
        trendType = kCepstrum_trendType::LINEAR;
    else if (strcmp(trendTypeStr, "exponential") != 0)
        mexWarnMsgIdAndTxt("praatmex:cpp:enum",
            "Unknown trendType '%s', using 'exponential'.", trendTypeStr);

    kCepstrum_trendFit fitMethod = kCepstrum_trendFit::ROBUST_SLOW;
    if (strcmp(fitMethodStr, "robust") == 0) fitMethod = kCepstrum_trendFit::ROBUST_FAST;
    else if (strcmp(fitMethodStr, "lsq") == 0) fitMethod = kCepstrum_trendFit::LEAST_SQUARES;

    /* Create Sound */
    autoSound sound = Sound_createSimple(1, (double)nSamples / fs, fs);
    for (long i = 1; i <= (long)nSamples; i++)
        sound->z[1][i] = samples[i - 1];

    /* Create PowerCepstrogram */
    autoPowerCepstrogram cepstrogram = Sound_to_PowerCepstrogram(
        sound.get(), pitchFloor, timeStep, maximumFrequency, preEmphasisFrom);
    if (!cepstrogram)
        mexErrMsgIdAndTxt("praatmex:cpp:fail", "PowerCepstrogram creation failed.");

    integer nFrames = cepstrogram->nx;

    /* Compute CPP per frame from raw cepstrogram */
    std::vector<double> cppArr(nFrames, 0.0);
    std::vector<double> timeArr(nFrames, 0.0);

    for (integer i = 1; i <= nFrames; i++) {
        double t = cepstrogram->x1 + (i - 1) * cepstrogram->dx;
        timeArr[i - 1] = t;

        autoPowerCepstrum slice = PowerCepstrogram_to_PowerCepstrum_slice(cepstrogram.get(), t);
        if (slice) {
            double qpeak;
            cppArr[i - 1] = PowerCepstrum_getPeakProminence(
                slice.get(), pitchFloor, pitchCeiling, peakInterp,
                trendLowerBound, trendUpperBound, trendType, fitMethod, qpeak);
        }
    }

    /* Compute CPPs per frame from smoothed cepstrogram */
    std::vector<double> cppsArr;
    if (computeCPPs) {
        autoPowerCepstrogram smoothed = PowerCepstrogram_smooth(
            cepstrogram.get(), timeAveragingWindow, quefrencyAveragingWindow);
        if (!smoothed)
            mexErrMsgIdAndTxt("praatmex:cpp:fail", "PowerCepstrogram smoothing failed.");

        cppsArr.resize(nFrames, 0.0);
        for (integer i = 1; i <= nFrames; i++) {
            double t = cepstrogram->x1 + (i - 1) * cepstrogram->dx;
            autoPowerCepstrum slice = PowerCepstrogram_to_PowerCepstrum_slice(smoothed.get(), t);
            if (slice) {
                double qpeak;
                cppsArr[i - 1] = PowerCepstrum_getPeakProminence(
                    slice.get(), pitchFloor, pitchCeiling, peakInterp,
                    trendLowerBound, trendUpperBound, trendType, fitMethod, qpeak);
            }
        }
    }

    /* Build output struct */
    const char *outFields[] = {"time", "cpp", "cpps"};
    int nFields = computeCPPs ? 3 : 2;
    mxArray *out = mxCreateStructMatrix(1, 1, nFields, outFields);

    mxSetFieldByNumber(out, 0, 0, doublesToMxArray(timeArr));
    mxSetFieldByNumber(out, 0, 1, doublesToMxArray(cppArr));

    if (computeCPPs) {
        mxSetFieldByNumber(out, 0, 2, doublesToMxArray(cppsArr));
    }

    plhs[0] = out;
}
