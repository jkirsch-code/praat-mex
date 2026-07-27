addpath(fullfile(fileparts(mfilename('fullpath')), 'build', 'Release'));

% Generate a 1-second sine wave at 440 Hz
fs = 44100;
t = (0:1/fs:1-1/fs)';
sig = 0.5 * sin(2 * pi * 440 * t);

fprintf('Signal: %d samples, %d Hz\n', length(sig), fs);

% Test pitch
try
    p = praatmex('pitch', sig, fs);
    fprintf('Pitch: %d frames\n', numel(p.freq));
    valid = p.freq(p.freq > 0);
    if ~isempty(valid)
        fprintf('  Median pitch: %.1f Hz\n', median(valid));
    end
catch e
    fprintf('Pitch FAILED: %s\n', e.message);
end

% Test formant
try
    f = praatmex('formant', sig, fs);
    fprintf('Formant: %d values in freq field\n', numel(f.freq));
catch e
    fprintf('Formant FAILED: %s\n', e.message);
end

% Test intensity
try
    i = praatmex('intensity', sig, fs);
    fprintf('Intensity: %d frames, mean=%.1f dB\n', numel(i.intensity), mean(i.intensity));
catch e
    fprintf('Intensity FAILED: %s\n', e.message);
end

% Test harmonicity
try
    h = praatmex('harmonicity', sig, fs);
    fprintf('Harmonicity: %d frames, mean=%.3f\n', numel(h.harmonicity), mean(h.harmonicity));
catch e
    fprintf('Harmonicity FAILED: %s\n', e.message);
end

fprintf('\nAll tests done.\n');
