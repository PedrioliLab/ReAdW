/*

File:         : TPPVersion.h
Author        : Brian Pratt, Insilicos LLC
Date          : 9-18-05

Version info to update for each user release, shows up throughout the TPP suite
This is used by the regenerate_VersionInfo build step.

Copyright (C) 2005 Insilicos LLC, Institute for Systems Biology

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "sysdepend.h"


//
// These strings should be updated for each official release
//
#define TPP_RELEASE_NAME "UWPR_MAINTAINED"
#define TPP_MAJOR_RELEASE_NUMBER 2014
#define TPP_MINOR_RELEASE_NUMBER 1
#define TPP_REV_RELEASE_NUMBER 1

#define TPP_PRODUCT_FULLNAME "Trans Proteomics Pipeline"
#define TPP_PRODUCT_NAME "TPP"
#define TPP_BRIEF_DESCRIPTION "The Trans Proteomic Pipeline was developed at the Institute for Systems Biology."

#include <time.h>

// but don't use them directly in your code, instead you normally
// use this constructed string, which includes build date
extern const char *szTPPVersionInfo; // in VersionInfo.o, generated at build time
extern const time_t globTPPBuildTime; // in VersionInfo.o, generated at build time

// for msconvert and other proteowizard-provided source
#define PWIZ_USER_VERSION_INFO_H_STR szTPPVersionInfo


