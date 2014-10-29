// -*- mode: c++ -*-


/*
    File: Base64.cpp
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


#include <iostream>
#include <stdlib.h>
#include <string.h>


#include "Base64.h"


using namespace std;

/**
   base64 encoding

   JMT: referring to 

   http://www.herongyang.com/data/base64.html
   quote:

   The encoding process is to:

   * Divide the input bytes stream into blocks of 3 bytes.

   * Divide the 24 bits of a 3-byte block into 4 groups of 6 bits.

   * Map each group of 6 bits to 1 printable character, based on
   the 6-bit value.

   * If the last 3-byte block has only 1 byte of input data, pad 2
   bytes of zero (\x0000). After encoding it as a normal block,
   override the last 2 characters with 2 equal signs (==), so
   the decoding process knows 2 bytes of zero were padded.
       
   * If the last 3-byte block has only 2 bytes of input data, pad
   1 byte of zero (\x00). After encoding it as a normal block,
   override the last 1 character with 1 equal signs (=), so the
   decoding process knows 1 byte of zero was padded.
       
   * Carriage return (\r) and new line (\n) are inserted into the
   output character stream. They will be ignored by the decoding
   process.

   end quote


   so: 
   (size of input buffer (in bytes) / 3 bytes) gives the
   number of 3-byte, or 24-bit, groups 

   (number of 24-bit groups times 4) gives the number of 6-bit groups
     
   (number of 6-bit groups) = (number of printable characters)
     
   -- max additional characters will be len("==\r\n\0) == 5
   -- (min additional is len("\r\n\0") == 3

   (number of printable characters) + 5 = max length of output buffer for
   base-64 encoded data.
     
*/



Base64::Base64() : 
  haveOutputBuffer_(false) 
{
}

Base64::~Base64() {
  if (haveOutputBuffer_) {
    free(curOutputBuffer_);
    haveOutputBuffer_ = false;
  }
}




unsigned char* 
Base64::getOutputBuffer(void) {
  if (!haveOutputBuffer_) {
    //error!
    exit(1);
  }
  else {
    return curOutputBuffer_;
  }
}




// avaliable encoding characters: 
// for unencoded 6-bit value i (0 <= i <= 63),
// the value i is encoded by b64_tbl[i]
const unsigned char * Base64::b64_tbl = 
  (const unsigned char*) "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
// padding character for last 3-character group
const unsigned char Base64::b64_pad = 
  (unsigned char) '='; 







/**
   given a vector of bytes and its length, produce encoded version of
   input in pre-allocated destination array.
   
   output buffer is null-terminated.

   returns number of bytes in encoded output buffer


   !!allocates internal buffer for output;
   user must call freeOutputBuffer() after use !!
   
*/
int
Base64::b64_encode(const unsigned char* unencodedInputBuf,
		   int numUnencodedInputBytes) {

  // remember, the actual length used may be up to 2 bytes less than
  // we allocated; store this so we know where to null-terminate
  int actualEncodedLength = 0;


  // no rounding up here; assume the worst case later (if the last group
  // has only 1 byte and is padded out with two 0 bytes)
  int num24BitGroups = numUnencodedInputBytes / 3; 


  int num6BitGroups = num24BitGroups * 4;


  int numEncodedCharacters =  num6BitGroups + 5;

  
  if (haveOutputBuffer_) {
    // error!
    cerr << "encode called before output buffer cleared with freeOutputBuffer()." << endl;
    exit(1);
  }
  
  // !! the user is responible for later calling freeOutputBuffer() !!
  curOutputBuffer_ = (unsigned char *) malloc(sizeof(unsigned char) * numEncodedCharacters);
  haveOutputBuffer_ = true;



  int remainingUnencodedInputBytes = numUnencodedInputBytes;
  while (remainingUnencodedInputBytes > 0) {
    encode_group( (unsigned char*)curOutputBuffer_ + actualEncodedLength, unencodedInputBuf, remainingUnencodedInputBytes > 3 ? 3 : remainingUnencodedInputBytes);
    remainingUnencodedInputBytes -= 3;
    unencodedInputBuf += 3;
    actualEncodedLength += 4;
  }

  
  // null-terminate the output buffer
  curOutputBuffer_[actualEncodedLength] = '\0';
  return actualEncodedLength;
}
			  












