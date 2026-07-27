/*
 * praatmex_mfcc.cpp — MFCC extraction via Praat C API
 *
 * Usage:
 *   m = praatmex('mfcc', samples, sr, ...)
 *
 * Parameters (name-value pairs):
 *   'numberOfCoefficients', integer (default 12)
 *   'analysisWidth',        seconds (default 0.015)
 *   'dt',                   seconds (default 0.01)
 *   'fmin_mel',             Hz (default 100)
 *   'fmax_mel',             Hz (default 0 = Nyquist)
 *   'df_mel',               Hz (default 100)
 *
 * Returns struct: coefficients (nt x nc matrix), time (1 x nt vector)
 */

#include "praat.h"
#include "praatmex_helpers.h"
#include "Sound.h"
#include "MFCC.h"
#include "Sound_to_MFCC.h"

void praatmex_mfcc(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    if (nrhs < 2)
        mexErrMsgIdAndTxt("praatmex:mfcc:args",
            "Usage: m = praatmex('mfcc', samples, sr, ...)");

    validateInputIsVector(prhs[0], "mfcc");
    const double *samples = mxGetPr(prhs[0]);
    size_t nSamples = mxGetNumberOfElements(prhs[0]);
    double fs = mxGetScalar(prhs[1]);

    integer numberOfCoefficients = 12;
    double analysisWidth = 0.015;
    double dt = 0.01;
    double fmin_mel = 100.0;
    double fmax_mel = 0.0;   /* 0 = auto (Nyquist) */
    double df_mel = 100.0;

    static const char *known[] = {"numberOfCoefficients","analysisWidth","dt",
        "fmin_mel","fmax_mel","df_mel"};
    checkUnknownParams(prhs, nrhs, 2, "mfcc", known, sizeof(known)/sizeof(known[0]));

    int idx = 2;
    while (idx + 1 < nrhs) {
        const char *name = mxArrayToString(prhs[idx]);
        if (!name) { idx += 2; continue; }
        const mxArray *val = prhs[idx + 1];
        if (strcmp(name, "numberOfCoefficients") == 0) numberOfCoefficients = (integer)mxGetScalar(val);
        else if (strcmp(name, "analysisWidth") == 0) analysisWidth = mxGetScalar(val);
        else if (strcmp(name, "dt") == 0) dt = mxGetScalar(val);
        else if (strcmp(name, "fmin_mel") == 0) fmin_mel = mxGetScalar(val);
        else if (strcmp(name, "fmax_mel") == 0) fmax_mel = mxGetScalar(val);
        else if (strcmp(name, "df_mel") == 0) df_mel = mxGetScalar(val);
        mxFree((void *)name);
        idx += 2;
    }

    autoSound sound = Sound_createSimple(1, (double)nSamples / fs, fs);
    for (long i = 1; i <= (long)nSamples; i++)
        sound->z[1][i] = samples[i - 1];

    autoMFCC mfcc = Sound_to_MFCC(sound.get(), numberOfCoefficients,
        analysisWidth, dt, fmin_mel, fmax_mel, df_mel);

    if (!mfcc)
        mexErrMsgIdAndTxt("praatmex:mfcc:fail", "MFCC extraction failed.");

    long nFrames = mfcc->nx;
    integer nc = numberOfCoefficients;
    double tmin = mfcc->x1;
    double dx = mfcc->dx;

    /* coefficients matrix: nFrames rows x nc columns */
    mxArray *coeffMatrix = mxCreateDoubleMatrix(nFrames, nc, mxREAL);
    double *cData = mxGetPr(coeffMatrix);
    std::vector<double> time(nFrames, 0.0);

    for (long i = 1; i <= nFrames; i++) {
        time[i - 1] = tmin + (i - 1) * dx;
        for (integer j = 1; j <= nc; j++) {
            /* MATLAB is column-major: cData[(j-1)*nFrames + (i-1)] */
            cData[(j - 1) * nFrames + (i - 1)] = CC_getValueInFrame(mfcc.get(), i, j);
        }
    }

    const char *fields[] = {"coefficients", "time"};
    mxArray *out = mxCreateStructMatrix(1, 1, 2, fields);
    mxSetFieldByNumber(out, 0, 0, coeffMatrix);
    mxSetFieldByNumber(out, 0, 1, doublesToMxArray(time));
    plhs[0] = out;
}
