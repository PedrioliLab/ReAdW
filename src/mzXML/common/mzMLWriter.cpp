// -*- mode: c++ -*-


/*
    File: mzMLWriter.cpp
    Description: instrument-independent mzML writer.
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



#include "mzMLWriter.h"
#include "MSUtilities.h"
#include "ScanConverter.h"
 
#include <io.h>
#include <vector>
#include <iostream>
#include <string>

using namespace std;



mzMLWriter::mzMLWriter(const std::string& programName,
					   const std::string& programVersion,
					   InstrumentInterface* instrumentInterface) : 
MassSpecXMLWriter(programName,
				  programVersion,
				  instrumentInterface)
{
}


void 
mzMLWriter::cvParam(
		const std::string& cvLabel,
		const std::string& accession,
		const std::string& name,
		const std::string& value,
		const std::string& unitAccession,
		const std::string& unitName) {
	open("cvParam");
	attr("cvLabel", cvLabel);
	attr("accession", accession);
	attr("name", name);
	attr("value", value);
	if (unitAccession != "") {
		attr("unitAccession", unitAccession);
		if (unitName != "") {
			attr("unitName", unitName);
		}
	}
	close(); //cvParam
}


void
mzMLWriter::writeDocument(void)	  
{
	bool isThermo = (
		instrumentInterface_->instrumentInfo_.manufacturer_ == THERMO
		||
		instrumentInterface_->instrumentInfo_.manufacturer_ == THERMO_SCIENTIFIC
		||
		instrumentInterface_->instrumentInfo_.manufacturer_ == THERMO_FINNIGAN
		);
	bool isWaters = (
		instrumentInterface_->instrumentInfo_.manufacturer_ == WATERS
		);

	if (!(isThermo||isWaters)) {
		cerr << "unknown manufacturer: " << toString(instrumentInterface_->instrumentInfo_.manufacturer_)
			<< endl;
		exit(-1);
	}

	//xml header and namespace info 
	startDocument();

	
	open("indexedmzML");
	attr("xmlns", "http://psi.hupo.org/schema_revision/mzML_0.99.1");
	attr("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	attr("xsi:schemaLocation", "http://psi.hupo.org/schema_revision/mzML_0.99.1 mzML0.99.1_idx.xsd");

	open("mzML");
	attr("xmlns", "http://psi.hupo.org/schema_revision/mzML_0.99.1");
	attr("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	attr("xsi:schemaLocation", "http://psi.hupo.org/schema_revision/mzML_0.99.1 mzML0.99.1.xsd");
	attr("accession", "");
	attr("id", convertToIDString(inputFileName_, hostName_));
	attr("version", "0.99.1");

	// ontology info
	open("cvList");
	attr("count", "1");
	open("cv");
	attr("cvLabel", "MS");
	attr("fullName", "Proteomics Standards Initiative Mass Spectrometry Ontology");
	attr("version", "2.0.2");
	attr("URI", "http://psidev.sourceforge.net/ms/xml/mzdata/psi-ms.2.0.2.obo");
	close(); // cv
	close(); // cvList


	open("fileDescription");

	open("fileContent");
	// TODO: fill in based on actual content (MS1 only, MSn, SRM, etc)
	cvParam("MS", "MS:1000580", "MSn spectrum", "");
	close(); //fileContent

	open("sourceFileList");
	string sourceFileType;
	if (isThermo) {
		sourceFileType = "Xcalibur RAW file";
	}
	else if (isWaters) {
		sourceFileType = "MassLynx raw format";
	}
	else {
		cerr << "unknown source file type" << endl;
		exit(-1);
	}

	if (isThermo) {
		attr("count", "1");
		open("sourceFile");
		attr("id", "1");
		attr("sourceFileName", inputFileName_);
		attr("sourceFileLocation", convertToURI(inputFileName_, hostName_)); // check if this strips filename
		cvParam("MS", "MS:1000563", sourceFileType, "");
		cvParam("MS", "MS:1000569", "SHA-1", sha1Report_);
		close(); // sourceFile
	}
	else if (isWaters) {

		attr("count", toString((int)inputFileNameList_.size()));
		for (std::vector<std::string>::size_type f=0; f<inputFileNameList_.size(); ++f) {
			open("sourceFile");
			attr("id", toString(f+1));
			attr("sourceFileName", inputFileNameList_[f]);
			attr("sourceFileLocation", convertToURI(inputFileNameList_[f], hostName_)); // check if this strips filename
			cvParam("MS", "MS:1000526", sourceFileType, "");
			cvParam("MS", "MS:1000569", "SHA-1", inputFileSHA1List_[f]);
			close(); // sourceFile
		}
	}
	close(); // sourceFileList

	//open("contact");
	//open("name");
	//data("(unset)"); // make this variable
	//close(); // name
	//open("institution");
	//data("(unset)"); // make this variable
	//close(); //institution
	//open("URI");
	//data("(unset)"); // make this variable
	//close(); //URI
	//close(); //contact

	close(); //fileDescription


	// skipping referenceable paramGroup

	open("sampleList");
	attr("count", "1");
	open("sample");
	attr("id", "1");
	attr("name", "Sample1");
	close(); //sample
	close(); //sampleList

	open("instrumentList");
	attr("count", "1");
	open("instrument");
	attr("id", 
		toString(instrumentInterface_->instrumentInfo_.instrumentModel_));

	// manufacturer
	if (instrumentInterface_->instrumentInfo_.manufacturer_ == MANUFACTURER_UNDEF){
		cerr << "unknown manufacturer." << endl;
		exit(-1);
	}
	string manufacturerAcc = toOBO(instrumentInterface_->instrumentInfo_.manufacturer_);
	string manufacturerName = toString(instrumentInterface_->instrumentInfo_.manufacturer_);
	
	// do not include manufacturer; now considered redundant if instrument model exists, (walk up obo tree)
	//cvParam("MS", manufacturerAcc, manufacturerName, "");

	// instrument model
	if (instrumentInterface_->instrumentInfo_.instrumentModel_ == INSTRUMENTMODEL_UNDEF){
		cerr << "unknown instrument." << endl;
		exit(-1);
	}
	string instModel = toString(instrumentInterface_->instrumentInfo_.instrumentModel_);
	string instAcc = toOBO(instrumentInterface_->instrumentInfo_.instrumentModel_);
	cvParam("MS", instAcc, instModel, "");

	// serial number
	if (instrumentInterface_->instrumentInfo_.instrumentSerialNumber_ != "") {
		cvParam(
			"MS", 
			"MS:1000529", 
			"instrument serial number",
			instrumentInterface_->instrumentInfo_.instrumentSerialNumber_);
	}


	// instrument components: source, detector, analyzer

	int componentCount = 0;
	bool haveIonSource = false;
	bool haveAnalyzer = false;
	bool haveDetector = false;
	string ionSource;
	string detector;
	string analyzer;
	string ionAcc;
	string analyzerAcc;
	string detectorAcc;
			
	if (instrumentInterface_->instrumentInfo_.ionSource_ == IONIZATION_UNDEF) {
			cerr << "unknown ion source. " << endl;
			exit(-1);
	} else {
		++componentCount;
		haveIonSource = true;
		ionSource = toOBOText(instrumentInterface_->instrumentInfo_.ionSource_);
		ionAcc = toOBO(instrumentInterface_->instrumentInfo_.ionSource_);
	}

	// TODO: figure out how to get detector
	// currently, no instrument interfaces set it
	//if (detector != "") {
		++componentCount;
		haveDetector = true;
		// dummy values:
		detector = "electron multiplier tube";
		detectorAcc = "MS:1000111";	
	//}

	if (instrumentInterface_->instrumentInfo_.analyzerList_.size() == 0) {
			cerr << "unknown analyzer." << endl;
			exit(-1);
	} else if (instrumentInterface_->instrumentInfo_.analyzerList_.size() == 1) {
		// TODO: deal with more than one analyzer
		haveAnalyzer = true;
		++componentCount;
		analyzer = toOBOText(instrumentInterface_->instrumentInfo_.analyzerList_[0]);
		analyzerAcc = toOBO(instrumentInterface_->instrumentInfo_.analyzerList_[0]);
	}
	else {
		cerr << "can't deal with more than one analyzer yet." << endl;
		exit(-1);
	}

	if (componentCount > 0) {
		open("componentList");
		attr("count", toString(componentCount));

		int curComponent = 0;
		if (haveIonSource) {
			curComponent++;
			open("source");
			attr("order", toString(curComponent));
			cvParam("MS", ionAcc, ionSource, "");
			close(); //source
		}

		if (haveAnalyzer) {
			curComponent++;
			open("analyzer");
			attr("order", toString(curComponent));
			cvParam("MS", analyzerAcc, analyzer, "");
			close(); //analyzer
		}

		if (haveDetector) {
			curComponent++;
			open("detector");
			attr("order", toString(curComponent));
			cvParam("MS", detectorAcc, detector, "");
			close(); //detector
		}

		close(); //componentList
	} // if # components > 0


	open("instrumentSoftwareRef");
	attr("ref", toString(instrumentInterface_->instrumentInfo_.acquisitionSoftware_));
	close(); //instrumentSoftwareRef

	close(); //instrument
	close(); //instrumentList

	open("softwareList");
	attr("count", "2");

	open("software");
	attr("id", programName_);
	open("softwareParam");
	attr("cvLabel", "MS");
	string paccession;
	if (programName_ == "ReAdW") {
		paccession = "MS:1000541";
	}
	else if (programName_ == "wolf") {
		paccession = "MS:1000538"; // waiting for OBO
		// TODO: fix when OBO adds wolf
		programName_ = "Wolf";
	}
	else {
		cerr << "unknown converter name " << programName_ << endl;
		exit(-1);
	}
	attr("accession", paccession);
	attr("name", programName_);
	attr("version", programVersion_);
	close(); //softwareParam
	close(); //software


	string instrumentSoftware = toString(instrumentInterface_->instrumentInfo_.acquisitionSoftware_);
	open("software");
	attr("id", instrumentSoftware);
	open("softwareParam");
	attr("cvLabel", "MS");
	string instSACC = toOBO(instrumentInterface_->instrumentInfo_.acquisitionSoftware_);
	attr("accession", instSACC);
	attr("name", instrumentSoftware);
	attr("version", instrumentInterface_->instrumentInfo_.acquisitionSoftwareVersion_);
	close(); //softwareParam
	close(); //software

	close(); //softwareList

	open("dataProcessingList");
	int dataProcessingCount = 1;
	if (doCentroiding_) {
		++dataProcessingCount;
	}
	attr("count", toString(dataProcessingCount));
	// SET VENDOR MACHINE, ACCQUISION, PROCESSING SOFTWARE HERE.

	// TODO: how to get instrument, vendor software processing info here?

	open("dataProcessing");
	attr("id", programName_ + " Conversion");
	attr("softwareRef", programName_);
	open("processingMethod");
	attr("order", "1");
	cvParam("MS", "MS:1000544", "Conversion to mzML", "");
	close(); //processingMethod
	close(); //dataProcessing

	if (doCentroiding_) {
		open("dataProcessing");
		attr("id", programName_ + " Centroiding");
		attr("softwareRef", programName_);
		open("processingMethod");
		attr("order", "1");
		cvParam("MS", "MS:1000127", "Centroid Mass Spectrum", "");
		close(); //processingMethod
		close(); //dataProcessing
	}

	close(); //dataProcessingList

	//open("runList");
	//attr("count", "1");
	open("run");
	attr("id", "Exp01");
	attr("instrumentRef", toString(instrumentInterface_->instrumentInfo_.instrumentModel_));
	attr("sampleRef", "1");
	attr("startTimeStamp", instrumentInterface_->timeStamp_);
	open("sourceFileRefList");
	attr("count","1");
	open("sourceFileRef");
	attr("ref", "1");
	close(); //sourceFileRef
	close(); //sourceFileRefList


	// scans
	condenseAttr_ = true;
	open("spectrumList");
	attr("count", toString(instrumentInterface_->totalNumScans_));
	condenseAttr_ = false;
	vector<gzstream_fileoffset_t> scanOffsetVec;
	scanOffsetVec.reserve(instrumentInterface_->totalNumScans_);
	for (long curScanNum=1; curScanNum <= instrumentInterface_->totalNumScans_; curScanNum++) {
		Scan* curScan = instrumentInterface_->getScan();

		// save index at beginning of "spectrum" element
		condenseAttr_ = true;
		gzstream_fileoffset_t spectrumOffset=open("spectrum",true);
		scanOffsetVec.push_back(spectrumOffset);

		attr("scanNumber", toString(curScanNum));
		condenseAttr_ = false;

		attr("id", string("S") + toString(curScanNum));
		attr("msLevel", toString(curScan->msLevel_));

		cvParam("MS", "MS:1000580", "MSn spectrum", "");
		open("spectrumDescription");

		if (curScan->isCentroided_) {
			cvParam("MS", "MS:1000127", "centroid mass spectrum", "");
		}
		
		cvParam("MS", "MS:1000528", "lowest m/z value", toString(curScan->minObservedMZ_));
		cvParam("MS", "MS:1000527", "highest m/z value", toString(curScan->maxObservedMZ_));
		cvParam("MS", "MS:1000504", "base peak m/z", toString(curScan->basePeakMZ_));
		cvParam("MS", "MS:1000505", "base peak intensity", toString(curScan->basePeakIntensity_));

		// applicable for non-current (i.e. ion counter) detectors?
		cvParam("MS", "MS:1000285", "total ion current",  toString(curScan->totalIonCurrent_));

		if (curScan->msLevel_ >= 2) {

			open("precursorList");
			attr("count", "1");
			open("precursor");
			long parentScanNum = curScan->precursorScanNumber_;
			// TODO: ensure all instrument interfaces produce valid scan number
			if (parentScanNum < 1) {
				parentScanNum = 1;
			}
			string spectrumRef = "S" + toString(parentScanNum);
			attr("spectrumRef", spectrumRef);


			open("ionSelection");
			cvParam("MS", "MS:1000040", "m/z", toString(curScan->precursorMZ_));
			if (curScan->precursorCharge_ > 0) {
				cvParam("MS", "MS:1000041", "charge state", toString(curScan->precursorCharge_));
			}
			close(); //ionSelection


			open("activation");
			// TODO: check if activation is unknown?
			cvParam("MS", 
				toOBO(curScan->activation_),
				toOBOText(curScan->activation_),
				"");
			cvParam("MS", 
					"MS:1000045", 
					"collision energy",
					toString(curScan->collisionEnergy_),
					"MS:1000137",
					"Electron Volt");
			close(); //activation

			close(); //precursor
			close(); //precursorList
		} // end msLevel >= 2

		open("scan");
		attr("instrumentRef", toString(instrumentInterface_->instrumentInfo_.instrumentModel_));
		cvParam("MS", "MS:1000016", "scan time",
			toString(curScan->retentionTimeInSec_));
		
		MSScanType scanType = curScan->scanType_;
		if (scanType == SCAN_UNDEF) {
			cerr << "unknown scan type: " << scanType << endl;
			cerr << "please contact developer." << endl;
			exit(-1);
		}
		else {
			string scanDescr = toOBOText(scanType);
			string scanAccession = toOBO(scanType);
			cvParam("MS", scanAccession, scanDescr, "");
		}

		if (curScan->polarity_ != POLARITY_UNDEF) {
			if (curScan->polarity_ == POSITIVE) {
				cvParam("MS", "MS:1000130", "positive scan", "");
			}
			else {
				// negative
				cvParam("MS", "MS:1000129", "negative scan", toString(curScan->minObservedMZ_));
			}
		}

		// TODO: paramGroupRef here

		if (curScan->isThermo_) {
			cvParam("MS", "MS:1000512", "filter string", curScan->thermoFilterLine_);
		}

		open("selectionWindowList");
		attr("count", "1");
		open("selectionWindow");
		cvParam("MS", "MS:1000501", "scan m/z lower limit", toString(curScan->startMZ_));
		cvParam("MS", "MS:1000500", "scan m/z upper limit", toString(curScan->endMZ_));
		close(); //selectionWindow
		close(); //selectionWindowList

		close(); //scan
		close(); //spectrumDescription

		// TODO: if instrument centroided, look into attr("dataProcessingRef", "Xcalibur Processing");


		{
			// no zlib compression for now
			ScanConverter scanConverter(curScan, false); // will free mem on descope

			// mzRatio array
			open("binaryDataArray");
			attr("arrayLength", toString(scanConverter.mzRatioArrayLength_));
			attr("encodedLength", toString(scanConverter.mzRatioB64StringEncodedByteLen_));

			cvParam("MS", "MS:1000521", "32-bit float", "");

			string mzCompressionType;
			string compressionAsc;
			if (scanConverter.isMZRatioCompressed_) {
				mzCompressionType = "zlib";
				compressionAsc = "MS:1000574";
			}
			else {
				mzCompressionType = "no compression";
				compressionAsc = "MS:1000576";
			}
			cvParam("MS", compressionAsc, mzCompressionType, "");
	
			cvParam("MS", "MS:1000514", "m/z array", "");

			open("binary");
			data(scanConverter.mzRatioB64String_);
			close(); //binary

			close(); //binaryDataArray

			// intensity array	
			open("binaryDataArray");
			attr("arrayLength", toString(scanConverter.intensityArrayLength_));
			attr("encodedLength", toString(scanConverter.intensityB64StringEncodedByteLen_));

			cvParam("MS", "MS:1000521", "32-bit float", "");
			string intensityCompressionType;
			if (scanConverter.isIntensityCompressed_) {
				intensityCompressionType = "zlib";
				compressionAsc = "MS:1000574";
			}
			else {
				intensityCompressionType = "no compression";
				compressionAsc = "MS:1000576";
			}
			cvParam("MS", compressionAsc, intensityCompressionType, "");

			cvParam("MS", "MS:1000515", "intensity array", "");

			open("binary");
			data(scanConverter.intensityB64String_);
			close(); //binary

			close(); //binaryDataArray
		}

		close(); //spectrum

		delete curScan;
	}
	// end scan loop
	close(); //spectrumList
	close(); //run
	//close(); //runList

	close(); //mzML


	// calculate SHA1 hash, from first character to last
	// character of "</mzML>"
	fout_.gzflush(); // we're going to SHA1 the file, so write out completely
	sha1Report_[0]=0;
	sha1_.Reset();
	if( !(sha1_.HashGZFile(outputFileName_.c_str()))) {
		cerr << "Cannot open file " << outputFileName_ << " for sha-1 calculation\n";
		exit(-1);// Cannot open file for sha1
	}
	sha1_.Final();
	sha1_.ReportHash(sha1Report_, SHA1::REPORT_HEX);
	cout << "--done (mzML sha1):" << sha1Report_ << endl;
	

	// Save the offset for the indexOffset element
	gzstream_fileoffset_t indexOffset;
	condenseAttr_ = true;
	indexOffset = open("index",true);
	attr("name", "spectrum");

	// TODO: handle more than one run
	//open("run");
	//attr("runNumber", "1");

	for (long scanNum = 0; scanNum < instrumentInterface_->totalNumScans_ ; scanNum++ ) {
		open("offset");
		attr("scanNumber", toString(scanNum+1)); // scans start at 1, not 0
		data(toString(scanOffsetVec[scanNum]));
		close(); // offset
	}

	//close(); //run
	close(); //index

	open("indexOffset");
	data(toString(indexOffset));
	close(); // indexOffset

	open("fileContentType");
	data("MSn spectrum");
	close(); //fileContentType
	
	open("fileChecksum");
	//attr("type","Sha1");
	data(sha1Report_);
	close(); //fileChecksum
	close(); //indexedmzML
}
