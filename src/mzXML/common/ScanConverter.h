// -*- mode: c++ -*-


/*
    File: ScanConverter.h
    Description: spectrum data to base64-encoded string converter.
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

#include "zlib.h"
#include "Scan.h"

// one time base64 encoder and compression,
// frees memory on destruction

class ScanConverter {
public:
	ScanConverter(double mz, double inten);
	ScanConverter(Scan* scan,
		bool tryZlibCompression);
	~ScanConverter();

	int mzRatioArrayLength_;
	int intensityArrayLength_;
	int mzIntArrayLength_;

	bool isMZRatioCompressed_; // was zlib compression sucessful?
	bool isIntensityCompressed_; // was zlib compression sucessful?
	bool isMZIntCompressed_; // was zlib compression sucessful?

	uLong mzRatioArrayCompressedByteSize_;
	uLong intensityArrayCompressedByteSize_;
	uLong mzIntArrayCompressedByteSize_;

	char* mzRatioB64String_;
	char* intensityB64String_;
	char* mzIntB64String_;

	int mzRatioB64StringEncodedByteLen_;
	int intensityB64StringEncodedByteLen_;
	int mzIntB64StringEncodedByteLen_;

private:
	Scan* scan_;
	bool tryZlibCompression_;
};
