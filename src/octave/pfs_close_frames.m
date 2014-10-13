function pfs_close_frames( pfs_struct )
  ## Close pfs stream opened with pfs_open_frames. Removes all temporary
  ## files.
  ##
  ## usage: pfs_close_frames( pfs_struct )
  ##

  if( !isfield( pfs_struct, "fid" ) )
    error( "pfs_close_frames: pfs_struct wasn't opened with pfs_open_frames\n" );
  endif

  pfsclose( pfs_struct );
  fclose( pfs_struct.fid );
  
endfunction
