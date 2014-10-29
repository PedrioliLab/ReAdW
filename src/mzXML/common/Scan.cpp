// -*- mode: c++ -*-


/*
    File: Scan.cpp
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


#include "Scan.h"
#include "math.h"
#include <string>
#include <iostream>
#include <vector>

using namespace std;

// From Spectrast, for Henry's centroiding:
// a peak - note that we use float's for intensities to save memory
typedef struct _peak {
	double mz;
	double intensity;
	string annotation;	
	string info;
} Peak;


void Scan::setNumDataPoints(int numDataPoints) {
	if (numDataPoints == 0) {
		numDataPoints_ = numDataPoints;
		return;
	}

	delete [] mzArray_;
	delete [] intensityArray_;

	numDataPoints_ = numDataPoints;
	mzArray_ = new double[numDataPoints_];
	intensityArray_ = new double[numDataPoints_];
}


void Scan::resetNumDataPoints(int numDataPoints) {
	numDataPoints_ = numDataPoints;
}


void Scan::setNumScanOrigins(int numScanOrigins) {
	numScanOrigins_ = numScanOrigins;
	scanOriginNums.resize(numScanOrigins, 0);
	scanOriginParentFileIDs.resize(numScanOrigins, "");
}


Scan::Scan() {
	numDataPoints_ = 0;

	msLevel_ = 0;
	charge_ = -1;
	startMZ_ = -1;
	endMZ_ = -1;
	minObservedMZ_ = -1;
	maxObservedMZ_ = -1;
	basePeakMZ_ = -1;
	basePeakIntensity_ = -1;
	totalIonCurrent_ = -1;
	retentionTimeInSec_ = -1;
	polarity_ = POLARITY_UNDEF;
	analyzer_ = ANALYZER_UNDEF;
	ionization_ = IONIZATION_UNDEF;
	scanType_ = SCAN_UNDEF;
	activation_ = ACTIVATION_UNDEF;

	isCentroided_ = false;
	precursorScanNumber_ = -1;
	precursorScanMSLevel_ = -1;
	precursorCharge_ = -1;
	precursorMZ_ = -1;
	accuratePrecursorMZ_ = false;
	precursorIntensity_ = -1;
	collisionEnergy_ = -1;

	// thermo scans only:
	isThermo_ = false;
	thermoFilterLine_ = "";

	// MassLynx scans only
	isMassLynx_ = false;
	isCalibrated_ = false;

	isMerged_ = false;
	mergedScanNum_ = -1;

	isThresholded_ = false;
	threshold_ = -1;

	// initialize to NULL so that delete can be called alway
	mzArray_ = NULL;
	intensityArray_ = NULL;

	numScanOrigins_ = 0;
}



Scan::Scan(const Scan& copy) {
	msLevel_ = copy.msLevel_;
	charge_ = copy.charge_;
	startMZ_ = copy.startMZ_;
	endMZ_ = copy.endMZ_;
	minObservedMZ_ = copy.minObservedMZ_;
	maxObservedMZ_ = copy.maxObservedMZ_;
	basePeakMZ_ = copy.basePeakMZ_;
	basePeakIntensity_ = copy.basePeakIntensity_;
	totalIonCurrent_ = copy.totalIonCurrent_;
	retentionTimeInSec_ = copy.retentionTimeInSec_;
	polarity_ = copy.polarity_;
	analyzer_ = copy.analyzer_;
	ionization_ = copy.ionization_;
	scanType_ = copy.scanType_;
	activation_ = copy.activation_;
	isCentroided_ = copy.isCentroided_;
	precursorScanNumber_ = copy.precursorScanNumber_;
	precursorScanMSLevel_ = copy.precursorScanMSLevel_;
	precursorCharge_ = copy.precursorCharge_;
	precursorMZ_ = copy.precursorMZ_;
	accuratePrecursorMZ_ = copy.accuratePrecursorMZ_;
	precursorIntensity_ = copy.precursorIntensity_;
	collisionEnergy_ = copy.collisionEnergy_;
	isThermo_ = copy.isThermo_;
	thermoFilterLine_ = copy.thermoFilterLine_;
	isMassLynx_ = copy.isMassLynx_;
	isCalibrated_ = copy.isCalibrated_;
	isMerged_ = copy.isMerged_;
	mergedScanNum_ = copy.mergedScanNum_;
	isThresholded_ = copy.isThresholded_;
	threshold_ = copy.threshold_;
	

	numScanOrigins_ = copy.numScanOrigins_;

	setNumDataPoints(copy.getNumDataPoints());
	for (int i=0; i<numDataPoints_; i++) {
		mzArray_[i] = copy.mzArray_[i];
		intensityArray_[i] = copy.intensityArray_[i];
	}

	nativeScanRef_.coordinateType_ = copy.nativeScanRef_.coordinateType_;
	nativeScanRef_.coordinates_.clear();
	for (std::vector<NativeScanRef::CoordinateNameValue>::size_type c = 0;
		c < copy.nativeScanRef_.coordinates_.size();
		c++) {
			nativeScanRef_.coordinates_.push_back(copy.nativeScanRef_.coordinates_[c]);
	}

}



Scan::~Scan() {
	delete [] mzArray_;
	delete [] intensityArray_;
}


void Scan::centroid(string instrument) {

	// presumed resolution - should be conservative?
	double res400 = 10000.0; // for TOF
	if (instrument == "FT") {
		res400 = 100000.0;
	} else if (instrument == "Orbitrap") {
		res400 = 50000.0;
	}

	// assume sorted
	//  sort(m_peaks.begin(), m_peaks.end(), sortPeaksByMzAsc);

	// determine smallest m/z-interval between peaks
	// this should tell us the frequency with which the
	// mass spectrometer takes readings
	// 
	// this step is helpful because in many profile spectra,
	// the peak is omitted completely if the intensity is below
	// a certain threshold. So the peak list has jumps in m/z values,
	// and neighboring peaks are not necessarily close in m/z. 

	double minInterval = 1000000.0;
	double minIntensity = 1000000.0;

	for (int p = 0; p < numDataPoints_ - 1; p++) {
		//double interval = mzArray_[p + 1] - intensityArray_[p];
        double interval = mzArray_[p + 1] - mzArray_[p];  // fixed typo - DT
		if (interval < 0.0) {
			cerr << "Peak list not sorted by m/z. No centroiding done." << endl;
			return;
		}
		if (minInterval > interval) minInterval = interval;
		if (intensityArray_[p] > 0 /* <- added to ignore zeroes -- DT*/ && minIntensity > intensityArray_[p]) minIntensity = intensityArray_[p];
	}

	vector<Peak>* allPeaks = new vector<Peak>;
	for (int i = 0; i < numDataPoints_ - 1; i++) {
		Peak p;
		p.mz = mzArray_[i];
		p.intensity = (float)intensityArray_[i];
		allPeaks->push_back(p);
		double gap = mzArray_[i + 1] - mzArray_[i];
		double curMz = mzArray_[i];
		int numZeros = 0;
		while (gap > 1.9 * minInterval) {
			if (numZeros < 3 || curMz > mzArray_[i + 1] - 3.1 * minInterval) {
				curMz += minInterval;
			} else {
				curMz = mzArray_[i + 1] - 3.0 * minInterval;
				gap = 4.0 * minInterval;
			}
			Peak pk;
			pk.mz = curMz;
			pk.intensity = 0.0;
			allPeaks->push_back(pk);
			gap -= minInterval;
			numZeros++;
		}

	}


	vector<Peak>* smoothedPeaks = new vector<Peak>;
	for (int i = 0; i < (int)allPeaks->size(); i++) {

		int weight = 6;
		Peak smoothPeak; 
		smoothPeak.mz = (*allPeaks)[i].mz;
		smoothPeak.intensity = 6 * (*allPeaks)[i].intensity;

		if (i >= 2){
			weight += 1;
			smoothPeak.intensity += (*allPeaks)[i - 2].intensity;
		}
		if (i >= 1) {
			weight += 4;
			smoothPeak.intensity += 4 * (*allPeaks)[i - 1].intensity;
		}
		if (i < (int)allPeaks->size() /*numDataPoints_*/ - 1) {   // DT
			weight += 4;
			smoothPeak.intensity += 4 * (*allPeaks)[i + 1].intensity;
		}
		if (i < (int)allPeaks->size() /*numDataPoints_*/ - 2) {   // DT
			weight += 1;
			smoothPeak.intensity += (*allPeaks)[i + 2].intensity;
		}

		smoothPeak.intensity /= (float)weight; 

		smoothedPeaks->push_back(smoothPeak);

	}

	delete (allPeaks);


	//  m_peaks.clear();
	// for (vector<Peak>::iterator sm = smoothedPeaks.begin(); sm != smoothedPeaks.end(); sm++) {
	//   m_peaks.push_back(*sm);
	// }
	// plot(m_pep->getSafeName() + "_smoothed", "");

	int j;
	double maxIntensity;
	int bestPeak;
	bool bLastPos;

	int nextBest;
	double FWHM;

	bLastPos=false;

	vector<Peak> newPeaks;
	//step along each point in spectrum
	for (int i = 0; i < (int)smoothedPeaks->size() - 1; i++){

		//check for change in direction
		if ((*smoothedPeaks)[i].intensity < (*smoothedPeaks)[i + 1].intensity) {

			bLastPos=true;
			continue;

		} else {

			if (bLastPos) {
				bLastPos=false;

				//find max 
				//This is an artifact of using a window of length n (user variable) for identifying
				//a better peak apex on low-res data. Feel free to remove this section if desired.
				//Replace with:
				//  bestPeak=j;
				//  maxIntensity=s[j].intensity;

				maxIntensity = 0;
				for (j = i; j < i + 1; j++) {
					if ((*smoothedPeaks)[j].intensity > maxIntensity) {
						maxIntensity = (*smoothedPeaks)[j].intensity;
						bestPeak = j;
					}
				}

				//Best estimate of Gaussian centroid
				//Get 2nd highest point of peak
				if (bestPeak == smoothedPeaks->size() - 1) {
					nextBest = bestPeak - 1;
				} else if ((*smoothedPeaks)[bestPeak - 1].intensity > (*smoothedPeaks)[bestPeak + 1].intensity) {
					nextBest = bestPeak - 1;
				} else {
					nextBest = bestPeak + 1;
				}

				//Get FWHM of Orbitrap
				//This is the function you must change for each type of instrument.
				//For example, the FT would be:
				//  FWHM = s[bestPeak].mz * s[bestPeak].mz / (400*res400);
				// Orbitrap
				// FWHM = (*smoothedPeaks)[bestPeak].mz * sqrt((*smoothedPeaks)[bestPeak].mz) / (20 * res400);

				if (instrument == "FT") {
					FWHM = (*smoothedPeaks)[bestPeak].mz * (*smoothedPeaks)[bestPeak].mz / (400 * res400);
				} else if (instrument == "Orbitrap") {
					FWHM = (*smoothedPeaks)[bestPeak].mz * sqrt((*smoothedPeaks)[bestPeak].mz) / (20 * res400);
				} else {
					// for TOF
					FWHM = (*smoothedPeaks)[bestPeak].mz / res400;
				}

				Peak centroid;
				//Calc centroid MZ (in three lines for easy reading)
				centroid.mz = pow(FWHM , 2) * log((*smoothedPeaks)[bestPeak].intensity / (*smoothedPeaks)[nextBest].intensity);
				centroid.mz /= 8 * log(2.0) * ((*smoothedPeaks)[bestPeak].mz - (*smoothedPeaks)[nextBest].mz);
                if ( fabs(centroid.mz) < fabs( ((*smoothedPeaks)[bestPeak].mz - (*smoothedPeaks)[nextBest].mz) / 2 ) ) // sanity check - DT
				    centroid.mz += ((*smoothedPeaks)[bestPeak].mz + (*smoothedPeaks)[nextBest].mz) / 2;
                else
                    centroid.mz = (*smoothedPeaks)[bestPeak].mz; // fail-safe - DT


				//Calc centroid intensity
				// double exponent = pow(((*smoothedPeaks)[bestPeak].mz - centroid.mz) / FWHM, 2) * (4 * log(2.0));
				//
				// if (exponent > 1.0) {
				//  centroid.intensity = (*smoothedPeaks)[bestPeak].intensity;
				// } else {
				//  centroid.intensity = (*smoothedPeaks)[bestPeak].intensity / exp(-exponent);
				// }

				centroid.intensity = (*smoothedPeaks)[bestPeak].intensity;

				//Hack until I put in mass ranges
				//Another fail-safe can be made for inappropriate intensities
				if (centroid.mz < 0 || centroid.mz > 2000 || centroid.intensity < 0.99 * minIntensity) {
					//do nothing if invalid mz
				} else {
					newPeaks.push_back(centroid);
				}

			}

		}
	}

	delete (smoothedPeaks);

	vector<Peak> finalPeakList;
	finalPeakList.clear();
	// m_origMaxIntensity = 0.0;
	// m_totalIonCurrent = 0.0;
	// m_isAnnotated = false;

	// reset Scan data arrays
	setNumDataPoints((int)newPeaks.size());

	// copy centroided results to Scan data arrays
	long ci=0;
	for (vector<Peak>::iterator j = newPeaks.begin(); j != newPeaks.end(); j++) {
		mzArray_[ci] = (*j).mz;
		intensityArray_[ci] = (*j).intensity;
        ci ++; // added -- DT
		//    if (m_origMaxIntensity < j->intensity) m_origMaxIntensity = j->intensity;
		//    m_totalIonCurrent += j->intensity;
	}

	isCentroided_ = true;

	// TODO: reset/recalc other scan values? 

	// m_isScaled = false;

	// if (m_bins) delete (m_bins);
	// m_bins = false;

	// delete (m_intensityRanked);
	// m_intensityRanked = NULL;

	//  plot(m_pep->getSafeName() + "_centroided", "");
}


