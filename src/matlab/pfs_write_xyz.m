function pfs_write_xyz( fileName, varargin )
%PFS_WRITE_XYZ write an XYZ image file.
% 
% PFS_WRITE_XYZ( file_name, X, Y, Z )
% PFS_WRITE_XYZ( file_name, img )
%
% X, Y, Z - XYZ color channels, given as linear response
% img - 3D matrix image, where img(:,:,1:3) represents XYZ color channels
%
% The format of the file is recognized based in the file name extension:
% .hdr for Radiance images, .exr for OpenEXR, .jpg for JPEG and .png for
% PNG. See manual of "pfsout" shell command for the full list of the
% supported formats. 
%
% See also: PFS_WRITE_IMAGE, PFS_WRITE_LUMINANCE, PFS_WRITE_RGB,
% PFS_READ_IMAGE.
%
% Copyright 2009 Rafal Mantiuk

%cmd = sprintf( '%spfsout %s%s', pfs_shell(), fileName, pfs_shell(1) )
fid = pfspopen( sprintf( '%spfsout ''%s''%s', pfs_shell(), fileName, pfs_shell(1) ), 'w' );    
pfs = pfsopen( fid, [size( varargin{1}, 1 ) size( varargin{1}, 2 )] );

if( nargin == 4 )
    if( ~isnumeric(varargin{1}) || ~isnumeric(varargin{2}) || ~isnumeric(varargin{1}) )
        error( 'pfs_write_xyz: matrices of the equal size expected as an arguments' );
    end
    pfs.channels.X = single(varargin{1});
    pfs.channels.Y = single(varargin{2});
    pfs.channels.Z = single(varargin{3});
elseif( nargin == 2 && ndims(varargin{1}) == 3 ) 
    pfs.channels.X = single(varargin{1}(:,:,1));
    pfs.channels.Y = single(varargin{1}(:,:,2));
    pfs.channels.Z = single(varargin{1}(:,:,3));
else
    error( 'pfs_write_xyz: improper usage' );
end

pfsput( pfs );
pfsclose( pfs );
pfspclose( fid );

end
