// -----------------------------------------------------------------------------------------
// ram_speed by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2011-2016 rigaya
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// --------------------------------------------------------------------------------------------

#ifndef _RAM_SPEED_VERSION_H_
#define _RAM_SPEED_VERSION_H_

#include "rgy_version.h"

#define VER_FILEVERSION             0,0,4,0
#define VER_STR_FILEVERSION          "0.04"
#define VER_STR_FILEVERSION_TCHAR _T("0.04")

#ifdef DEBUG
#define VER_DEBUG   VS_FF_DEBUG
#define VER_PRIVATE VS_FF_PRIVATEBUILD
#else
#define VER_DEBUG   0
#define VER_PRIVATE 0
#endif

#ifdef _M_IX86
#define CHECKCLINFO_FILENAME "ram_speed (x86)"
#else
#define CHECKCLINFO_FILENAME "ram_speed (x64)"
#endif

#define VER_STR_COMMENTS         "ram_speed"
#define VER_STR_COMPANYNAME      ""
#define VER_STR_FILEDESCRIPTION  CHECKCLINFO_FILENAME
#define VER_STR_INTERNALNAME     CHECKCLINFO_FILENAME
#define VER_STR_ORIGINALFILENAME "ram_speed.exe"
#define VER_STR_LEGALCOPYRIGHT   "ram_speed by rigaya"
#define VER_STR_PRODUCTNAME      CHECKCLINFO_FILENAME
#define VER_PRODUCTVERSION       VER_FILEVERSION
#define VER_STR_PRODUCTVERSION   VER_STR_FILEVERSION

#endif //_RAM_SPEED_VERSION_H_
