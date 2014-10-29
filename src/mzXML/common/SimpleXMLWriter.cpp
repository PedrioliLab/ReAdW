// -*- mode: c++ -*-


/*
    File: SimpleXMLWriter.cpp
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


#include "SimpleXMLWriter.h"

#include <sstream>
#include <iostream>
#include <io.h> // for optional grabbing start of open element (byte offset)

using namespace std;



SimpleXMLWriter::SimpleXMLWriter() : 
indentStr_(""), 
spaceStr_(" "),
indent_(0),
tagOpen_(false),
hasAttr_(false),
hasData_(false),
condenseAttr_(false)
{
}


SimpleXMLWriter::~SimpleXMLWriter() {
}


void 
SimpleXMLWriter::startDocument(void) {
	fout_ << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>" << endl;
}


gzstream_fileoffset_t 
SimpleXMLWriter::open(const string& tagname,bool getOffset) {

	if (tagOpen_) {
		if (hasAttr_) {
			fout_ << " ";
		}
		fout_ << ">";
		tagOpen_ = false;
	} 
	if (!tags_.empty()) {
		fout_ << endl;
	}
	tags_.push(tagname);

	tagOpen_ = true;
	indent_ += 1;
	setIndentStr();
	fout_ << indentStr_;
	
	// grab the offset
	gzstream_fileoffset_t offset = getOffset?fout_.bytes_written():0;
	fout_ << "<" << tagname;
	hasAttr_ = false;
	hasData_ = false;
	return offset;
}




void 
SimpleXMLWriter::attr(const string& attrname, 
					  const string& val) {
						  if (!hasAttr_) {
							  // whitespace formatting to assist parsing:
							  // first attribute should appear after element name,
							  // separated by one space character, on same line
							  // as the element start.
							  fout_ << " ";
						  }
						  else if (!condenseAttr_) { 
							fout_ << endl << indentStr_ << spaceStr_;
						  }
						  else {
							  fout_ << spaceStr_;
						  }
						  fout_ << attrname << "=" << "\"" << val << "\"";
						  hasAttr_ = true;
}



void 
SimpleXMLWriter::attr(const vector< pair< string, string> > & attrlist) {
	vector< pair< string, string> >::size_type numAttr  = attrlist.size();
	if (numAttr > 0) {
		hasAttr_ = true;
		for (vector< pair< string,string> >::size_type i=0; i<attrlist.size(); ++i) {
			fout_ << " " << attrlist[i].first << "=" << "\"" << attrlist[i].second << "\"";
		}
	}
}


void
SimpleXMLWriter::noattr(void) {
	if (tagOpen_) {
		fout_ << ">";
		tagOpen_ = false;
	} 
}

void 
SimpleXMLWriter::data(const string& data) {
	if (tagOpen_) {
		if (hasAttr_) {
			fout_ << " ";
		}
		fout_ << ">";
		tagOpen_ = false;
	} 
	fout_ << data;
	hasData_ = true;
}



void 
SimpleXMLWriter::close() {
	if (tagOpen_) {
		fout_ << " />";
	} else {
		if (!hasData_) {
			fout_ << endl << indentStr_;
		}
		fout_ << "</" << tags_.top() << ">";
	}

	tagOpen_ = false;
	hasData_ = false;

	--indent_;
	setIndentStr();

	tags_.pop();
	if (tags_.empty()) {
		fout_ << endl;
	}
}



void 
SimpleXMLWriter::closeAll() {
	while (!tags_.empty()) {
		this->close();
	}
}





void 
SimpleXMLWriter::setIndentStr() {
	indentStr_ = "";
	for (int i=1; i<indent_; i++) {
		indentStr_ += spaceStr_;
	}
}
