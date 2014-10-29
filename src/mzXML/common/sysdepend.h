//
// sysdepend.h
//
// system dependencies, syntactic sugar hidden here.
//

#ifndef SYSDEPEND_INC
#define SYSDEPEND_INC

#ifdef _MSC_VER // microsoft weirdness
#pragma warning(disable:4305) // don't bark about double to float conversion
#pragma warning(disable:4244) // don't bark about double to float conversion
#pragma warning(disable:4786) // don't bark about "identifier was truncated to '255' characters in the browser information"
#pragma warning(disable:4996) // don't bark about "unsafe" functions
#endif

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>

#ifndef TPPLIB
#define TPPLIB // so RAMP, for one, knows it can call on other tpplib functions
#endif

//
// installdir issues (functions in util.c)
//
bool getIsInteractiveMode(); // TPP (web oriented) vs LabKey (headless)
const char *getLocalBin(); // filesystem path
const char *getLocalHTML(); // filesystem path
const char *getCGIBin(); // web path
const char *getCGIFullBin(); // filesystem path
const char *getCometLinksDir();
const char *getPepXML_std_xsl();
const char *getPepXML_std_xsl_web_path();
const char *getHelpDir(); // web path
const char *getHTMLDir(); // web path
#ifndef LOCAL_BIN
#define LOCAL_BIN getLocalBin()
#endif
#define LOCAL_HTML getLocalHTML()
#define CGI_BIN getCGIBin()
#define CGI_FULL_BIN getCGIFullBin()
#define COMETLINKSDIR getCometLinksDir()
#define PEPXML_STD_XSL getPepXML_std_xsl()
#define PEPXML_STD_XSL_WEB_PATH getPepXML_std_xsl_web_path()
#define HELP_DIR getHelpDir()
#define HTML_DIR getHTMLDir()

//
// stuff for cygwin vs win32
//
#include <stdio.h> // for FILE decl
#ifndef TPPLIB_SWIG  // don't need these for SWIG wrapper
int win32_system(const char *cmd);
FILE *win32_popen(const char *cmd, const char *mode);
#else
#define _WINSOCK2API_ // prevent include of winsock2
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)  // MSVC or MinGW
#define WINDOWS_NATIVE
#endif

#ifdef WINDOWS_NATIVE
#include <winsock2.h> // including this here makes sure the Array class doesn't mess up SDK includes later
// rand isn't as random as rand48, but hobble by
#define drand48() ((double)rand()/(double)RAND_MAX))
#define lrand48() rand()
#define srand48(seed) srand(seed)
#define srandom(seed) srand(seed)
#define random() rand()
#include <direct.h>
#include <io.h>
#ifndef TPPLIB_SWIG  // don't need these for SWIG wrapper
#define sleep(sec) Sleep(sec*1000)
typedef int gid_t;
typedef int uid_t;
#endif
#if defined(_MSC_VER)
#define TPP_ARCH "MSVC"
#ifndef TPPLIB_SWIG  // don't need these for SWIG wrapper
typedef int mode_t;
#endif
#define lrint(num) (long)floor((num) + 0.5)
#define round(num) (long)floor((num) + 0.5)
#define isnan _isnan
#define isfinite _finite
#define isinf !_finite
#define tempnam _tempnam
#define mktemp(s) _mktemp(s)
#ifndef TPPLIB_SWIG  // don't need these for SWIG wrapper
#define S_ISREG(mode) ((mode)&_S_IFREG)
#define S_ISDIR(mode) ((mode)&_S_IFDIR)
#endif
#ifndef F_OK // for use with access()
#define F_OK 0 // exists
#define W_OK 2 // write OK
#define R_OK 4 // read OK
#endif
#define snprintf _snprintf
#ifndef strcasecmp
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif
inline _int64 strtoull(const char *str, const char **end,int base) {
	if ((end!=NULL) || (10!=base)) {
		puts("bad call to fake strtoull");
		exit(-1);
	}
	return _atoi64(str);
}
#else
#define TPP_ARCH "MinGW"
#include <sys/types.h>
#endif
#define GNUPLOT_BINARY "wgnuplot.exe"
#define GNUPLOT_BINARY_PIPE "pgnuplot.exe"
#define USING_RELATIVE_WEBSERVER_PATH 1
// we have to do a bit of preprocessing to deal with all the unixy system and pipe calls in TPP
#define system(a) win32_system(a)  /* function is in win32_system.c */
#define popen(a,b) win32_popen(a,b) /* function is in win32_system.c */
// this may seem redundant, but not when "using ::system;" is in play as in some gcc hdr files
#define tpplib_system(a) win32_system(a)  /* function is in win32_system.c */
#define tpplib_popen(a,b) win32_popen(a,b) /* function is in win32_system.c */
#define pclose(a) _pclose(a)
#else // GCC
#include <unistd.h>
#ifdef __CYGWIN__
#define GNUPLOT_BINARY "gnuplot.exe"
#define GNUPLOT_BINARY_PIPE "gnuplot.exe"
#define USING_RELATIVE_WEBSERVER_PATH 1
// we have to do a bit of preprocessing to make sure we invoke cygwin utils and not anything else
#define system(a) win32_system(a)  /* function is in win32_system.c */
#define popen(a,b) win32_popen(a,b) /* function is in win32_system.c */
// this may seem redundant, but not when "using ::system;" is in play as in some gcc hdr files
#define tpplib_system(a) win32_system(a)  /* function is in win32_system.c */
#define tpplib_popen(a,b) win32_popen(a,b) /* function is in win32_system.c */
#define TPP_ARCH "Cygwin"
#else
#define TPP_ARCH "linux"
#define GNUPLOT_BINARY "gnuplot"
#define GNUPLOT_BINARY_PIPE "gnuplot"
#define tpplib_system(a) system(a)  
#define tpplib_popen(a,b) popen(a,b)
#endif
#endif

