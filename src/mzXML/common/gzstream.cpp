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
// File          : gzstream.C
// Revision      : $Revision: 1.7 $
// Revision_date : $Date: 2003/01/08 14:41:27 $
// Author(s)     : Deepak Bandyopadhyay, Lutz Kettner
// 
// Standard streambuf implementation following Nicolai Josuttis, "The 
// Standard C++ Library".
// ============================================================================

#include "gzstream.h"
#include <iostream>
#include <string.h>  // for memcpy

#if defined(_WIN32) || defined(__MINGW32__)  // MSVC or MinGW
#define strcasecmp _stricmp
#endif

#ifdef GZSTREAM_NAMESPACE
namespace GZSTREAM_NAMESPACE {
#endif

// ----------------------------------------------------------------------------
// Internal classes to implement gzstream. See header file for user classes.
// ----------------------------------------------------------------------------

// --------------------------------------
// class gzstreambuf:
// --------------------------------------

gzstreambuf* gzstreambuf::open( const char* name, int open_mode, int compressionlevel) {
    if ( is_open())
        return (gzstreambuf*)0;
    mode = open_mode;
    // no append nor read/write mode
    if ((mode & std::ios::ate) || (mode & std::ios::app)
        || ((mode & std::ios::in) && (mode & std::ios::out)))
        return (gzstreambuf*)0;
    char  fmode[11];
    char* fmodeptr = fmode;
    if ( mode & std::ios::in)
        *fmodeptr++ = 'r';
    else if ( mode & std::ios::out)
        *fmodeptr++ = 'w';
    *fmodeptr++ = 'b';
	if (( mode & std::ios::out) && (compressionlevel<0)) {
		// undeclared compression level - go all or nothing depending on filename
		int len = (int)strlen(name);
		compressionlevel = (!strcasecmp(name+len-3,".gz"))?9:0;
	}
	if (( mode & std::ios::out) && (compressionlevel > 0))
        *fmodeptr++ = '0'+compressionlevel; // bpratt insilicos 2008
    *fmodeptr = '\0';
	if (compressionlevel || (mode & std::ios::in)) {
		file = gzopen( name, fmode);
		uncompressedfile = NULL;
	} else {
		file = 0;
		uncompressedfile = fopen( name, fmode);
	}
	if ((!file) && (!uncompressedfile))
        return (gzstreambuf*)0;
    opened = 1;
    return this;
}

gzstreambuf * gzstreambuf::close() {
    if ( is_open()) {
        sync();
        opened = 0;
        if (file && (gzclose( file) == Z_OK))
            return this;
        if (uncompressedfile && (fclose( uncompressedfile) == 0))
            return this;
    }
    return (gzstreambuf*)0;
}

int gzstreambuf::underflow() { // used for input buffer only
    if ( gptr() && ( gptr() < egptr()))
        return * reinterpret_cast<unsigned char *>( gptr());

    if ( ! (mode & std::ios::in) || ! opened)
        return EOF;
    // Josuttis' implementation of inbuf
    size_t n_putback = gptr() - eback();
    if ( n_putback > 4)
        n_putback = 4;
    memcpy( buffer + (4 - n_putback), gptr() - n_putback, n_putback);

    int num = gzread( file, buffer+4, bufferSize-4);
    if (num <= 0) // ERROR or EOF
        return EOF;

    // reset buffer pointers
    setg( buffer + (4 - n_putback),   // beginning of putback area
          buffer + 4,                 // read position
          buffer + 4 + num);          // end of buffer

    // return next character
    return * reinterpret_cast<unsigned char *>( gptr());    
}

int gzstreambuf::flush_buffer() {
    // Separate the writing of the buffer from overflow() and
    // sync() operation.
    int w = (int)(pptr() - pbase());
	int ww = file?gzwrite(file,pbase(),w):(int)fwrite(pbase(),1,w,uncompressedfile);
	if (ww != w)
        return EOF;
	uncompressed_bytes_written += ww; // for bytes_written() usage
    pbump( -w);
    return w;
}

int gzstreambuf::overflow( int c) { // used for output buffer only
    if ( ! ( mode & std::ios::out) || ! opened)
        return EOF;
    if (c != EOF) {
        *pptr() = c;
        pbump(1);
    }
    if ( flush_buffer() == EOF)
        return EOF;
    return c;
}

int gzstreambuf::sync() {
    // Changed to use flush_buffer() instead of overflow( EOF)
    // which caused improper behavior with std::endl and flush(),
    // bug reported by Vincent Ricard.
    if ( pptr() && pptr() > pbase()) {
        if ( flush_buffer() == EOF)
            return -1;
    }
    return 0;
}

// --------------------------------------
// class gzstreambase:
// --------------------------------------

gzstreambase::gzstreambase( const char* name, int mode, int compressionlevel) {
    init( &buf);
    open( name, mode, compressionlevel);
}

gzstreambase::~gzstreambase() {
    buf.close();
}

void gzstreambase::open( const char* name, int open_mode, int compressionlevel) {
    if ( ! buf.open( name, open_mode, compressionlevel))
        clear( rdstate() | std::ios::badbit);
}

void gzstreambase::close() {
    if ( buf.is_open())
        if ( ! buf.close())
            clear( rdstate() | std::ios::badbit);
}

// prepare file for possible read even while writing
// use this sparingly, it degrades compression rate
void gzstreambuf::gzflush() {
	flush_buffer();
	if (file) {
		::gzflush(file,Z_FULL_FLUSH); // zlib
	} else {
		fflush(uncompressedfile);
	}
}

// a little helper func suite to make it easier to swap in with fprintf oriented code
#include <stdarg.h>
// open for output as gzip if named (something).gz
ogzstream *ogzfopen(const char *fname) {
	return new ogzstream(fname);
}
int fprintf(ogzstream *s,const char * format, ...) {
	va_list args;
	va_start(args, format);
#define OGZSTREAM_PRINTF_MAXLEN 32768
	char *printfbuf = new char[OGZSTREAM_PRINTF_MAXLEN];
	int len = printfbuf?(int)vsnprintf(printfbuf,OGZSTREAM_PRINTF_MAXLEN,format, args ):0;
	if (len) {
		(*s) << printfbuf;
	}
	delete[] printfbuf;
	return len;
}
void fclose(ogzstream *s) {
	if (s) {
		s->close();
		delete s;
	}
}

#ifdef GZSTREAM_NAMESPACE
} // namespace GZSTREAM_NAMESPACE
#endif

// ============================================================================
// EOF //
