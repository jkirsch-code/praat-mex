/*
 * praatmex_spectrum.cpp — Spectrum analysis via Praat C API
 */

#include "praat.h"
#include "praatmex_helpers.h"
#include "Sound.h"
#include "Spectrum.h"
#include "Sound_and_Spectrum.h"

void praatmex_spectrum(int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs) {
    if (nrhs < 2)
        mexErrMsgIdAndTxt("praatmex:spectrum:args",
            "Usage: praatmex('spectrum', samples, sr)");

    validateInputIsVector(prhs[0], "spectrum");
    const double *samples = mxGetPr(prhs[0]);
    size_t nSamples = mxGetNumberOfElements(prhs[0]);
    double fs = mxGetScalar(prhs[1]);

    autoSound sound = Sound_createSimple(1, (double)nSamples / fs, fs);
    for (long i = 1; i <= (long)nSamples; i++)
        sound->z[1][i] = samples[i - 1];

    /* Sound_to_Spectrum: me, fast */
    autoSpectrum spectrum = Sound_to_Spectrum(sound.get(), true);
    if (!spectrum)
        mexErrMsgIdAndTxt("praatmex:spectrum:fail", "Spectrum analysis failed.");

    long nBins = spectrum->nx;
    std::vector<double> freq(nBins), amp(nBins), phase(nBins);
    double df = spectrum->dx;

    for (long i = 1; i <= nBins; i++) {
        freq[i - 1] = (i - 1) * df;
        amp[i - 1] = sqrt(spectrum->z[1][i] * spectrum->z[1][i] +
                          spectrum->z[2][i] * spectrum->z[2][i]);
        phase[i - 1] = atan2(spectrum->z[2][i], spectrum->z[1][i]);
    }

    const char *fields[] = {"freq", "amplitude", "phase"};
    mxArray *out = mxCreateStructMatrix(1, 1, 3, fields);
    mxSetFieldByNumber(out, 0, 0, doublesToMxArray(freq));
    mxSetFieldByNumber(out, 0, 1, doublesToMxArray(amp));
    mxSetFieldByNumber(out, 0, 2, doublesToMxArray(phase));
    plhs[0] = out;
}
