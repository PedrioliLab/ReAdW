// -*- mode: c++ -*-


/*
    File: Base64.h
    Description: Tools for encoding and decoding an array of bytes in base64.
    Date: March 27, 2004

    Copyright (C) 2004 Pedrioli Patrick, ISB, Proteomics (original author)
    - additional work by Natalie Tasman, ISB Seattle


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


#ifndef _INCLUDED_BASE64_H_
#define _INCLUDED_BASE64_H_

/** @file Base64.h
    @brief 
*/

/**
   Base64
   
*/
class Base64 {
 public:

  Base64();
  ~Base64();

  unsigned char* getOutputBuffer(void);

  /** 
      NOTE: allocates internal buffer for output;
      user must call freeOutputBuffer() after use
  */
  int b64_encode(const unsigned char* inputBuf,
		 int numInputBytes);

  /** 
      NOTE: allocates internal buffer for output;
      user must call freeOutputBuffer() after use
  */  
  int b64_decode (const char *src,
		  int expectedNumEncodedBytes);

  /** 
      release the internal output buffer's memory 
      (allocated in b64_encode)
  */
  void freeOutputBuffer(void);

 protected:

  bool haveOutputBuffer_;
  unsigned char* curOutputBuffer_;
  

  int b64_encode_helper (char *dest,
			 const unsigned char *src,
			 int len);

  // avaliable encoding characters
  static const unsigned char *b64_tbl;
    
  // padding character for last 3-character group
  static const unsigned char b64_pad;
  
  inline int getPosition( char buf );

  /* base64 encode a group of between 1 and 3 input chars into a group of  4 output chars */
  void encode_group (unsigned char output[],
		     const unsigned char input[],
		     int n);

  /* base64 decode a group of 4 input chars into a group of between 0 and
   * 3 output chars */
  void decode_group (unsigned char output[],
		     const unsigned char input[],
		     int *n);
};
#endif // header guards
