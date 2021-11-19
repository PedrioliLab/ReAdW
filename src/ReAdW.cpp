// -*- mode: c++ -*-

/*
	Program: ReAdW ("read double-u")
	Description: read Thermo/Xcalibur native RAW mass-spec data files,
	and produce XML (mzXML, mzML) output.  Please note, the program 
	requires the XRawfile library from ThermoFinnigan to run.

	Date: July 2007
	Author: Natalie Tasman, ISB Seattle, 2007, based on original work by 
	Pedrioli Patrick, ISB, Proteomics (original author), and Brian
	Pratt, InSilicos

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


#include "stdafx.h"
#include <string>
#include <process.h> // for system

#include "ThermoInterface.h"
#include "mzXML/common/mzXMLWriter.h"
#include "mzXML/common/ConverterArgs.h"
#include "mzXML/common/MSUtilities.h"
#include "mzXML/common/sysdepend.h"
#include "mzXML/common/util.h"
#include "mzXML/common/TPPVersion.h"



using namespace std;

void usage(const string& exename, const string& version) {
	cout << endl << exename << " " << version << endl << endl;
	cout << "Usage: " << exename << " [options] <raw file path> [<output file>]" << endl << endl
		<< " Options\n"
		<< endl
		<< "  --centroid, -c: Centroid all scans (MS1 and MS2)" << endl
		<< "      meaningful only if data was acquired in profile mode;"  << endl
		<< "      default: off" << endl
		<< "  [Advanced option, default OFF] --precursorFromFilterLine: only" << endl
		<< "      try to get the precursor MZ value from the Thermo" << endl
		<< "      \"filterline\" text; only use this if you have a good reason!" << endl
		<< "      Otherwise, the program first will try to obtain a more accurate" << endl
		<< "       mass from the \"Monoisotopic M/Z:\" \"trailer value\"" << endl 
		<< "  --nocompress, -n: Do not use zlib for compressing peaks" << endl
		<< "      default: on" << endl
		<< "  --verbose, -v:   verbose" << endl
		<< "  --gzip, -g:   gzip the output file (independent of peak compression)" << endl
		<< endl
		<< "  output file: (Optional) Filename for output file;" << endl
		<< "      if not supplied, the output file will be created" << endl
		<< "      in the same directory as the input file." << endl
		<< endl
		<< endl
		<< "Example: convert input.raw file to output.mzXML, centroiding MS1 and MS2 scans" << endl << endl
		<< "      " << exename << " --centroid C:\\test\\input.raw c:\\test\\output.mzXML" << endl << endl
		<< "Author: Natalie Tasman (SPC/ISB), with Jimmy Eng, Brian Pratt, and Matt Chambers," << endl
		<< "      based on orignal work by Patrick Pedriolli." << endl;
}





int main(int argc, char* argv[])
{
	// force the program name to ReAdW,
	// regardless of what it was called on the command line
	const char *execName = "ReAdW"; // argv[0];
	string version = toString(TPP_MAJOR_RELEASE_NUMBER) + "." + toString(TPP_MINOR_RELEASE_NUMBER) + "." + toString(TPP_REV_RELEASE_NUMBER) + "(build " __DATE__ " " __TIME__ ")";

	if (argc < 2) {
		// min args: output mode and input file 
		usage(execName, version);
		return -1;
	}
	cout << execName << " " << version << endl;

	ConverterArgs args;

	if (!args.parseArgs(argc, argv)) {
		usage(execName, version);
		return -1;
	}

	if (args.verbose) {
		args.printArgs();
	}

	ThermoInterface thermoInterface;
	// try to init the thermo library
	if (!thermoInterface.initInterface()) {
		cerr << "unable to interface with Thermo library" << endl;
		exit(-1);
	}


	if (!thermoInterface.setInputFile(args.inputFileName)) {
		cerr << "unable to open " << args.inputFileName << " with thermo interface" << endl;
		exit(-1);
	}

	thermoInterface.setCentroiding(args.centroidScans);
	thermoInterface.forcePrecursorFromFilter(args.forcePrecursorFromFilterLine);
	
	MassSpecXMLWriter* msWriter;
	if (args.mzXMLMode) {
		msWriter = new mzXMLWriter(execName, version, &thermoInterface);
	}

	if (args.verbose) {
		msWriter->setVerbose(true);
		thermoInterface.setVerbose(true);
	}

	if (!msWriter->setInputFile(args.inputFileName)) {
		cerr << "unable to set input file " << args.inputFileName << endl;
		exit(-1);
	}

	if (!msWriter->setOutputFile(args.outputFileName)) {
		cerr << "unable to open " << args.outputFileName << endl;
		exit(-1);
	}

	msWriter->setCentroiding(args.centroidScans);
	msWriter->setCompression(args.compressScans);

	msWriter->writeDocument();
	
	if (args.verbose) {
		int totalObsCharges = 0;
		cout << "observed precursor charges" << endl;
		for (int i=0; i<int(thermoInterface.chargeCounts_.size()); i++) {
			cout << "charge " << i << ":\t" << thermoInterface.chargeCounts_[i] << endl;
			totalObsCharges += thermoInterface.chargeCounts_[i];
		}
		cout << "total precursor charges: " << totalObsCharges << endl;
	}
	
	delete msWriter;
	return 0;
}
