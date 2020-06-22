% readLua.m Help file for readLua MEX-file 
%
% readLua reads symbols from a lua script
%   S = readLua(filname, cellWithSymbols)
%   opens the lua file 'filename', executes it and reads
%   all symbols from it, that are specified in the cell
%   array 'cellWithSymbols'. It returns a struct with fields
%   whose names are the same as the strings in cellWithSymbols
%   and whose values are read from the lua file.
%
%   Created with:
%   MATLAB 9.0
%   (R2016a) %          Platform: glnxa64
%     MEX extension: mexa64
%          Compiler: gcc

%   MEX-File function.
