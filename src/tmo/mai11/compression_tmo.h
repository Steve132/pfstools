#ifndef COMPRESSION_TMO
#define COMPRESSION_TMO

#include <pfstmo.h>

class CompressionTMO
{
 public:
  void tonemap(const float *R_in, const float *G_in, float *B_in, int width, int height,
        float *R_out, float *G_out, float *B_out, const float *L_in,
        pfstmo_progress_callback progress_cb = NULL );
};


#endif
