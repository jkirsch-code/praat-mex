/*
 * praatmex_harmonicity.cpp — Harmonicity (HNR) and GNE via Praat C API
 *
 * Usage:
 *   h = praatmex('harmonicity', samples, sr, ...)
 *
 * Parameters (name-value pairs):
 *   'method',            'HNRac' (default), 'HNRcc', 'GNE'
 *   'timeStep',          seconds (default 0.01)
 *   'pitchFloor',        Hz (default 75)
 *   'silenceThreshold',  0..1 (default 0.1)
 *   'periodsPerWindow',  double (default 4.5)
 *   'gneMinFreq',        Hz (default 500, GNE only)
 *   'gneMaxFreq',        Hz (default 4500, GNE only)
 *   'gneBandwidth',      Hz (default 1000, GNE only)
 *   'gneStep',           Hz (default 80, GNE only)
 *
 * Returns struct for 'HNRac'/'HNRcc': time, harmonicity
 * Returns struct for 'GNE': matrix (nBands x nBands cross-correlation),
 *   frequency, nBands, minFreq, maxFreq, bandwidth, step
 */

#include "praat.h"
#include "praatmex_praat_helpers.h"
#include "Harmonicity.h"
#include "Sound_to_Harmonicity.h"

void praatmex_harmonicity(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    if (nrhs < 2)
        mexErrMsgIdAndTxt("praatmex:harmonicity:args",
            "Usage: praatmex('harmonicity', samples, sr, ...)");

    validateInputIsVector(prhs[0], "harmonicity");
    const double *samples = mxGetPr(prhs[0]);
    size_t nSamples = mxGetNumberOfElements(prhs[0]);
    double fs = mxGetScalar(prhs[1]);

    char methodBuf[64] = "HNRac";
    double timeStep = 0.01;
    double pitchFloor = 75.0;
    double silenceThreshold = 0.1;
    double periodsPerWindow = 4.5;
    /* GNE params */
    double gneMinFreq = 500.0;
    double gneMaxFreq = 4500.0;
    double gneBandwidth = 1000.0;
    double gneStep = 80.0;

    static const char *known[] = {"method","timeStep","pitchFloor","silenceThreshold",
        "periodsPerWindow","gneMinFreq","gneMaxFreq","gneBandwidth","gneStep"};
    checkUnknownParams(prhs, nrhs, 2, "harmonicity", known, sizeof(known)/sizeof(known[0]));

    int idx = 2;
    while (idx + 1 < nrhs) {
        const char *name = mxArrayToString(prhs[idx]);
        if (!name) { idx += 2; continue; }
        const mxArray *val = prhs[idx + 1];
        if (strcmp(name, "method") == 0) mxGetString(val, methodBuf, sizeof(methodBuf));
        else if (strcmp(name, "timeStep") == 0) timeStep = mxGetScalar(val);
        else if (strcmp(name, "pitchFloor") == 0) pitchFloor = mxGetScalar(val);
        else if (strcmp(name, "silenceThreshold") == 0) silenceThreshold = mxGetScalar(val);
        else if (strcmp(name, "periodsPerWindow") == 0) periodsPerWindow = mxGetScalar(val);
        else if (strcmp(name, "gneMinFreq") == 0) gneMinFreq = mxGetScalar(val);
        else if (strcmp(name, "gneMaxFreq") == 0) gneMaxFreq = mxGetScalar(val);
        else if (strcmp(name, "gneBandwidth") == 0) gneBandwidth = mxGetScalar(val);
        else if (strcmp(name, "gneStep") == 0) gneStep = mxGetScalar(val);
        mxFree((void *)name);
        idx += 2;
    }

    autoSound sound = createSoundFromSamples(samples, nSamples, fs);

    if (strcmp(methodBuf, "GNE") == 0) {
        /*
         * GNE returns an NxN cross-correlation matrix of Hilbert envelope bands.
         * nx = number of bands (rows = band index), ny = number of bands (cols = band index).
         * z[row][col] = max cross-correlation between band row and band col.
         */
        autoMatrix result = Sound_to_Harmonicity_GNE(
            sound.get(), gneMinFreq, gneMaxFreq, gneBandwidth, gneStep);

        if (!result)
            mexErrMsgIdAndTxt("praatmex:harmonicity:fail", "GNE analysis failed.");

        long nBands = result->nx;

        /*
         * Compute band center frequencies from parameters.
         * Praat's Matrix_createSimple uses default axes (x1=1, dx=1),
         * so frequency info must be derived from fmin/step.
         * Band i has center = gneMinFreq + (i-1) * gneStep.
         */
        std::vector<double> frequency(nBands);
        for (long i = 0; i < nBands; i++) {
            frequency[i] = gneMinFreq + i * gneStep;
        }

        /* Copy NxN matrix directly to MATLAB column-major double array */
        mxArray *matArray = mxCreateDoubleMatrix(nBands, nBands, mxREAL);
        double *matData = mxGetPr(matArray);
        for (long row = 1; row <= nBands; row++) {
            for (long col = 1; col <= nBands; col++) {
                /* MATLAB column-major: matData[(col-1)*nBands + (row-1)] */
                matData[(col - 1) * nBands + (row - 1)] = result->z[row][col];
            }
        }

        const char *fields[] = {"matrix", "frequency", "nBands", "minFreq", "maxFreq", "bandwidth", "step"};
        mxArray *out = mxCreateStructMatrix(1, 1, 7, fields);
        mxSetFieldByNumber(out, 0, 0, matArray);
        mxSetFieldByNumber(out, 0, 1, doublesToMxArray(frequency));
        mxSetFieldByNumber(out, 0, 2, mxCreateDoubleScalar(nBands));
        mxSetFieldByNumber(out, 0, 3, mxCreateDoubleScalar(gneMinFreq));
        mxSetFieldByNumber(out, 0, 4, mxCreateDoubleScalar(gneMaxFreq));
        mxSetFieldByNumber(out, 0, 5, mxCreateDoubleScalar(gneBandwidth));
        mxSetFieldByNumber(out, 0, 6, mxCreateDoubleScalar(gneStep));
        plhs[0] = out;
    } else {
        /* HNRac or HNRcc — same params, same output shape */
        autoHarmonicity harmonicity;
        if (strcmp(methodBuf, "HNRcc") == 0) {
            harmonicity = Sound_to_Harmonicity_cc(
                sound.get(), timeStep, pitchFloor, silenceThreshold, periodsPerWindow);
        } else if (strcmp(methodBuf, "HNRac") == 0) {
            harmonicity = Sound_to_Harmonicity_ac(
                sound.get(), timeStep, pitchFloor, silenceThreshold, periodsPerWindow);
        } else {
            mexErrMsgIdAndTxt("praatmex:harmonicity:method",
                "Unknown method '%s'. Use 'HNRac', 'HNRcc', or 'GNE'.", methodBuf);
        }

        if (!harmonicity)
            mexErrMsgIdAndTxt("praatmex:harmonicity:fail", "Harmonicity analysis failed.");

        long nFrames = harmonicity->nx;
        std::vector<double> time(nFrames), val(nFrames);
        for (long i = 1; i <= nFrames; i++) {
            time[i - 1] = harmonicity->x1 + (i - 1) * harmonicity->dx;
            val[i - 1] = harmonicity->z[1][i];
        }

        const char *fields[] = {"time", "harmonicity"};
        mxArray *out = mxCreateStructMatrix(1, 1, 2, fields);
        mxSetFieldByNumber(out, 0, 0, doublesToMxArray(time));
        mxSetFieldByNumber(out, 0, 1, doublesToMxArray(val));
        plhs[0] = out;
    }
}
