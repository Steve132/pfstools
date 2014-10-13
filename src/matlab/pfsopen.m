% Open pfs stream for reading or writing. pfs is an interchange format for high dynamic range images (see http://pfstools.sourceforge.net).
% 
% usage: pfs_struct = pfsopen( fileName );
%        pfs_struct = pfsopen( fileName, rows, columns );
%        pfs_struct = pfsopen( fileName, [ rows columns ] );
%        pfs_struct = pfsopen( fid, ... );
%        
%   fileName - name of the file to read or write. \"stdin\" or \"stdout\" for standard input and output
%   rows - height of images to write
%   columns - width of images to write
% 
% The first usage of pfsopen opens pfs stream for reading, the second and
% the third for writing. pfsopen also accepts file descriptor returned from
% pfspopen, which can be used instead of a file name (the fourth usage). Use
% pfsget or pfsput to read or write frames or single images. You must close
% pfs stream with pfsclose. The stream will not be closed when pfs_struct
% is deleted (for example with 'clear pfs_struct').
% 
% pfs_struct is a structure that contains the following fields:
%   EOF - set to 1 if there are no more frames; 0 otherwise
%   FH - file handle of the file. For internal pruposes, do not use
%   MODE - file open mode: R - for reading, W - for writing
%   columns, rows - dimensions of each channel in the stream
%   channels - structure that contains channels represented as real matrices
%   tags - structure that contains tags represented as strings
%   channelTags - structure that contains a structure for each channel,
%         which contains tags. The format of the latter structure is the same as
%         for 'tags' field.  
