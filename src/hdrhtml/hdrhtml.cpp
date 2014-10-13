/**
 * @brief Create a web page with an HDR viewer
 * 
 * This file is a part of PFSTOOLS package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2009 Rafal Mantiuk
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
 * $Id: hdrhtml.cpp,v 1.8 2014/06/16 21:50:08 rafm Exp $
 */

#include "hdrhtml.h"

#include <algorithm>
#include <math.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>

#include <pfs.h>

#include <Magick++.h>
// This is to get rid of warnings due to double defines from Magick++
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef VERSION

#include <config.h>

using namespace std;

// ================================================
//        Parameters controllig the web page 
// ================================================

const int f_step_res = 3; // How many steps per f-stop (do not change)
const int pix_per_fstop = 25;   // Distance in pixels between f-stops shown on the histogram
const char *hdrhtml_version = "1.0"; // Version of the HDRHTML code


// ================================================
//                Histogram
// ================================================

template<class T>
class Histogram
{
public:
  T *x;                         // Bin centers
  size_t *n;                       // No of items in a bin
  size_t bins;
  
  Histogram() : x( NULL ), n( NULL )
  {
  }

  ~Histogram()
  {
    free();
  }

  void free()
  {
    delete []x;
    delete []n;
    bins = 0;
  }

  void compute( const T *data, size_t d_size, int bins = 30, T min_val = 1, T max_val = -1, bool reject_outofrange = true ) 
  {
    assert( bins > 0 );
    
    free();
    this->bins = bins;
    
    if( min_val > max_val )             // missing min/max info
    {
      min_val = numeric_limits<T>::max();
      max_val = numeric_limits<T>::min();

      for( int k=0; k < d_size; k++ ) {
        if( data[k] > max_val ) max_val = data[k];
        if( data[k] < min_val ) min_val = data[k];
      }
    }

    x = new T[bins];
    n = new size_t[bins];

    T delta = (max_val-min_val) / (float)bins; // width of a single bin

//     T *e = new T[bins+1];        // bin edges
//     for( int k=0; k <= bins; k++ ) {
//       e[k] = min_val + (float)k * delta;
//     }
    for( int k=0; k < bins; k++ ) {
      x[k] = min_val + (float)k * delta + delta/2;
      n[k] = 0;
    }

    if( reject_outofrange ) {
      for( int k=0; k < d_size; k++ ) {      
        int ind = floor( (data[k]-min_val) / (max_val-min_val) * (float)bins );
        if( ind < 0 )
          continue;
        if( ind >= bins )
          continue;
        n[ind]++;
      }    
    } else {
      for( int k=0; k < d_size; k++ ) {      
        int ind = floor( (data[k]-min_val) / (max_val-min_val) * (float)bins );
        if( ind < 0 ) {
          n[0]++;
          continue;
        }
        if( ind >= bins ) {
          n[bins-1]++;
          continue;
        }
        n[ind]++;
      }    
    }
    
    
    
  }
  
};

// ================================================
//                  Lookup table
// ================================================

/**
 * Lookup table on a uniform array & interpolation
 *
 * x_i must be at least two elements 
 * y_i must be initialized after creating an object
 */
class UniformArrayLUT
{
  const float *x_i;
  size_t lut_size;
  float delta;

  bool own_y_i;
public:
  float *y_i;

  UniformArrayLUT( size_t lut_size, const float *x_i, float *y_i = NULL ) : x_i( x_i ), lut_size( lut_size ), delta( x_i[1]-x_i[0] )
  {
    if( y_i == NULL ) {
      this->y_i = new float[lut_size];
      own_y_i = true;
    } else {
      this->y_i = y_i;
      own_y_i = false;
    }
  }

  UniformArrayLUT() : x_i( 0 ), y_i(0), lut_size( 0 ), delta( 0. ) {}

