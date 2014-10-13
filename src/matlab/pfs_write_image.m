function pfs_write_image( file_name, img, options )
%PFS_WRITE_IMAGE write an RGB, luminance or multichannel image to a file.
%
% PFS_WRITE_IMAGE( file_name, IMG [, options] )
%
% Writes a luminance image if IMG is a 2D matrix, an RGB image if
% size(IMG,3) == 3, and a multi-channel image otherwise. Each channel in a
% multi-channel image has a name C1, C2, .., Cn. 
%
% 'options' passes a string with additional options recognized by each of
% the image format writers. For example '--compression=PXR24' modifies the
% compression algorithm for OpenEXR files. See the manual pages of the
% "pfsout*" shell commands for the list of supported options. 
%
% The format of the file is recognized based in the file name extension:
% .hdr for Radiance images, .exr for OpenEXR, .jpg for JPEG and .png for
% PNG. See manual of "pfsout" shell command for the full list of the
% supported formats. 
%
% Currently only OpenEXR and PFS formats support multi-channel files. 
%
% Pass single precission matrices for better performance. 
%
% Example:
%   pfs_write_image( 'image.exr', cat( 3, R, G, B ) );
% 
%   R, G, and B are 2D matrices with red, green and blue channel data.
%
% See also: PFS_WRITE_RGB, PFS_WRITE_LUMINANCE, PFS_WRITE_XYZ,
% PFS_READ_IMAGE, PFSVIEW.
%
% Copyright 2009 Rafal Mantiuk

img_sz = size( img );
dims = length( img_sz );

if( dims > 3 )
    error( 'image matrix has too many dimenstions' );
end

if( ~exist( 'options', 'var' ) )
    options = '';
end

fid = pfspopen( sprintf( '%spfsout ''%s'' %s%s', pfs_shell(), file_name, options, pfs_shell(1) ), 'w' );    
pfs = pfsopen( fid, [img_sz(1) img_sz(2)] );

if( dims == 2 || img_sz(3) == 1 )
    pfs.channels.Y = single(img);
elseif( dims == 3 && img_sz(3) == 3 )
    [pfs.channels.X pfs.channels.Y pfs.channels.Z] = pfs_transform_colorspace( 'RGB', img, 'XYZ' );    
else 
    for k=1:img_sz(3)
        pfs.channels.(sprintf('C%d', k)) = single(img(:,:,k));
    end
end

pfsput( pfs );
pfsclose( pfs );
pfspclose( fid );

end