// if not discard, rewrite as zero
void Scan::threshold(double inclusiveCutoff, bool discard) {

	vector<Peak> newPeakList;
	newPeakList.clear();

	int i;
	int orig = numDataPoints_;

	/*for (i=0; i<numDataPoints_; i++) {
		cout << mzArray_[i] << "\t" << intensityArray_[i] << endl;
	}
	cout << endl << endl;*/


	for (i=0; i<numDataPoints_; i++) {
		double curIntensity = intensityArray_[i];
		double curMZ = mzArray_[i];
		if (curIntensity >= inclusiveCutoff) {
			// save the value
			Peak p;
			p.intensity = curIntensity;
			p.mz = curMZ;
			newPeakList.push_back(p);
		}
		else {
			// this failed the cutoff.
			// do we discard it, or zero it out?
			if (!discard) {
					// insert a zero'd intensity here
					Peak p;
					p.intensity = 0;
					p.mz = curMZ;
					newPeakList.push_back(p);
			}
			/*else {
				cout << "discarded " << curMZ << "\t" << curIntensity << endl;
			}*/
		}
	}

	// rewrite our actual internal data:

	// reset Scan data arrays:
	setNumDataPoints((int)newPeakList.size());
	//if (numDataPoints_ > 0){
	//	//cout << "scan " <<  
	//	cout << "threshold: cutoff is " << inclusiveCutoff
	//	<< " discard is " << discard << endl;
	//	cout << "original #data points: " << numDataPoints_ << endl;
	//	cout << "new #data points: " << numDataPoints_ << endl;
	//}
	//cout << endl << endl;

	// copy thresholded results to Scan data arrays
	long ci=0;
	for (vector<Peak>::iterator j = newPeakList.begin(); j != newPeakList.end(); j++) {
		mzArray_[ci] = (*j).mz;
		intensityArray_[ci] = (*j).intensity;
		ci++;
	}

	isThresholded_ = true;

	// TODO: recalulate other Scan values (basepeak, range, etc)?
}
