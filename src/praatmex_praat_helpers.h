#pragma once
/*
 * praatmex_praat_helpers.h — Shared Praat helper utilities (pitch, pointprocess creation)
 *
 * This header must be included AFTER the Praat headers that define
 * autoSound, autoPitch, autoPointProcess, etc.
 */

#include "praatmex_helpers.h"
#include "praat.h"
#include "Sound.h"
#include "Pitch.h"
#include "PointProcess.h"

/* Shared pitch creation parameters — parsed from name-value pairs */
struct PraatPitchParams {
    char method[64] = "ac";
    double timeStep = 0.0;
    double pitchFloor = 75.0;
    double pitchCeiling = 600.0;
    integer maxnCandidates = 15;
    bool veryAccurate = false;
    double periodsPerWindow = 3.0;
    double silenceThreshold = 0.03;
    double voicingThreshold = 0.45;
    double octaveCost = 0.01;
    double octaveJumpCost = 0.35;
    double voicedUnvoicedCost = 0.14;
/*
 * NOTE — attenuationAtTop default differs from Praat.exe
 *
 * Praat scripts set attenuationAtTop = 0.03 for filtered_ac / filtered_cc
 * pitch methods.  The MEX default here is 0.74 (the value Praat's C++
 * constructor uses for its own Pitch objects).  This mismatch causes the
 * PointProcess count to differ from Praat.exe by ~10 % when the user
 * does not explicitly pass 'attenuationAtTop'.
 *
 * To match Praat.exe exactly, pass  'attenuationAtTop', 0.03  explicitly.
 */
    double attenuationAtTop = 0.74;
    /* SHS-specific */
    double shsHighestFrequency = 1250.0;
    integer shsMaxnSubharmonics = 15;
    double shsCompressionFactor = 0.84;
    integer shsPointsPerOctave = 48;
    /* SPINET-specific */
    double spinetWindowLength = 0.04;
    double spinetMinFreq = 70.0;
    double spinetMaxFreq = 5000.0;
    integer spinetNumFilters = 250;
};

/* Shared PointProcess creation parameters */
struct PraatPointProcessParams {
    char method[64] = "periodic_cc";
    bool includeMaxima = true;
    bool includeMinima = false;
};

/* Parse name-value pairs for pitch params (starting at idx in prhs) */
void parsePitchParams(const mxArray **prhs, int nrhs, int &idx, PraatPitchParams &p);

/* Parse name-value pairs for point process params */
void parsePointProcessParams(const mxArray **prhs, int nrhs, int &idx, PraatPointProcessParams &p);

/* Parse a single name-value pair for pitch params. Returns true if recognized. */
bool parsePitchParam(const char *name, const mxArray *val, PraatPitchParams &p);

/* Parse a single name-value pair for point process params. Returns true if recognized. */
bool parsePointProcessParam(const char *name, const mxArray *val, PraatPointProcessParams &p);

/*
 * Create a Sound from double-precision samples.
 *
 * When useFloat32 is true, writes a temporary float32 WAV file and reads
 * it back with Praat's own Sound_readFromSoundFile.  This goes through
 * the exact same code path (bingetr32LE -> Sound z[]) that Praat.exe uses,
 * so the resulting Sound object is byte-identical to what Praat.exe would
 * produce when reading the same WAV file.
 *
 * When useFloat32 is false (default), samples are stored directly in
 * the Sound z[] array as double.  This preserves full MATLAB precision
 * but produces slightly different Pitch / PointProcess values compared
 * to Praat.exe (PP count differs by ~10 %) because Praat.exe always
 * reads from float32 WAV files.
 */
autoSound createSoundFromSamples(const double *samples, size_t nSamples, double fs, bool useFloat32 = false);

/* Create Pitch from an existing Sound */
autoPitch createPitchFromSound(Sound sound, const PraatPitchParams &p);

/* Create Pitch from params (creates Sound internally) */
autoPitch createPitch(const double *samples, size_t nSamples, double fs, const PraatPitchParams &p);

/* Create PointProcess from sound + pitch */
autoPointProcess createPointProcess(const double *samples, size_t nSamples, double fs,
    const PraatPitchParams &pp, const PraatPointProcessParams &ppp);
