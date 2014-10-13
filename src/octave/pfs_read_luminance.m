function Y = pfs_read_luminance( fileName )
  ## Read hdr image file (formats accepted by pfsin) and return
  ## luminance channel Y.
  ##
  ##  Y = hdr_read_luminance( fileName )

  ## Check if file exists
  fid = fopen( fileName, "rb" );
  if( fid == -1 ) 
    error( sprintf( "pfs_read_luminance: File '%s' does not exist", fileName ) );
  endif
  fclose( fid );

  unwind_protect
    fid = popen( sprintf( "pfsin %s", fileName ), "r" );
    pin = pfsopen( fid );      
    pin = pfsget( pin );
    Y = pin.channels.Y;  

  unwind_protect_cleanup
    pfsclose( pin );
    fclose( fid );
  end_unwind_protect    
    
endfunction
