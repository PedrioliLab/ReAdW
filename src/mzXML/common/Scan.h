// -*- mode: c++ -*-


/*
    File: Scan.h
    Description: instrument-independent scan representation.
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
#include <vector>

#include "MSTypes.h"

class NativeScanRef {
public:
	typedef std::pair<ScanCoordinateType,std::string> CoordinateNameValue;
public:
	// TODO: use another type for manufacturers with multiple acq. systems, like Agilent
	MSManufacturerType coordinateType_;
	std::vector<CoordinateNameValue> coordinates_;

public:
	inline std::vector<CoordinateNameValue>::size_type getNumCoordinates(void) const 
	{
		return coordinates_.size();
	}
	inline void addCoordinate(ScanCoordinateType name, const std::string &value)
	{
		coordinates_.push_back(CoordinateNameValue(name, value));
	}
	inline void getCoordinate(std::vector<CoordinateNameValue>::size_type index, ScanCoordinateType &name, std::string &value)
	{
		if (index>=0 && index<coordinates_.size()) {
			const CoordinateNameValue &coordinate = coordinates_[index];
			name = coordinate.first;
			value = coordinate.second;
		} else {
			name = SCAN_COORDINATE_UNDEF;
			value.resize(0);
		}
	}
	inline void setCoordinateType (MSManufacturerType coordinateType)
	{
		coordinateType_ = coordinateType;
	}
	inline MSManufacturerType getCoordinateType (void)
	{
		return coordinateType_;
	}

public:
	NativeScanRef() {}
	NativeScanRef(MSManufacturerType coordinateType) {setCoordinateType(coordinateType);}
	~NativeScanRef() {}
};

class Scan {
public:
	int msLevel_;
	int charge_;

	// scan range
	double startMZ_; // min scanned
	double endMZ_; // max scanned

	double minObservedMZ_; // min observed
	double maxObservedMZ_; // min observed

	// base peak: peak with greatest intesity
	double basePeakMZ_;
	double basePeakIntensity_;

	double totalIonCurrent_;
	double retentionTimeInSec_;

	double injectionTimeInSec_;

	MSPolarityType polarity_;
	MSAnalyzerType analyzer_; // keep this per-scan, for example for FT which has IT or FT
	MSIonizationType ionization_;
	MSScanType scanType_; // full, zoom, srm, etc
	MSActivationType activation_;

	bool isCentroided_;

	// for MSn >= 2
	long precursorScanNumber_;
	int precursorScanMSLevel_;
	int precursorCharge_;
	double precursorMZ_;
	bool accuratePrecursorMZ_;
	double precursorIntensity_;
	double collisionEnergy_;  // for MSn, this refers to the collision which produced the nth level fragment

	// for thermo scans only
	bool isThermo_;
	std::string thermoFilterLine_;

	// for MassLynx_ scans only
	bool isMassLynx_;
	bool isCalibrated_;

	// for MS2 averaging or merging
	bool isMerged_;
	long mergedScanNum_;

	bool isThresholded_;
	double threshold_;

	NativeScanRef nativeScanRef_;

protected:
	int numDataPoints_;

public:
	int getNumDataPoints(void) const {return numDataPoints_;}
	void setNumDataPoints(int numDataPoints); // (re)allocates arrays
	void resetNumDataPoints(int numDataPoints); // set actual number of data points

	double* mzArray_;
	double* intensityArray_;


protected:
	int numScanOrigins_;

public:
	void setNumScanOrigins(int numScanOrigins);
	int getNumScanOrigins(void) {return numScanOrigins_;}
	int isScanMergedResult() { return (scanOriginNums.size()>0 ? 1 : 0); }


	std::vector<long> scanOriginNums;
	std::vector<std::string> scanOriginParentFileIDs;

	// centroid processing
        // copied from SpectraSTPeakList, with Henry's permission
	void centroid(std::string instrument);

	// thresholding -- rewrite the spectra, either deleting or zeroing
	void threshold(double inclusiveCutoff, bool discard); // if not discard, rewrite as zero

public:
	Scan();
	Scan(const Scan& copy);
	~Scan();
};