  UniformArrayLUT(const UniformArrayLUT& other) : x_i( other.x_i ), lut_size( other.lut_size ), delta( other.delta )
  {
    this->y_i = new float[lut_size];
    own_y_i = true;
    memcpy(this->y_i, other.y_i, lut_size * sizeof(float));
  }

  UniformArrayLUT& operator = (const UniformArrayLUT& other)
  {
    this->lut_size = other.lut_size;
    this->delta = other.delta;
    this->x_i = other.x_i;
    this->y_i = new float[lut_size];
    own_y_i = true;
    memcpy(this->y_i, other.y_i, lut_size * sizeof(float));
  }

  ~UniformArrayLUT()
  {
    if( own_y_i )
      delete []y_i;
  }

  float interp( float x )
  {
    const float ind_f = (x - x_i[0])/delta;
    const size_t ind_low = (size_t)(ind_f);
    const size_t ind_hi = (size_t)ceil(ind_f);

    if( (ind_f < 0) )           // Out of range checks
      return y_i[0];
    if( (ind_hi >= lut_size) )
      return y_i[lut_size-1];
    
    if( (ind_low == ind_hi) )
      return y_i[ind_low];      // No interpolation necessary

    return y_i[ind_low] + (y_i[ind_hi]-y_i[ind_low])*(ind_f-(float)ind_low); // Interpolation
  }
  
};

template<class T>
inline T clamp( T x, T min, T max )
{
  if( x < min )
    return min;
  if( x > max )
    return max;
  return x;
}

/**
 * Lookup table on an arbitrary array
 *
 * x_i must be at least two elements 
 * y_i must be initialized after creating an object
 */
template<class Tx, class Ty>
class ArrayLUT
{
  const Tx *x_i;
  size_t lut_size;

  bool own_y_i;
public:
  Ty *y_i;

  ArrayLUT( size_t lut_size, const Tx *x_i, Ty *y_i = NULL ) : x_i( x_i ), lut_size( lut_size )
  {
    assert( lut_size > 0 );
    
    if( y_i == NULL ) {
      this->y_i = new Ty[lut_size];
      own_y_i = true;
    } else {
      this->y_i = y_i;
      own_y_i = false;
    }
  }

  ArrayLUT() : x_i( 0 ), y_i(0), lut_size( 0 ) {}

  ArrayLUT(const ArrayLUT& other) : x_i( other.x_i ), lut_size( other.lut_size )
  {
    this->y_i = new Ty[lut_size];
    own_y_i = true;
    memcpy(this->y_i, other.y_i, lut_size * sizeof(Ty));
  }

  ArrayLUT& operator = (const ArrayLUT& other)
  {
    this->lut_size = other.lut_size;
    this->x_i = other.x_i;
    this->y_i = new Ty[lut_size];
    own_y_i = true;
    memcpy(this->y_i, other.y_i, lut_size * sizeof(Ty));
  }

  ~ArrayLUT()
  {
    if( own_y_i )
      delete []y_i;
  }

  Ty interp( Tx x )
  {
    if( (x <= x_i[0]) )           // Out of range checks
      return y_i[0];
    if( (x >= x_i[lut_size-1]) )
      return y_i[lut_size-1];

    // binary search
    size_t l = 0, r = lut_size-1;
    while( true ) {
      size_t m = (l+r)/2;
      if( m == l ) break;
      if( x < x_i[m] )
        r = m;
      else
        l = m;
    }
    
    float alpha = (x - x_i[l])/(x_i[r]-x_i[l]);

    return y_i[l] + (Ty)(alpha * (y_i[r]-y_i[l]));
  }
  
  
};


// ================================================
//                  Percentiles
// ================================================


/**
 * Compute prctiles using image cummulative histogram, which is less
 * accurate method but much faster than sorting.
 */
template<class T>
class Percentiles
{
  Histogram<T> hist;
  const size_t bin_n;
  size_t d_size;
public:

