// -*- mode: c++ -*-


/*
    File: MassSpecXMLWriter.h
    Description: instrument-independent common parent to mzXML and mzML writers.
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

#include "SHA1.h"
#include "SimpleXMLWriter.h"
#include "InstrumentInterface.h"



class MassSpecXMLWriter : public SimpleXMLWriter {
protected:
	std::vector<gzstream_fileoffset_t> scanOffsets_;
	InstrumentInterface* instrumentInterface_;
	SHA1 sha1_; // md5sum calculator
	char sha1Report_[1024]; // sha1 hash
	std::string inputFileName_;
	// only used for MassLynx, currently:
	std::vector<std::string> inputFileNameList_;
	std::vector<std::string> inputFileSHA1List_;

	std::string outputFileName_;
	std::string programName_;
	std::string programVersion_;
	std::string hostName_;
	bool doCentroiding_;
	bool doDeisotoping_;
	bool doCompression_;
	bool shotgunFragmentation_;
	bool lockspray_;
	bool verbose_;

	bool doChecksum_;
public:
	MassSpecXMLWriter(
		const std::string& programName,
		const std::string& programVersion,
		InstrumentInterface* instrumentInterface);

	virtual ~MassSpecXMLWriter();

	virtual void setCentroiding(bool centroid);
	virtual void setDeisotoping(bool deisotope);
	virtual void setCompression(bool compression);
	virtual void setShotgunFragmentation(bool sf);
	virtual void setLockspray(bool ls);
	virtual void setVerbose(bool verbose);
	virtual bool setInputFile(const std::string& inputFileName);
	virtual bool setOutputFile(const std::string& outputFileName);

	virtual void writeDocument(void) = 0;
};
