/*
 * praatmex_intensity.cpp — Intensity contour via Praat C API
 *
 * Usage:
 *   i = praatmex('intensity', samples, sr, ...)
 *
 * Parameters (name-value pairs):
 *   'pitchFloor',        Hz (default 100)
 *   'timeStep',          seconds (default 0.0 = auto)
 *   'subtractMean',      true/false (default true)
 */

#include "praat.h"
#include "praatmex_praat_helpers.h"
#include "Intensity.h"
#include "Sound_to_Intensity.h"

void praatmex_intensity(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    if (nrhs < 2)
        mexErrMsgIdAndTxt("praatmex:intensity:args",
            "Usage: praatmex('intensity', samples, sr, ...)");

    validateInputIsVector(prhs[0], "intensity");
    const double *samples = mxGetPr(prhs[0]);
    size_t nSamples = mxGetNumberOfElements(prhs[0]);
    double fs = mxGetScalar(prhs[1]);

    double pitchFloor = 100.0;
    double timeStep = 0.0;
    bool subtractMean = true;

    static const char *known[] = {"pitchFloor","timeStep","subtractMean"};
    checkUnknownParams(prhs, nrhs, 2, "intensity", known, sizeof(known)/sizeof(known[0]));

    int idx = 2;
    while (idx + 1 < nrhs) {
        const char *name = mxArrayToString(prhs[idx]);
        if (!name) { idx += 2; continue; }
        const mxArray *val = prhs[idx + 1];
        if (strcmp(name, "pitchFloor") == 0) pitchFloor = mxGetScalar(val);
        else if (strcmp(name, "timeStep") == 0) timeStep = mxGetScalar(val);
        else if (strcmp(name, "subtractMean") == 0) subtractMean = (mxGetScalar(val) != 0.0);
        mxFree((void *)name);
        idx += 2;
    }

    autoSound sound = createSoundFromSamples(samples, nSamples, fs);

    /* Sound_to_Intensity: me, pitchFloor, timeStep, subtractMean */
    autoIntensity intensity = Sound_to_Intensity(sound.get(), pitchFloor, timeStep, subtractMean);

    if (!intensity)
        mexErrMsgIdAndTxt("praatmex:intensity:fail", "Intensity analysis failed.");

    long nFrames = intensity->nx;
    std::vector<double> time(nFrames), val(nFrames);
    for (long i = 1; i <= nFrames; i++) {
        time[i - 1] = intensity->x1 + (i - 1) * intensity->dx;
        val[i - 1] = intensity->z[1][i];
    }

    const char *fields[] = {"time", "intensity"};
    mxArray *out = mxCreateStructMatrix(1, 1, 2, fields);
    mxSetFieldByNumber(out, 0, 0, doublesToMxArray(time));
    mxSetFieldByNumber(out, 0, 1, doublesToMxArray(val));
    plhs[0] = out;
}
