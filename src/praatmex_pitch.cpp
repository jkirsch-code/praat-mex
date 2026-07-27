/*
 * praatmex_pitch.cpp — Pitch extraction via Praat C API
 *
 * Usage:
 *   p = praatmex('pitch', samples, sr, 'pitchMethod', 'ac', ...)
 *
 * Parameters (name-value pairs):
 *   'pitchMethod',       'ac' (default), 'cc', 'filtered_ac', 'filtered_cc', 'shs', 'spinet'
 *   'timeStep',          seconds (0 = auto, default)
 *   'pitchFloor',        Hz (default 75)
 *   'pitchCeiling',      Hz (default 600)
 *   'maxnCandidates',    integer (default 15)
 *   'veryAccurate',      true/false (default false)
 *   'periodsPerWindow',  double (default 3.0)
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
 *
 * Returns struct: freq, strength, voiced, time
 */

#include "praat.h"
#include "praatmex_praat_helpers.h"
#include "Sound_to_Pitch.h"
#include <cmath>

void praatmex_pitch(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    if (nrhs < 2)
        mexErrMsgIdAndTxt("praatmex:pitch:args",
            "Usage: p = praatmex('pitch', samples, sr, ...)");

    validateInputIsVector(prhs[0], "pitch");
    const double *samples = mxGetPr(prhs[0]);
    size_t nSamples = mxGetNumberOfElements(prhs[0]);
    double fs = mxGetScalar(prhs[1]);

    PraatPitchParams p;
    static const char *known[] = {"pitchMethod","timeStep","pitchFloor","pitchCeiling",
        "maxnCandidates","veryAccurate","periodsPerWindow","silenceThreshold",
        "voicingThreshold","octaveCost","octaveJumpCost","voicedUnvoicedCost",
        "attenuationAtTop","shsHighestFrequency","shsMaxnSubharmonics",
        "shsCompressionFactor","shsPointsPerOctave","spinetWindowLength",
        "spinetMinFreq","spinetMaxFreq","spinetNumFilters"};
    checkUnknownParams(prhs, nrhs, 2, "pitch", known, sizeof(known)/sizeof(known[0]));
    int idx = 2;
    parsePitchParams(prhs, nrhs, idx, p);

    autoPitch pitch = createPitch(samples, nSamples, fs, p);

    if (!pitch)
        mexErrMsgIdAndTxt("praatmex:pitch:fail", "Pitch extraction failed.");

    long nFrames = pitch->nx;
    double tmin = pitch->x1;
    double dt = pitch->dx;

    std::vector<double> freq(nFrames), strength(nFrames), voiced(nFrames), time(nFrames);
    for (long i = 1; i <= nFrames; i++) {
        double t = tmin + (i - 1) * dt;
        double f = Pitch_getValueAtTime(pitch.get(), t, kPitch_unit::HERTZ, true);
        double s = Pitch_getStrengthAtTime(pitch.get(), t, kPitch_unit::HERTZ, true);
        freq[i - 1] = (std::isfinite(f) && f > 0.0) ? f : 0.0;
        strength[i - 1] = (std::isfinite(s)) ? s : 0.0;
        voiced[i - 1] = (freq[i - 1] > 0.0) ? 1.0 : 0.0;
        time[i - 1] = t;
    }

    const char *fields[] = {"freq", "strength", "voiced", "time"};
    mxArray *out = mxCreateStructMatrix(1, 1, 4, fields);
    mxSetFieldByNumber(out, 0, 0, doublesToMxArray(freq));
    mxSetFieldByNumber(out, 0, 1, doublesToMxArray(strength));
    mxSetFieldByNumber(out, 0, 2, doublesToMxArray(voiced));
    mxSetFieldByNumber(out, 0, 3, doublesToMxArray(time));
    plhs[0] = out;
}