  /**
   * @param data - table with samples
   * @param d_size - number of samples
   */
  Percentiles( const T *data, size_t d_size ) :
    bin_n( 1000 ), d_size( d_size ) // Accuracy 0.1 prctile
  {
    hist.compute( data, d_size, bin_n, 1, -1, false );
    // Compute cummulative histogram
    for( int k = 1; k < bin_n; k++ )
      hist.n[k] += hist.n[k-1];

//    cerr << "d_size: " << d_size << "  hist.n: " << hist.n[bin_n-1] << "\n";
    assert( hist.n[bin_n-1] == d_size );    
  }

  T prctile( double p )
  {
    ArrayLUT<size_t,T> lut( hist.bins, hist.n, hist.x );
    
    return lut.interp( (size_t)(p*(double)d_size/100.) );
  }
  
  
  
};


// ================================================
//            Text template file utils
// ================================================

typedef void (*replace_callback)( ostream &out, void *user_data, const char *parameter );

class ReplacePattern
{
  
public:

  const char* pattern;
  std::string replace_with;
  replace_callback callback;
  void *user_data;

  ReplacePattern( const char* pattern, std::string replace_with ) :
    pattern( pattern ), replace_with( replace_with ), callback( NULL )
  {
  }

  ReplacePattern( const char* pattern, float replace_with_num ) :
    pattern( pattern ), callback( NULL )
  {
    std::ostringstream num_str;
    num_str << replace_with_num;
    replace_with = num_str.str();
  }

  ReplacePattern( const char* pattern, int replace_with_num ) :
    pattern( pattern ), callback( NULL )
  {
    std::ostringstream num_str;
    num_str << replace_with_num;
    replace_with = num_str.str();
  }
  
  ReplacePattern( const char* pattern, replace_callback callback, void *user_data = NULL ) :
    pattern( pattern ), callback( callback ), user_data( user_data )
  {
  }
  
  ReplacePattern() : pattern( NULL ), callback( NULL )
  {
  }
  
  virtual void write_replacement( ostream &out, const char *parameter = NULL )
  {
    if( callback != NULL )
      callback( out, user_data, parameter );
    else
      out << replace_with;
  }
  

};

  
void create_from_template( std::ostream &outfs, const char *template_file_name,
  ReplacePattern *pattern_list )
{
  std::ifstream infs( template_file_name );
  if( !infs.good() ) {
    std::ostringstream error_message;
    error_message << "Cannot open '" << template_file_name << "' for reading";
    throw pfs::Exception( error_message.str().c_str() );
  }
  

  const int MAX_LINE_LENGTH = 2048;
//  int lines = 0;
  while( true ) {
    char line[MAX_LINE_LENGTH];
    infs.getline( line, MAX_LINE_LENGTH );

    if( !infs.good() )
      break;

    std::string line_str( line );
    int pos = 0;

    while( true ) {
      int find_pos = line_str.find_first_of( '@', pos );
      if( find_pos == std::string::npos ) {
        outfs << line_str.substr( pos, std::string::npos );
        break;
      }

      bool replaced = false;
      int end_marker = line_str.find_first_of( "@[", find_pos+1 );
      if( end_marker != std::string::npos ) {
        
        for( int k = 0; pattern_list[k].pattern != NULL; k++ )
        {
          if( line_str.compare( find_pos+1, end_marker-find_pos-1, pattern_list[k].pattern ) == 0 ) {
            outfs << line_str.substr( pos, find_pos-pos );

            std::string parameter;
            if( line_str[end_marker] == '[' ) {              
              int param_endmarker = line_str.find_first_of( ']', end_marker+1 );
              if( param_endmarker == std::string::npos )
                throw pfs::Exception( "Non-closed bracker in the replacement keyword" );
              parameter = line_str.substr( end_marker+1, param_endmarker-end_marker-1 );
              end_marker = param_endmarker+1;                
            }
          
            pattern_list[k].write_replacement( outfs, parameter.empty() ? NULL : parameter.c_str() );
            pos = end_marker + 1;
            replaced = true;
            break;
          }
        }

      }
      if( !replaced ) {
        outfs << line_str.substr( pos, find_pos-pos+1 );
        pos = find_pos+1;
      }
      
    }
    

    outfs << "\n";
      
  }
  
}

