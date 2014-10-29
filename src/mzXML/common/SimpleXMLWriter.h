// -*- mode: c++ -*-


/*
    File: SimpleXMLWriter.h
    Description: Basic stack-based XML writer.
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



#ifndef _INCLUDED_SIMPLEXMLWRITER_H_
#define _INCLUDED_SIMPLEXMLWRITER_H_

#include <string>
#include <stack>
#include <vector>
#include <fstream>

#include "Base64.h"
#include "SHA1.h"
#include "gzstream.h"

class SimpleXMLWriter {
public:
	SimpleXMLWriter();
	virtual ~SimpleXMLWriter();

	void startDocument(void);


	gzstream_fileoffset_t open(const std::string& tagname,bool getOffset=false); // optionally returns offset right before the start of the element, or 0

	void open(const std::string& tagname, const std::vector< std::pair<std::string, std::string> > & attrlist);

	void attr(const std::string& attrname, 
		const std::string& val);

	void attr(const std::vector< std::pair< std::string, std::string> > & attrlist);

	void noattr(void);

	void data(const std::string& data);

	void close();

	void closeAll();

	bool condenseAttr_;

protected:

	// ogzstream writes in gzip format if filename ends with ".gz" (case insensitive).
	ogzstream fout_; // must be set before use.  Add error checking for unset case.
	void setIndentStr();

	int indent_;
	bool tagOpen_;
	bool hasAttr_;
	bool hasData_;

	std::stack<std::string> tags_;
	std::string indentStr_;
	std::string spaceStr_;

	Base64 base64Transformer_;
	SHA1 sha1_;  // md5sum calculator
	char sha1Report_[1024]; // sha1 hash
};


#endif // _INCLUDED_SIMPLEXMLWRITER_H_
