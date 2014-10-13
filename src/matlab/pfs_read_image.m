function img = pfs_read_image( fileName )
%PFS_READ_IMAGE Read image file and return RGB, luminance or multichannel matrix.
%
% IMG = PFS_READ_IMAGE( file_name )
%
% If input is a gray-scale or luminance image, IMG is a 2D matrix. If input is
% a color image, IMG(:,:,k) represents red, blue and green color channels for k=1,2 and 3.
% If input is a multi-channel image (channel names C1, C2, ..., Cn), IMG is a
% 3D matrix with 3rd dimension corresponding to the channels. 
%
% PFS_READ_IMAGE accepts all formats recognized by the shell "pfsin"
% command.
%
% Example: 
%   img = PFS_READ_IMAGE( 'hdr_images/memorial.exr' );
%
% See also: PFS_READ_RGB, PFS_READ_LUMINANCE, PFS_READ_XYZ,
% PFS_WRITE_IMAGE, PFSVIEW.
%
% Copyright 2009 Rafal Mantiuk

  %Check if file exists
  fid = fopen( fileName, 'rb' );
  if( fid == -1 ) 
    error( 'pfs_read_image: File "%s" does not exist', fileName );
  end
  fclose( fid );

  fid = pfspopen( sprintf( '%spfsin ''%s''%s', pfs_shell(), fileName, pfs_shell( 1 ) ), 'r' );
  pin = pfsopen( fid );
  pin = pfsget( pin );

  if( isfield( pin.channels, 'X' ) && isfield( pin.channels, 'Z' ) )
      img = pfs_transform_colorspace( 'XYZ', pin.channels.X, pin.channels.Y, pin.channels.Z, 'RGB' );
  elseif( isfield( pin.channels, 'Y' ) )
      img = pin.channels.Y;
  elseif( isfield( pin.channels, 'C1' ) )
      ch=1;
      % count the number of channels
      while( isfield( pin.channels, sprintf( 'C%d', ch ) ) )
          ch = ch+1;
      end
      ch_max = ch-1;
      img = zeros(pin.rows, pin.columns, ch_max);
      for ch=1:ch_max
          img(:,:,ch) = pin.channels.(sprintf( 'C%d', ch ));
      end
  else
      error( 'Color channels missing in the pfs frame' );
  end  
  
  pfsclose( pin );
  % TODO: Check why crashes on windows
  if ~ispc()
      pfspclose( fid );
  end
end