void 
Base64::freeOutputBuffer(void) {
  if (!haveOutputBuffer_) {
    //error!
    cerr << "freeOutputBuffer called with no active input buffer." << endl;
    exit(1);
  }
  else {
    free(curOutputBuffer_);
    haveOutputBuffer_ = false;
  }
}









/* base64 encode a group of between 1 and 3 input chars into a group
   of 4 output chars */
void 
Base64::encode_group (unsigned char output[],
		      const unsigned char input[],
		      int n) {
  unsigned char ingrp[3];

  ingrp[0] = n > 0 ? input[0] : 0;
  ingrp[1] = n > 1 ? input[1] : 0;
  ingrp[2] = n > 2 ? input[2] : 0;

  /* upper 6 bits of ingrp[0] */
  output[0] = n > 0 ? b64_tbl[ingrp[0] >> 2] : b64_pad;

  /* lower 2 bits of ingrp[0] | upper 4 bits of ingrp[1] */
  output[1] = n > 0 ? b64_tbl[((ingrp[0] & 0x3) << 4) | (ingrp[1] >> 4)] : b64_pad;

  /* lower 4 bits of ingrp[1] | upper 2 bits of ingrp[2] */
  output[2] = n > 1 ? b64_tbl[((ingrp[1] & 0xf) << 2) | (ingrp[2] >> 6)] : b64_pad;

  /* lower 6 bits of ingrp[2] */
  output[3] = n > 2 ? b64_tbl[ingrp[2] & 0x3f] : b64_pad;

}




// bpratt note: this looks like a very slow implementation, those calls
// to strchr are going to suck.  have a look at my implementation in
// ramp_base64.h/cpp for a faster method
#ifdef DONT_MIND_WAITING

/* base64 decode a group of 4 input chars into a group of between 0 and
 * 3 output chars */
void 
Base64::decode_group (unsigned char output[],
		      const unsigned char input[],
		      int *n) {
  unsigned char *t1, *t2;
  *n = 0;

  if (input[0] == '=')
    return;

  t1 = (unsigned char*) strchr ((const char*)b64_tbl, input[0]);
  t2 = (unsigned char*) strchr ((const char*)b64_tbl, input[1]);

  output[(*n)++] = (unsigned char)(((t1 - b64_tbl) << 2) | ((t2 - b64_tbl) >> 4));

  if (input[2] == '=')
    return;

  t1 = (unsigned char*) strchr ((const char*)b64_tbl, input[2]);

  output[(*n)++] = (unsigned char)(((t2 - b64_tbl) << 4) | ((t1 - b64_tbl) >> 2));

  if (input[3] == '=')
    return;

  t2 = (unsigned char*) strchr ((const char*)b64_tbl, input[3]);

  output[(*n)++] = (unsigned char)(((t1 - b64_tbl) << 6) | (t2 - b64_tbl));

  return;
}
#endif







int 
Base64::getPosition( char buf ) {

  if( buf > 96 )		// [a-z]
    return (buf - 71);
  else if( buf > 64 )		// [A-Z]
    return (buf - 65);
  else if( buf > 47 )		// [0-9]
    return (buf + 4);
  else if( buf == 43 )
    return 63;
  else				// buf == '/'
    return 64;
}








