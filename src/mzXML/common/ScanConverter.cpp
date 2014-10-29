// -*- mode: c++ -*-


/*
    File: ScanConverter.cpp
    Description: spectrum data to base64-encoded string converter.
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

#include "ScanConverter.h"
#include <iostream>
#include <winsock2.h> // htonl
#include <math.h>
#include "zlib.h"
#include "Base64.h"

using namespace std;

#define LITTLE_ENDIAN
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

#ifdef __LITTLE_ENDIAN
#define swapbytes(x) ntohl(x)  /* use system byteswap (ntohl is a noop on bigendian systems) */
#else
uint32_t swapbytes(uint32_t x) {
	return 
		((x & 0x000000ffU) << 24) |
		((x & 0x0000ff00U) <<  8) |
		((x & 0x00ff0000U) >>  8) |
		((x & 0xff000000U) >> 24);
}
#endif

uint64_t swapbytes64(uint64_t x) {
	return 
		((uint64_t)swapbytes((uint32_t)(x & 0xffffffffU)) << 32) |
		((uint64_t)swapbytes((uint32_t)(x >> 32)));
}


// for zlib:
#define CHECK_ERR(err, msg) {						\
	if (err != Z_OK) {								\
	fprintf(stderr, "%s error: %d\n", msg, err);	\
	exit(1);										\
	}												\
}



