function varargout = pfssize( n_rows, n_cols, varargin )
  ## Resizes the set of matrices using pfssize. All matrices must be
  ## of the same size.
  ##
  ## Usage: [rchannel [,rchannel...]] = pfssize( rows, cols, [filter], channel [,channel...]  )
  ## n_rows - number of rows of the destination matrix
  ## n_cols - number of columns of the destination matrix
  ## filter - a string with the name of the filter to use. Recognized
  ##          names: BOX, LINEAR, MITCHEL
  ## channel - a real matrix
  ## rchannel - resized matrix

  if( !isscalar( n_rows ) || !isscalar( n_cols ) )
    error( "pfssize: bad arguments" );
    return
  endif
  
  execStr = "pfswrite( hdr_in_name";
  filter_type = "LINEAR";
  channel_num = 1;
  for i=1:length( varargin )
    if( ischar( varargin{i} ) )
      filter_type = varargin{i};
      continue;
    elseif( !ismatrix(varargin{i}) || !isreal(varargin{i}) )
      error( "pfssize: each channel must be a matrix of real-valued numbers" );
      return
    endif
    chStr = sprintf( ", \"ch%d\", varargin{%d}", channel_num++, i );
    execStr = [ execStr chStr ];
  endfor
  execStr = strcat( execStr, " );" );

  hdr_in_name = tmpnam();
  hdr_out_name = tmpnam();
  eval( execStr );

  cmd = sprintf ("pfssize -x %d -y %d --filter %s <%s >%s", \
                     n_cols, n_rows, filter_type, hdr_in_name, hdr_out_name );
  system( cmd );
  
  execStr = "pfsread( hdr_out_name";
  retStr = "[";
  channel_num = 1;
  for i=1:length( varargin )
    if( ischar( varargin{i} ) )
      continue;
    endif   
    if( channel_num!=1 )
      separator = ", ";
    else
      separator = "";
    endif
    retStr = [ retStr separator sprintf( "varargout{%d} ", channel_num ) ];
    execStr = [ execStr sprintf( ", \"ch%d\"", channel_num++ ) ];
  endfor
  retStr = [ retStr "] = " ];
  execStr = [ retStr execStr " );" ];
  eval( execStr );

  unlink( hdr_in_name );
  unlink( hdr_out_name );
  
endfunction
