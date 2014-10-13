%PFS_TRANSFORM_COLORSPACE Tranform between color spaces using pfs library.
%
% [C1 C2 C2] = PFS_TRANSFORM_COLORSPACE( inCSname, c1, c2, c3, outCSname );
% img_out = PFS_TRANSFORM_COLORSPACE( inCSname, img_in, outCSname );
% img_out = PFS_TRANSFORM_COLORSPACE( inCSname, c1, c2, c3, outCSname );
% [C1 C2 C2] = PFS_TRANSFORM_COLORSPACE( inCSname, img_in, outCSname );
%
%   inCSname - name of the input color space
%   c<n> - matrix with n-th channel of input color space
%   C<n> - matrix with n-th channel of output color space
%   img_in - input image given as 3D height/width/3 matrix 
%   img_out - output image given as 3D height/width/3 matrix 
%   outCSname - name of the output color space
%
% Recognized color space names: 'XYZ', 'RGB', 'sRGB', 'YUV', 'Yxy'.
% Color space names are case insensitive.
%
% See also: PFS_READ_IMAGE, PFS_WRITE_IMAGE, PFSVIEW.
%
% Copyright 2009 Rafal Mantiuk
