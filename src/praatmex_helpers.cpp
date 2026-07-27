#include "praatmex_praat_helpers.h"
#include <cstring>
#include <cmath>
#include <cstdio>
#ifdef _WIN32
  #include <windows.h>          /* GetTempPathA */
#endif
#include "Sound_to_Pitch.h"
#include "Sound_to_PointProcess.h"
#include "Pitch_to_PointProcess.h"
#include "Sound_to_Pitch2.h"     /* SHS, SPINET */
#include "Sound.h"               /* Sound_readFromSoundFile */
#include "melder_textencoding.h" /* Melder_8to32 */
#include "melder_files.h"        /* Melder_pathToFile, MelderFile_delete, MelderFile_setToNull */

std::vector<char32_t> mxUint16ToChar32(const mxArray *a) {
    if (!a) return {};
    const uint16_t *p = (const uint16_t *)mxGetData(a);
    size_t n = mxGetNumberOfElements(a);
    std::vector<char32_t> out(n);
    for (size_t i = 0; i < n; i++) out[i] = (char32_t)p[i];
    return out;
}

mxArray *char32ToMxArray(const char32_t *s, size_t len) {
    mxArray *out = mxCreateNumericMatrix(1, len, mxUINT16_CLASS, mxREAL);
    uint16_t *p = (uint16_t *)mxGetData(out);
    for (size_t i = 0; i < len; i++) p[i] = (uint16_t)s[i];
    return out;
}

void parsePitchParams(const mxArray **prhs, int nrhs, int &idx, PraatPitchParams &p) {
    while (idx + 1 < nrhs) {
        const char *name = mxArrayToString(prhs[idx]);
        if (!name) { idx += 2; continue; }
        const mxArray *val = prhs[idx + 1];
        bool known = true;
        if (strcmp(name, "pitchMethod") == 0) mxGetString(val, p.method, sizeof(p.method));
        else if (strcmp(name, "timeStep") == 0) p.timeStep = mxGetScalar(val);
        else if (strcmp(name, "pitchFloor") == 0) p.pitchFloor = mxGetScalar(val);
        else if (strcmp(name, "pitchCeiling") == 0) p.pitchCeiling = mxGetScalar(val);
        else if (strcmp(name, "maxnCandidates") == 0) p.maxnCandidates = (integer)mxGetScalar(val);
        else if (strcmp(name, "veryAccurate") == 0) p.veryAccurate = (mxGetScalar(val) != 0.0);
        else if (strcmp(name, "periodsPerWindow") == 0) p.periodsPerWindow = mxGetScalar(val);
        else if (strcmp(name, "silenceThreshold") == 0) p.silenceThreshold = mxGetScalar(val);
        else if (strcmp(name, "voicingThreshold") == 0) p.voicingThreshold = mxGetScalar(val);
        else if (strcmp(name, "octaveCost") == 0) p.octaveCost = mxGetScalar(val);
        else if (strcmp(name, "octaveJumpCost") == 0) p.octaveJumpCost = mxGetScalar(val);
        else if (strcmp(name, "voicedUnvoicedCost") == 0) p.voicedUnvoicedCost = mxGetScalar(val);
        else if (strcmp(name, "attenuationAtTop") == 0) p.attenuationAtTop = mxGetScalar(val);
        else if (strcmp(name, "shsHighestFrequency") == 0) p.shsHighestFrequency = mxGetScalar(val);
        else if (strcmp(name, "shsMaxnSubharmonics") == 0) p.shsMaxnSubharmonics = (integer)mxGetScalar(val);
        else if (strcmp(name, "shsCompressionFactor") == 0) p.shsCompressionFactor = mxGetScalar(val);
        else if (strcmp(name, "shsPointsPerOctave") == 0) p.shsPointsPerOctave = (integer)mxGetScalar(val);
        else if (strcmp(name, "spinetWindowLength") == 0) p.spinetWindowLength = mxGetScalar(val);
        else if (strcmp(name, "spinetMinFreq") == 0) p.spinetMinFreq = mxGetScalar(val);
        else if (strcmp(name, "spinetMaxFreq") == 0) p.spinetMaxFreq = mxGetScalar(val);
        else if (strcmp(name, "spinetNumFilters") == 0) p.spinetNumFilters = (integer)mxGetScalar(val);
        else known = false;
        mxFree((void *)name);
        if (!known) break;
        idx += 2;
    }
}

