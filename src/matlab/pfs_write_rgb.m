function pfs_write_rgb( fileName, varargin )
%PFS_WRITE_RGB write an RGB image file.
% 
% PFS_WRITE_RGB( file_name, R, G, B )
% PFS_WRITE_RGB( file_name, img )
%
% R, G, B - red, green and blue color channels, given as linear response
% img - 3D matrix image, where img(:,:,1:3) represents red, blue and green
%       color channels
%
% The format of the file is recognized based in the file name extension:
% .hdr for Radiance images, .exr for OpenEXR, .jpg for JPEG and .png for
% PNG. See manual of "pfsout" shell command for the full list of the
% supported formats. 
%
% See also: PFS_WRITE_IMAGE, PFS_WRITE_LUMINANCE, PFS_WRITE_XYZ,
% PFS_READ_IMAGE.
%
% Copyright 2009 Rafal Mantiuk

if( nargin == 4 )
    if( ~isnumeric(varargin{1}) || ~isnumeric(varargin{2}) || ~isnumeric(varargin{1}) )
        error( 'pfs_write_rgb: matrices of the equal size expected as an arguments' );
    end
    [X Y Z] = pfs_transform_colorspace( 'RGB', varargin{1}, varargin{2}, varargin{3}, 'XYZ' );    
elseif( nargin == 2 && ndims(varargin{1}) == 3 ) 
    [X Y Z] = pfs_transform_colorspace( 'RGB', varargin{1}(:,:,1), varargin{1}(:,:,2), varargin{1}(:,:,3), 'XYZ' );
else
    error( 'pfs_write_rgb: improper usage' );
end

%cmd = sprintf( '%spfsout %s%s', pfs_shell(), fileName, pfs_shell(1) )
fid = pfspopen( sprintf( '%spfsout ''%s''%s', pfs_shell(), fileName, pfs_shell(1) ), 'w' );    
pfs = pfsopen( fid, size( X ) );

pfs.channels.X = X;
pfs.channels.Y = Y;
pfs.channels.Z = Z;

pfsput( pfs );
pfsclose( pfs );
pfspclose( fid );

end
