// -*- mode: c++ -*-


/*
    File: ConverterArgs.cpp
    Description: common program argument parsing for MS data converters.
    Date: July 25, 2007

    Copyright (C) 2007 Natalie Tasman, ISB Seattle


    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/


#include "ConverterArgs.h"
#include <iostream>

using namespace std;
#if defined(_MSC_VER) || defined(__MINGW32__)  // MSVC or MinGW
#ifndef strcasecmp
#define strcasecmp _stricmp
#endif
#endif


ConverterArgs::ConverterArgs() {
	centroidScans = false;
	compressScans = false;
	verbose = false;
	lockspray = true;
	shotgunFragmentation = false;
	mzMLMode = false;
	mzXMLMode = false;
	forcePrecursorFromFilterLine = false;
	// experimental:
	threshold = false;
	thresholdDiscard = false;
	inclusiveCutoff = -1;
	inputFileName="";
	outputFileName="";
	gzipOutputFile = false; // write entire file as gzip? (independent of scan compression)

}

void
ConverterArgs::printArgs(void) {
	cout 
		<< "Settings:" << endl
		<< "  centroid scans: " << centroidScans << endl
		<< "  compress scans: " << compressScans << endl
		<< "  verbose mode: " << verbose << endl
		<< "  lockspray: " << lockspray << endl
		<< "  MS^E: " << shotgunFragmentation << endl
		<< "  threshold: " << threshold << endl
		<< "  force precursor selection from filter line: " << forcePrecursorFromFilterLine << endl;
	if (threshold) {
		cout << "    threshold inclusive cutoff: " << inclusiveCutoff << endl;
		cout << "    threshold mode: ";
		if (thresholdDiscard) {
			cout << "discarding peaks below cutoff" << endl;
		}
		else {
			cout << "setting intensity to zero for peaks below cutoff" << endl;
		}
	}
	cout
		<< "  mzML mode: " << mzMLMode << endl
		<< "  mzXML mode: " << mzXMLMode << endl
		<< "  input filename: " << inputFileName << endl
		<< "  output filename: " << outputFileName << endl
		<< "  output file as gzip: " << gzipOutputFile << endl
		<< endl;
};

bool 
ConverterArgs::parseArgs(int argc, char* argv[]) {
	int curArg = 1;
	for( curArg = 1 ; curArg < argc ; ++curArg ) {
		if (*argv[curArg] == '-') {
			if (strcmp(argv[curArg], "-c") == 0) {
				centroidScans = true;
			}
			else if (strcmp(argv[curArg], "--centroid") == 0) {
				centroidScans = true;
			}
			else if (strcmp(argv[curArg], "-z") == 0 ) { 
				compressScans = true;
			}
			else if (strcmp(argv[curArg], "--compress") == 0) {
				compressScans = true;
			}
			else if (strcmp(argv[curArg], "-g") == 0) {
				gzipOutputFile = true;
			}
			else if (strcmp(argv[curArg], "--gzip") == 0) {
				gzipOutputFile = true;
			}
			else if (strcmp(argv[curArg], "-v") == 0 ) { 
				verbose = true;
			}
			else if (strcmp(argv[curArg] , "--verbose") == 0) {
				verbose = true;
			}
			else if (strcmp(argv[curArg] , "--precursorFromFilterLine") == 0) {
				forcePrecursorFromFilterLine = true;
			}
			else if (strcmp(argv[curArg], "--MSe") == 0) {
				shotgunFragmentation = true;
			}
			else if (strcmp(argv[curArg], "--nolockspray") == 0) {
				lockspray = false;
			}
			else if(strcmp( argv[curArg] , "--mzXML") == 0) {
				mzXMLMode = true;
			}
			else if (strcmp(argv[curArg] , "--mzML") == 0) {
				mzMLMode = true;
			}
			else if (strcmp(argv[curArg] , "--TD") == 0) {
				threshold = true;
				thresholdDiscard = true;
				++curArg;
				if (curArg >= argc) {
					cerr << "  error: expected threshold cutoff value after --TD" << endl;
					exit(-1);
				}
				inclusiveCutoff = atof(argv[curArg]);
			}
			else if (strcmp(argv[curArg] , "--TZ") == 0) {
				threshold = true;
				thresholdDiscard = false;
				++curArg;
				if (curArg >= argc) {
					cerr << "  error: expected threshold cutoff value after --TZ" << endl;
					exit(-1);
				}
				inclusiveCutoff = atof(argv[curArg]);
			}
			else {
				cerr << "  error: unrecognized option " << argv[curArg] << endl;
				return false;
			}
		}
		else {
			// we're done with options; anything after should be filename stuff
			break;
		}
	}

	if (mzMLMode && mzXMLMode) {
		cerr << "  error: only one of mzXML or mzML mode can be selected" << endl;
		return false;
	}
	else if (!(mzXMLMode || mzMLMode)) {
		cerr << "  error: either mzXML or mzML mode must be selected" << endl;
		return false;
	}

	if (curArg > argc-1) {
		cerr << "  error: no input file specified" << endl;
		return false;
	} else {
		inputFileName = argv[curArg];
	}

	curArg++;
	if (curArg > argc-1) {
		// no output filename specified
		// cout << "(no output filename specified; using inputfile filename)" << endl;
		string::size_type dotPos = inputFileName.find_last_of('.');
		if (dotPos == string::npos) {
			cerr << "  error: input file name did not have extension" << endl;
			return false;
		}
		else {
			outputFileName = inputFileName.substr(0, dotPos);
			if (mzMLMode) {
				outputFileName += ".mzML";
			}
			else if (mzXMLMode) {
				outputFileName += ".mzXML";
			}
		}
	} else {
		outputFileName = argv[curArg];
	}

	if (gzipOutputFile) { // write entire file as gzip? (independent of scan compression)
		string::size_type dotPos = outputFileName.find_last_of('.');
		if ((dotPos==string::npos) || strcasecmp(outputFileName.c_str()+dotPos,".gz")) {
			outputFileName += ".gz";  // this will trigger gzip compression on output
		}
	}

	return true;
}
