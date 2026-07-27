/*
 * praatmex_pointprocess.cpp — PointProcess extraction via Praat C API
 *
 * Usage:
 *   pp = praatmex('pointprocess', samples, sr, ...)
 *
 * Parameters (name-value pairs):
 *   'ppMethod',          'periodic_cc' (default), 'periodic_peaks', 'pitch_only', 'extrema', 'periodic_direct'
 *   'pitchMethod',       'ac' (default), 'cc', 'filtered_ac', 'filtered_cc', 'shs', 'spinet'
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
 *
 * Returns struct: time (array of pulse times)
 */

#include "praat.h"
#include "praatmex_praat_helpers.h"
#include "Sound_to_PointProcess.h"

void praatmex_pointprocess(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    if (nrhs < 2)
        mexErrMsgIdAndTxt("praatmex:pointprocess:args",
            "Usage: pp = praatmex('pointprocess', samples, sr, ...)");

    validateInputIsVector(prhs[0], "pointprocess");
    const double *samples = mxGetPr(prhs[0]);
    size_t nSamples = mxGetNumberOfElements(prhs[0]);
    double fs = mxGetScalar(prhs[1]);

    PraatPitchParams pitchP;
    PraatPointProcessParams ppP;

    static const char *known[] = {"ppMethod","includeMaxima","includeMinima",
        "pitchMethod","timeStep","pitchFloor","pitchCeiling","maxnCandidates","veryAccurate",
        "periodsPerWindow","silenceThreshold","voicingThreshold","octaveCost","octaveJumpCost",
        "voicedUnvoicedCost","attenuationAtTop","shsHighestFrequency","shsMaxnSubharmonics",
        "shsCompressionFactor","shsPointsPerOctave","spinetWindowLength","spinetMinFreq",
        "spinetMaxFreq","spinetNumFilters"};
    checkUnknownParams(prhs, nrhs, 2, "pointprocess", known, sizeof(known)/sizeof(known[0]));

    int idx = 2;
    parsePitchParams(prhs, nrhs, idx, pitchP);
    parsePointProcessParams(prhs, nrhs, idx, ppP);

    autoPointProcess pp = createPointProcess(samples, nSamples, fs, pitchP, ppP);
    if (!pp)
        mexErrMsgIdAndTxt("praatmex:pointprocess:fail", "PointProcess creation failed.");

    long nPulses = pp->nt;
    std::vector<double> time(nPulses);
    for (long i = 1; i <= nPulses; i++)
        time[i - 1] = pp->t[i];

    const char *fields[] = {"time"};
    mxArray *out = mxCreateStructMatrix(1, 1, 1, fields);
    mxSetFieldByNumber(out, 0, 0, doublesToMxArray(time));
    plhs[0] = out;
}
