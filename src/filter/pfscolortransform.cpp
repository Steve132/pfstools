/**
 * @brief Apply color transformation on the pfs stream
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
 * @author Delvin Varghese, <delvin.friends@gmail.com>
 *
 */

 // TODO: Remove memory leaks (create matrix class)
 
#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <sstream>

#include <getopt.h>
#include <pfs.h>

#define NUMELEMS(x)  (sizeof(x)/sizeof(*(x)))
#define PROG_NAME "pfscolortransform"

/*  EXAMPLE CONTENTS OF matrix_file.txt
  0.111424,0.222579,0.333464
  0.111656,0.222158,0.333186
  0.111332,0.222193,0.333444
*/
// static const float rgb2xyzD65Mat[3][3] =
static const float rgb2xyzD65Mat[3][3] =
{ { 0.412424f, 0.357579f, 0.180464f },
  { 0.212656f, 0.715158f, 0.072186f },
  { 0.019332f, 0.119193f, 0.950444f } };

// static const float xyz2rgbD65Mat[3][3] =
static const float xyz2rgbD65Mat[3][3] =
{ {  3.240708, -1.537259, -0.498570 },
  { -0.969257,  1.875995,  0.041555 },
  {  0.055636, -0.203996,  1.057069 } };

class QuietException {};

void printHelp()
{
  std::cerr << PROG_NAME " [--xyzrgb matrix_file.txt | --rgbxyz matrix_file.txt] [--transpose]" << std::endl
            << "See man page for more information." << std::endl;
}

float** initialiseArray(const float referenceMat[][3]){
  float** outputMat;
  outputMat = new float*[3];
  for (int i = 0;i<3;i++){
        outputMat[i] = new float[3];
    }
    for (int i=0;i<3;i++){
        for(int j=0;j<3;j++){
          outputMat[i][j] = referenceMat[i][j];
        }
      }
    return outputMat;
}

float** readMatrixFile(char* fn, bool transpose){

  float** matArray;
    FILE *file = fopen(fn, "r");
    if ( file ){
		matArray = new float*[3];
        for (int i = 0;i<3;i++){
            matArray[i] = new float[3];
        }
        size_t i, j, k;
        char buffer[BUFSIZ], *ptr, *end_ptr;
        for ( i = 0; fgets(buffer, sizeof buffer, file); ++i ){
            for ( j = 0, ptr = buffer; j <= NUMELEMS(*matArray); ++j, ++ptr ){
                if(!transpose){
                    matArray[i][j] = (float)strtof(ptr, &end_ptr);
                } else {
                    matArray[j][i] = (float)strtof(ptr, &end_ptr);
                }
				if( end_ptr == ptr ) {
					// Parsing number issue
					std::ostringstream error_msg;
					error_msg << "Failed to parse line " << i+1 << " in '" << fn << "'";
					throw pfs::Exception( error_msg.str().c_str() );
				}
				ptr = end_ptr;
            }
        }
        fclose(file);
    }
    else{
		// Parsing number issue
		std::ostringstream error_msg;
		error_msg << "Cannot open matrix file '" << fn << "'";
		throw pfs::Exception( error_msg.str().c_str() );
    }
    return matArray;
}