void create_from_template( const char *output_file_name, const char *template_file_name,
  ReplacePattern *pattern_list )
{
  std::ofstream outfs( output_file_name );
  if( !outfs.good() ) {
    std::ostringstream error_message;
    error_message << "Cannot open '" << output_file_name << "' for writing";
    throw pfs::Exception( error_message.str().c_str() );
  }
  create_from_template( outfs, template_file_name, pattern_list );
}




// ================================================
//            Read and parse CVS files
// ================================================


class CSVTable
{
public:

  float **data;
  int columns, rows;

  CSVTable() : data( NULL )
  {
  }
  
  
  ~CSVTable() 
  {
    free();
  }

  void free()
  {
    if( data == NULL )
      return;

    for( int k = 0; k < columns; k++ )
      delete [] data[k];

    delete []data;
    
    data = NULL;
  }
  
  
  void read( const char *file_name, int columns )
  {
    free();
    
    this->columns = columns;
    
    std::ifstream ifs( file_name );

    if( !ifs.is_open() ) {
      std::string full_message( "Cannot open file: " );
      full_message += file_name;
      throw pfs::Exception( full_message.c_str() );
    }

    std::list<float> value_list;
  
    const int MAX_LINE_LENGTH = 1024;
    int lines = 0;
    while( 1 ) {
      char line[MAX_LINE_LENGTH];
      ifs.getline( line, MAX_LINE_LENGTH );
    
      if( !ifs.good() )
        break;

      std::string line_str( line );
      int pos = 0;
      for( int k=0; k < columns; k++ ) {
        // Skip white spaces
        while( line_str[pos] == ' ' || line_str[pos] == '\t' ) pos++;
        int new_pos = line_str.find_first_of( ',', pos );
        size_t len;
        if( new_pos == std::string::npos ) {
          if( k != columns-1 ) {
            std::string full_message( "Missing column data in the file: " );
            full_message += file_name;
            throw pfs::Exception( full_message.c_str() );
          }
          len = std::string::npos;
        } else
          len = new_pos-pos;
        
        float value;
        if( len == 0 ) {
          value = numeric_limits<float>::quiet_NaN();
        } else {
          std::string token = line_str.substr( pos, len );
          const char *str_beg = token.c_str();
          char *str_end;
//          cerr << "token: " << str_beg << "\n";
          value = strtof( str_beg, &str_end );
          if( str_beg == str_end ) {
            std::ostringstream error_message;
            error_message << "Error parsing line " << lines+1 << " of " << file_name << "\n";
            throw pfs::Exception( error_message.str().c_str() );          
          }
        }
        
        
        value_list.push_back( value );

        pos = new_pos+1;      
      }

      lines++;
    }

    float **table = new float*[columns];
    for( int c=0; c < columns; c++ )
      table[c] = new float[lines];
  
    for( int l=0; l < lines; l++ )
      for( int c=0; c < columns; c++ ) {
        table[c][l] = value_list.front();
        value_list.pop_front();
      }

    data = table;
    this->rows = lines;
  }

};

  
// ================================================
//                 HDR HTML code
// ================================================


