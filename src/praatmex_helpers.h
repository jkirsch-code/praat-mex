#pragma once
/*
 * praatmex_helpers.h — Shared MEX helper utilities (no Praat dependencies)
 */

#include "mex.h"
#include <cstring>
#include <string>
#include <vector>

inline std::string mxGetStringSafe(const mxArray *a) {
    if (!a) return "";
    char buf[1024];
    mxGetString(a, buf, sizeof(buf));
    return buf;
}

inline std::vector<double> mxToDoubles(const mxArray *a) {
    if (!a) return {};
    const double *p = mxGetPr(a);
    size_t n = mxGetNumberOfElements(a);
    return {p, p + n};
}

inline mxArray *doublesToMxArray(const std::vector<double> &v) {
    mxArray *out = mxCreateDoubleMatrix(1, v.size(), mxREAL);
    if (!v.empty()) memcpy(mxGetPr(out), v.data(), v.size() * sizeof(double));
    return out;
}

inline mxArray *doublesToMxArray(const double *data, size_t n) {
    mxArray *out = mxCreateDoubleMatrix(1, n, mxREAL);
    if (n > 0) memcpy(mxGetPr(out), data, n * sizeof(double));
    return out;
}

inline double mxGetScalarDefault(const mxArray *a, double def) {
    return (a && mxGetNumberOfElements(a) > 0) ? mxGetScalar(a) : def;
}

/* Convert MATLAB uint16 vector to char32_t string (Praat's string type) */
std::vector<char32_t> mxUint16ToChar32(const mxArray *a);

/* Convert char32_t string to mxArray (uint16 vector) */
mxArray *char32ToMxArray(const char32_t *s, size_t len);

/* Validate that input samples array is 1D (not a matrix/multi-channel) */
inline void validateInputIsVector(const mxArray *a, const char *cmdName) {
    if (mxGetNumberOfDimensions(a) > 2 || (mxGetM(a) > 1 && mxGetN(a) > 1))
        mexErrMsgIdAndTxt("praatmex:input:notVector",
            "%s expects a 1D vector. Multi-channel audio not supported.", cmdName);
}

/* Check that all name-value pairs in prhs[startIdx..nrhs-1] are in the known set.
   Calls mexErrMsgIdAndTxt on the first unknown parameter. */
inline void checkUnknownParams(const mxArray **prhs, int nrhs, int startIdx,
        const char *cmdName, const char *const *knownNames, int nKnown) {
    for (int i = startIdx; i + 1 < nrhs; i += 2) {
        const char *name = mxArrayToString(prhs[i]);
        if (!name) continue;
        bool found = false;
        for (int k = 0; k < nKnown; k++) {
            if (strcmp(name, knownNames[k]) == 0) { found = true; break; }
        }
        if (!found) {
            mexErrMsgIdAndTxt("praatmex:input:unknownParam",
                "%s: unknown parameter '%s'.", cmdName, name);
        }
        mxFree((void *)name);
    }
}