void parsePointProcessParams(const mxArray **prhs, int nrhs, int &idx, PraatPointProcessParams &p) {
    while (idx + 1 < nrhs) {
        const char *name = mxArrayToString(prhs[idx]);
        if (!name) { idx += 2; continue; }
        const mxArray *val = prhs[idx + 1];
        bool known = true;
        if (strcmp(name, "ppMethod") == 0) mxGetString(val, p.method, sizeof(p.method));
        else if (strcmp(name, "includeMaxima") == 0) p.includeMaxima = (mxGetScalar(val) != 0.0);
        else if (strcmp(name, "includeMinima") == 0) p.includeMinima = (mxGetScalar(val) != 0.0);
        else known = false;
        mxFree((void *)name);
        if (!known) break;
        idx += 2;
    }
}

bool parsePitchParam(const char *name, const mxArray *val, PraatPitchParams &p) {
    if (strcmp(name, "pitchMethod") == 0) { mxGetString(val, p.method, sizeof(p.method)); return true; }
    if (strcmp(name, "timeStep") == 0) { p.timeStep = mxGetScalar(val); return true; }
    if (strcmp(name, "pitchFloor") == 0) { p.pitchFloor = mxGetScalar(val); return true; }
    if (strcmp(name, "pitchCeiling") == 0) { p.pitchCeiling = mxGetScalar(val); return true; }
    if (strcmp(name, "maxnCandidates") == 0) { p.maxnCandidates = (integer)mxGetScalar(val); return true; }
    if (strcmp(name, "veryAccurate") == 0) { p.veryAccurate = (mxGetScalar(val) != 0.0); return true; }
    if (strcmp(name, "periodsPerWindow") == 0) { p.periodsPerWindow = mxGetScalar(val); return true; }
    if (strcmp(name, "silenceThreshold") == 0) { p.silenceThreshold = mxGetScalar(val); return true; }
    if (strcmp(name, "voicingThreshold") == 0) { p.voicingThreshold = mxGetScalar(val); return true; }
    if (strcmp(name, "octaveCost") == 0) { p.octaveCost = mxGetScalar(val); return true; }
    if (strcmp(name, "octaveJumpCost") == 0) { p.octaveJumpCost = mxGetScalar(val); return true; }
    if (strcmp(name, "voicedUnvoicedCost") == 0) { p.voicedUnvoicedCost = mxGetScalar(val); return true; }
    if (strcmp(name, "attenuationAtTop") == 0) { p.attenuationAtTop = mxGetScalar(val); return true; }
    if (strcmp(name, "shsHighestFrequency") == 0) { p.shsHighestFrequency = mxGetScalar(val); return true; }
    if (strcmp(name, "shsMaxnSubharmonics") == 0) { p.shsMaxnSubharmonics = (integer)mxGetScalar(val); return true; }
    if (strcmp(name, "shsCompressionFactor") == 0) { p.shsCompressionFactor = mxGetScalar(val); return true; }
    if (strcmp(name, "shsPointsPerOctave") == 0) { p.shsPointsPerOctave = (integer)mxGetScalar(val); return true; }
    if (strcmp(name, "spinetWindowLength") == 0) { p.spinetWindowLength = mxGetScalar(val); return true; }
    if (strcmp(name, "spinetMinFreq") == 0) { p.spinetMinFreq = mxGetScalar(val); return true; }
    if (strcmp(name, "spinetMaxFreq") == 0) { p.spinetMaxFreq = mxGetScalar(val); return true; }
    if (strcmp(name, "spinetNumFilters") == 0) { p.spinetNumFilters = (integer)mxGetScalar(val); return true; }
    return false;
}

