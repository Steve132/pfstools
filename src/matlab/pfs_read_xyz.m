function varargout = pfs_read_xyz( fileName )
%PFS_READ_XYZ read image file and return X, Y, and Z color channels.
%
% [X Y Z] = PFS_READ_XYZ( file_name )
% IMG = PFS_READ_XYZ( file_name )
%
% PFS_READ_XYZ accepts all formats recognized by the shell "pfsin"
% command.
%
% See also: PFS_READ_IMAGE, PFS_READ_LUMINANCE, PFS_READ_XYZ, PFS_WRITE_IMAGE.
%
% Copyright 2009 Rafal Mantiuk
  
  % Check if file exists
  fid = fopen( fileName, 'rb' );
  if( fid == -1 ) 
    error( 'pfs_read_xyz: File "%s" does not exist', fileName );
  end
  fclose( fid );

  fid = pfspopen( sprintf( '%spfsin ''%s''%s', pfs_shell(), fileName, pfs_shell( 1 ) ), 'r' );
  pin = pfsopen( fid );
  pin = pfsget( pin );
  pfsclose( pin );
  pfspclose( fid );
  
  if( nargout == 3 )
      varargout{1} = pin.channels.X;
      varargout{2} = pin.channels.Y;
      varargout{3} = pin.channels.Z;      
  elseif( nargout == 1 )
      varargout{1} = cat( 3, pin.channels.X, pin.channels.Y, pin.channels.Z );
  else
      error( 'Wrong number of output parameters' );
  end
    
end
  