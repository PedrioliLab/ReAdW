// -*- mode: c++ -*-


/*
File: ThermoInterface.cpp
Description: Encapsulation for Thermo Xcalibur interface.
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


#include <iostream>
#include <iomanip>

#include "stdafx.h"
#include "ThermoInterface.h"
#include "mzXML/common/Scan.h"
#include "mzXML/common/MSUtilities.h"

using namespace std;



ThermoInterface::ThermoInterface(void)
{

	// InstrumentInterface members
	totalNumScans_ = -1;
	curScanNum_ = -1;
	firstScanNumber_ = -1;
	lastScanNumber_ = -1;
	doCompression_ =false;
	doCentroid_=false;
	doDeisotope_=false;
	forcePrecursorFromFilter_=false;
	verbose_=false;
	accurateMasses_=0;
	inaccurateMasses_=0;
	// store counts for charges up to +15
	chargeCounts_.clear();
	chargeCounts_.resize(1, 0);
	instrumentInfo_.manufacturer_ = THERMO;
	instrumentInfo_.acquisitionSoftware_ = XCALIBUR;

	// ThermoInterface members
	xrawfile2_ = NULL; // smartptr will be initialized in initInterface
	IXRawfileVersion_ = -1;
	msControllerType_ = 0;
	startTimeInSec_ = -1;
	endTimeInSec_ = -1;

	firstTime_ = true;

	getPreInfoCount_ = 0;
	filterLineCount_ = 0;
	oldAPICount_ = 0;
}

ThermoInterface::~ThermoInterface(void)
{
	xrawfile2_->Close();
}

bool ThermoInterface::initInterface(void) {

	// temp smartptrs; xrawfile2_ will be set to highest interface that succeeds
    IXRawfile2Ptr xrawfile2IXRawfile2_ = NULL;
    IXRawfile3Ptr xrawfile2IXRawfile3_ = NULL;
    IXRawfile4Ptr xrawfile2IXRawfile4_ = NULL;
	IXRawfile5Ptr xrawfile2IXRawfile5_ = NULL;

	// first, init MFC or return false
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		return false;
	}

	// Initializes the COM library on the current thread 
	// and identifies the concurrency model as single-thread 
	// apartment (STA)
	CoInitialize( NULL );

#ifdef MSFILEREADER
	char xrawfilestring[] = "MSFileReader.XRawfile.1"; 
#else
	char xrawfilestring[] = "XRawfile.XRawfile.1";
#endif

	HRESULT hr;
	
	hr = xrawfile2IXRawfile5_.CreateInstance(xrawfilestring);
	if (!FAILED(hr)) {
		xrawfile2_ = xrawfile2IXRawfile5_;
		IXRawfileVersion_ = 5;
		cout << "Xcalibur interface initialized (5)." << endl;
		return true;
	}

	hr = xrawfile2IXRawfile4_.CreateInstance(xrawfilestring);
	if (!FAILED(hr)) {
		xrawfile2_ = xrawfile2IXRawfile4_;
		IXRawfileVersion_ = 4;
		cout << "Xcalibur interface initialized (4)." << endl;
		return true;
	}

	hr = xrawfile2IXRawfile3_.CreateInstance(xrawfilestring);
	if (!FAILED(hr)) {
		xrawfile2_ = xrawfile2IXRawfile3_;
		IXRawfileVersion_ = 3;
		cout << "Xcalibur interface initialized (2.0)." << endl;
		return true;
	}

	hr = xrawfile2IXRawfile2_.CreateInstance(xrawfilestring);
	if (!FAILED(hr)) {
		xrawfile2_ = xrawfile2IXRawfile2_;
		IXRawfileVersion_ = 2;
		cout << "Xcalibur interface initialized (1.4)." << endl;
		return true;
	}

	cerr << "Unable to initialize XCalibur interface; check installation" << endl;
	cerr << "hint: try running the command \"regsvr32 C:\\<path_to_Xcalibur_dll>\\XRawfile2.dll\"" << endl;
	return false;

}

bool ThermoInterface::setInputFile(const string& filename) {

	// try to open raw file or die
	HRESULT hr = xrawfile2_->Open(filename.c_str());
	if (hr != ERROR_SUCCESS) {

		if (hr == ERROR_PATH_NOT_FOUND) {
			cerr << filename << ": path not found" << endl;
		}
		if (hr == ERROR_FILE_NOT_FOUND) {
			cerr << filename << ": file not found" << endl;
		}

		cerr << "Cannot open file " << filename << endl;
		cerr << "(If you get this error with a valid filename," << endl;
		cerr << " you may be using an older version of Xcalibur libraries" << endl;	
		cerr << " with a file created by a newer version of Xcalibur.)" << endl;	
		
		return false;
	}


	cout << "(Thermo lib opened file " << filename << ")" << endl;

	// test the file format version number
	long fileVersionNumber = -1;
	xrawfile2_->GetVersionNumber(&fileVersionNumber);
	cout << "file version is " << fileVersionNumber << ", interface version is ";
	if (IXRawfileVersion_ == 2) {
			cout << "1.4" << endl;
	}
	else if (IXRawfileVersion_ >= 3) {
			cout << "2.0 or greater" << endl;
	}
	
	xrawfile2_->SetCurrentController(msControllerType_, 1);
	// Get the total number of scans

	xrawfile2_->GetFirstSpectrumNumber(&firstScanNumber_);
	xrawfile2_->GetLastSpectrumNumber(&lastScanNumber_);
	cout << "file should contain scan numbers " << firstScanNumber_ << " through " << lastScanNumber_ << endl;

	curScanNum_ = firstScanNumber_;

	totalNumScans_ = (lastScanNumber_ - firstScanNumber_) + 1;

	// get the start and the end time of the run
	double startTime, endTime;
	xrawfile2_->GetStartTime(&startTime);
	xrawfile2_->GetEndTime(&endTime);
	// convert from minutes to seconds
	startTimeInSec_ = 60.0 * startTime;
	endTimeInSec_ = 60.0 * endTime;

	// get the instrument model
	BSTR bstrInstModel=NULL;
	xrawfile2_->GetInstModel(&bstrInstModel);

   instrumentInfo_.strInstrumentModel_ = convertBstrToString(bstrInstModel);
	SysFreeString(bstrInstModel);



	// get instrument name
	BSTR bstrInstName=NULL;
	xrawfile2_->GetInstName(&bstrInstName);
	//cout << convertBstrToString(bstrInstName) << endl;
	instrumentInfo_.instrumentName_ = convertBstrToString(bstrInstName);
	SysFreeString(bstrInstName);

	// TODO: is this the Xcalibur or inst version?
	// get acquisition software version
	BSTR bstrAcquSoftwareVersion=NULL;
	xrawfile2_->GetInstSoftwareVersion(&bstrAcquSoftwareVersion);
	instrumentInfo_.acquisitionSoftwareVersion_ = convertBstrToString(bstrAcquSoftwareVersion);
	SysFreeString(bstrAcquSoftwareVersion);

	// get instrument hardware version
	BSTR bstrInstHardwareVersion=NULL;
	xrawfile2_->GetInstHardwareVersion(&bstrInstHardwareVersion);
	//cout << convertBstrToString(bstrInstHardwareVersion) << endl;
	instrumentInfo_.instrumentHardwareVersion_ = convertBstrToString(bstrInstHardwareVersion);
	SysFreeString(bstrInstHardwareVersion);

	// get instrument description
	BSTR bstrInstDescription=NULL;
	xrawfile2_->GetInstrumentDescription(&bstrInstDescription);
	//cout << convertBstrToString(bstrInstDescription) << endl;
	SysFreeString(bstrInstDescription);

	// get instrument serial number
	BSTR bstrInstSerialNumber=NULL;
	xrawfile2_->GetInstSerialNumber(&bstrInstSerialNumber);
	//cout << convertBstrToString(bstrInstSerialNumber) << endl;
	SysFreeString(bstrInstSerialNumber);

	// get instrument ion source, analyzer 
	// assuming first scan's filter line info is true for all scans
	// TODO: This is known to be wrong in LTQ-FT case (both FT and ion trap)
	BSTR bstrFilter = NULL;
	xrawfile2_->GetFilterForScanNum(1, &bstrFilter);
	string filterString = convertBstrToString(bstrFilter);
	SysFreeString(bstrFilter);
	FilterLine filterLine;
	if (!filterLine.parse(filterString)) {
		cerr << "error parsing filter line. exiting." << endl;
		cerr << "line: " << filterString << endl;
		exit(-1);
	}

	// TODO: deal with muliple analyzers, like FT/IT
	MSAnalyzerType analyzer = filterLine.analyzer_;
	// TODO: hack! do this properly with library somehow?
	if (analyzer == ANALYZER_UNDEF) {
		// set default
		analyzer = ITMS;
	}
	instrumentInfo_.analyzerList_.push_back(analyzer);

	MSIonizationType ionization = filterLine.ionizationMode_;
	// TODO: hack! determine instrument info better
	if (ionization == IONIZATION_UNDEF) {
		// set default
		ionization = NSI;
	}
	instrumentInfo_.ionSource_ = ionization;

	// get time from file
	DATE date=NULL;
	xrawfile2_->GetCreationDate(&date);
	//instrumentInfo_.runTime_ = ""; // FIX
	//CTime c(date);
	COleDateTime dateTime(date);
	string dateStr=dateTime.Format("%Y-%m-%dT%H:%M:%S");
	timeStamp_ = dateStr;

	return true;
}

void ThermoInterface::setVerbose(bool verbose) {
	verbose_ = verbose;
}

void ThermoInterface::setCentroiding(bool centroid) {
	doCentroid_ = centroid;
}


void ThermoInterface::setDeisotoping(bool deisotope) {
	doDeisotope_ = deisotope;
}


void ThermoInterface::forcePrecursorFromFilter(bool force) {
	forcePrecursorFromFilter_ = force;
}

void ThermoInterface::setCompression(bool compression) {
	doCompression_ = compression;
}



Scan* ThermoInterface::getScan(void) {
	if (!firstTime_) {
		++curScanNum_;
		if (curScanNum_ > lastScanNumber_) {
			// we're done
			return NULL;
		}
	} 
	else {
		firstTime_ = false;
	}


	Scan* curScan = new Scan();
	curScan->isThermo_ = true;	

	//// test the "scan event" call
	//// gives a more through scan filter line
	//BSTR bstrScanEvent = NULL;
	//xrawfile2_->GetScanEventForScanNum(curScanNum_, &bstrScanEvent);
	//SysFreeString(bstrScanEvent);

	// Get the scan filter
	// (ex: "ITMS + c NSI Full ms [ 300.00-2000.00]")
	BSTR bstrFilter = NULL;
	xrawfile2_->GetFilterForScanNum(curScanNum_, &bstrFilter);
	curScan->thermoFilterLine_ = convertBstrToString(bstrFilter);
	SysFreeString(bstrFilter);
	FilterLine filterLine;
	if (!filterLine.parse(curScan->thermoFilterLine_)) {
		cerr << "error parsing filter line. exiting." << endl;
		cerr << "line: " << curScan->thermoFilterLine_ << endl;
		exit(-1);
	}

	// we should now have:
	// msLevel
	// polarity
	// scanType (full, zoom, etc)
	// ionization type
	// analyzer
	// scan range
	// parent mass and CID energy, if MS >=2 

	// record msLevel from filter line
	curScan->msLevel_ = filterLine.msLevel_;	

	// record polarity from filter line
	curScan->polarity_ = filterLine.polarity_;

	// record analyzer from filter line
	curScan->analyzer_ = filterLine.analyzer_;

	// record ionization from filter line
	curScan->ionization_ = filterLine.ionizationMode_;

	// record scan type from filter line
	// (zoom, full, srm, etc)
	curScan->scanType_ = filterLine.scanType_;

	// record activation (CID, etc)
	// check FilterLine: this may be default set to CID now
	if (filterLine.activationMethod_ != ACTIVATION_UNDEF) {
		curScan->activation_ = filterLine.activationMethod_;
	}
	else {
		curScan->activation_ = CID;
	}

	// record scan ranges from filter line
	// Note: SRM fills this with range of q3 transtion lists
	if (curScan->scanType_ != SRM) {
		curScan->startMZ_ = filterLine.scanRangeMin_[ filterLine.scanRangeMin_.size() - 1 ];
		curScan->endMZ_ = filterLine.scanRangeMax_[ filterLine.scanRangeMax_.size() - 1 ];
	}
	else {
		// SRM: record range of q3 transition lists
		// start mz is average of first transition range
		// end mz is average of last transition range
		curScan->startMZ_ = (filterLine.transitionRangeMin_[0] + 
							 filterLine.transitionRangeMin_ [filterLine.transitionRangeMin_.size() - 1 ])
							 / 2;
		curScan->endMZ_ =  (filterLine.transitionRangeMax_[0] + 
							 filterLine.transitionRangeMax_ [filterLine.transitionRangeMax_.size() - 1 ])
							 / 2;
	}
	// get additional header information through Xcalibur:
	// retention time
	// min/max observed mz
	// total ion current
	// base peak mz and intensity
	// precursor (JMT: if avalible from interface, else use filter line info)

	long numDataPoints = -1; // points in both the m/z and intensity arrays
	double retentionTimeInMinutes = -1;
	long channel; // unused
	long uniformTime; // unused
	double frequency; // unused

	xrawfile2_->GetScanHeaderInfoForScanNum(
		curScanNum_, 
		&numDataPoints, 
		&retentionTimeInMinutes, 
		&(curScan->minObservedMZ_),
		&(curScan->maxObservedMZ_),
		&(curScan->totalIonCurrent_),
		&(curScan->basePeakMZ_),
		&(curScan->basePeakIntensity_),
		&channel, // unused
		&uniformTime, // unused
		&frequency // unused
		);

	// NOTE! the returned numDataPoints is invalid!
	// use the value from GetMassListFromScanNum below

	// record the retention time
	curScan->retentionTimeInSec_ = retentionTimeInMinutes * 60.0;

	// if ms level 2 or above, get precursor info
	if (curScan->msLevel_ > 1)  {
		getPrecursorInfo(*curScan, curScanNum_, filterLine);
	}

	// get ion fill time
	VARIANT varValue;
	VariantInit(&varValue);
	xrawfile2_->GetTrailerExtraValueForScanNum(curScanNum_, "Ion Injection Time (ms):" , &varValue);

	if( varValue.vt == VT_R4 ) {
		curScan->injectionTimeInSec_ = varValue.fltVal / 1000.0;
	}
	else if( varValue.vt == VT_R8 ) {
		curScan->injectionTimeInSec_ = varValue.dblVal / 1000.0;
	}
	else if ( varValue.vt != VT_ERROR ) {
		cerr << "Scan: " << curScanNum_ << " Unexpected type when looking for ion injection time\n";
		exit(-1);
	}
   else {
      curScan->injectionTimeInSec_ = -1.0;
  		cerr << "Scan: " << curScanNum_ << " Warning cannot read ion injection time; ignoring\n";

   }

	//
	// get the m/z intensity pairs list for the current scan
	// !and correct min/max observed m/z info here!
	//

	
	curScan->minObservedMZ_ = 0;
	curScan->maxObservedMZ_ = 0;

	if (numDataPoints != 0) {


		// cout << "reading data points for scan " << curScanNum_ << endl;
		VARIANT varMassList;
		// initiallize variant to VT_EMPTY
		VariantInit(&varMassList);

		VARIANT varPeakFlags; // unused
		// initiallize variant to VT_EMPTY
		VariantInit(&varPeakFlags);

		// set up the parameters to read the scan
		// TODO make centroid parameter user customizable
		long dataPoints = 0;
		long scanNum = curScanNum_;
		LPCTSTR szFilter = NULL;		// No filter
		long intensityCutoffType = 1;		// 1 : absolute intensity units
		long intensityCutoffValue = 1;	// Remove 0 intensity points
		long maxNumberOfPeaks = 0;		// 0 : return all data peaks
		double centroidPeakWidth = 0;		// No centroiding


		// record centroiding info
		//
		// scan may have been centroided at accquision time,
		// rather than conversion time (now)
		// even if user didn't request it.
		if ( (doCentroid_ /* && curScan->msLevel_ > 1 */)
			|| filterLine.scanData_ == CENTROID) {
				curScan->isCentroided_ = true;	
		}

		// Note: special case for FT centroiding, contributed from Matt Chambers
		if (doCentroid_ /* && (curScan->msLevel_ > 1) */ && (curScan->analyzer_ == FTMS)) {
			// use GetLabelData to workaround bug in Thermo centroiding of FT profile data

			VARIANT varLabels;
			_variant_t vSpecData;
			xrawfile2_->GetLabelData(&varLabels, NULL, &scanNum);
			vSpecData.Attach(varLabels);
			dataPoints = vSpecData.parray->rgsabound[0].cElements;

			// record the number of data point (allocates memory for arrays)
			curScan->setNumDataPoints(dataPoints);

			double* pdval = (double*) vSpecData.parray->pvData;
			for(long i=0; i < dataPoints; ++i) {
				curScan->mzArray_[i] = (double) pdval[(i*6)+0];
				curScan->intensityArray_[i] = (double) pdval[(i*6)+1];
			}
		} else {
			// get peaks with GetMassListFromScanNum as usual

			// do centroid should have no effect on already centroided data
			// but make sure to turn off centroiding for ms1 scans!
			bool centroidThisScan = doCentroid_;
			/*
			if (curScan->msLevel_ < 2) {
			centroidThisScan = false;
			}
			*/

			xrawfile2_->GetMassListFromScanNum(
				&scanNum,
				szFilter,			 // filter
				intensityCutoffType, // intensityCutoffType
				intensityCutoffValue, // intensityCutoffValue
				maxNumberOfPeaks,	 // maxNumberOfPeaks
				centroidThisScan,		// centroid result?
				&centroidPeakWidth,	// centroidingPeakWidth
				&varMassList,		// massList
				&varPeakFlags,		// peakFlags
				&dataPoints);		// array size

			// record the number of data point (allocates memory for arrays)
			curScan->setNumDataPoints(dataPoints);
			/*
			if (dataPoints != curScan->getNumDataPoints()) {
			cerr << "exiting with error at " 
			<< __FILE__ << ", line "
			<< __LINE__ << endl;
			exit(-1);
			}
			*/
			// Get a pointer to the SafeArray
			SAFEARRAY FAR* psa = varMassList.parray;
			DataPeak* pDataPeaks = NULL;
			SafeArrayAccessData(psa, (void**)(&pDataPeaks));


			// record mass list information in scan object
			if( doCentroid_ )
			{ // If we centroided the data we need to correct the basePeak m/z and intensity 
			  //(since GetScanHeaderInfoForScanNum returns information relevant to the profile data)!!
				curScan->basePeakIntensity_ = 0;
				for (long j=0; j<dataPoints; j++) {
					curScan->mzArray_[j] = pDataPeaks[j].dMass;
					curScan->intensityArray_[j] = pDataPeaks[j].dIntensity;
					if( pDataPeaks[j].dIntensity > curScan->basePeakIntensity_ )
					{
						curScan->basePeakMZ_ = pDataPeaks[j].dMass;
						curScan->basePeakIntensity_ = pDataPeaks[j].dIntensity;
					}
				}
			}
			else
			{
				for (long j=0; j<dataPoints; j++) {
					curScan->mzArray_[j] = pDataPeaks[j].dMass;
					curScan->intensityArray_[j] = pDataPeaks[j].dIntensity;
				}
			}

			// cleanup
			SafeArrayUnaccessData(psa); // Release the data handle
			VariantClear(&varMassList); // Delete all memory associated with the variant
			VariantClear(&varPeakFlags); // and reinitialize to VT_EMPTY

			if( varMassList.vt != VT_EMPTY ) {
				SAFEARRAY FAR* psa = varMassList.parray;
				varMassList.parray = NULL;
				SafeArrayDestroy( psa ); // Delete the SafeArray
			}

			if(varPeakFlags.vt != VT_EMPTY ) {
				SAFEARRAY FAR* psa = varPeakFlags.parray;
				varPeakFlags.parray = NULL;
				SafeArrayDestroy( psa ); // Delete the SafeArray
			}
		}

		// !!
		// Fix to overcome bug in ThermoFinnigan library GetScanHeaderInfoForScanNum() function
		// !!
		if (dataPoints > 0) {
			// don't do this on an empty scan!
			curScan->minObservedMZ_ = curScan->mzArray_[0];
			curScan->maxObservedMZ_ = curScan->mzArray_[dataPoints-1];
		}
	} // end 'not empty scan'
	else {
		// if empty scan:
		if (verbose_) {
			cout << "Note: empty scan detected (scan # " << curScanNum_ << ")" << endl;
			cout.flush();
		}
	}

	return curScan;
}




