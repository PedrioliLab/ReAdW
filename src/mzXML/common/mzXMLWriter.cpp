// -*- mode: c++ -*-


/*
    File: mzXMLWriter.cpp
    Description: instrument-independent mzXML writer.
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



#include "mzXMLWriter.h"
#include "MSUtilities.h"
#include "ScanConverter.h"
 
#include <io.h>
#include <vector>
#include <iostream>
#include <string>

using namespace std;



mzXMLWriter::mzXMLWriter(const std::string& programName,
						 const std::string& programVersion,
						 InstrumentInterface* instrumentInterface) : 
MassSpecXMLWriter(programName,
				  programVersion,
				  instrumentInterface)
{
}




void mzXMLWriter::writeDocument(void)	  
{
	vector<gzstream_fileoffset_t> scanOffsetVec;
	scanOffsetVec.reserve(instrumentInterface_->totalNumScans_);

	startDocument();

	//xml header and namespace info 
	open("mzXML");
	attr("xmlns", "http://sashimi.sourceforge.net/schema_revision/mzXML_3.1");
	attr("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	attr("xsi:schemaLocation", "http://sashimi.sourceforge.net/schema_revision/mzXML_3.1 http://sashimi.sourceforge.net/schema_revision/mzXML_3.1/mzXML_idx_3.1.xsd");

	condenseAttr_ = true;
	open("msRun");
	attr("scanCount", toString(instrumentInterface_->totalNumScans_));
	string startTime("PT");
	startTime += toString(instrumentInterface_->startTimeInSec_);
	startTime += "S";
	attr("startTime", startTime);
	string endTime("PT");
	endTime += toString(instrumentInterface_->endTimeInSec_);
	endTime += "S";
	attr("endTime", endTime);

	if (instrumentInterface_->instrumentInfo_.manufacturer_ != WATERS) {
		open("parentFile");
		attr("fileName", convertToURI(inputFileName_, hostName_));
		attr("fileType", "RAWData");
		attr("fileSha1", sha1Report_);
		close(); // parentFile
	} else {
		for (std::vector<std::string>::size_type f=0; f<inputFileNameList_.size(); ++f) {
			open("parentFile");
			attr("fileName", convertToURI(inputFileNameList_[f], hostName_));
			attr("fileType", "RAWData");
			attr("fileSha1", inputFileSHA1List_[f]);
			close(); // parentFile
		}
	}

	open("msInstrument");

	open("msManufacturer");
	attr("category", "msManufacturer");
	attr(
		"value",
		toString(instrumentInterface_->instrumentInfo_.manufacturer_));
	close(); // msManufacturer

	open("msModel");
	attr("category", "msModel");
	attr(
		"value",
		toString(instrumentInterface_->instrumentInfo_.instrumentModel_));
	close(); // msModel

	open("msIonisation");
	attr("category", "msIonisation");
	attr(
		"value", 
		toString(instrumentInterface_->instrumentInfo_.ionSource_));
	close(); // msIonisation

	// todo: deal with more than one analyzer
	if (instrumentInterface_->instrumentInfo_.analyzerList_.size() == 1) {
		open("msMassAnalyzer");
		attr("category", "msMassAnalyzer");
		attr(
			"value", 
			toString(instrumentInterface_->instrumentInfo_.analyzerList_[0]));
		close(); // msMassAnalyzer
	}

	open("msDetector");
	attr("category", "msDetector");
	attr(
		"value", 
		toString(instrumentInterface_->instrumentInfo_.detector_)); 
	close(); // msDetector

	open("software");
	attr("type", "acquisition");
	attr(
		"name", 
		toString(instrumentInterface_->instrumentInfo_.acquisitionSoftware_));
	attr("version", instrumentInterface_->instrumentInfo_.acquisitionSoftwareVersion_);
	close(); // software

	close(); // msInstrument

	open("dataProcessing");
	if (doCentroiding_) attr("centroided", "1");
	if (doDeisotoping_) attr("deisotoped", "1");

	open("software");
	attr("type", "conversion"); // fix
	attr("name", programName_); // fix
	attr("version", programVersion_); // fix
	close(); // software
	close(); // dataProcessing

	stack<int> lastMSLevel;


	for (long curScanNum=1; curScanNum <= instrumentInterface_->totalNumScans_; curScanNum++) {
		if (verbose_) {
			if ((curScanNum % 100) == 0) {
				cout << curScanNum << "/" << instrumentInterface_->totalNumScans_ << endl;
				cout.flush();
			}
			else if ((curScanNum % 10) == 0) {
				cout << ".";
				cout.flush();
			}
		}

		Scan* curScan = instrumentInterface_->getScan();
		if (curScan == NULL) {
			cerr << "empty scan object" << endl;
			exit(-1);
		}

		// do we need to close the previous scan?
		// (yes, if the stack is not empty,
		// and this level is greater than the last)
		// WCH: merged (resultant) scan is standalone
		//      (i.e. no single associated MS1 scan)
		while (!lastMSLevel.empty() && 
			(curScan->msLevel_ <= lastMSLevel.top() 
			|| 0!=curScan->isScanMergedResult())) {
				close(); // scan
				lastMSLevel.pop();
		}

		lastMSLevel.push(curScan->msLevel_);

		// save index
		//fout_ << endl; // to be safe
		condenseAttr_ = true;
		gzstream_fileoffset_t scanOffset = open("scan",true);
		scanOffsetVec.push_back(scanOffset);
		attr("num", toString(curScanNum));
		condenseAttr_ = false;
		attr("msLevel", toString(curScan->msLevel_));
		attr("peaksCount", toString(curScan->getNumDataPoints()));

		string polarity;
		if (curScan->polarity_ == NEGATIVE) {
			polarity = "-";
		}
		else if (curScan->polarity_ == POSITIVE) {
			polarity = "+";
		}
		else if (curScan->polarity_ == POLARITY_UNDEF) {
			polarity = "any";
		}
		else {
			cerr << "unknown polarity" << endl;
			exit(-1);
		}
		attr("polarity", polarity);


		attr("scanType", toString(curScan->scanType_));

		if (instrumentInterface_->instrumentInfo_.manufacturer_ == THERMO ||
			instrumentInterface_->instrumentInfo_.manufacturer_ == THERMO_SCIENTIFIC ||
			instrumentInterface_->instrumentInfo_.manufacturer_ == THERMO_FINNIGAN ) {
			attr("filterLine", curScan->thermoFilterLine_);
		}

		attr("retentionTime", string("PT") + 
			toString(curScan->retentionTimeInSec_) + string("S")); // in seconds
		attr("injectionTime", string("PT") + toString(curScan->injectionTimeInSec_,4)  + string("S"));
		attr("lowMz", toString(curScan->minObservedMZ_));
		attr("highMz", toString(curScan->maxObservedMZ_));
		attr("basePeakMz", toString(curScan->basePeakMZ_));
		attr("basePeakIntensity", toString(curScan->basePeakIntensity_));
		attr("totIonCurrent", toString(curScan->totalIonCurrent_));

/*		// MassLynx only
		if (curScan->isMassLynx_) {
			if (curScan->isCalibrated_) {
				attr("scanType", "calibration");
			}
		}	
*/ // calibration scanType is now regular type

		// <scan ... merged="1" ...>
		if (curScan->isMerged_ ) {
			attr("merged", "1");
			if (-1 != curScan->mergedScanNum_) {
				attr("mergedScanNum", toString(curScan->mergedScanNum_));
			}
		}


		if (curScan->msLevel_ >= 2) {
			// precursor info
			if (curScan->collisionEnergy_ != 0) {
				attr("collisionEnergy", toString(curScan->collisionEnergy_));
			}
			if (!shotgunFragmentation_) {
				condenseAttr_ = true;
				open("precursorMz");
				if (curScan->precursorIntensity_ > 0) {
					attr("precursorIntensity", toString(curScan->precursorIntensity_));
				}
				else {
					// precursorIntensity is required
					attr("precursorIntensity", toString(0));
				}
				if (curScan->precursorCharge_ > 0) {
					attr("precursorCharge", toString(curScan->precursorCharge_));
				}
				if (curScan->activation_ != ACTIVATION_UNDEF) {
					attr("activationMethod", toString(curScan->activation_));
				}
				data(toString(curScan->precursorMZ_, 11));
				close(); // precursorMz
				condenseAttr_ = false;
			} // !shotgun
		} // precursor


		// <nativeScanRef ...>
		if (curScan->nativeScanRef_.getNumCoordinates()>0) {
			NativeScanRef &nativeScanRef = curScan->nativeScanRef_;
			open("nativeScanRef");
			attr("coordinateType", toString(nativeScanRef.getCoordinateType()));
			ScanCoordinateType name;
			std::string value;
			for (std::vector<NativeScanRef::CoordinateNameValue>::size_type i=0; 
				i<nativeScanRef.getNumCoordinates(); i++) {
				open("coordinate");
				nativeScanRef.getCoordinate(i, name, value);
				attr("name", toString(name));
				attr("value", value);
				close(); // coordinate
			}
			close(); // nativeScanRef
		}

		// <scanOrigin ...>
		if (curScan->getNumScanOrigins()>1) {
			for (long i=0; i<curScan->getNumScanOrigins(); i++) {
				open("scanOrigin");
				attr("parentFileID", curScan->scanOriginParentFileIDs[i]);
				attr("num", toString(curScan->scanOriginNums[i]));
				close(); // scanOrigin
			}
		}

		{
			ScanConverter scanConverter(curScan, doCompression_); // will free mem on descope
			condenseAttr_ = true;
			open("peaks");
			attr("precision", "32");
			condenseAttr_ = false;
			attr("byteOrder", "network");
			attr("contentType", "m/z-int");
			if (scanConverter.isMZIntCompressed_) {
				attr("compressionType", "zlib");
				attr("compressedLen", toString((long)scanConverter.mzIntArrayCompressedByteSize_));
			}
			else {
				attr("compressionType", "none");
				attr("compressedLen", "0");
			}
			data(scanConverter.mzIntB64String_);
			close(); // peaks
		}

		// scan is closed at top of loop

		delete curScan;
	}

	// close the last scan(s)
	while (!lastMSLevel.empty()) {
		close(); // scan
		lastMSLevel.pop();
	}

	close(); //msRun

	// index
	//fout_ << endl; // to be safe

	// Save the offset for the indexOffset element
	gzstream_fileoffset_t indexOffset;

	condenseAttr_ = true;
	indexOffset = open("index",true);
	attr("name", "scan");

	for (long scanNum = 0; scanNum < instrumentInterface_->totalNumScans_ ; scanNum++ ) {
		open("offset");
		attr("id", toString(scanNum+1)); // scans start at 1, not 0
		data(toString(scanOffsetVec[scanNum]));
		close(); // offset
	}

	close(); //index

	open("indexOffset");
	data(toString(indexOffset));
	close(); // indexOffset

	open("sha1");
	noattr();
	fout_.gzflush(); // we're going to SHA1 the file, so write out completely
	cout << "Calculating sha1-sum of mzXML" << endl;
	sha1Report_[0] = 0;
	sha1_.Reset();
	if( !(sha1_.HashGZFile(outputFileName_.c_str()))) {
		cerr << "Cannot open file " << outputFileName_ << " for sha-1 calculation\n";
		exit(-1);// Cannot open file for sha1
	}
	sha1_.Final();
	sha1_.ReportHash(sha1Report_, SHA1::REPORT_HEX);
	cout << "--done (mzXML sha1):" << sha1Report_ << endl;
	data(sha1Report_);
	close(); //sha1

	close(); //mzXML
}
