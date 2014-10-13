#include <math.h>
#include <algorithm>
#include <iostream>

#include <config.h>

#include "compression_tmo.h"


/**
 * Lookup table on a uniform array & interpolation
 *
 * x_i must be at least two elements
 * y_i must be initialized after creating an object
 */
class UniformArrayLUT
{
    double start_v;
    size_t lut_size;
    double delta;

    bool own_y_i;
public:
    double *y_i;

    UniformArrayLUT( double from, double to, int lut_size, double *y_i = NULL  ) :
        start_v(from), lut_size( lut_size ), delta( (to-from)/(double)lut_size )
    {
        if( y_i == NULL ) {
            this->y_i = new double[lut_size];
            own_y_i = true;
        } else {
            this->y_i = y_i;
            own_y_i = false;
        }
    }

    UniformArrayLUT() : y_i(NULL), lut_size( 0 ), delta( 0. ), own_y_i( false ) {}

    UniformArrayLUT(const UniformArrayLUT& other) : start_v( other.start_v ), lut_size( other.lut_size ), delta( other.delta )
    {
        this->y_i = new double[lut_size];
        own_y_i = true;
        memcpy(this->y_i, other.y_i, lut_size * sizeof(double));
    }

    UniformArrayLUT& operator = (const UniformArrayLUT& other)
    {
        this->lut_size = other.lut_size;
        this->delta = other.delta;
        this->start_v = other.start_v;
        this->y_i = new double[lut_size];
        own_y_i = true;
        memcpy(this->y_i, other.y_i, lut_size * sizeof(double));
        return *this;
    }

    ~UniformArrayLUT()
    {
        if( own_y_i )
            delete []y_i;
    }

    double interp( double x )
    {
        const double ind_f = (x - start_v)/delta;
        const size_t ind_low = (size_t)(ind_f);
        const size_t ind_hi = (size_t)ceil(ind_f);

        if( unlikely(ind_f < 0) )           // Out of range checks
            return y_i[0];
        if( unlikely(ind_hi >= lut_size) )
            return y_i[lut_size-1];

        if( unlikely(ind_low == ind_hi) )
            return y_i[ind_low];      // No interpolation necessary

        return y_i[ind_low] + (y_i[ind_hi]-y_i[ind_low])*(ind_f-(double)ind_low); // Interpolation
    }

};


class ImgHistogram
{
public:
    const float L_min, L_max;
    const float delta;
    int *bins;
    double *p;
    int bin_count;

    ImgHistogram() : L_min( -6.f ), L_max( 9.f ), delta( 0.1 ), bins( NULL ), p( NULL )
    {
        bin_count = (int)ceil((L_max-L_min)/delta);
        bins = new int[bin_count];
        p = new double[bin_count];
    }

    ~ImgHistogram()
    {
        delete [] bins;
        delete [] p;
    }

    void compute( const float *img, size_t pixel_count )
    {

        std::fill( bins, bins + bin_count, 0 );

        int pp_count = 0;
        for( size_t pp = 0; pp < pixel_count; pp++ )
        {
            int bin_index = (img[pp]-L_min)/delta;
            // ignore anything outside the range
            if( bin_index < 0 || bin_index >= bin_count )
                continue;
            bins[bin_index]++;
            pp_count++;
        }

        for( int bb = 0; bb < bin_count; bb++ ) {
            p[bb] = (double)bins[bb] / (double)pp_count;
        }
    }


};


void CompressionTMO::tonemap( const float *R_in, const float *G_in, float *B_in, int width, int height,
                              float *R_out, float *G_out, float *B_out, const float *L_in,
                              pfstmo_progress_callback progress_cb )
{

    const size_t pix_count = width*height;

    // Compute log of Luminance
    float *logL = new float[pix_count];
//    std::unique_ptr<float[]> logL(new float[pix_count]);
    for( size_t pp = 0; pp < pix_count; pp++ ) {
        logL[pp] = log10f( std::max( 1e-5f, L_in[pp] ) );
    }

    ImgHistogram H;
    H.compute(logL, pix_count );

    //Compute slopes
//    std::unique_ptr<double[]> s(new double[H.bin_count]);
    double *s = new double[H.bin_count];
    {
        double d = 0;
        for( int bb = 0; bb < H.bin_count; bb++ ) {
            d += pow( H.p[bb], 1./3. );
        }
        d *= H.delta;
        for( int bb = 0; bb < H.bin_count; bb++ ) {
            s[bb] = pow( H.p[bb], 1./3. )/d;
        }

    }

#if 0
    // TODO: Handling of degenerated cases, e.g. when an image contains uniform color
    const double s_max = 2.; // Maximum slope, to avoid enhancing noise
    double s_renorm = 1;
    for( int bb = 0; bb < H.bin_count; bb++ ) {
        if( s[bb] >= s_max ) {
            s[bb] = s_max;
            s_renorm -= s_max * H.delta;
        }
    }
    for( int bb = 0; bb < H.bin_count; bb++ ) {
        if( s[bb] < s_max ) {
            s[bb] = s_max;
            s_renorm -= s_max * H.delta;
        }

    }

#endif
    progress_cb( 50 );

    //Create a tone-curve
    UniformArrayLUT lut( H.L_min, H.L_max, H.bin_count );
    lut.y_i[0] = 0;
    for( int bb = 1; bb < H.bin_count; bb++ ) {
        lut.y_i[bb] = lut.y_i[bb-1] + s[bb] * H.delta;
    }

    // Apply the tone-curve
    for( int pp = 0; pp < pix_count; pp++ ) {
        R_out[pp] = lut.interp( log10f(R_in[pp]) );
        G_out[pp] = lut.interp( log10f(G_in[pp]) );
        B_out[pp] = lut.interp( log10f(B_in[pp]) );
    }

    delete [] s;
    delete [] logL;

}
