function [X Y Z] = pfs_read_xyz( fileName )
  ## Read hdr image file (formats accepted by pfsin) and return X, Y,
  ## and Z color channels.
  ##
  ## [X Y Z] = hdr_read_rgb( fileName )
  
  ## Check if file exists
  fid = fopen( fileName, "rb" );
  if( fid == -1 ) 
    error( sprintf( "pfs_read_xyz: File '%s' does not exist", fileName ) );
  endif
  fclose( fid );

  unwind_protect
    fid = popen( sprintf( "pfsin %s", fileName ), "r" );
    pin = pfsopen( fid );      
    pin = pfsget( pin );
    X = pin.channels.X;
    Y = pin.channels.Y;
    Z = pin.channels.Z;
  unwind_protect_cleanup
    pfsclose( pin );
    fclose( fid );
  end_unwind_protect    
    