ScanConverter::ScanConverter(double mz, double inten)
{
	float* mzIntArray = new float[2];
	mzIntArray[0] = (float)mz;
	mzIntArray[1] = (float)inten;

	for (long c=0; c<2; c++){
		uint32_t mzIntBytes = (*(uint32_t *)(&(mzIntArray[c])) );
		( (uint32_t *) mzIntArray)[c] = htonl(mzIntBytes);
		}
	Base64 base64Encoder;

	mzIntB64StringEncodedByteLen_ = base64Encoder.b64_encode(
				(const unsigned char *) mzIntArray, 
				(2 * sizeof(float)) );

	mzIntB64String_= new char[mzIntB64StringEncodedByteLen_ + 1]; // +1 for \0
	strcpy(mzIntB64String_, (const char*)base64Encoder.getOutputBuffer());
	base64Encoder.freeOutputBuffer();

	delete[] mzIntArray;

}
ScanConverter::ScanConverter(Scan *scan,
							 bool tryZlibCompression) 
							 : scan_(scan),
							 tryZlibCompression_(tryZlibCompression),
							 mzRatioArrayLength_(-1),
							 intensityArrayLength_(-1),
							 mzIntArrayLength_(-1),
							 isMZRatioCompressed_(false),
							 isIntensityCompressed_(false),
							 isMZIntCompressed_(false),
							 mzRatioB64String_(NULL),
							 intensityB64String_(NULL),
							 mzIntB64String_(NULL),
							 mzRatioB64StringEncodedByteLen_(-1),
							 intensityB64StringEncodedByteLen_(-1),
							 mzIntB64StringEncodedByteLen_(-1)
{
	Base64 base64Encoder;
	// TODO: try compressing if requested, encode

	// handle zero length case
	if (scan->getNumDataPoints() == 0) {
		mzRatioArrayLength_ = 0;
		intensityArrayLength_ = 0;
		mzIntArrayLength_ = 0;
		isMZRatioCompressed_ = false;
		isIntensityCompressed_ = false;
		isMZIntCompressed_ = false;

		mzRatioArrayCompressedByteSize_ = 0;
		intensityArrayCompressedByteSize_ = 0;
		mzIntArrayCompressedByteSize_ = 0;

		mzRatioB64StringEncodedByteLen_ = 12;
		intensityB64StringEncodedByteLen_ = 12;
		mzIntB64StringEncodedByteLen_ = 12;

		mzRatioB64String_ = new char[13];
		intensityB64String_ = new char[13];
		mzIntB64String_ = new char[13];

		strcpy(mzRatioB64String_ , "AAAAAAAAAAA=\0");	
		strcpy(intensityB64String_ , "AAAAAAAAAAA=\0");
		// single 0 0 pair
		strcpy(mzIntB64String_ , "AAAAAAAAAAA=\0");	
	}
	else {

		// create three separate float arrays: m/z ratio list, intensity list, m/z-intensity pairs
		mzRatioArrayLength_ = scan->getNumDataPoints();
		intensityArrayLength_ = scan->getNumDataPoints();
		mzIntArrayLength_ = scan->getNumDataPoints() * 2;

		float* mzRatioArray = new float[mzRatioArrayLength_];
		float* intensityArray = new float[intensityArrayLength_];
		float* mzIntArray = new float[mzIntArrayLength_];

		// copy actual data

		for (long c=0; c<mzRatioArrayLength_; c++){
			mzRatioArray[c] = (float) scan->mzArray_[c];
		}

		for (long c=0; c<intensityArrayLength_; c++){
			intensityArray[c] =  (float) scan->intensityArray_[c];
		}

		long mzIntIndex = 0;
		for (long c=0; c<mzRatioArrayLength_; c++){
			mzIntArray[mzIntIndex++] = mzRatioArray[c];
			mzIntArray[mzIntIndex++] = intensityArray[c];
		}

		// convert each float to network byte order
		for (long c=0; c<mzRatioArrayLength_; c++){
			// get the 4 bytes that represent a float:
			// get the addr of the float, treat it as a uint32 ptr, and dereference
			uint32_t mzBytes = (*(uint32_t *)(&(mzRatioArray[c])) );
			// pretend that the array stores 4-byte unsigned ints, rather
			// than 4-byte floats, for the sake of the conversion
			( (uint32_t *) mzRatioArray)[c] = htonl(mzBytes);
			//printf("%f, %8.8X, %8.8X; ", mz, mzBytes, ( (uint32_t *) pDataNetwork)[n] );

			// sanity check
			/* 
			uint32_t mb2;
			uint32_t ib2;
			mb2 = ntohl(( (uint32_t *) pDataNetwork)[n-2]);
			ib2 = ntohl(( (uint32_t *) pDataNetwork)[n-1]);
			printf("%f, %f\n\n", (*(float*)(&mb2)), (*(float*)(&ib2))  );
			*/

		}


		for (long c=0; c<intensityArrayLength_; c++){
			uint32_t intensityBytes = (*(uint32_t *)(&(intensityArray[c])) );
			( (uint32_t *) intensityArray)[c] = htonl(intensityBytes);
		}

		for (long c=0; c<mzIntArrayLength_; c++){
			uint32_t mzIntBytes = (*(uint32_t *)(&(mzIntArray[c])) );
			( (uint32_t *) mzIntArray)[c] = htonl(mzIntBytes);
		}


		// zlib compression

		Byte* mzRatioCompressedArray = NULL;
		Byte* intensityCompressedArray = NULL;
		Byte* mzIntCompressedArray = NULL;

		if (tryZlibCompression_) {
			int err;

			// compression buffer must be at least 0.1% of the size of the
			// data to be compressed + 12 bytes
			mzRatioArrayCompressedByteSize_ = 
				(mzRatioArrayLength_ * sizeof(uint32_t));
			mzRatioArrayCompressedByteSize_ += (uLong)ceil(mzRatioArrayCompressedByteSize_ * 0.1) + 12;
			mzRatioCompressedArray = (Byte*)calloc((uInt) mzRatioArrayCompressedByteSize_, 1);
			if (mzRatioCompressedArray == NULL) {
				cerr << "Error allocating memory for peaks compression\n";
				exit(1);
			}

			intensityArrayCompressedByteSize_ = 
				(intensityArrayLength_ * sizeof(uint32_t));
			intensityArrayCompressedByteSize_ += (uLong)ceil(intensityArrayCompressedByteSize_ * 0.1) + 12;
			intensityCompressedArray = (Byte*)calloc((uInt) intensityArrayCompressedByteSize_, 1);
			if (intensityCompressedArray == NULL) {
				cerr << "Error allocating memory for peaks compression\n";
				exit(1);
			}

			mzIntArrayCompressedByteSize_ = 
				(mzIntArrayLength_ * sizeof(uint32_t));
			mzIntArrayCompressedByteSize_ += (uLong)ceil(mzIntArrayCompressedByteSize_ * 0.1) + 12;
			mzIntCompressedArray = (Byte*)calloc((uInt) mzIntArrayCompressedByteSize_, 1);
			if (mzIntCompressedArray == NULL) {
				cerr << "Error allocating memory for peaks compression\n";
				exit(1);
			}


			err = compress(
				mzRatioCompressedArray,
				&mzRatioArrayCompressedByteSize_,
				(const Bytef*) mzRatioArray,
				mzRatioArrayLength_ * sizeof(uint32_t));
			if (err != Z_OK) {
				cerr << "compression error" << endl;
				exit(1);
			};
			isMZRatioCompressed_ = // sometimes zlib actually causes bloat
			  (mzRatioArrayCompressedByteSize_ < (mzRatioArrayLength_ * sizeof(uint32_t)));


			err = compress(
				intensityCompressedArray,
				&intensityArrayCompressedByteSize_,
				(const Bytef*) intensityArray,
				intensityArrayLength_ * sizeof(uint32_t));
			if (err != Z_OK) {
				cerr << "compression error" << endl;
				exit(1);
			};
			isIntensityCompressed_ = // sometimes zlib actually causes bloat
			(intensityArrayCompressedByteSize_ < (intensityArrayLength_ * sizeof(uint32_t)));


			err = compress(
				mzIntCompressedArray,
				&mzIntArrayCompressedByteSize_,
				(const Bytef*) mzIntArray,
				mzIntArrayLength_ * sizeof(uint32_t));
			if (err != Z_OK) {
				cerr << "compression error" << endl;
				exit(1);
			};
			isMZIntCompressed_ = // sometimes zlib actually causes bloat
				(mzIntArrayCompressedByteSize_<(mzIntArrayLength_ * sizeof(uint32_t)));
		}



		// do base64 encoding
		Base64 base64Encoder;

		if (isMZRatioCompressed_) {
			mzRatioB64StringEncodedByteLen_ = base64Encoder.b64_encode(
				(const unsigned char *) mzRatioCompressedArray, 
				mzRatioArrayCompressedByteSize_);	
		}
		else {
			mzRatioB64StringEncodedByteLen_ = base64Encoder.b64_encode(
				(const unsigned char *) mzRatioArray, 
				(mzRatioArrayLength_ * sizeof(float)) );
		}
		mzRatioB64String_ = new char[mzRatioB64StringEncodedByteLen_ + 1]; // +1 for \0
		strcpy(mzRatioB64String_, (const char*)base64Encoder.getOutputBuffer());
		base64Encoder.freeOutputBuffer();


		if (isIntensityCompressed_) {
			intensityB64StringEncodedByteLen_ = base64Encoder.b64_encode(
				(const unsigned char *) intensityCompressedArray, 
				intensityArrayCompressedByteSize_);
		}
		else {
			intensityB64StringEncodedByteLen_ = base64Encoder.b64_encode(
				(const unsigned char *) intensityArray, 
				(intensityArrayLength_ * sizeof(float)) );
		}
		intensityB64String_ = new char[intensityB64StringEncodedByteLen_ + 1]; // +1 for \0
		strcpy(intensityB64String_, (const char*)base64Encoder.getOutputBuffer());
		base64Encoder.freeOutputBuffer();


		if (isMZIntCompressed_) {
			mzIntB64StringEncodedByteLen_ = base64Encoder.b64_encode(
				(const unsigned char *) mzIntCompressedArray,
				mzIntArrayCompressedByteSize_);
		}
		else {
			mzIntB64StringEncodedByteLen_ = base64Encoder.b64_encode(
				(const unsigned char *) mzIntArray, 
				(mzIntArrayLength_ * sizeof(float)) );
		}
		mzIntB64String_ = new char[mzIntB64StringEncodedByteLen_ + 1]; // +1 for \0
		strcpy(mzIntB64String_, (const char*)base64Encoder.getOutputBuffer());
		base64Encoder.freeOutputBuffer();
		


		// cleanup
		delete [] mzRatioArray;
		delete [] intensityArray;
		delete [] mzIntArray;

		free(mzRatioCompressedArray);
		free(intensityCompressedArray);
		free(mzIntCompressedArray);
	}
}




ScanConverter::~ScanConverter() {
	// cleanup
	delete [] mzRatioB64String_;
	delete [] intensityB64String_;
	delete [] mzIntB64String_;
}
