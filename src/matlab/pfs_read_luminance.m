function Y = pfs_read_luminance( fileName )
%PFS_READ_LUMINANCE read image file and return luminance channel Y.
%
%  Y = PFS_READ_LUMINANCE( file_name )
%
% PFS_READ_LUMINANCE accepts all formats recognized by the shell "pfsin"
% command.
%
% See also: PFS_READ_IMAGE, PFS_READ_LUMINANCE, PFS_READ_XYZ, PFS_WRITE_IMAGE.
%
% Copyright 2009 Rafal Mantiuk

  % Check if file exists
  fid = fopen( fileName, 'r' );
  if( fid == -1 ) 
    error( 'pfs_read_luminance: File "%s" does not exist', fileName );
  end
  fclose( fid );
     
%  try
    cmd = sprintf( '%spfsin ''%s''%s', pfs_shell(), fileName, pfs_shell( 1 ) );
    fid = pfspopen( cmd, 'r' );
    pin = pfsopen( fid );
    pin = pfsget( pin );
    Y = pin.channels.Y;  
    pfsclose( pin );
    % TODO: Check why crashes on windows
    if ~ispc()
        pfspclose( fid );
    end
%  catch
 %   pfsclose( pin );
 %   pfspclose( fid );
%  end

end