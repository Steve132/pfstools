function pfsview( varargin )
  ## Shows set of high-dynamic range matrices using pfsview
  ##
  ## Usage: pfsview( ["title",] channel [,"channel name"], ... )
  ##
  ## title - (optional) a title to be displayed in the title bar of pfsview
  ## channel - a real matrix, or a cell array of real matrixes
  ## channel name - (optional) name for the channel given as next argument

  C = varargin{1};
  width = -1;
  height = -1;

  title = "Octave";
  n = 1;
  for i=1:length( varargin )
    if iscell(varargin{i})
      C = varargin{i};
      width = columns( C{1} );
      height = rows( C{1} );
      for j=1:length(C)
        ch_name = sprintf( "ch%d", n++ );
        if( !exist( "channels" ) )
          channels = struct( ch_name, C{j} );
        else
          channels = setfield( channels, ch_name, C{j} );
        endif
      endfor
    elseif( ismatrix( varargin{i} ) )
      ch_name = sprintf( "ch%d", n++ );
      if( !exist( "channels" ) )
        channels = struct( ch_name, varargin{i} );
      else
        channels = setfield( channels, ch_name, varargin{i} );
      endif
      width = columns( varargin{i} );
      height = rows( varargin{i} );
    elseif( ischar( varargin{i} ) )
      if( i == 1 )
        title = varargin{i};
      else
        if( !exist( "channels" ) || !exist( "ch_name" ) )
          error( "channel_name argument must follow channel argument" );
        else
          channels = setfield( channels, varargin{i}, ...
                              getfield( channels, ch_name ) );
          channels = rmfield( channels, ch_name );
          clear ch_name;
        endif
      endif
    endif
  endfor

  hdr_name = tmpnam();

  pfsout = pfsopen( hdr_name, height, width );
  pfsout.channels = channels;
  pfsout.tags.FILE_NAME = title;

  ## This should go after 'system' call, but it is so fifo seems to be
  ## not initialized for reading
  pfsput( pfsout );
  
  xv = sprintf ("pfsview <%s", hdr_name);
  rm = sprintf ("rm -f %s", hdr_name);

  system (sprintf ("( %s && %s ) > /dev/null 2>&1 &",
                   xv, rm));

  pfsclose( pfsout );
  
endfunction
