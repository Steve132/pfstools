
#ifndef __GLENV_H
#define __GLENV_H

#ifdef __APPLE__ // "Think different" for OS/X :)
#include "GLUT/glut.h"
#include "OPENGL/gl.h"
#include "OPENGL/glu.h"
#include "OPENGL/glext.h"
#else
 #if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
   #include <windef.h>
 #endif
#include "GL/glut.h"
#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glext.h"
#endif


// uncomment the define for a console live call graph
// this will not work in all situations, since some code is executed concurrently

//#define CALLGRAPH

#include <iostream>
#ifdef CALLGRAPH
extern int ident ;
  inline void dbs(std::string message) {           for(int i = ident-1; i > 0 ; i--)
	                                                 ((i==1)?printf("\u251c\u2500\u2500"):printf("\u2502  "));
                                                   printf("\u252c\u2500"); printf("%s\n", message.c_str()); ident++;
                                       }
  inline void dbe(std::string message) {           ident--;
  // comment out next three lines for more compact display
                                                   for(int i = ident; i > 0 ; i--)
	                                                 ((i==1)?printf("\u2514\u2500\u2500"):printf("\u2502  "));
                                                   printf("\u2518");   printf("\n",   message.c_str());
	                                   }
#else
  inline void dbs(std::string message) {}
  inline void dbe(std::string message) {}
#endif



#endif
