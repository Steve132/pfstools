function [varargout] = pfs_read_rgb( fileName )
%PFS_READ_RGB Read image file and return R, G, and B color channels or a single 3D matrix.
%
% [R G B] = PFS_READ_RGB( file_name )
% IMG = PFS_READ_RGB( file_name)
%
% R, G, B - red, green and blue color channels, given as linear response
% img - 3D matrix image, where img(:,:,1:3) represents red, blue and green
%       color channels
%
% PFS_READ_RGB accepts all formats recognized by the shell "pfsin"
% command.
%
% See also: PFS_READ_IMAGE, PFS_READ_LUMINANCE, PFS_READ_XYZ, PFS_WRITE_IMAGE.
%
% Copyright 2009 Rafal Mantiuk

  %Check if file exists
  fid = fopen( fileName, 'rb' );
  if( fid == -1 ) 
    error( 'pfs_read_rgb: File "%s" does not exist', fileName );
  end
  fclose( fid );

  fid = pfspopen( sprintf( '%spfsin ''%s''%s', pfs_shell(), fileName, pfs_shell( 1 ) ), 'r' );
  pin = pfsopen( fid );
  pin = pfsget( pin );

  if( isfield( pin.channels, 'X' ) && isfield( pin.channels, 'Z' ) )
      [R G B] = pfs_transform_colorspace( 'XYZ', pin.channels.X, pin.channels.Y, pin.channels.Z, 'RGB' );
  elseif( isfield( pin.channels, 'Y' ) )
      R = pin.channels.Y;
      G = pin.channels.Y;
      B = pin.channels.Y;
  else
      error( 'Color channels missing in the pfs frame' );
  end
  
  if( nargout == 3 )
      varargout{1} = R;
      varargout{2} = G;
      varargout{3} = B;
  elseif( nargout  == 1 )
      varargout{1}(:,:,1) = R;
      varargout{1}(:,:,2) = G;
      varargout{1}(:,:,3) = B;
  else
      error( 'pfs_read_rgb: wrong number of output arguments' );
  end
  
  pfsclose( pin );
  % TODO: Check why crashes on windows
  if ~ispc()
      pfspclose( fid );
  end
end