// get precursor m/z, collision energy, precursor charge, and precursor intensity
void
ThermoInterface::getPrecursorInfo(Scan& scan, long scanNumber, FilterLine& filterLine) {
	// TODO: assert scan->msLevel_ > 1

	if (scanNumber == 1) {
		// if this is the first scan, only use the info from the filter lin
		// (API calls are no use, as there's no precursor scan)
		// An example of a first scan WITH filterline precursor info:
		// "+ c NSI sid=5.00  SRM ms2 580.310@cid25.00 [523.955-523.965, 674.405-674.415, 773.295-773.305]"

		scan.accuratePrecursorMZ_ = false;
		// use the low-precision parent mass in the filter line
		inaccurateMasses_++;
		scan.precursorMZ_ = filterLine.cidParentMass_[filterLine.cidParentMass_.size() - 1];
		scan.accuratePrecursorMZ_ = false;		

		// use the low-precision collision energy recorded in the filter line
		scan.collisionEnergy_ = filterLine.cidEnergy_[filterLine.cidEnergy_.size() - 1];

		scan.precursorCharge_ = -1; // undetermined
		chargeCounts_[0]++; // with "charge 0" being undetermined

		scan.precursorIntensity_ = 0; // undetermined

		return;
	}

	VARIANT varValue;
	VariantInit(&varValue);

	//
	// precursor m/z fallbacks
	// try the Thermo API first, then falling back on the value extracted from the filter line
	//

	//if (scan.precursorMZ_ <= 0) { // restore if getPrec() call works in the future
	if (1) {
		// we'll try to get the "accurate mass" recorded by the machine, and fall back to the filter line value otherwise.
		scan.accuratePrecursorMZ_ = false;

		// see if we can get an accurate value from the machine

		double precursorMZ = 0;


		// don't try to get Monoisotopic M/Z if the user has selected "force precursor from filter line"
		if (!forcePrecursorFromFilter_) {
			// ignore return value from this call
			xrawfile2_->GetTrailerExtraValueForScanNum(curScanNum_, "Monoisotopic M/Z:" , &varValue);

			if( varValue.vt == VT_R4 ) {
				precursorMZ = varValue.fltVal;
			}
			else if( varValue.vt == VT_R8 ) {
				precursorMZ = varValue.dblVal;
			}
			else if ( varValue.vt != VT_ERROR ) {
				cerr << "Scan: " << curScanNum_ << " MS level: " << scan.msLevel_ 
					<< " unexpected type when looking for precursorMz\n";
				exit(-1);
			}
		}

		scan.precursorMZ_ = filterLine.cidParentMass_[filterLine.cidParentMass_.size() - 1];
		if (precursorMZ > 0 && fabs(precursorMZ-scan.precursorMZ_)<=10.0) { // (note: this could only true if we tried and had sucess
			// with the monoisotopic call above.)
			// Sanity check to make sure mono mass is in ballpark of filter line mass.

			// sucess-- higher accuracy m/z recorded through API
			accurateMasses_++;
			scan.precursorMZ_ = precursorMZ;
			scan.accuratePrecursorMZ_ = true;
			oldAPICount_++;
		}
		else { 
			// use the low-precision parent mass in the filter line
			inaccurateMasses_++;
			//scan.precursorMZ_ = filterLine.cidParentMass_[filterLine.cidParentMass_.size() - 1];
			scan.accuratePrecursorMZ_ = false;
			filterLineCount_++;
		}
	}


	//
	// collision energy, trying the Thermo API first, then falling back on the value extracted from the filter line
	//

	double collisionEnergy = 0;
	VariantClear(&varValue);
	xrawfile2_->GetTrailerExtraValueForScanNum(curScanNum_, "API Source CID Energy:" , &varValue);
	if( varValue.vt == VT_R4 ) {
		// VT_R4: OLE float type
		collisionEnergy = varValue.fltVal;
	}		
	else if ( varValue.vt != VT_ERROR ) {
		cerr << "Unexpected type when looking for CE\n";
		exit(-1);
	}

	if (fabs(collisionEnergy)<0.00001) {
		// sucess-- collision energy recorded through API
		scan.collisionEnergy_ = collisionEnergy;
	}
	else {
		// use the low-precision collision energy recorded in the filter line
		scan.collisionEnergy_ = filterLine.cidEnergy_[filterLine.cidEnergy_.size() - 1];
	}

	int trailerPrecursorCharge = 0;
	if (scan.precursorCharge_ < 1) {
		// precursor charge state fallbacks, again trying first from the API, then resorting to filter line value
		VariantClear(&varValue);
		
		xrawfile2_->GetTrailerExtraValueForScanNum(curScanNum_, "Charge State:" , &varValue);

		// First try to use the OCX
		if( varValue.vt == VT_I2 || varValue.vt == VT_I4 || varValue.vt == VT_UI1) {
			trailerPrecursorCharge = varValue.iVal;
		}

		if (trailerPrecursorCharge != 0) {
			// sucess-- precursor charge recorded through API
			scan.precursorCharge_ = trailerPrecursorCharge;
		}
		else {
			// no luck
			scan.precursorCharge_ = -1; // undetermined
		}
	}

	// track the counts of precursor charges
	// with "0" being undetermined
	if (scan.precursorCharge_ < 1) {
		chargeCounts_[0]++;
	}
	else {
		if ( (scan.precursorCharge_ + 1) > int(chargeCounts_.size())) {
			chargeCounts_.resize(scan.precursorCharge_ + 1, 0);
			//if (verbose_) {
			//	cout << "new max observed charge: " << scan.precursorCharge_ << "(scan " << curScanNum_ << ")" << endl;
			//	cout.flush();
			//}
		}
		chargeCounts_[scan.precursorCharge_]++;
	}


	//
	// precursor intensity determiniation
	//

	// go to precursor scan and try to find highest intensity around precursor m/z.
	// (could this be improved to handle more complex acquisition methods?)

	//if( numDataPoints != 0 ) { // if this isn't an empty scan
	VARIANT varMassList;
	VariantInit(&varMassList);	// initiallize variant to VT_EMPTY
	VARIANT varPeakFlags; // unused
	VariantInit(&varPeakFlags);	// initiallize variant to VT_EMPTY

	// set up the parameters to read the precursor scan
	long numPrecursorDataPoints = 0;
	LPCTSTR szFilter = "!d";		// First previous not-dependent scan
	long intensityCutoffType = 0;	// No cutoff
	long intensityCutoffValue = 0;	// No cutoff
	long maxNumberOfPeaks = 0;		// Return all data peaks
	BOOL centroidResult = FALSE;	// No centroiding of the precursor
	double centroidPeakWidth = 0;	// (see above: no centroiding)

	// cout << "reading scan " << curScanNum_ << endl;

	long curScanNum = scanNumber;
	
	// the goal is to get the parent scan's info
	xrawfile2_->GetPrevMassListFromScanNum(
		&curScanNum,
		szFilter,				// filter
		intensityCutoffType,	// intensityCutoffType
		intensityCutoffValue,	// intensityCutoffValue
		maxNumberOfPeaks,		// maxNumberOfPeaks
		centroidResult,			// centroidResult
		&centroidPeakWidth,		// centroidingPeakWidth
		&varMassList,			// massList
		&varPeakFlags,			// peakFlags -- unused
		&numPrecursorDataPoints);	// arraySize

	// record the precursor scan number
	scan.precursorScanNumber_ = curScanNum; // set during last xrawfile2 call
	
	// numPrecursorDataPoints: number of mass/intensity pairs for precursor scan
	// if empty, no precursor scan
	if (numPrecursorDataPoints != 0) {
		// Get a pointer to the SafeArray
		SAFEARRAY FAR* psa = varMassList.parray;
		DataPeak* pDataPeaks = NULL;
		// use our pDataPeaks array to access the SafeArray data.
		SafeArrayAccessData(psa, (void**)(&pDataPeaks) );
		double precursorIntensity = 0;

		// Find most intense peak around precursorMz:		
		// search the precursor's scan data near precursorMZ
		// to get the intensity.
		for (long j=0; j<numPrecursorDataPoints; j++) {
			double dMass = pDataPeaks[j].dMass;

			if (fabs(scan.precursorMZ_ - dMass) < 0.05 ) {
				if( pDataPeaks[j].dIntensity > precursorIntensity ) {
					precursorIntensity = pDataPeaks[j].dIntensity;
				}
			}

			// stop if we've gone too far
			if (dMass - scan.precursorMZ_ > 0.05) {
				break;
			}
		}

		// record the precursor intensity
		scan.precursorIntensity_ = precursorIntensity;

		// Release the data handle
		SafeArrayUnaccessData(psa);
		// Delete all memory associated with the variant and
		// reinitialize to VT_EMPTY
		VariantClear(&varMassList);	
		VariantClear(&varPeakFlags);
	}

	// double check on deallocating memory
	if (varMassList.vt != VT_EMPTY) {
		SAFEARRAY FAR* psa = varMassList.parray;
		varMassList.parray = NULL;
		// Delete the SafeArray
		SafeArrayDestroy( psa );
	}

	if (varPeakFlags.vt != VT_EMPTY) {
		SAFEARRAY FAR* psa = varPeakFlags.parray;
		varPeakFlags.parray = NULL;
		// Delete the SafeArray
		SafeArrayDestroy(psa);
	}

}
