function pfs_struct = pfs_open_frames( filePattern, frameSize )
  ## Open frames / image in one of the suppoted formats for reading or
  ## writing. Reading or writting frames is donw with pfsput or pfsget.
  ##
  ## usage: pfs_struct = pfs_open_frames( filePattern, size )
  ##        pfs_struct = pfs_open_frames( filePattern )
  ##
  ## This command is equivalent to pfsopen, but can handle files in any
  ## format supported by pfstools. 'filePattern' can specify files,
  ## frames, including '--frames' and '--skip-missing', similarly as
  ## 'pfsin' / 'pfsout' commands. You can pass also additional options,
  ## like --compression=RLE for exr files in filePattern. 
  ## All option that take an argument (except --frames) must given in
  ## the form --option=value, that is without a space between an
  ## option and its argument.
  ##
  ## pfs_close_frames should be used instead of pfs_close to close pfsstream.
  ##
  
  doWrite = exist( "frameSize" );
 
  try
    if( doWrite ) 
      execStr = sprintf( "pfsout %s", filePattern );
      fid = popen( execStr, "w" );
      pfs_struct = pfsopen( fid, frameSize );
      pfs_struct.fid = fid;
    else
      execStr = sprintf( "pfsin %s", filePattern );
      fid = popen( execStr, "r" );
      pfs_struct = pfsopen( fid );
      pfs_struct.fid = fid;
    endif
  catch
    fclose( pfs_struct.fid );
    error( [ "pfs_open_frames: " __error_text__ ] );
  end_try_catch      
    
endfunction