bool parsePointProcessParam(const char *name, const mxArray *val, PraatPointProcessParams &p) {
    if (strcmp(name, "ppMethod") == 0) { mxGetString(val, p.method, sizeof(p.method)); return true; }
    if (strcmp(name, "includeMaxima") == 0) { p.includeMaxima = (mxGetScalar(val) != 0.0); return true; }
    if (strcmp(name, "includeMinima") == 0) { p.includeMinima = (mxGetScalar(val) != 0.0); return true; }
    return false;
}

autoSound createSoundFromSamples(const double *samples, size_t nSamples, double fs, bool useFloat32) {
	if (useFloat32) {
		/*
		 * Write a temp float32 WAV file via plain C I/O, then read it back
		 * with Praat's own Sound_readFromSoundFile.  This goes through the
		 * exact same code path (bingetr32LE -> Sound z[]) that Praat.exe
		 * uses when reading WAV files, so the resulting Sound object is
		 * byte-identical to Praat.exe's.
		 */
		char tempPath[MAX_PATH];
		GetTempPathA(MAX_PATH, tempPath);
		strcat(tempPath, "praatmex_temp.wav");

		/* Write standard 32-bit float WAV (mono, IEEE float) */
		{
			FILE *f = fopen(tempPath, "wb");
			if (!f)
				mexErrMsgIdAndTxt("praatmex:sound:tempfile",
					"Cannot create temporary WAV file.");

			int32 nSamps32   = (int32) nSamples;
			int32 sampleRate  = (int32) fs;
			int32 byteRate    = sampleRate * 1 * 4;
			int16 blockAlign  = 1 * 4;
			int16 bitsPerSamp = 32;
			int32 dataSize    = (int32) (nSamples * 4);

			/* RIFF header */
			fwrite("RIFF", 1, 4, f);
			int32 riffSize = 36 + dataSize;
			fwrite(&riffSize, 4, 1, f);
			fwrite("WAVE", 1, 4, f);

			/* fmt chunk */
			fwrite("fmt ", 1, 4, f);
			int32 fmtSize = 16;
			fwrite(&fmtSize, 4, 1, f);
			int16 audioFmt = 3;  /* IEEE float */
			fwrite(&audioFmt, 2, 1, f);
			int16 nCh = 1;
			fwrite(&nCh, 2, 1, f);
			fwrite(&sampleRate, 4, 1, f);
			fwrite(&byteRate, 4, 1, f);
			fwrite(&blockAlign, 2, 1, f);
			fwrite(&bitsPerSamp, 2, 1, f);

			/* data chunk */
			fwrite("data", 1, 4, f);
			fwrite(&dataSize, 4, 1, f);
			for (size_t i = 0; i < nSamples; i++) {
				float val = (float) samples[i];
				fwrite(&val, sizeof(float), 1, f);
			}
			fclose(f);
		}

		/* Read back with Praat's own WAV reader */
		structMelderFile structFile { };
		MelderFile_setToNull(& structFile);
		Melder_pathToFile(Melder_8to32_e(tempPath).get(), & structFile);
		autoSound sound;
		try {
			sound = Sound_readFromSoundFile(& structFile);
		} catch (...) {
			MelderFile_delete(& structFile);   /* clean up temp file on error */
			throw;
		}
		MelderFile_delete(& structFile);   /* removes the temp file from disk */
		return sound;
	}
	autoSound sound = Sound_createSimple(1, (double)nSamples / fs, fs);
	for (long i = 1; i <= (long)nSamples; i++)
		sound->z[1][i] = samples[i - 1];
	return sound;
}

