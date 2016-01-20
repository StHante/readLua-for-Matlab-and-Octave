% Build readLua
%
% Simply edit the next cell according to your system and run this file in
% order to build readLua.

%% This cell needs to be edited

% These standard options apply to an Ubuntu 14.04.2 64bit with installed
% package 'liblua5.2-dev' and mex is set up to compile C language using
% the compiler gcc.
luaHeadersDir = '/usr/include/lua5.2/';
luaLibDir = '/usr/lib/x86_64-linux-gnu/';
luaLibName = 'lua5.2';

%% Find out if we are in Matlab or in Octave

if exist('OCTAVE_VERSION', 'builtin')
   isoctave = true;
else
   isoctave = false;
end


%% Build mex file

if isoctave
   mex(             '-v', ... % Be verbose
    ['-I' luaHeadersDir], ... % Where to look for headers
             'readLua.c', ... % Actual file to compile
        ['-L' luaLibDir], ... % Where to look for libraries
       ['-l' luaLibName]  );  % Which library to use
else
   mex(             '-v', ... % Be verbose
       '-largeArrayDims', ... % In Matlab: Don't use compatible mode,
                          ... % which will be deprecated in the future.
    ['-I' luaHeadersDir], ... % Where to look for headers
             'readLua.c', ... % Actual file to compile
        ['-L' luaLibDir], ... % Where to look for libraries
       ['-l' luaLibName]  );  % Which library to use
end   


 
 
%% Run test
 
% Execute newly-built mex function readLua 
out = readLua('test.cfg', ...
              {'number', 'array', 'string'});

% Test if the output is as it should be
if       ~isstruct(out) ...
      || ~isfield(out,'number') ...
      || ~isnumeric(out.number) ...
      || out.number ~= 135 ...
      || ~isfield(out,'array') ...
      || ~isnumeric(out.array) ...
      || length(out.array) ~= 4 ...
      || any(out.array(:) ~= [127; 127; 128; 129]) ...
      || ~isfield(out,'string') ...
      || ~ischar(out.string) ...
      || ~strcmp(out.string, 'asd')
   error('readLua:make:failedTest', 'Failed simple test.');
end

% Print nice success message
fprintf('\nTest passed!\n');

%% Create help function

fh = fopen('readLua.m','w');

fprintf(fh,'%% readLua.m Help file for readLua MEX-file \n');
fprintf(fh,'%%\n');
fprintf(fh,'%% readLua reads symbols from a lua script\n');
fprintf(fh,'%%   S = readLua(filname, cellWithSymbols)\n');
fprintf(fh,'%%   opens the lua file ''filename'', executes it and reads\n');
fprintf(fh,'%%   all symbols from it, that are specified in the cell\n');
fprintf(fh,'%%   array ''cellWithSymbols''. It returns a struct with fields\n');
fprintf(fh,'%%   whose names are the same as the strings in cellWithSymbols\n');
fprintf(fh,'%%   and whose values are read from the lua file.\n');
fprintf(fh,'%%\n');
fprintf(fh,'%%   Created with:\n');
if isoctave
   v = ver('octave');
   release = '';
else
   v = ver('matlab');
   release = v.Release;
end
fprintf(fh,'%%   %s %s\n', v.Name, v.Version, release);
fprintf(fh,'%%          Platform: %s\n', computer('arch'));
fprintf(fh,'%%     MEX extension: %s\n', mexext);
if ~isoctave
   fprintf(fh,'%%          Compiler: %s\n', getfield(mex.getCompilerConfigurations('C','Selected'),'Name'));
end   
fprintf(fh,'\n');
fprintf(fh,'%%   MEX-File function.\n');

fclose(fh);