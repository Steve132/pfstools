% Write frame to the pfs stream. See also help for pfsopen function.
% 
% usage: pfsput( pfs_struct );
%   pfs_struct - the structure returned by pfsopen. The stream must be open for writing
% 
% Typical usage:
% 
% ## Create you image 
% Y = ones( 512, 512 );
% Y(256,:) = 10;
% 
% ## Create pfs stream for writing
% pout = pfsopen( \"output.pfs\", size( Y ) );
% 
% ## Add channels
% pout.channels.Y = Y;
% 
% ## Write frame. You can 'put' multiple frames.
% pfsput( pout );
% 
% ##Close stream
% pfsclose( pout )
