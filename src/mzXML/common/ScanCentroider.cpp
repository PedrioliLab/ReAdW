// -*- mode: c++ -*-


/*
File: ScanConverter.cpp
Description: centroider helper.  Kindly contributed from Mike Hoopman,
originally from the Hardklor project
Date: June 23, 2008


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

#include "ScanCentroider.h"
#include "common/spectStrct.h"

#include "math.h"
#include <iostream>
#include <vector>

using namespace std;


//Generic peak pair
typedef struct Peak_T{
	double mz;
	double intensity;
} Peak_T;

typedef struct scanLine{
	int header;
	double time;
	double mz;
	double intensity;
} scanLine;


ScanCentroider::ScanCentroider(Scan* scan) {
	scan_ = scan;
}

ScanCentroider::~ScanCentroider() {}


void
ScanCentroider::smooth(void) {
	spectStrct* ss=new spectStrct();

	ss->setsize(scan_->getNumDataPoints());

	// why is there no set_xvals?
	memmove(ss->xval, scan_->mzArray_, (scan_->getNumDataPoints())*sizeof(double));
	ss->set_yvals(scan_->intensityArray_);

	// range: 1 dalton minimum
	// repeat: 4 times
	// smoothing window: 10 data point minimum


	ss->smoothSpectFlx(1,4,10);

	
	memmove(scan_->mzArray_, ss->xval, (scan_->getNumDataPoints())*sizeof(double));
	memmove(scan_->intensityArray_, ss->yval, (scan_->getNumDataPoints())*sizeof(double));
}




bool
ScanCentroider::centroidScan(bool smooth) {

	int res400 = 15000; // TODO: set this
	if (scan_->isCentroided_) {
		cerr << "Error: attempt to centroid previously centroided scan" << endl;
		return false;
	}

	if (smooth) {
		this->smooth();
	}

	// == begin centroid code originally from Mike Hoopman ('MH' in comments) ==

	//MH:
	// First derivative method (quick hack - not elegant), assumes high enough resolution that each 
	// change in direction is a true peak.
	long i,j;
	double maxIntensity;
	long bestPeak;
	bool bLastPos;
	vector<Peak_T> centroidedSpectra;

	int nextBest;
	double FWHM;
	Peak_T centroid;

	bLastPos=false;

	//MH: step along each point in spectrum
	for (i=0; i<scan_->getNumDataPoints() - 1; i++){

		//MH: check for change in direction
		if(scan_->intensityArray_[i] < scan_->intensityArray_[i+1]) {
			bLastPos=true;
			continue;
		}
		else {
			if (bLastPos){
				bLastPos=false; // (jmt: reset, for next apex search)

				//MH: find max 
				// This is an artifact of using a window of length n (user variable) for identifying
				// a better peak apex on low-res data. Feel free to remove this section if desired.
				// Replace with:
				//   bestPeak=j;
				//   maxIntensity=s[j].intensity;
				maxIntensity=0;
				for(j=i;j<i+1;j++) { // jmt: why only looking 1 point ahead?
					if (scan_->intensityArray_[j] > maxIntensity){
						maxIntensity=scan_->intensityArray_[j];
						bestPeak = j;
					}
				}

				//MH:
				// Best estimate of Gaussian centroid
				// Get 2nd highest point of peak

				if (bestPeak==scan_->getNumDataPoints() ){
					nextBest=bestPeak-1;
				} else if (scan_->intensityArray_[bestPeak-1] > scan_->intensityArray_[bestPeak+1]) {
					nextBest=bestPeak-1;
				} else {
					nextBest=bestPeak+1;
				}

				//MH:
				// Get FWHM of Orbitrap
				//
				// This is the function you must change for each type of instrument.
				// For example, the FT would be:
				//   FWHM = s[bestPeak].mz * s[bestPeak].mz / (400*res400);
				FWHM = scan_->mzArray_[bestPeak] * sqrt(scan_->mzArray_[bestPeak]) / (20*res400);

				//MH: Calc centroid MZ (in three lines for easy reading)
				centroid.mz = pow(FWHM,2)*log(scan_->intensityArray_[bestPeak] / scan_->intensityArray_[nextBest]);
				centroid.mz /= 8*log(2.0)*(scan_->mzArray_[bestPeak] - scan_->mzArray_[nextBest]);
				centroid.mz += (scan_->mzArray_[bestPeak] + scan_->mzArray_[nextBest])/2;

				//MH: Calc centroid intensity
				centroid.intensity=scan_->intensityArray_[bestPeak] / exp(-pow((scan_->mzArray_[bestPeak]-centroid.mz)/FWHM,2)*(4*log(2.0)));

				//MH:
				// Hack until I put in mass ranges
				// Another fail-safe can be made for inappropriate intensities
				if (centroid.mz<0 || centroid.mz>2000) {
					//do nothing if invalid mz
				}
				else {
					centroidedSpectra.push_back(centroid);
				}

			}

		}
	}
	// == end of centroiding code originally from Mike Hoopman ==	


	// reset original scan object's arrays and copy data back
	scan_->setNumDataPoints(centroidedSpectra.size());
	for (int i=0; i < (int) centroidedSpectra.size(); i++) {
		scan_->mzArray_[i] = centroidedSpectra[i].mz;
		scan_->intensityArray_[i] = centroidedSpectra[i].intensity;
	}

	scan_->isCentroided_ = true;
	return true;
}

