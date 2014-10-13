% Execute the command line and create a fifo pipe for reading/writing
% output/input. Used internally by pfstools.
%
% fid = pfspopen( command_line, mode )
% 
%   command_line - shell command line that should be executed
%   mode - either 'r' for reading output, or 'w' for writing input
%   fid - file ID, which should be passed to pfsopen
%
% Always use pfspclose() to close the pipe created with pfspopen
