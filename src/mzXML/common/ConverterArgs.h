// -*- mode: c++ -*-


/*
    File: ConverterArgs.h
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

#pragma once

#include <string>


class ConverterArgs {
public:
	bool centroidScans;
	bool compressScans;
	bool verbose;
	bool lockspray;
	bool shotgunFragmentation;
	bool mzMLMode;
	bool mzXMLMode;
	bool forcePrecursorFromFilterLine; // if true, only get the precursorMZ from the Thermo "filterLine" text
	// experimental:
	bool threshold;
	bool thresholdDiscard;
	double inclusiveCutoff;
	std::string inputFileName;
	std::string outputFileName;
	bool gzipOutputFile; // write entire file as gzip? (independent of scan compression

	ConverterArgs();
	void printArgs(void);
	bool parseArgs(int argc, char* argv[]);
};
