function [R G B] = pfs_read_rgb( fileName )
  ## Read hdr image file (formats accepted by pfsin) and return R, G,
  ## and B color channels.
  ##
  ## [R G B] = hdr_read_rgb( fileName )
  
  ## Check if file exists
  fid = fopen( fileName, "rb" );
  if( fid == -1 ) 
    error( sprintf( "pfs_read_rgb: File '%s' does not exist", fileName ) );
  endif
  fclose( fid );

  unwind_protect
    fid = popen( sprintf( "pfsin %s", fileName ), "r" );
    pin = pfsopen( fid );      
    pin = pfsget( pin );
    [R G B] = pfstransform_colorspace( "XYZ", pin.channels.X, 
				      pin.channels.Y, pin.channels.Z, \
                                      "RGB" );    
  unwind_protect_cleanup
    pfsclose( pin );
    fclose( fid );
  end_unwind_protect    
    
endfunction