autoPitch createPitchFromSound(Sound sound, const PraatPitchParams &p) {
    autoPitch pitch;

    if (strcmp(p.method, "ac") == 0) {
        pitch = Sound_to_Pitch_rawAc(sound, p.timeStep, p.pitchFloor, p.pitchCeiling,
            p.maxnCandidates, p.veryAccurate, p.silenceThreshold, p.voicingThreshold,
            p.octaveCost, p.octaveJumpCost, p.voicedUnvoicedCost);
    } else if (strcmp(p.method, "cc") == 0) {
        pitch = Sound_to_Pitch_rawCc(sound, p.timeStep, p.pitchFloor, p.pitchCeiling,
            p.maxnCandidates, p.veryAccurate, p.silenceThreshold, p.voicingThreshold,
            p.octaveCost, p.octaveJumpCost, p.voicedUnvoicedCost);
    } else if (strcmp(p.method, "filtered_ac") == 0) {
        pitch = Sound_to_Pitch_filteredAc(sound, p.timeStep, p.pitchFloor, p.pitchCeiling,
            p.maxnCandidates, p.veryAccurate, p.attenuationAtTop, p.silenceThreshold, p.voicingThreshold,
            p.octaveCost, p.octaveJumpCost, p.voicedUnvoicedCost);
    } else if (strcmp(p.method, "filtered_cc") == 0) {
        pitch = Sound_to_Pitch_filteredCc(sound, p.timeStep, p.pitchFloor, p.pitchCeiling,
            p.maxnCandidates, p.veryAccurate, p.attenuationAtTop, p.silenceThreshold, p.voicingThreshold,
            p.octaveCost, p.octaveJumpCost, p.voicedUnvoicedCost);
    } else if (strcmp(p.method, "shs") == 0) {
        pitch = Sound_to_Pitch_shs(sound, p.timeStep, p.pitchFloor,
            p.shsHighestFrequency, p.pitchCeiling, p.shsMaxnSubharmonics,
            p.maxnCandidates, p.shsCompressionFactor, p.shsPointsPerOctave);
    } else if (strcmp(p.method, "spinet") == 0) {
        pitch = Sound_to_Pitch_SPINET(sound, p.timeStep, p.spinetWindowLength,
            p.spinetMinFreq, p.spinetMaxFreq, p.spinetNumFilters,
            p.pitchCeiling, p.maxnCandidates);
    } else {
        mexErrMsgIdAndTxt("praatmex:pitch:method",
            "Unknown pitch method '%s'. Use 'ac', 'cc', 'filtered_ac', 'filtered_cc', 'shs', or 'spinet'.", p.method);
    }
    return pitch;
}

autoPitch createPitch(const double *samples, size_t nSamples, double fs, const PraatPitchParams &p) {
    autoSound sound = createSoundFromSamples(samples, nSamples, fs);
    return createPitchFromSound(sound.get(), p);
}

autoPointProcess createPointProcess(const double *samples, size_t nSamples, double fs,
    const PraatPitchParams &pp, const PraatPointProcessParams &ppp)
{
    autoSound sound = createSoundFromSamples(samples, nSamples, fs);
    autoPitch pitch = createPitchFromSound(sound.get(), pp);
    autoPointProcess pt;

    bool isShsOrSpinet = (strcmp(pp.method, "shs") == 0 || strcmp(pp.method, "spinet") == 0);

    if (strcmp(ppp.method, "periodic_cc") == 0) {
        if (isShsOrSpinet) {
            pt = Pitch_to_PointProcess(pitch.get());
        } else {
            pt = Sound_Pitch_to_PointProcess_cc(sound.get(), pitch.get());
        }
    } else if (strcmp(ppp.method, "periodic_peaks") == 0) {
        if (isShsOrSpinet) {
            pt = Pitch_to_PointProcess(pitch.get());
        } else {
            pt = Sound_Pitch_to_PointProcess_peaks(sound.get(), pitch.get(),
                ppp.includeMaxima, ppp.includeMinima);
        }
    } else if (strcmp(ppp.method, "pitch_only") == 0) {
        pt = Pitch_to_PointProcess(pitch.get());
    } else if (strcmp(ppp.method, "extrema") == 0) {
        pt = Sound_to_PointProcess_extrema(sound.get(), 1,
            kVector_peakInterpolation::PARABOLIC, ppp.includeMaxima, ppp.includeMinima);
    } else if (strcmp(ppp.method, "periodic_direct") == 0) {
        pt = Sound_to_PointProcess_periodic_cc(sound.get(), pp.pitchFloor, pp.pitchCeiling);
    } else {
        mexErrMsgIdAndTxt("praatmex:pointprocess:method",
            "Unknown PP method '%s'. Use 'periodic_cc', 'periodic_peaks', 'pitch_only', 'extrema', or 'periodic_direct'.",
            ppp.method);
    }
    return pt;
}
