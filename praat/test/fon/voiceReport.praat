# voiceReport.praat
# Paul Boersma, 20 May 2026

#
# A sound without voicing should give "undefined" for many measurements.
#

sound = Create Sound from formula: "noise", 1, 0.0, 1.0, 44100, ~ randomGauss (0, 0.1)
pitch = To Pitch (raw cross-correlation): 0.0, 75.0, 600.0, 15, "no", 0.03, 0.45, 0.01, 0.35, 0.14
plusObject: sound
pulses = To PointProcess (cc)
plusObject: sound, pitch
voiceReport$ = Voice report: 0.0, 0.0, 75.0, 600.0, 1.1, 1.6, 0.03, 0.45
writeInfoLine: voiceReport$
removeObject: sound, pitch, pulses

assert extractNumber (voiceReport$, "Median pitch: ") = undefined
assert extractNumber (voiceReport$, "Mean pitch: ") = undefined
assert extractNumber (voiceReport$, "Median pitch: ") = undefined

