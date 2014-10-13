function pfs_write_luminance( fileName, img )
%PFS_WRITE_LUMINANCE writes an image file
% 
% PFS_WRITE_LUMINANCE ( file_name, img )
%
% img - 2D matrix representing luminance values
%
% The format of the file is recognized based in the file name extension:
% .hdr for Radiance images, .exr for OpenEXR, .jpg for JPEG and .png for
% PNG. See manual of "pfsout" shell command for the full list of the
% supported formats. 
%
% See also: PFS_WRITE_IMAGE, PFS_WRITE_RGB, PFS_WRITE_XYZ,
% PFS_READ_IMAGE.
%
% Copyright 2009 Rafal Mantiuk

if( nargin ~= 2 )
    error( 'pfs_write_luminance: improper usage' );
end

%cmd = sprintf( '%spfsout %s%s', pfs_shell(), fileName, pfs_shell(1) )
fid = pfspopen( sprintf( '%spfsout ''%s''%s', pfs_shell(), fileName, pfs_shell(1) ), 'w' );    
pfs = pfsopen( fid, size( img ) );

pfs.channels.Y = img;

pfsput( pfs );
pfsclose( pfs );
pfspclose( fid );

end
