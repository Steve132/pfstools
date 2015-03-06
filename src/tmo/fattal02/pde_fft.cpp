//////////////////////////////////////////////////////////////////////
// Direct Poisson solver using the discrete cosine transform
//////////////////////////////////////////////////////////////////////
// by Tino Kluge (tino.kluge@hrz.tu-chemnitz.de)
//
// let U and F be matrices of order (n1,n2), ie n1=height, n2=width
// and L_x of order (n2,n2) and L_y of order (n1,n1) and both
// representing the 1d Laplace operator with Neumann boundary conditions,
// ie L_x and L_y are tridiagonal matrices of the form
//
//  ( -2  2          )
//  (  1 -2  1       )
//  (     .  .  .    )
//  (        1 -2  1 )
//  (           2 -2 )
//
// then this solver computes U given F based on the equation
//
//  -------------------------
//  L_y U + (L_x U^tr)^tr = F
//  -------------------------
//
// Note, if the first and last row of L_x and L_y contained one's instead of
// two's then this equation would be exactly the 2d Poisson equation with
// Neumann boundary conditions. As a simple rule:
// - Neumann: assume U(-1)=U(0) --> U(i-1) - 2 U(i) + U(i+1) becomes
//        i=0: U(0) - 2 U(0) + U(1) = -U(0) + U(1)
// - our system: assume U(-1)=U(1) --> this becomes
//        i=0: U(1) - 2(0) + U(1) = -2 U(0) + 2 U(1)
//
// The multi grid solver solve_pde_multigrid() solves the 2d Poisson pde
// with the right Neumann boundary conditions, U(-1)=U(0), see function
// atimes(). This means the assembly of the right hand side F is different
// for both solvers.

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <vector>
#include <fftw3.h>

#include <array2d.h>

#include <config.h>

#include "pde.h"

using namespace std;


#ifndef SQR
#define SQR(x) (x)*(x)
#endif


// returns T = EVy A EVx^tr
// note, modifies input data
void transform_ev2normal(pfstmo::Array2Dd *A, pfstmo::Array2Dd *T)
{
  int width = A->getCols();
  int height = A->getRows();
  assert((int)T->getCols()==width && (int)T->getRows()==height);

  // the discrete cosine transform is not exactly the transform needed
  // need to scale input values to get the right transformation
  for(int y=1 ; y<height-1 ; y++ )
    for(int x=1 ; x<width-1 ; x++ )
      (*A)(x,y)*=0.25;

  for(int x=1 ; x<width-1 ; x++ )
  {
    (*A)(x,0)*=0.5;
    (*A)(x,height-1)*=0.5;
  }
  for(int y=1 ; y<height-1 ; y++ )
  {
    (*A)(0,y)*=0.5;
    (*A)(width-1,y)*=0.5;
  }

  // note, fftw provides its own memory allocation routines which
  // ensure that memory is properly 16/32 byte aligned so it can
  // use SSE/AVX operations (2/4 double ops in parallel), if our
  // data is not properly aligned fftw won't use SSE/AVX
  // (I believe new() aligns memory to 16 byte so avoid overhead here)
  //
  // double* in = (double*) fftw_malloc(sizeof(double) * width*height);
  // fftw_free(in);

  // executes 2d discrete cosine transform
  fftw_plan p;
  p=fftw_plan_r2r_2d(height, width, A->getRawData(), T->getRawData(),
                        FFTW_REDFT00, FFTW_REDFT00, FFTW_ESTIMATE);
  fftw_execute(p); 
  fftw_destroy_plan(p);
}


