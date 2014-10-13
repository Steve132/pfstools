/**
 * @brief Compatibility header file
 * 
 * This file is a part of PFSTOOLS package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2003,2004 Rafal Mantiuk and Grzegorz Krawczyk
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * ---------------------------------------------------------------------- 
 *
 * @author Grzegorz Krawczyk, <krawczyk@mpi-sb.mpg.de>
 *
 * $Id: compatibility.h,v 1.1 2007/03/01 14:10:57 rdmantiuk Exp $
 */

#ifndef COMPATIBILITY_H
#define COMPATIBILITY_H

#ifdef _MSC_VER

#include <io.h>

#define popen _popen
#define dup _dup
//#define fileno _fileno
#define fdopen _fdopen
#define open _open
#define pclose _pclose

#define O_CREAT _O_CREAT
#define O_TRUNC _O_TRUNC
#define O_RDWR _O_RDWR
#define S_IREAD _S_IREAD
#define S_IWRITE _S_IWRITE
#define O_BINARY _O_BINARY

#endif


#ifdef __linux__

#include <unistd.h>

#define O_BINARY 0

#define stricmp strcasecmp

#endif

#define DEBUG_STR if(1); else cerr

#endif
