// NOTE: this copy of the gzstream code has been altered by bpratt of Insilicos LLC: 
// a couple of tweaks for VC8 and to support different compression levels, including 
// none at all (write a regular file instead of a gzip file), and adding methods
// to make it easier to swap in with code using fprintf.  Oct 2008

// ============================================================================
// gzstream, C++ iostream classes wrapping the zlib compression library.
// Copyright (C) 2001  Deepak Bandyopadhyay, Lutz Kettner
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// ============================================================================
//
// File          : gzstream.h
// Revision      : $Revision: 1.5 $
// Revision_date : $Date: 2002/04/26 23:30:15 $
// Author(s)     : Deepak Bandyopadhyay, Lutz Kettner
// 
// Standard streambuf implementation following Nicolai Josuttis, "The 
// Standard C++ Library".
// ============================================================================

#ifndef GZSTREAM_H
#define GZSTREAM_H 1

// standard C++ with new header file names and std:: namespace
#include <iostream>
#include <fstream>
#include <zlib.h>

#ifdef _MSC_VER
typedef __int64 gzstream_fileoffset_t;
#elif defined __MINGW32__
typedef off64_t gzstream_fileoffset_t;
#else
typedef off_t gzstream_fileoffset_t;
#endif

#ifdef GZSTREAM_NAMESPACE
namespace GZSTREAM_NAMESPACE {
#endif

// ----------------------------------------------------------------------------
// Internal classes to implement gzstream. See below for user classes.
// ----------------------------------------------------------------------------

class gzstreambuf : public std::streambuf {
private:
    static const int bufferSize = 47+256;    // size of data buff
    // totals 512 bytes under g++ for igzstream at the end.
    gzFile           file;               // file handle for compressed file
	FILE*			 uncompressedfile;   // in case we're not actually compressing
    char             buffer[bufferSize]; // data buffer
    char             opened;             // open/close state of stream
    int              mode;               // I/O mode
	gzstream_fileoffset_t uncompressed_bytes_written; // number of bytes written, before compression

    int flush_buffer();
public:
    gzstreambuf() : opened(0), uncompressed_bytes_written(0) {
        setp( buffer, buffer + (bufferSize-1));
        setg( buffer + 4,     // beginning of putback area
              buffer + 4,     // read position
              buffer + 4);    // end position      
        // ASSERT: both input & output capabilities will not be used together
    }
    int is_open() { return opened; }
    gzstreambuf* open( const char* name, int open_mode, int compressionlevel);
    gzstreambuf* close();
    ~gzstreambuf() { close(); }
	// interface for telling how many uncompressed bytes have been written
	gzstream_fileoffset_t bytes_written() {
		flush_buffer();
		return uncompressed_bytes_written;
	}
	// prepare file for possible read even while writing
	// use this sparingly, it degrades compression rate
	void gzflush();

    virtual int     overflow( int c = EOF);
    virtual int     underflow();
    virtual int     sync();
};

class gzstreambase : virtual public std::ios {
protected:
    gzstreambuf buf;
public:
    gzstreambase() { init(&buf); }
    gzstreambase( const char* name, int open_mode, int compressionlevel);
    ~gzstreambase();
    void open( const char* name, int open_mode, int compressionlevel);
    void close();
    gzstreambuf* rdbuf() { return &buf; }
	bool is_open() {return buf.is_open()!=0; }
};

// ----------------------------------------------------------------------------
// User classes. Use igzstream and ogzstream analogously to ifstream and
// ofstream respectively. They read and write files based on the gz* 
// function interface of the zlib. Files are compatible with gzip compression.
// ----------------------------------------------------------------------------

class igzstream : public gzstreambase, public std::istream {
public:
    igzstream() : std::istream( &buf) {} 
    igzstream( const char* name )
        : gzstreambase( name, std::ios::in, -1), std::istream( &buf) {}  
    gzstreambuf* rdbuf() { return gzstreambase::rdbuf(); }
    void open( const char* name ) {
        gzstreambase::open( name, std::ios::in, -1);
    }
};

class ogzstream : public gzstreambase, public std::ostream {
public:
    ogzstream() : std::ostream( &buf) {}
    ogzstream( const char* name, int compressionlevel=-1)
        : gzstreambase( name, std::ios::out, compressionlevel), std::ostream( &buf) {}  
    gzstreambuf* rdbuf() { return gzstreambase::rdbuf(); }
    void open( const char* name, int compressionlevel=-1) { // use level=0 to write plain text (-1=default compression, 9 is max)
        gzstreambase::open( name, std::ios::out, compressionlevel);
    }
    // interface for telling how many uncompressed bytes have been written
	gzstream_fileoffset_t bytes_written() {
		return rdbuf()->bytes_written();
	}
	// prepare file for possible read even while writing
	// use this sparingly, it degrades compression rate
	void gzflush() {
		rdbuf()->gzflush();
	}

	// for ogzprintf use
#define OGZSTREAM_PRINTF_MAXLEN 32768
	char printfbuf[OGZSTREAM_PRINTF_MAXLEN];
};

// a little helper func suite to make it easier to swap in with fprintf oriented code
ogzstream *ogzfopen(const char *fname); // open for output as gzip if named (something).gz
int fprintf(ogzstream *s,const char * format, ...);
void fclose(ogzstream *);


#ifdef GZSTREAM_NAMESPACE
} // namespace GZSTREAM_NAMESPACE
#endif

#endif // GZSTREAM_H
// ============================================================================
// EOF //

