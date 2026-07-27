# Praats/src/test/speed/transcribe.praat
# Paul Boersma, 10 May 2026

# This script measures how fast transcription can work on an x64 processor.
# The speed may depend on the following settings:
# -mavx2 -mfma -mf16c

Debug: "no", 2001
maximumNumberOfThreads = 8
Debug multi-threading: "yes", maximumNumberOfThreads, 0, "no"

sound = Read from file: "chaintje.wav"
textgrid = To TextGrid: "tier", ""
selectObject: sound, textgrid
t = clock()
Transcribe interval: 1, 1, "yes", "no", "yes", 0.5, 0.1, 0.25, 0.03, "ggml-base.bin", "German",
... 3, 0, 0, 0, 0.7045654963945799, 90
writeInfoLine: fixed$ (clock() - t, 3)

removeObject: sound, textgrid
Debug: "no", 0
Debug multi-threading: "yes", 0, 0, "no"

# Praat for Mac on Paul's 2023 Mac with M3 Max processor ("Report system properties" reports 16 threads)
#     (128 GB RAM, 12 performance cores, 4 efficiency cores):
# 18 threads: 344.529 seconds
# 16 threads with Parallels Desktop Windows 11 (with 8 threads) on: 23.401 / 13.726 / 16.767 / 4.720 / 14.063 seconds
# 16 threads: 1.923 / 2.219 / 1.923 / 1.705 / 1.756 seconds  5.304 / 2.132 / 16.768 / 12.657 / 21.550
# 12 threads: 1.744 / 1.664 / 1.523 / 1.504 / 1.502 seconds
# 8 threads: 1.843 / 1.907 / 1.911 / 1.825 / 1.807 seconds  1.818 / 1.811 / 1.811 / 1.808 / 1.736
# 4 threads: 2.646 / 2.643 / 2.652 / 2.632 / 2.634 seconds
#
# Conclusion: we should take the number of performance threads (12),
# and if we don't know that, we can take half of the number of cores (i.e. 8).

# Praat for ARM64 under Windows 11 under 8-thread Parallels Desktop on Paul's Mac: 3.993 / 3.463 / 3.531 / 3.856 / 3.817 seconds

# Praat for old x64 under Windows 11 under 8-thread Parallels Desktop on Paul's Mac: 20.681 / 20.894 / 32.526 / 20.506 / 21.465 seconds

# Praat for old x64 under Windows 11 on a Lenovo Intel Core Ultra 9 285H processor ("Report system properties" reports 16 threads)
#     (2.90 GHz CPU, 32 GB RAM, 8 GB GPU, 6 Performance-cores, 8 Efficient-cores, 2 Low Power Efficient-cores):
# 16 threads: 838.454 seconds
# 8 threads: 11.112 / 11.212 seconds
# 6 threads: 13.626 seconds
# 4 threads: 20 seconds
#
# Conclusion: we could take the number of performance threads (6),
# or apparently we can also take half the number of reported threads (i.e. 8), for some reason.

# same, compiled with -mavx2:
#
# 8 threads: 10.619 / 10.673 / 10.664 seconds
# 6 threads: 12.855 / 12.965 seconds
# 4 threads: 18.688 / 18.697 seconds
#
# Hardly faster!

# same, compiled with -mavx2 -mfma:
#
# 8 threads: 10.675 / 10.553 / 10.665 seconds
# 6 threads: 12.806 / 12.874 seconds
# 4 threads: 18.477 / 18.514 seconds
#
# Again hardly faster!

# same, compiled with -mavx2 -mfma -m16c:
#
# 8 threads: 2.692 / 2.611 / 2.596 seconds
# 6 threads: 2.914 / 2.917 / 2.876 seconds
# 4 threads: 3.800 / 3.743 / 3.769 seconds
#
# Much faster!