void HDRHTMLSet::add_image( int width, int height, float *R, float *G, float *B,
  float *Y,
  const char *base_name, int quality )
{

  const int pixels = width*height;
  const int basis_no = quality;
  
  // Load LUT for the basis tone-curves
  std::ostringstream lut_filename;
  lut_filename << PKG_DATADIR "/hdrhtml_t_b" << basis_no+1 << ".csv";
  CSVTable basis_table;
  basis_table.read( lut_filename.str().c_str(), basis_no+1 );
  // Transform the first row (luminance factors) to the log domain
  for( int k = 0; k < basis_table.rows; k++ )
    basis_table.data[0][k] = log2f( basis_table.data[0][k] );
  
// Fix zero and negative values in the image, convert to log2 space, find min and max values
  float img_min = numeric_limits<float>::max();
  float img_max = numeric_limits<float>::min();
  {
    float *arrays[] = { R, G, B, Y };
    int k;

    for( k = 0; k < 4; k++ ) {
      float *x = arrays[k];
      float min_val = numeric_limits<float>::max(), max_val = numeric_limits<float>::min();
      for( int i=0; i < pixels; i++ ) {
        if( x[i] < min_val && x[i] > 0)
          min_val = x[i];
        if( x[i] > max_val )
          max_val = x[i];        
      }
      img_max = std::max( img_max, log2f(max_val) );
      img_min = std::min( img_min, log2f(min_val) );      

      for( int i=0; i < pixels; i++ ) {
        if( x[i] < min_val )
          x[i] = log2f(min_val);
        else
          x[i] = log2f(x[i]);
      }
    }
  }


  Percentiles<float> prc( Y, pixels );
  img_min = prc.prctile( 0.1 );
  img_max = prc.prctile( 99.9 );
  
  img_min -= 4;  // give extra room for brightenning
  // how many 8-fstop segments we need to cover the DR
  int f8_stops = ceil((img_max-img_min)/8);

  // start with this f-stop
  float l_start = img_min + (img_max-img_min-f8_stops*8)/2;

  float l_med = prc.prctile( 50 );
  float best_exp = round(l_med-l_start-4);

// pix_per_fstop = 25;

// % generate image histogram

  const int hist_height = 36;
  int hist_width = width;
//  float hist_img_sz = [36 size(img,2)];
  float hist_fstops = hist_width / pix_per_fstop;
  float hist_start = (img_max-img_min-hist_fstops)/2;
  {
    
    Histogram<float> hist;
    hist.compute( Y, pixels, hist_width, img_min+hist_start, img_min+hist_start+hist_fstops );

    unsigned short *hist_buffer = new unsigned short[hist_width*hist_height*3];
    float hist_n_max = -1;
    for( int k = 0; k < hist_width; k++ )
      hist_n_max = std::max( hist_n_max, (float)hist.n[k] );

    for( int k = 0; k < hist_width; k++ ) {
      float top = hist_height - round((float)hist.n[k]/hist_n_max * hist_height);
      for( int r = 0; r < hist_height; r++ ) {
        hist_buffer[(r*hist_width+k)*3+0] = 0;
        hist_buffer[(r*hist_width+k)*3+1] = (r>=top ? (1<<16) -1 : 0);
        hist_buffer[(r*hist_width+k)*3+2] = 0;
      }
    }
    
      
// tick_fstops = (floor(hist_l(end))-ceil(hist_l(1)));
// ticks = round((ceil(hist_l(1))-hist_l(1))*pix_per_fstop) + (1:tick_fstops)*pix_per_fstop;
// hist_img(1:5,ticks,1:2) = 0.5;
// hist_img(end-4:end,ticks,1:2) = 0.5;
// plot_name = sprintf( '%s_hist.png', base_name );
// imwrite( hist_img, plot_name );

    Magick::Image hist_image( hist_width, hist_height,
      "RGB", Magick::ShortPixel, hist_buffer );

    std::ostringstream img_filename;
    if( image_dir != NULL )
      img_filename << image_dir << "/";
    img_filename << base_name << "_hist.png";
    std::cerr << "Writing: " << img_filename.str() << "\n";        
    hist_image.write( img_filename.str().c_str() );

    delete []hist_buffer;
  }
  
  // generate basis images

  unsigned short *imgBuffer =
    new unsigned short[pixels*3];  
  for( int k=1; k <= f8_stops+1; k++ ) {


    float max_value = (float)numeric_limits<unsigned short>::max(); //(1<<16) -1;

    float exp_multip = log2f(1/powf( 2, l_start + k*8 ));

    int max_basis = basis_no;
    if( k == f8_stops+1 )     // Do only one shared basis for the last 8-fstop segment
      max_basis = 1;
    
    for( int b=0; b < max_basis; b++ ) {
      UniformArrayLUT basis_lut( basis_table.rows, basis_table.data[0], basis_table.data[b+1] );

      int i = 0;
      for( int pix = 0; pix < pixels; pix++ ) {

        float rgb[3];
        rgb[0] = R[pix];
        rgb[1] = G[pix];
        rgb[2] = B[pix];

        for( int c=0; c < 3; c++ ) {
          float exposure_comp_v = rgb[c] + exp_multip;
          float v = (basis_lut.interp(exposure_comp_v)*max_value);              
          imgBuffer[i++] = (unsigned short)(basis_lut.interp(exposure_comp_v)*max_value);              
        }          
      }
      Magick::Image imImage( width, height,
        "RGB", Magick::ShortPixel, imgBuffer );

      std::ostringstream img_filename;
      if( image_dir != NULL )
        img_filename << image_dir << "/";
      img_filename << base_name << '_' << k-1 << '_' << b+1 << ".jpg";
      std::cerr << "Writing: " << img_filename.str() << "\n";
      imImage.write( img_filename.str().c_str() );
    }

  }
  delete [] imgBuffer;

  HDRHTMLImage new_image( base_name, width, height );
  
  new_image.hist_width = hist_width;
  new_image.f8_stops = f8_stops;
  new_image.f_step_res = f_step_res;
  new_image.basis = basis_no;
  new_image.shared_basis = 1;
  new_image.pix_per_fstop = pix_per_fstop;
  new_image.hist_start = hist_start;
  new_image.hist_width = hist_width;
  new_image.best_exp = best_exp;

  image_list.push_back( new_image );
  
}

