/*
 * praatmex_shimmer.cpp — Shimmer analysis via Praat C API
 *
 * Usage:
 *   s = praatmex('shimmer', samples, sr, ...)
 *
 * Parameters (name-value pairs):
 *   'method',            'local' (default), 'local_dB', 'apq3', 'apq5', 'apq11', 'dda', 'all'
 *   'pitchMethod',       'ac' (default), 'cc', 'filtered_ac', 'filtered_cc', 'shs', 'spinet'
 *   'ppMethod',          'periodic_cc' (default), 'periodic_peaks', 'pitch_only', 'extrema', 'periodic_direct'
 *   'pitchFloor',        Hz (default 75)
 *   'pitchCeiling',      Hz (default 600)
 *   'maxnCandidates',    integer (default 15)
 *   'veryAccurate',      true/false (default false)
 *   'silenceThreshold',  0..1 (default 0.03)
 *   'voicingThreshold',  0..1 (default 0.45)
 *   'octaveCost',        0..1 (default 0.01)
 *   'octaveJumpCost',    0..1 (default 0.35)
 *   'voicedUnvoicedCost', 0..1 (default 0.14)
 *   'attenuationAtTop',  0..1 (default 0.74, filtered methods only)
 *   'shsHighestFrequency', Hz (default 1250, shs only)
 *   'shsMaxnSubharmonics', integer (default 15, shs only)
 *   'shsCompressionFactor', double (default 0.84, shs only)
 *   'shsPointsPerOctave', integer (default 48, shs only)
 *   'spinetWindowLength', seconds (default 0.04, spinet only)
 *   'spinetMinFreq',     Hz (default 70, spinet only)
 *   'spinetMaxFreq',     Hz (default 5000, spinet only)
 *   'spinetNumFilters',  integer (default 250, spinet only)
 *   'includeMaxima',     true/false (default true, periodic_peaks/extrema only)
 *   'includeMinima',     true/false (default false, periodic_peaks/extrema only)
 *   'maxPeriodFactor',   double (default 1.3)
 *   'maxAmplitudeFactor', double (default 1.6)
 *   'windowLength',      seconds (default 0 = no windowing, single value)
 *   'windowStep',        seconds (default 0 = same as windowLength)
 *
 * No windowing (default): returns scalar per method or struct with scalar fields
 * With windowing: returns struct with time and value arrays
 */

#include "praat.h"
#include "praatmex_praat_helpers.h"
#include "Sound_to_PointProcess.h"
#include "VoiceAnalysis.h"
#include <vector>

static double getShimmer(const char *method, PointProcess pp, Sound sound,
    double tmin, double tmax, double minimumPeriod, double maximumPeriod,
    double maxPeriodFactor, double maxAmplitudeFactor)
{
    if (strcmp(method, "local") == 0)
        return PointProcess_Sound_getShimmer_local(pp, sound, tmin, tmax, minimumPeriod, maximumPeriod, maxPeriodFactor, maxAmplitudeFactor);
    if (strcmp(method, "local_dB") == 0)
        return PointProcess_Sound_getShimmer_local_dB(pp, sound, tmin, tmax, minimumPeriod, maximumPeriod, maxPeriodFactor, maxAmplitudeFactor);
    if (strcmp(method, "apq3") == 0)
        return PointProcess_Sound_getShimmer_apq3(pp, sound, tmin, tmax, minimumPeriod, maximumPeriod, maxPeriodFactor, maxAmplitudeFactor);
    if (strcmp(method, "apq5") == 0)
        return PointProcess_Sound_getShimmer_apq5(pp, sound, tmin, tmax, minimumPeriod, maximumPeriod, maxPeriodFactor, maxAmplitudeFactor);
    if (strcmp(method, "apq11") == 0)
        return PointProcess_Sound_getShimmer_apq11(pp, sound, tmin, tmax, minimumPeriod, maximumPeriod, maxPeriodFactor, maxAmplitudeFactor);
    if (strcmp(method, "dda") == 0)
        return PointProcess_Sound_getShimmer_dda(pp, sound, tmin, tmax, minimumPeriod, maximumPeriod, maxPeriodFactor, maxAmplitudeFactor);
    mexErrMsgIdAndTxt("praatmex:shimmer:method",
        "Unknown method '%s'. Use 'local', 'local_dB', 'apq3', 'apq5', 'apq11', 'dda', or 'all'.", method);
    return 0.0;
}