// returns T = EVy^-1 * A * (EVx^-1)^tr
void transform_normal2ev(pfstmo::Array2Dd *A, pfstmo::Array2Dd *T)
{
  int width = A->getCols();
  int height = A->getRows();
  assert((int)T->getCols()==width && (int)T->getRows()==height);

  // executes 2d discrete cosine transform
  fftw_plan p;
  p=fftw_plan_r2r_2d(height, width, A->getRawData(), T->getRawData(),
                        FFTW_REDFT00, FFTW_REDFT00, FFTW_ESTIMATE);
  fftw_execute(p); 
  fftw_destroy_plan(p);

  // need to scale the output matrix to get the right transform
  for(int y=0 ; y<height ; y++ )
    for(int x=0 ; x<width ; x++ )
      (*T)(x,y)*=(1.0/((height-1)*(width-1)));

  for(int x=0 ; x<width ; x++ )
  {
    (*T)(x,0)*=0.5;
    (*T)(x,height-1)*=0.5;
  }
  for(int y=0 ; y<height ; y++ )
  {
    (*T)(0,y)*=0.5;
    (*T)(width-1,y)*=0.5;
  }
}

// returns the eigenvalues of the 1d laplace operator
std::vector<double> get_lambda(int n)
{
  assert(n>1);
  std::vector<double> v(n);
  for(int i=0; i<n; i++)
    v[i]=-4.0*SQR(sin((double)i/(2*(n-1))*M_PI));

  return v;
}

// makes boundary conditions compatible so that a solution exists
void make_compatible_boundary(pfstmo::Array2Dd *F)
{
  int width = F->getCols();
  int height = F->getRows();

  double sum=0.0;
  for(int y=1 ; y<height-1 ; y++ )
    for(int x=1 ; x<width-1 ; x++ )
      sum+=(*F)(x,y);

  for(int x=1 ; x<width-1 ; x++ )
    sum+=0.5*((*F)(x,0)+(*F)(x,height-1));

  for(int y=1 ; y<height-1 ; y++ )
    sum+=0.5*((*F)(0,y)+(*F)(width-1,y));

  sum+=0.25*((*F)(0,0)+(*F)(0,height-1)+(*F)(width-1,0)+(*F)(width-1,height-1));

  DEBUG_STR << "compatible_boundary: int F = " << sum ;
  DEBUG_STR << " (should be 0 to be solvable)" << std::endl;

  double add=-sum/(height+width-3);
  DEBUG_STR << "compatible_boundary: adjusting boundary by " << add << std::endl;
  for(int x=0 ; x<width ; x++ )
  {
    (*F)(x,0)+=add;
    (*F)(x,height-1)+=add;
  }
  for(int y=1 ; y<height-1 ; y++ )
  {
    (*F)(0,y)+=add;
    (*F)(width-1,y)+=add;
  }
}



