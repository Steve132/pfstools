function pfsview( varargin )
%PFSVIEW Shows set of matrices as images using pfsview
%
% PFSVIEW( ['title',] channel [,'channel name'], ... )
%
% title - (optional) a title to be displayed in the title bar of pfsview
% channel - a real matrix, or a cell array of real matrixes. If the matrix
%           has the size(..,3)==3, it is recognized as an RGB color
%           image.
% channel name - (optional) name for the channel given as next argument
%           after channel matrix. 
%
% See also: PFS_READ_IMAGE, PFS_WRITE_IMAGE, PFS_TRANSFORM_COLORSPACE.
%
% Copyright 2009 Rafal Mantiuk

  width = -1;
  height = -1;

  title = 'matlab';
  n = 1;
  for i=1:length( varargin )
    if iscell(varargin{i})
      C = varargin{i};
      width = size( C{1}, 2 );
      height = size( C{1}, 1 );
      if( size( C, 1 ) > 1 )
          for j=1:size(C,1)
              for k=1:size(C,2)
                  if( any( size(C{j,k}) ~= [height width] ) )
                      continue; % empty or invalid cell
                  end
                  ch_name = sprintf( 'c_%d_%dx%d', n, j, k );
                  channels.(ch_name) = C{j, k};
              end              
          end
      else
          for j=1:length(C)
              ch_name = sprintf( 'cell_%d_%d', n, j );
              channels.(ch_name) = C{j};
          end
      end
      n = n+1;
    elseif( isnumeric( varargin{i} ) )
      m_size = size( varargin{i} );
      width = m_size(2);
      height = m_size(1);
      if( length(m_size) == 2 )
          matrix_name = inputname( i );
          if( isempty( matrix_name ) )
              ch_name = sprintf( 'matrix_%d', n );
          else
              ch_name = matrix_name;
          end
          n = n+1;
          channels.( ch_name ) = single( varargin{i} );
      elseif( length(m_size) == 3 && m_size(3)==3 )
          % Color channels
          [channels.X channels.Y channels.Z] = pfs_transform_colorspace( 'RGB', single(varargin{i}), 'XYZ' );
      elseif( length(m_size) == 3 && m_size(3)<10 )
          matrix_name = inputname( i );
          if( isempty( matrix_name ) )
              ch_name = sprintf( 'matrix_%d', n );
              n = n+1;
          else
              ch_name = matrix_name;
          end
          for j=1:m_size(3)
              channels.( sprintf( '%s_%d', ch_name, j ) ) = single( varargin{i}(:,:,j) );
          end
      else
          error( [ 'Cannot display matrix of the size [' num2str( m_size ) ']' ] );
      end
    elseif( islogical( varargin{i} ) )
      ch_name = sprintf( 'bool_%d', n );
      n = n+1;
      channels.(ch_name) = double(varargin{i});
      m_size = size( varargin{i} );
      width = m_size(2);
      height = m_size(1);
    elseif( ischar( varargin{i} ) )
      if( i == 1 )
        title = varargin{i};
      else
        if( ~exist( 'channels', 'var' ) || ~exist( 'ch_name', 'var' ) )
          error( 'channel_name argument must follow channel argument' );
        else 
          channels.(varargin{i}) = channels.( ch_name );
          channels = rmfield( channels, ch_name );
          clear ch_name;
        end
      end
    end
  end

  % tmp file is used rather than pipes to run pfsview in background without
  % blocking matlab
  tmp_file = tempname();
  pfsout = pfsopen( tmp_file, height, width );
  pfsout.channels = channels;
  pfsout.tags.FILE_NAME = title;
  pfsput( pfsout );  
  pfsclose( pfsout );
  
  system( sprintf( '%s(pfsview <''%s'' && rm -f %s) &%s', pfs_shell(), tmp_file, tmp_file, pfs_shell( 1 ) ) );
  
end
