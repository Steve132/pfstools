function pfsview_rgb( R, G, B, window_min, window_max )
  ## Shows high-dynamic range RGB image using pfsview
  ##
  ## Usage: pfsview( R, G, B[, min, max] )
  ## R, G, B - red, green and blue color channels, given as linear
  ## window_min - minimum luminance to show (in log10 units)
  ## window_max - maksimum luminance to show (in log10 units)
  ## response function

  if( !is_matrix(R) || !is_matrix(G) || !is_matrix(B) || \
     any(size(R) != size(G)) || any(size(G) != size(B)) )
    error( "pfsview_rgb: matrices of the equal size expected as an arguments" );
  endif
  
  [X Y Z] = pfstransform_colorspace( "RGB", R, G, B, "XYZ" );
  
  hdr_name = tmpnam();
  pfswrite( hdr_name, "X", X, "Y", Y, "Z", Z );
  
  rm = sprintf ("rm -f %s", hdr_name);
  if( exist( "window_min" ) && exist( "window_max" ) )
    minmax_window = sprintf( "--window_min %g --window_max %g ", \
                            window_min, window_max );
  else
    minmax_window = "";
  endif
  pfsv = sprintf ("pfsview %s <%s", minmax_window, hdr_name);

 system (sprintf ("( %s && %s ) > /dev/null 2>&1 &",
                  pfsv, rm));
endfunction