// solves Laplace U = F with Neumann boundary conditions
// if adjust_bound is true then boundary values in F are modified so that
// the equation has a solution, if adjust_bound is set to false then F is
// not modified and the equation might not have a solution but an
// approximate solution with a minimum error is then calculated
// double precision version
// note, input data F might be modified
void solve_pde_fft(pfstmo::Array2Dd *F, pfstmo::Array2Dd *U, bool adjust_bound)
{
  DEBUG_STR << "solve_pde_fft: solving Laplace U = F ..." << std::endl;
  int width = F->getCols();
  int height = F->getRows();
  assert((int)U->getCols()==width && (int)U->getRows()==height);

  // activate parallel execution of fft routines
  fftw_init_threads();
  fftw_plan_with_nthreads(omp_get_max_threads());

  // in general there might not be a solution to the Poisson pde
  // with Neumann boundary conditions unless the boundary satisfies
  // an integral condition, this function modifies the boundary so that
  // the condition is exactly satisfied
  if(adjust_bound)
  {
    DEBUG_STR << "solve_pde_fft: checking boundary conditions" << std::endl;
    make_compatible_boundary(F);
  }

  // transforms F into eigenvector space: Ftr = 
  DEBUG_STR << "solve_pde_fft: transform F to ev space (fft)" << std::endl;
  pfstmo::Array2Dd* F_tr = new pfstmo::Array2Dd(width,height);
  transform_normal2ev(F, F_tr);
  F->allocate(1,1);       // no longer needed so release memory (better not?)
  DEBUG_STR << "solve_pde_fft: F_tr(0,0) = " << (*F_tr)(0,0);
  DEBUG_STR << " (must be 0 for solution to exist)" << std::endl;

  // in the eigenvector space the solution is very simple
  DEBUG_STR << "solve_pde_fft: solve in eigenvector space" << std::endl;
  pfstmo::Array2Dd* U_tr = new pfstmo::Array2Dd(width,height);
  std::vector<double> l1=get_lambda(height);
  std::vector<double> l2=get_lambda(width);
  for(int y=0 ; y<height ; y++ )
    for(int x=0 ; x<width ; x++ )
    {
      if(x==0 && y==0)
        (*U_tr)(x,y)=0.0; // any value ok, only adds a const to the solution
      else
        (*U_tr)(x,y)=(*F_tr)(x,y)/(l1[y]+l2[x]);
    }
  delete F_tr;    // no longer needed so release memory


  // transforms U_tr back to the normal space
  DEBUG_STR << "solve_pde_fft: transform U_tr to normal space (fft)" << std::endl;
  transform_ev2normal(U_tr, U);
  delete U_tr;    // no longer needed so release memory

  // the solution U as calculated will satisfy something like int U = 0
  // since for any constant c, U-c is also a solution and we are mainly
  // working in the logspace of (0,1) data we prefer to have
  // a solution which has no positive values: U_new(x,y)=U(x,y)-max
  // (not really needed but good for numerics as we later take exp(U))
  DEBUG_STR << "solve_pde_fft: removing constant from solution" << std::endl;
  double max=0.0;
  for(int i=0; i<width*height; i++) {    
    if(max<(*U)(i))
      max=(*U)(i);
  }

  for(int i=0; i<width*height; i++)
    (*U)(i)-=max;


  // fft parallel threads cleanup, better handled outside this function
  // fftw_cleanup_threads();

  DEBUG_STR << "solve_pde_fft: done" << std::endl;
}

// solves Laplace U = F
// single precision version (right now simply calls the double prec version)
void solve_pde_fft(pfstmo::Array2D *F, pfstmo::Array2D *U, bool adjust_bound)
{
  int width = F->getCols();
  int height = F->getRows();
  assert((int)U->getCols()==width && (int)U->getRows()==height);

  pfstmo::Array2Dd* Fd = new pfstmo::Array2Dd(width,height);
  pfstmo::Array2Dd* Ud = new pfstmo::Array2Dd(width,height);

  // convert float array to double array
  for(int i=0; i<width*height; i++)
    (*Fd)(i)=(*F)(i);

  solve_pde_fft(Fd,Ud,adjust_bound);

  // convert float array to double array
  for(int i=0; i<width*height; i++)
    (*U)(i)=(*Ud)(i);
}

// ---------------------------------------------------------------------
// the functions below are only for test purposes to check the accuracy
// of the pde solvers


// returns the norm of (Laplace U - F) of all interior points
// useful to compare solvers
float residual_pde(pfstmo::Array2D* U, pfstmo::Array2D* F)
{
  int width = U->getCols();
  int height = U->getRows();
  assert((int)F->getCols()==width && (int)F->getRows()==height);

  double res=0.0;
  for(int y=1;y<height-1;y++)
    for(int x=1;x<width-1;x++)
    {
      double laplace=-4.0*(*U)(x,y)+(*U)(x-1,y)+(*U)(x+1,y)
                     +(*U)(x,y-1)+(*U)(x,y+1);
      res += SQR( laplace-(*F)(x,y) );
    }
  return (float) sqrt(res);
}