void praatmex_shimmer(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    if (nrhs < 2)
        mexErrMsgIdAndTxt("praatmex:shimmer:args",
            "Usage: s = praatmex('shimmer', samples, sr, ...)");

    validateInputIsVector(prhs[0], "shimmer");
    const double *samples = mxGetPr(prhs[0]);
    size_t nSamples = mxGetNumberOfElements(prhs[0]);
    double fs = mxGetScalar(prhs[1]);

    char methodBuf[64] = "local";
    double maxPeriodFactor = 1.3;
    double maxAmplitudeFactor = 1.6;
    double windowLength = 0.0;
    double windowStep = 0.0;
    PraatPitchParams pitchP;
    PraatPointProcessParams ppP;

    static const char *known[] = {"method","maxPeriodFactor","maxAmplitudeFactor","windowLength","windowStep",
        "pitchMethod","timeStep","pitchFloor","pitchCeiling","maxnCandidates","veryAccurate",
        "periodsPerWindow","silenceThreshold","voicingThreshold","octaveCost","octaveJumpCost",
        "voicedUnvoicedCost","attenuationAtTop","shsHighestFrequency","shsMaxnSubharmonics",
        "shsCompressionFactor","shsPointsPerOctave","spinetWindowLength","spinetMinFreq",
        "spinetMaxFreq","spinetNumFilters","ppMethod","includeMaxima","includeMinima"};
    checkUnknownParams(prhs, nrhs, 2, "shimmer", known, sizeof(known)/sizeof(known[0]));

    int idx = 2;
    /* Single pass: route each param to the correct handler */
    while (idx + 1 < nrhs) {
        const char *name = mxArrayToString(prhs[idx]);
        const mxArray *val = prhs[idx + 1];
        if (!name) { idx += 2; continue; }
        if (strcmp(name, "method") == 0) { mxGetString(val, methodBuf, sizeof(methodBuf)); idx += 2; }
        else if (strcmp(name, "maxPeriodFactor") == 0) { maxPeriodFactor = mxGetScalar(val); idx += 2; }
        else if (strcmp(name, "maxAmplitudeFactor") == 0) { maxAmplitudeFactor = mxGetScalar(val); idx += 2; }
        else if (strcmp(name, "windowLength") == 0) { windowLength = mxGetScalar(val); idx += 2; }
        else if (strcmp(name, "windowStep") == 0) { windowStep = mxGetScalar(val); idx += 2; }
        else if (parsePitchParam(name, val, pitchP)) { idx += 2; }
        else if (parsePointProcessParam(name, val, ppP)) { idx += 2; }
        else { idx += 2; }
        mxFree((void *)name);
    }

    autoSound sound = createSoundFromSamples(samples, nSamples, fs);

    autoPointProcess pp = createPointProcess(samples, nSamples, fs, pitchP, ppP);
    if (!pp)
        mexErrMsgIdAndTxt("praatmex:shimmer:fail", "PointProcess creation failed.");

    double minimumPeriod = 0.8 / pitchP.pitchCeiling;
    double maximumPeriod = 1.25 / pitchP.pitchFloor;

    /* Build list of time windows */
    struct Window { double tmin, tmax; };
    std::vector<Window> windows;

    double sigStart = sound->xmin;
    double sigEnd = sound->xmax;

    if (windowLength <= 0.0) {
        windows.push_back({sigStart, sigEnd});
    } else {
        if (windowStep <= 0.0)
            windowStep = windowLength;
        if (windowStep > windowLength)
            mexErrMsgIdAndTxt("praatmex:shimmer:args",
                "windowStep must not exceed windowLength.");

        double t = sigStart;
        while (t < sigEnd) {
            double tend = t + windowLength;
            if (tend > sigEnd) tend = sigEnd;
            windows.push_back({t, tend});
            t += windowStep;
        }
    }

    size_t nWin = windows.size();
    bool useWindowing = (nWin > 1);

    const char *allFields[] = {"local", "local_dB", "apq3", "apq5", "apq11", "dda"};

    if (strcmp(methodBuf, "all") == 0) {
        if (!useWindowing) {
            mxArray *out = mxCreateStructMatrix(1, 1, 6, allFields);
            plhs[0] = out;
            for (int f = 0; f < 6; f++)
                mxSetFieldByNumber(out, 0, f, mxCreateDoubleScalar(
                    getShimmer(allFields[f], pp.get(), sound.get(), windows[0].tmin, windows[0].tmax,
                        minimumPeriod, maximumPeriod, maxPeriodFactor, maxAmplitudeFactor)));
        } else {
            const char *outFields[] = {"time", "local", "local_dB", "apq3", "apq5", "apq11", "dda"};
            mxArray *out = mxCreateStructMatrix(1, 1, 7, outFields);
            plhs[0] = out;

            mxArray *timeMx = mxCreateDoubleMatrix(1, nWin, mxREAL);
            double *timePr = mxGetPr(timeMx);
            for (size_t i = 0; i < nWin; i++)
                timePr[i] = (windows[i].tmin + windows[i].tmax) / 2.0;
            mxSetFieldByNumber(out, 0, 0, timeMx);

            for (int f = 0; f < 6; f++) {
                mxArray *vm = mxCreateDoubleMatrix(1, nWin, mxREAL);
                double *vp = mxGetPr(vm);
                for (size_t i = 0; i < nWin; i++)
                    vp[i] = getShimmer(allFields[f], pp.get(), sound.get(),
                        windows[i].tmin, windows[i].tmax,
                        minimumPeriod, maximumPeriod, maxPeriodFactor, maxAmplitudeFactor);
                mxSetFieldByNumber(out, 0, f + 1, vm);
            }
        }
    } else {
        if (!useWindowing) {
            plhs[0] = mxCreateDoubleScalar(
                getShimmer(methodBuf, pp.get(), sound.get(), windows[0].tmin, windows[0].tmax,
                    minimumPeriod, maximumPeriod, maxPeriodFactor, maxAmplitudeFactor));
        } else {
            const char *outFields[] = {"time", "value"};
            mxArray *out = mxCreateStructMatrix(1, 1, 2, outFields);
            plhs[0] = out;

            mxArray *timeMx = mxCreateDoubleMatrix(1, nWin, mxREAL);
            mxArray *valMx = mxCreateDoubleMatrix(1, nWin, mxREAL);
            double *timePr = mxGetPr(timeMx);
            double *valPr = mxGetPr(valMx);

            for (size_t i = 0; i < nWin; i++) {
                timePr[i] = (windows[i].tmin + windows[i].tmax) / 2.0;
                valPr[i] = getShimmer(methodBuf, pp.get(), sound.get(),
                    windows[i].tmin, windows[i].tmax,
                    minimumPeriod, maximumPeriod, maxPeriodFactor, maxAmplitudeFactor);
            }

            mxSetFieldByNumber(out, 0, 0, timeMx);
            mxSetFieldByNumber(out, 0, 1, valMx);
        }
    }
}
