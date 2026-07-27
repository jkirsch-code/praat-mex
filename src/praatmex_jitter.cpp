/*
 * praatmex_jitter.cpp — Jitter analysis via Praat C API
 *
 * Usage:
 *   j = praatmex('jitter', samples, sr, ...)
 *
 * Parameters (name-value pairs):
 *   'method',            'local' (default), 'local_absolute', 'rap', 'ppq5', 'ddp', 'all'
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

static double getJitter(const char *method, PointProcess pp,
    double tmin, double tmax, double minimumPeriod, double maximumPeriod, double maxPeriodFactor)
{
    if (strcmp(method, "local") == 0)
        return PointProcess_getJitter_local(pp, tmin, tmax, minimumPeriod, maximumPeriod, maxPeriodFactor);
    if (strcmp(method, "local_absolute") == 0)
        return PointProcess_getJitter_local_absolute(pp, tmin, tmax, minimumPeriod, maximumPeriod, maxPeriodFactor);
    if (strcmp(method, "rap") == 0)
        return PointProcess_getJitter_rap(pp, tmin, tmax, minimumPeriod, maximumPeriod, maxPeriodFactor);
    if (strcmp(method, "ppq5") == 0)
        return PointProcess_getJitter_ppq5(pp, tmin, tmax, minimumPeriod, maximumPeriod, maxPeriodFactor);
    if (strcmp(method, "ddp") == 0)
        return PointProcess_getJitter_ddp(pp, tmin, tmax, minimumPeriod, maximumPeriod, maxPeriodFactor);
    mexErrMsgIdAndTxt("praatmex:jitter:method",
        "Unknown method '%s'. Use 'local', 'local_absolute', 'rap', 'ppq5', 'ddp', or 'all'.", method);
    return 0.0;
}

void praatmex_jitter(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    if (nrhs < 2)
        mexErrMsgIdAndTxt("praatmex:jitter:args",
            "Usage: j = praatmex('jitter', samples, sr, ...)");

    validateInputIsVector(prhs[0], "jitter");
    const double *samples = mxGetPr(prhs[0]);
    size_t nSamples = mxGetNumberOfElements(prhs[0]);
    double fs = mxGetScalar(prhs[1]);

    char methodBuf[64] = "local";
    double maxPeriodFactor = 1.3;
    double windowLength = 0.0;
    double windowStep = 0.0;
    PraatPitchParams pitchP;
    PraatPointProcessParams ppP;

    static const char *known[] = {"method","maxPeriodFactor","windowLength","windowStep",
        "pitchMethod","timeStep","pitchFloor","pitchCeiling","maxnCandidates","veryAccurate",
        "periodsPerWindow","silenceThreshold","voicingThreshold","octaveCost","octaveJumpCost",
        "voicedUnvoicedCost","attenuationAtTop","shsHighestFrequency","shsMaxnSubharmonics",
        "shsCompressionFactor","shsPointsPerOctave","spinetWindowLength","spinetMinFreq",
        "spinetMaxFreq","spinetNumFilters","ppMethod","includeMaxima","includeMinima"};
    checkUnknownParams(prhs, nrhs, 2, "jitter", known, sizeof(known)/sizeof(known[0]));

    int idx = 2;
    /* Single pass: route each param to the correct handler */
    while (idx + 1 < nrhs) {
        const char *name = mxArrayToString(prhs[idx]);
        const mxArray *val = prhs[idx + 1];
        if (!name) { idx += 2; continue; }
        if (strcmp(name, "method") == 0) { mxGetString(val, methodBuf, sizeof(methodBuf)); idx += 2; }
        else if (strcmp(name, "maxPeriodFactor") == 0) { maxPeriodFactor = mxGetScalar(val); idx += 2; }
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
        mexErrMsgIdAndTxt("praatmex:jitter:fail", "PointProcess creation failed.");

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
            mexErrMsgIdAndTxt("praatmex:jitter:args",
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

    const char *allFields[] = {"local", "local_absolute", "rap", "ppq5", "ddp"};

    if (strcmp(methodBuf, "all") == 0) {
        if (!useWindowing) {
            mxArray *out = mxCreateStructMatrix(1, 1, 5, allFields);
            plhs[0] = out;
            mxSetFieldByNumber(out, 0, 0, mxCreateDoubleScalar(
                getJitter("local", pp.get(), windows[0].tmin, windows[0].tmax,
                    minimumPeriod, maximumPeriod, maxPeriodFactor)));
            mxSetFieldByNumber(out, 0, 1, mxCreateDoubleScalar(
                getJitter("local_absolute", pp.get(), windows[0].tmin, windows[0].tmax,
                    minimumPeriod, maximumPeriod, maxPeriodFactor)));
            mxSetFieldByNumber(out, 0, 2, mxCreateDoubleScalar(
                getJitter("rap", pp.get(), windows[0].tmin, windows[0].tmax,
                    minimumPeriod, maximumPeriod, maxPeriodFactor)));
            mxSetFieldByNumber(out, 0, 3, mxCreateDoubleScalar(
                getJitter("ppq5", pp.get(), windows[0].tmin, windows[0].tmax,
                    minimumPeriod, maximumPeriod, maxPeriodFactor)));
            mxSetFieldByNumber(out, 0, 4, mxCreateDoubleScalar(
                getJitter("ddp", pp.get(), windows[0].tmin, windows[0].tmax,
                    minimumPeriod, maximumPeriod, maxPeriodFactor)));
        } else {
            const char *outFields[] = {"time", "local", "local_absolute", "rap", "ppq5", "ddp"};
            mxArray *out = mxCreateStructMatrix(1, 1, 6, outFields);
            plhs[0] = out;

            std::vector<double> timeVec(nWin, 0.0);
            std::vector<double> vals[5];
            for (int f = 0; f < 5; f++)
                vals[f].resize(nWin, 0.0);

            for (size_t i = 0; i < nWin; i++) {
                double center = (windows[i].tmin + windows[i].tmax) / 2.0;
                timeVec[i] = center;
                vals[0][i] = getJitter("local", pp.get(), windows[i].tmin, windows[i].tmax,
                    minimumPeriod, maximumPeriod, maxPeriodFactor);
                vals[1][i] = getJitter("local_absolute", pp.get(), windows[i].tmin, windows[i].tmax,
                    minimumPeriod, maximumPeriod, maxPeriodFactor);
                vals[2][i] = getJitter("rap", pp.get(), windows[i].tmin, windows[i].tmax,
                    minimumPeriod, maximumPeriod, maxPeriodFactor);
                vals[3][i] = getJitter("ppq5", pp.get(), windows[i].tmin, windows[i].tmax,
                    minimumPeriod, maximumPeriod, maxPeriodFactor);
                vals[4][i] = getJitter("ddp", pp.get(), windows[i].tmin, windows[i].tmax,
                    minimumPeriod, maximumPeriod, maxPeriodFactor);
            }

            mxSetFieldByNumber(out, 0, 0, doublesToMxArray(timeVec));
            for (int f = 0; f < 5; f++)
                mxSetFieldByNumber(out, 0, f + 1, doublesToMxArray(vals[f]));
        }
    } else {
        if (!useWindowing) {
            plhs[0] = mxCreateDoubleScalar(
                getJitter(methodBuf, pp.get(), windows[0].tmin, windows[0].tmax,
                    minimumPeriod, maximumPeriod, maxPeriodFactor));
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
                valPr[i] = getJitter(methodBuf, pp.get(), windows[i].tmin, windows[i].tmax,
                    minimumPeriod, maximumPeriod, maxPeriodFactor);
            }

            mxSetFieldByNumber(out, 0, 0, timeMx);
            mxSetFieldByNumber(out, 0, 1, valMx);
        }
    }
}
