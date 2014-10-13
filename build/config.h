/* config.h.in.  Generated from configure.ac by autoheader.  */


/* Define to the version of this package. */
#define PACKAGE_VERSION "1.9.0"

#define PACKAGE_STRING "pfstools 1.9.0" 

/* Directory where data files are located. */
#define PKG_DATADIR ""

/* Define to 1 if you have the ANSI C header files. */
//#undef STDC_HEADERS

/* Version number of package */
//#undef VERSION

/* Windows compilation. */
//#undef WIN32

#if 0
  #define CYGWIN
#endif


/* Output stream for debug messages. */
#ifdef DEBUG
#define DEBUG_STR std::cerr
#else
#define DEBUG_STR if(1) {;} else std::cerr
#endif

/* Output stream for verbose messages */        
#define VERBOSE_STR if(verbose) std::cerr << PROG_NAME << ": "        

#if defined(_WIN32) || defined(_WIN64) 
#define strcasecmp _stricmp 
#define strncasecmp _strnicmp 
#endif

#ifdef BRANCH_PREDICTION
#define likely(x)	__builtin_expect((x),1)
#define unlikely(x)	__builtin_expect((x),0)
#else
#define likely(x)	(x)
#define unlikely(x)	(x)
#endif

