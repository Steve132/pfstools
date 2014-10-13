function pfs_write_rgb( fileName, R, G, B )
  ## Write an hdr image file (formats accepted by pfsout). You can
  ## specify additional options in fileName, such as
  ## "--compression=PXR24" for OpenEXR files. Check manual pages of
  ## pfsout* commands for the list of available options. 
  ## 
  ##  pfs_write_rgb( fileName, R, G, B )
 
  unwind_protect
    fid = popen( sprintf( "pfsout %s", fileName ), "w" );
    
    pfs = pfsopen( fid, size( R ) );
    [pfs.channels.X pfs.channels.Y pfs.channels.Z] = \
        pfstransform_colorspace( "RGB", R, G, B, "XYZ" );   
    pfsput( pfs );
  unwind_protect_cleanup
    pfsclose( pfs );
    fclose( fid );
  end_unwind_protect    
      
endfunction