// laplace operator with boundary conditions consistent with the
// fft solver, ie U(-1)=U(1)
// calulates F with F = Laplace U
template <typename T>
void laplace_fft(pfstmo::Array2DBase<T>* U, pfstmo::Array2DBase<T>* F)
{
  int width = U->getCols();
  int height = U->getRows();
  assert((int) F->getCols()==width && (int)F->getRows()==height);
  for(int y=0;y<height;y++)
    for(int x=0;x<width;x++)
    {
      // x+1 and y+1
      int xp = (x+1==width)  ? width-2  : x+1;
      int yp = (y+1==height) ? height-2 : y+1;
      // x-1, y-1
      int xm = (x==0) ? 1 : x-1;
      int ym = (y==0) ? 1 : y-1;

      (*F)(x,y)=-4.0*(*U)(x,y)+(*U)(xm,y)+(*U)(xp,y)+(*U)(x,ym)+(*U)(x,yp);
    }
} 

// gives a component wise error estimate of the fft pde solver
// it initialises a solution Uexact with random values between 0 and 1,
// applies the laplace operator and then inverts it,
// basically it solves for U in Laplace U = Laplace Uexact
// and then compares U with Uexact
// examples results:
//    9000x6000: 6.6*10^-13
//    6000x4000: 1.8*10^-13
//    3000x2000: 1.2*10^-13
//    1680x1050: 1.1*10^-13
//     640x 480: 1.9*10^-14
//     100x 100: 3.4*10^-15
//      10x  10: 1.4*10^-16
double error_estim_pde_fft_d(unsigned int width, unsigned int height)
{
  pfstmo::Array2Dd* F = new pfstmo::Array2Dd(width,height);
  pfstmo::Array2Dd* Uexact = new pfstmo::Array2Dd(width,height);
  pfstmo::Array2Dd* U = new pfstmo::Array2Dd(width,height);

  // initialise exact solution
  for(unsigned int i=0; i<width*height; i++)
    (*Uexact)(i)= (double) random() / (RAND_MAX+1.0);

  // apply the laplace operator
  laplace_fft(Uexact, F);

  // solve equation Laplace U = F
  solve_pde_fft(F, U, false);

  // compare U with exact result Uexact, difference must be a constant
  double mean=0.0;
  for(unsigned int i=0; i<width*height; i++)
    mean+=(*U)(i)-(*Uexact)(i);
  mean/=(width*height);
  double var=0.0;
  for(unsigned int i=0; i<width*height; i++)
    var+= SQR( (*U)(i)-(*Uexact)(i)-mean );
  var/=(width*height-1);

  DEBUG_STR << "test_pde_fft(" << width << ", " << height << "): ";
  DEBUG_STR << "U-Uexact mean diff: " << mean << ", stdev: " << sqrt(var);
  DEBUG_STR << std::endl;

  return sqrt(var);
}
// error estim for the single precision version
// examples results when used with internal double precision fft:
//    9000x6000: 3.8*10^-5
//    2000x3000: 1.1*10^-5
//     640x 480: 1.6*10^-6
//      10x  10: 4.3*10^-8
double error_estim_pde_fft(unsigned int width, unsigned int height)
{
  pfstmo::Array2D* F = new pfstmo::Array2D(width,height);
  pfstmo::Array2D* Uexact = new pfstmo::Array2D(width,height);
  pfstmo::Array2D* U = new pfstmo::Array2D(width,height);

  // initialise exact solution
  for(unsigned int i=0; i<width*height; i++)
    (*Uexact)(i)= (double) random() / (RAND_MAX+1.0);

  // apply the laplace operator
  laplace_fft(Uexact, F);

  // solve equation Laplace U = F
  solve_pde_fft(F, U, false);

  // compare U with exact result Uexact, difference must be a constant
  double mean=0.0;
  for(unsigned int i=0; i<width*height; i++)
    mean+=(double) (*U)(i)-(*Uexact)(i);
  mean/=(width*height);
  double var=0.0;
  for(unsigned int i=0; i<width*height; i++)
    var+= SQR( (double) (*U)(i)-(*Uexact)(i)-mean );
  var/=(width*height-1);

  DEBUG_STR << "test_pde_fft(" << width << ", " << height << "): ";
  DEBUG_STR << "U-Uexact mean diff: " << mean << ", stdev: " << sqrt(var);
  DEBUG_STR << std::endl;

  return sqrt(var);
}