#ifndef WEXITSTATUS
#define WEXITSTATUS(x) x
#endif

#ifndef Stat_t
#define Stat_t struct stat
#endif

#ifndef Boolean 
typedef unsigned int Boolean;
#endif

#ifndef u_char
typedef unsigned char u_char;
#endif

#ifndef u_short
typedef unsigned short u_short;
#endif

#ifndef u_int
typedef unsigned int u_int;
#endif

#ifndef u_long
typedef unsigned long u_long;
#endif

#ifndef True
#define True 1
#endif

#ifndef False
#define False 0
#endif

#define bDebug False

#define myfabs(a) ((a) < 0.0 ? (a) * -1.0 : (a))

#define DABS(a) ((a) < 0.0 ? (a) * -1.0 : (a))

//#define MIN(a,b) (a < b ? a : b)
//#define MAX(a,b) (a > b ? a : b)

//
// class to instatiate at program start to handle install dir issues etc
//
#include "hooks_tpp.h"

/*
  like strdup(), but uses new instead of malloc
*/
inline char* strCopy(const char* orig) {
   if (!orig) {
      return NULL;
   }
   size_t len;
   char* output = new char[(len=strlen(orig)+1)];
   memcpy(output, orig, len);
   return output;
}

// faster and safer than #define style macro, which may evaluate twice
template<typename T> inline const T& Min(const T &a, const T &b) {
   return (a<b)?a:b; 
}
template<typename T> inline const T& Max(const T &a, const T &b) {
   return (a>b)?a:b; 
}


inline char* strlwr(char* orig) {
   if (!orig) {
       return NULL;
   }
   for (char *o=orig;*o;o++) {
     *o = tolower(*o);
   }
   return orig;
}

inline void cnvtUpper(char* o) {
   while (*o) {
     *o = toupper(*o);
	 o++;
   }
}

inline bool isEmptyFile(const char *file) {
  Stat_t statbuf;
  bool result=(0==stat(file,&statbuf));
  if (result) {
    if (statbuf.st_size <= 0) {
      result = true;
    }
    else {
      result = false;
    }
  }
  else {
    result = true;
  }
  return result;
}

inline double sqr(double x) {
   return x*x;
}

inline float sqr(float x) {
   return x*x;
}

inline const char *getCmdlineQuoteChar() {
#ifdef WINDOWS_NATIVE
    return "\"";
#else 	
    return "'";
#endif
}
#endif      // SYDEPEND_INC