/**
   get output with getOutputBuffer();
   NOTE: the user is responible for later calling freeOutputBuffer()

   returns number of decoded bytes
*/
int
Base64::b64_decode ( const char *src,
		     int numEncodedCharacters) {

  if (strlen(src) != numEncodedCharacters) {
    cerr << "decode called with inconsistent input data size." << endl;
    exit(1);
  }

  /**
	

     Base64 Decoding

     (JMT: referring to 

      http://www.herongyang.com/data/base64.html)


     
     reverse encoding steps:


     * Carriage return (\r) and new line (\n) are inserted into the
       output character stream. They will be ignored by the decoding
       process.
  

     * If the last 3-byte block has only 2 bytes of input data, pad 1
       byte of zero (\x00). After encoding it as a normal block,
       override the last 1 character with 1 equal signs (=), so the
       decoding process knows 1 byte of zero was padded.

     * If the last 3-byte block has only 1 byte of input data, pad 2
       bytes of zero (\x0000). After encoding it as a normal block,
       override the last 2 characters with 2 equal signs (==), so the
       decoding process knows 2 bytes of zero were padded.
    


     * Map each group of 6 bits to 1 printable character, based on the
       6-bit value.


     * Divide the 24 bits of a 3-byte block into 4 groups of 6 bits.
    

     * Divide the input bytes stream into blocks of 3 bytes.
    
    



   */

  if (haveOutputBuffer_) {
    // error!
    cerr << "decode called before output buffer cleared with freeOutputBuffer()." << endl;
    exit(1);
  }
  
  // !! the user is responible for later calling freeOutputBuffer() !!
  curOutputBuffer_ = (unsigned char *) malloc(sizeof(unsigned char) * numEncodedCharacters);
  haveOutputBuffer_ = true;


  unsigned char* decodedBytePtr = curOutputBuffer_;
  const char*  encodedBytePtr = src;
  int register a;
  int register b;
  int byte1,byte2,byte3,byte4;



  /** 
      we stop when we reach '=' or /0
  */
  while (*encodedBytePtr) {

      /**
	 (Decoded === Original)
	 
	 Goal: given 4 encoded bytes, which correspond to 3 decoded
	 (original) bytes, produce the 3 original bytes.
	 
	 The 3 original bytes (24 bits) were conceptually divided into
	 4 groups, each of 6 bits.

	 Given 4 encoded bytes:
	   
	   For each byte, map the ascii value to the original 6-bit value (0-63).
	   
	   Then, assemble the four 6-bit groups back into three bytes,
	   via bit-shifting.

	 
	               byte 1   byte 2   byte 3   byte 4
	 encoded bits: 11111111 22222222 33333333 44444444
	                  |         |       |         |
	                  |         |       |         |
	                  V         V       V         V

	 6-bit values: 00aaaaaa 00bbbbbb 00cccccc 00dddddd
	   (0-63)

	                  |         |       |         |
	                  \         |       |         /
	                   \        |       |        / 
			    V       V       V       V

			     a << 2   b << 4   c << 6
			       or       or       or
			     b >> 6   c >> 2    d
	 3 (orig.):         aaaaaabb bbbbcccc ccdddddd
	    bytes

       */

    byte1 = encodedBytePtr[0];
    byte2 = encodedBytePtr[1];
    byte3 = encodedBytePtr[2];
    byte4 = encodedBytePtr[3];

    if (byte1 == 61 )		// if '='
      break;
     
    if( byte1 > 96 )		// [a-z]
      a = (byte1 - 71);
    else if( byte1 > 64 )		// [A-Z]
      a = (byte1 - 65);
    else if( byte1 > 47 )		// [0-9]
      a = (byte1 + 4);
    else if( byte1 == 43 )		// '+'
      a = 62;
    else			// '/'
      a = 63;     


    if( byte2 > 96 )		// [a-z]
      b = (byte2 - 71);
    else if( byte2 > 64 )		// [A-Z]
      b = (byte2 - 65);
    else if( byte2 > 47 )		// [0-9]
      b = (byte2 + 4);
    else if( byte2 == 43 )		// '+'
      b = 62;
    else			// '/'
      b = 63;     
    

      
    *decodedBytePtr++ = ( a << 2) | ( b >> 4);
     
    if (byte3 == 61)		// '='
      break;

    if( byte3 > 96 )		// [a-z]
      a = (byte3 - 71);
    else if( byte3 > 64 )		// [A-Z]
      a = (byte3 - 65);
    else if( byte3 > 47 )		// [0-9]
      a = (byte3 + 4);
    else if( byte3 == 43 )		// '+'
      a = 62;
    else			// '/'
      a = 63;     


    // a is now 'c' in diagram above
    *decodedBytePtr++ = ( b << 4) | ( a >> 2);

    if (byte4 == 61) // '='
      break;

    if( byte4 > 96 )		// [a-z]
      b = (byte4 - 71);
    else if( byte4 > 64 )		// [A-Z]
      b = (byte4 - 65);
    else if( byte4 > 47 )		// [0-9]
      b = (byte4 + 4);
    else if( byte4 == 43 )		// '+'
      b = 62;
    else			// '/'
      b = 63;    

    // a is now 'c' and b is now 'd'
    // in diagram above
    *decodedBytePtr++ = ( a << 6) | ( b );

    encodedBytePtr += 4;
  }
  
  // return actual number of encoded bytes
  return (int)(decodedBytePtr - curOutputBuffer_);
}
