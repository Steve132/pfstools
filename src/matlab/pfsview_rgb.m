function pfsview_rgb( varargin )
% Shows high-dynamic range RGB image using pfsview
%
% Usage: pfsview( R, G, B )
%        pfsview( img )
% R, G, B - red, green and blue color channels, given as linear response
% img - 3D matrix image, where img(:,:,1:3) represents red, blue and green
%       color channels

if( nargin == 3 )
    if( ~isnumeric(varargin{1}) || ~isnumeric(varargin{2}) || ~isnumeric(varargin{1}) )
        error( 'pfsview_rgb: matrices of the equal size expected as an arguments' );
    end
    [X Y Z] = pfs_transform_colorspace( 'RGB', varargin{1}, varargin{2}, varargin{3}, 'XYZ' );
    [height width] = size( varargin{1} );
elseif( nargin == 1 && ndims(varargin{1}) == 3 ) 
    [X Y Z] = pfs_transform_colorspace( 'RGB', varargin{1}(:,:,1), varargin{1}(:,:,2), varargin{1}(:,:,3), 'XYZ' );
    [height width] = size( varargin{1}(:,:,1) );
else
    error( 'pfsview_rgb: improper usage' );
end
  

  if( exist( 'window_min', 'var' ) == 1 && exist( 'window_max', 'var' ) == 1 )
    minmax_window = sprintf( '--window_min %g --window_max %g ', window_min, window_max );
  else
    minmax_window = '';
  end
  
  % tmp file is used rather than pipes to run pfsview in background without
  % blocking matlab
  tmp_file = tempname();
%  hv_fid = pfspopen( sprintf( '%spfsview %s%s', pfs_shell(), minmax_window, pfs_shell(1) ), 'w' );
  pfsout = pfsopen( tmp_file, height, width );
  pfsout.channels.X = X;
  pfsout.channels.Y = Y;
  pfsout.channels.Z = Z;
  pfsout.tags.FILE_NAME = 'matlab';
  pfsput( pfsout );  
  pfsclose( pfsout );

  system( sprintf( '%s(pfsview <''%s'' && rm -f %s) &%s', pfs_shell(), tmp_file, tmp_file, pfs_shell( 1 ) ) );
  
end