void print_image_objects( ostream &out, void *user_data, const char *parameter );
void print_cf_table( ostream &out, void *user_data, const char *parameter );
void print_image_htmlcode( ostream &out, void *user_data, const char *parameter );

void HDRHTMLSet::generate_webpage( const char *page_template, const char *image_template,
  const char *object_output, const char *html_output)
{
  if( image_list.empty() )
    return;

  std::ostringstream out_file_name;
  if( page_name == NULL )
    out_file_name << image_list.front().base_name << ".html";
  else
    out_file_name << page_name;

  // Load the table of the opacity coeffcients
  std::ostringstream lut_filename;
  lut_filename << PKG_DATADIR "/hdrhtml_c_b" << image_list.front().basis+1 << ".csv";
  CSVTable coeff_table;
  coeff_table.read( lut_filename.str().c_str(), image_list.front().basis+1 );
  
  ReplacePattern replace_list[] = {
    ReplacePattern( "cf_array_def", print_cf_table, &coeff_table ),
    ReplacePattern( "hdr_img_def", print_image_objects, this ),
    ReplacePattern( "image_htmlcode", print_image_htmlcode, this ),
    ReplacePattern( "title", page_name == NULL ? "HDRHTML viewer" : page_name ),
    ReplacePattern( "version", hdrhtml_version ),
    ReplacePattern()
  };

  this->image_template = image_template;
  create_from_template( out_file_name.str().c_str(), page_template, replace_list );

  if( object_output != NULL ) {
    std::ofstream oofs( object_output );
    if( !oofs.good() ) {
      std::ostringstream error_message;
      error_message << "Cannot open '" << object_output << "' for writing";
      throw pfs::Exception( error_message.str().c_str() );
    }
    print_image_objects( oofs, this, NULL );
  }

  if( html_output != NULL ) {
    std::ofstream hofs( html_output );
    if( !hofs.good() ) {
      std::ostringstream error_message;
      error_message << "Cannot open '" << html_output << "' for writing";
      throw pfs::Exception( error_message.str().c_str() );
    }
    print_image_htmlcode( hofs, this, NULL );
  }
  
}

