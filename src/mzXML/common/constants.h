#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "sysdepend.h"

#define SIZE_BUF             8192
#define SIZE_FILE            1024
#define SIZE_PEP             128
#define INIT_INTERACT_LINES  1000
#define PERCENTAGE           0.75
#define EXTRAITRS            20

#define USE_LOCAL_TIME 1

#define SIZE_BUF 8192

#define MAX_CHARGE 7
//
// CONFIGURATION DEFINITIONS
// note these defaults can be overridden at runtime by the code
// in hooks_tpp.cxx
//

//
// Linux Installation
//
#if !(defined(__CYGWIN__)||defined(_WIN32))

#define TPP_DIR "/tpp/"

// Web Paths
#define DEFAULT_WEBSERVER_HOME_URL TPP_DIR
#define DEFAULT_CGI_BIN TPP_DIR"cgi-bin/"
#define DEFAULT_HELP_DIR TPP_DIR"html/"
#define DEFAULT_HTML_DIR TPP_DIR"html/"

// Filesystem Paths
//#define DEFAULT_TPP_INSTALL_ROOT "/tools/bin/TPP/tpp/"
#ifndef DEFAULT_TPP_INSTALL_ROOT
#error DEFAULT_TPP_INSTALL_ROOT must be defined as something like "/tools/bin/TPP/tpp/"
#endif
#define DEFAULT_CGI_FULL_BIN DEFAULT_TPP_INSTALL_ROOT"cgi-bin/"
#define DEFAULT_LOCAL_BIN DEFAULT_TPP_INSTALL_ROOT"bin/"
#define DEFAULT_COMETLINKSDIR DEFAULT_TPP_INSTALL_ROOT"etc/"
#define DEFAULT_LOCAL_HTML DEFAULT_TPP_INSTALL_ROOT"html/"



// schema
#define DEFAULT_PEPXML_STD_XSL DEFAULT_TPP_INSTALL_ROOT"schema/"  // a webserver reference, to the directory, filename is hard coded

// schema namespace constants
#define PEPXML_NAMESPACE "http://regis-web.systemsbiology.net/pepXML"

//
// Win32 or Cygwin Installation
//
#else  // not linux

#define TPP_DIR "/tpp-bin/"
#define DEFAULT_TPP_INSTALL_ROOT "C:/Inetpub"TPP_DIR

// these are for pepXML specific functionality
#define PEPXML_NAMESPACE "http://regis-web.systemsbiology.net/pepXML"
#define DEFAULT_COMETLINKSDIR DEFAULT_TPP_INSTALL_ROOT
#define DEFAULT_CGI_BIN TPP_DIR
#define DEFAULT_HELP_DIR DEFAULT_CGI_BIN
#define DEFAULT_HTML_DIR "/ISB/html/"
#define DEFAULT_PEPXML_STD_XSL "http://localhost/"
#define DEFAULT_CGI_FULL_BIN DEFAULT_TPP_INSTALL_ROOT

#ifdef __CYGWIN__
#define DEFAULT_LOCAL_BIN "/bin/"
#define DEFAULT_PEPXML_STD_XSL_WEB_PATH "http://localhost:1441/"
#define WINDOWS_CYGWIN  
#else // MinGW or MSVC
#define DEFAULT_LOCAL_BIN DEFAULT_TPP_INSTALL_ROOT
#define DEFAULT_PEPXML_STD_XSL_WEB_PATH "http://localhost/"
#endif
#endif // not linux

// pepXML stuff that's platform independent
#define DEFAULT_PEPXML_FILENAME_DOTEXT ".pep.xml" // was .xml before Jan 2008, can override with env var PEPXML_EXT
#define PEPXML_NAMESPACE_PX "pepx"
#define PEPXML_SCHEMA "pepXML_v114.xsd"

// protXML stuff that's platform independent
#define DEFAULT_PROTXML_FILENAME_DOTEXT ".prot.xml" // was -prot.xml before Jan 2008,, can override with env var PROTXML_EXT
#define PROTXML_SCHEMA "protXML_v6.xsd"

#define _ASAPRATIO_MXQ_ 5
#define CON_SIZE_FILE        256
#define CON_SIZE_PATH       4096
#define LABEL_MASS_SIZE 1000
#define AMINO_ACIDS "ARNDCEQGHILKMFPSTWYV"
#define LINE_WIDTH 1000000
#define USE_STD_MODS 1  // comment this out if don't use standard way of representing modifications
#define MOD_ERROR 0.5
# define _ISOMASSDIS_ 1.0033548378

//Comet constant (moved from Comet.h)
#define COMETLINKSFILE  "cometlinks.def"

struct RatioStruct
{
   int iNumPeptides;
   double dRatio;
   double dStdDev;
   double dh2lRatio;
   double dh2lStdDev;
};

#include <string.h>

class InputStruct
{
public:
   InputStruct() {
      memset(this,0,sizeof(InputStruct));
   }
   int iAnalysisFirstScan;
   int iAnalysisLastScan;
   int bZeroAllBackGrnd;      /* command line option */
   int bQuantHighBackGrnd;      /* command line option */
   int bUseSameScanRange;      /* command line option */
   int bUseFixedScanRange;     /* command line option (not really a bool, but a 3 way flag) */
   int iFixedScanRange;          /* command line option */
   int bQuantCIDChrgOnly;     /* command line option */
   int bXpressLight1;          /* command line option */
   int iMetabolicLabeling;
   char szXpressResidues1[256]; /* command line option */
   char szXpressResidues2[256]; /* command line option */
   char szXpressResidues3[256]; /* command line option */
   char szXpressResidues[256]; /* total of above */
   double dXpressMassDiff;     /* TO BE DEPRECATED AFTER ASAP BROUGHT ONBOARD */
   double dXpressMassDiff1;     /* command line option */
   double dXpressMassDiff2;     /* command line option */
   double dXpressMassDiff3;     /* command line option */
   double dMassTol;            /* command line option */
   char szMzXMLDir[CON_SIZE_PATH]; /* command line option */

   int iFirstScan;             /* from xml */
   int iLastScan;              /* from xml */
   int iChargeState;           /* from xml */
   char szPeptide[128];        /* from xml */
   double dPeptideMass;        /* from xml Measured Mass + H */
   double dCalcPeptideMass;        /* from xml Search Computed */
   char szOutFile[CON_SIZE_FILE];  /* from xml */
   char szSpectrumName[CON_SIZE_FILE];  /* from xml */
   bool staticQuant; // whether or not quant is based on static variables
   char labelMasses[LABEL_MASS_SIZE];
};

struct lcPeakStrct {
  int indx;
  int peak;
  int valley[2];
  double bckgrnd;
  double area[2];
  double time[2];
};


struct pepDataStrct {
  int indx;
  long scan;
  int chrg;
  int cidIndx;
  double msLight;
  double msHeavy;
  int eltn;
  int areaFlag;
  struct lcPeakStrct peaks[_ASAPRATIO_MXQ_][2];
  double pkRatio[_ASAPRATIO_MXQ_];
  double pkError[_ASAPRATIO_MXQ_];
  int pkCount[_ASAPRATIO_MXQ_];
  double pepRatio[2];
  double pepH2LRatio[2];
  double pepTime[2][2];
  double pepArea;
};


struct pairedLabel {
  char labelA[3];
  char labelB[3];
};

struct Pep3D_dataStrct {
  double prob;
  double score;
  int startScan;
  int endScan;
  int charge;
};

class spectStrct; // defined in spectStrct.h

#endif