// float** multiplyMatrices(float matA[][3], float matB[][3]){
float** multiplyMatrices(float** matA, float** matB){
  float** resultMat;
    resultMat = new float*[3];
    for (int i = 0;i<3;i++){
        resultMat[i] = new float[3];
    }
    for (int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            for(int k=0;k<3;k++){
                resultMat[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }
    return resultMat;
}

// void applyTransform( pfs::Array2D *arrayX, pfs::Array2D *arrayY, pfs::Array2D *arrayZ, const float finalMatrix[][3])
void applyTransform( pfs::Array2D *arrayX, pfs::Array2D *arrayY, pfs::Array2D *arrayZ, float** finalMatrix)
{

  int imgSize = arrayX->getRows()*arrayX->getCols();
  for( int index = 0; index < imgSize ; index++ ) {    
    
    float &x = (*arrayX)(index);
//    if( x < 0 ) x = 0;
    float &y = (*arrayY)(index);
//    if( y < 0 ) y = 0;
    float &z = (*arrayZ)(index);
//    if( z < 0 ) z = 0;
    
    x = finalMatrix[0][0]*x + finalMatrix[0][1]*x + finalMatrix[0][2]*x;
    y = finalMatrix[1][0]*y + finalMatrix[1][1]*y + finalMatrix[1][2]*y;
    z = finalMatrix[2][0]*z + finalMatrix[2][1]*z + finalMatrix[2][2]*z;

    // x = finalMatrix[0][0]*x + finalMatrix[0][1]*y + finalMatrix[0][2]*z;
    // y = finalMatrix[1][0]*x + finalMatrix[1][1]*y + finalMatrix[1][2]*z;
    // z = finalMatrix[2][0]*x + finalMatrix[2][1]*y + finalMatrix[2][2]*z;
  }    

}

void printColorTransform( float **mat, bool opt_xyzrgb )
{
  fprintf( stderr, "Using color transformation:\n");
  const char *left = "XYZ";
  const char *right = "RGB";
  if( opt_xyzrgb )
	std::swap( left, right );
	
  for(int i=0;i<3;i++){
		if( i == 1 ) {
			fprintf( stderr, "[%s]' = [", left);
		} else
			fprintf( stderr, "         [");
		  
          for(int j=0;j<3;j++){
			if( j > 0 ) 
				fprintf( stderr, "  " );				
            fprintf( stderr, "%4f", mat[i][j]);
          }
		  if( i == 1 ) {
			fprintf( stderr, "] * [%s]'\n", right );
		  } else
			fprintf( stderr, "]\n");
  }
}

void applyTransformOnFrames( int argc, char *argv[] ){
	pfs::DOMIO pfsio;

  bool verbose = false;
	bool transpose  = false;
	bool opt_xyzrgb = false;
	bool opt_rgbxyz = false;

  // File name of Matrix file.
  char *matrix_file_name;

	static struct option cmdLineOptions[] = {
    { "help", 			no_argument, 		NULL, 'h' },
    { "verbose", 		no_argument, 		NULL, 'v' },
    { "transpose", 		no_argument, 		NULL, 't' },
    { "xyzrgb", 		required_argument, 	NULL, 'x' },
    { "rgbxyz", 		required_argument, 	NULL, 'r' },
    { NULL, 0, NULL, 0 }
  };

  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, "r:x:htv", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':{
      printHelp();
      throw QuietException();
    }
    case 'v':{
      verbose = true;
      break;
    }
    case 't':{
      transpose = true;
      break;
    }
    case 'x':{
      //char* matrixFn = optarg;
      matrix_file_name = optarg;
      opt_xyzrgb = true;
      break;
    }
    case 'r':{
      matrix_file_name = optarg;
      opt_rgbxyz = true;
      break;
    }
    case '?':
      throw QuietException();
      break;
    case ':':
      throw QuietException();
      break;
    }
  }

  float** matrixFile = NULL;
  float** rec709Xuserval = NULL;
  float** referenceMat = NULL;
  matrixFile = readMatrixFile(matrix_file_name,transpose);

  if( verbose ) {
	printColorTransform( matrixFile, opt_xyzrgb );
  }
  
  if(opt_rgbxyz){
    referenceMat = initialiseArray(xyz2rgbD65Mat);
    rec709Xuserval = multiplyMatrices(matrixFile,referenceMat);
  } else if(opt_xyzrgb){
    referenceMat = initialiseArray(rgb2xyzD65Mat);
    rec709Xuserval = multiplyMatrices(referenceMat,matrixFile);
  }
  
  while( true ){
  	pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) break; // No more frames

    pfs::Channel *X, *Y, *Z;
    frame->getXYZChannels( X, Y, Z );

    // TODO : Processing here
    if( X != NULL ) {           // Color, XYZ    
      applyTransform( X, Y, Z, rec709Xuserval);
    } else
    throw pfs::Exception( "Missing X, Y, Z channels in the PFS stream" );

    pfsio.writeFrame( frame, stdout );
    pfsio.freeFrame( frame );    
  }
}

int main( int argc, char* argv[] )
{
  try {
    applyTransformOnFrames( argc, argv );
  }
  catch( pfs::Exception ex ) {
    std::cerr << PROG_NAME << " error: " << ex.getMessage() << std::endl;
    return EXIT_FAILURE;
  }        
  catch( QuietException  ex ) {
    return EXIT_FAILURE;
  }        
  return EXIT_SUCCESS;
}