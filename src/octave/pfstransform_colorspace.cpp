/**
 * @brief GNU Octave wrapper - tranform between color spaces using pfs library
 * 
 * This file is a part of PFSTOOLS package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2003,2004 Rafal Mantiuk and Grzegorz Krawczyk
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * ---------------------------------------------------------------------- 
 *
 * @author Rafal Mantiuk, <mantiuk@mpi-sb.mpg.de>
 *
 * $Id: pfstransform_colorspace.cpp,v 1.1 2005/06/15 13:36:54 rafm Exp $
 */
#include <string>

#include <octave/oct.h>

#include <array2d.h>
#include <pfs.h>

#include "octave_util.h"

static pfs::ColorSpace findColorSpace( const char *name );

#define SCRIPT_NAME "pfstransform_colorspace"

static const char *helpString =
#include "pfstransform_colorspace_help.h"

DEFUN_DLD( pfstransform_colorspace, args, , helpString)
{
    octave_value_list retval;
	
    int nargin = args.length();

    if( nargin != 5 ||
        !args(0).is_string() ||
        !args(1).is_real_matrix() ||
        !args(2).is_real_matrix() ||
        !args(3).is_real_matrix() ||
        !args(4).is_string()
        )
    {
        error( SCRIPT_NAME ": Improper usage!");
        return retval;
    }
  
    try {
        pfs::ColorSpace inCS = findColorSpace( args(0).string_value().c_str() );
        pfs::ColorSpace outCS = findColorSpace( args(4).string_value().c_str() );
        
        const int rows = args(1).matrix_value().rows(), cols = args(1).matrix_value().cols();
        
        pfs::Array2DImpl c1Buf( cols, rows ), c2Buf( cols, rows ), c3Buf( cols, rows );
        
        copy( args(1).matrix_value(), &c1Buf );
        copy( args(2).matrix_value(), &c2Buf );
        copy( args(3).matrix_value(), &c3Buf );

        pfs::transformColorSpace( inCS, &c1Buf, &c2Buf, &c3Buf, outCS, &c1Buf, &c2Buf, &c3Buf );

        Matrix c1 = Matrix( rows, cols );
        Matrix c2 = Matrix( rows, cols );
        Matrix c3 = Matrix( rows, cols );
        
        copy( &c1Buf, c1 );
        copy( &c2Buf, c2 );
        copy( &c3Buf, c3 );
        
        retval(0) = c1;
        retval(1) = c2;
        retval(2) = c3;
    }
    catch( pfs::Exception ex )
    {
        char error_message[100];
        sprintf( error_message, "%s: %s", SCRIPT_NAME, ex.getMessage() );
        error( error_message );      
    }
    
    return retval;
}

static pfs::ColorSpace findColorSpace( const char *name )
{
    if( !strcasecmp( name, "XYZ" ) ) return pfs::CS_XYZ;
    else if( !strcasecmp( name, "RGB" ) ) return pfs::CS_RGB;
    else if( !strcasecmp( name, "SRGB" ) ) return pfs::CS_SRGB;
    else if( !strcasecmp( name, "YUV" ) ) return pfs::CS_YUV;
    else if( !strcasecmp( name, "YXY" ) ) return pfs::CS_Yxy;

    throw pfs::Exception( "Not recognized color space" );
}