void print_image_objects( ostream &out, void *user_data, const char *parameter )
{
  HDRHTMLSet *hdrhtml_set = (HDRHTMLSet*)user_data;

  list<HDRHTMLImage>::iterator it;
  for( it = hdrhtml_set->image_list.begin(); it != hdrhtml_set->image_list.end(); it++ ) {  
    std::string obj_name( "hdr_" );
    obj_name.append( it->base_name );

    out << obj_name << " = new Object();\n";
    out << obj_name << ".width = " << it->width << ";\n";
    out << obj_name << ".height = " << it->height << ";\n";
    out << obj_name << ".f8_stops = " << it->f8_stops << ";\n";
    out << obj_name << ".f_step_res = " << it->f_step_res << ";\n";
    out << obj_name << ".base_name = \"" << it->base_name << "\";\n";
    if( hdrhtml_set->image_dir==NULL )
      out << obj_name << ".image_dir = \"\";\n";
    else
      out << obj_name << ".image_dir = \"" << hdrhtml_set->image_dir << "/\";\n";
    out << obj_name << ".basis = " << it->basis << ";\n";
    out << obj_name << ".shared_basis = " << it->shared_basis << ";\n";
    out << obj_name << ".pix_per_fstop = " << it->pix_per_fstop << ";\n";
    out << obj_name << ".hist_start = " << it->hist_start << ";\n";
    out << obj_name << ".hist_width = " << it->hist_width << ";\n";
    out << obj_name << ".exposure = " << it->best_exp << ";\n";
    out << obj_name << ".best_exp = " << it->best_exp << ";\n\n";    
  }
  
}

void print_image_htmlcode( ostream &out, HDRHTMLSet *hdrhtml_set, const HDRHTMLImage &it )
{
    std::string obj_name( "hdr_" );
    obj_name.append( it.base_name );

    std::ostringstream img_dir;
    if( hdrhtml_set->image_dir != NULL )
      img_dir << hdrhtml_set->image_dir << "/";  
    
    ReplacePattern replace_list[] = {
      ReplacePattern( "hdr_img_width", it.width ),
      ReplacePattern( "hdr_img_height", it.height ),
      ReplacePattern( "img_dir", img_dir.str() ),
      ReplacePattern( "hist_width", it.hist_width ),
      ReplacePattern( "base_name", it.base_name ),
      ReplacePattern( "help_mark_pos", it.hist_width-12 ),
      ReplacePattern( "hdr_img_object", obj_name ),      
      ReplacePattern( "version", hdrhtml_version ),
      ReplacePattern()
    };

    create_from_template( out, hdrhtml_set->image_template, replace_list );
 
}

void print_image_htmlcode( ostream &out, void *user_data, const char *parameter )
{
  HDRHTMLSet *hdrhtml_set = (HDRHTMLSet*)user_data;

  if( parameter != NULL ) {

    list<HDRHTMLImage>::iterator it;
    for( it = hdrhtml_set->image_list.begin(); it != hdrhtml_set->image_list.end(); it++ ) {
      if( it->base_name.compare( parameter ) == 0 )
        break;
    }
    if( it == hdrhtml_set->image_list.end() )
      std::cerr << "Warning: image '" << parameter << "' not found\n";

    print_image_htmlcode( out, hdrhtml_set, *it );
    
  } else {
    
    list<HDRHTMLImage>::iterator it;
    for( it = hdrhtml_set->image_list.begin(); it != hdrhtml_set->image_list.end(); it++ ) {
      
      print_image_htmlcode( out, hdrhtml_set, *it );
    
    }
  }
  
}

void print_cf_table( ostream &out, void *user_data, const char *parameter )
{
  CSVTable *cf = (CSVTable*)user_data;
  
  out << "var cf = new Array(\n";
  for( int b=0; b < cf->rows; b++ ) {
    out << "   new Array(";
    for( int ex=0; ex < cf->columns; ex++ ) {
      out << ' ' << cf->data[ex][b];
      if( ex != cf->columns-1 )
        out << ',';
    }
    out << ')';
    if( b != cf->rows-1 )
      out << ',';
    out << "\n";    
  }
  out << ");\n";

}


