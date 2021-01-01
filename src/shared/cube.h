#pragma once

#define _FILE_OFFSET_BITS 64

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <assert.h>
#include <time.h>

#ifdef WIN32
  #define WIN32_LEAN_AND_MEAN
  #ifdef _WIN32_WINNT
  #undef _WIN32_WINNT
  #endif
  #define _WIN32_WINNT 0x0500
  #include "windows.h"
  #ifndef _WINDOWS
    #define _WINDOWS
  #endif
  #ifdef _MSC_VER
    #include <eh.h>
    #include <dbghelp.h>
    #include <intrin.h>
  #endif
//  #define ZLIB_DLL
#endif

#ifndef STANDALONE
#include "engine/includegl.h"
#endif

#include <enet/enet.h>
#include <zlib.h>

// C++ STD Lib.
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <memory>
#include <functional>

// Libraries from the ext folder.
#include <nlohmann/json.hpp>

// Engine and game related includes.
#include "tools.h"
#include "geom.h"
#include "command.h"

#ifndef STANDALONE
//#include "glexts.h"
#include "glemu.h"
#endif

#include "iengine.h"
#include "igame.h"


