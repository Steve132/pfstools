function pfsview_list( M, idx )
  ## Shows list of high-dynamic range matrices using pfsview
  ##
  ## Usage: pfsview_list( list, idx )
  ## list - list of channels
	## idx - vector of list indexes to show (optional)

	if !exist("idx")
		idx = 1:length(M);
	endif

  execStr = "pfswrite( hdr_name";
	for i=idx
    chStr = sprintf( ", \"ch%d\",real(nth(M,%d))", i, i );
    execStr = [ execStr chStr ];
  endfor
  execStr = strcat( execStr, " );" );

  hdr_name = tmpnam();
  eval( execStr );

  xv = sprintf ("pfsview <%s", hdr_name);
  rm = sprintf ("rm -f %s", hdr_name);

	system (sprintf ("( %s && %s ) > /dev/null 2>&1 &", xv, rm));
endfunction
  
