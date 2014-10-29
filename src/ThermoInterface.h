// -*- mode: c++ -*-


/*
    File: ThermoInterface.h
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


#pragma once

#include <string>
#include "mzXML/common/InstrumentInterface.h"
#include "FilterLine.h"

// uncomment "#define MSFILEREADER to use Thermo Foundation DLL, otherwise will use MSFileReader DLLs
// uncomment "define BIT64" to use 32-bit DLLs, otherwise will use 64-bit DLLs
#define MSFILEREADER
//#define BIT64

#ifdef MSFILEREADER
// will use MSFILEREADER
#ifdef BIT64
#define XRAWFILE_DLL "C:\Program Files\Thermo\MSFileReader\XRawfile2_x64.dll"   //32-bit MSFileReader
#else
#define XRAWFILE_DLL "C:\Program Files\Thermo\MSFileReader\XRawfile2.dll"       //64-bit MSFileReader
#endif
// done with MSFILEREADER
#else
// will use Thermo
#ifdef BIT64
#define XRAWFILE_DLL "C:\Program Files (x86)\Thermo\Foundation\XRawfile2.dll"   //64-bit Thermo
#else
#define XRAWFILE_DLL "C:\Program Files\Thermo\Foundation\XRawfile2.dll"         //32-bit Thermo
#endif
// done with Thermo
#endif

#import XRAWFILE_DLL rename_namespace("XRawfile")
using namespace XRawfile;

typedef struct _datapeak
{
	double dMass;
	double dIntensity;
} DataPeak;


class ThermoInterface : public InstrumentInterface {
private:
	// COleDispatchDriver object instance to xrawfile2 dll
	
	// this is the smart pointer we'll use as the entry point;
	// it may actually be initialized as IXRawfile2 or IXRawfile3
	// depending on runtime-detected installed Xcalibur DLL version
	//XRAWFILE2Lib::IXRawfile2Ptr xrawfile2_;
    IXRawfile2Ptr xrawfile2_;
	int IXRawfileVersion_; // which IXRawfile interface was initialized?

	int msControllerType_;
	long firstScanNumber_;
	long lastScanNumber_;
	bool firstTime_;
	void getPrecursorInfo(Scan& scan, long scanNumber, FilterLine& filterLine);
	bool forcePrecursorFromFilter_;

public:
	int getPreInfoCount_;
	int filterLineCount_;
	int oldAPICount_;

public:
	long curScanNum_;

public:
	ThermoInterface(void);
	~ThermoInterface(void);

	virtual bool initInterface(void);
	virtual bool setInputFile(const std::string& fileName);
	virtual void setCentroiding(bool centroid);
	virtual void setDeisotoping(bool deisotope);
	virtual void setCompression(bool compression);
	virtual void forcePrecursorFromFilter(bool mode);
	virtual void setVerbose(bool verbose);
	virtual void setShotgunFragmentation(bool sf) {}
	virtual void setLockspray(bool ls) {}
	virtual Scan* getScan(void);
};


