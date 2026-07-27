addpath(fullfile(fileparts(mfilename('fullpath')), 'build', 'Release'));
fprintf('path added\n');
try
    praatmex('init');
    fprintf('init OK\n');
catch e
    fprintf('init FAILED: %s\n', e.message);
end
