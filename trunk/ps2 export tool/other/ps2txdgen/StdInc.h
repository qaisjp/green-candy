#ifndef _MAIN_
#define _MAIN_

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// Gay quats
typedef struct quat_s
{
	double w;
	double x;
	double y;
	double z;
} quat_t;

#pragma warning(disable: 4996)

//#define USE_HEAP_DEBUGGING
#define USE_FULL_PAGE_HEAP
#define PAGE_HEAP_INTEGRITY_CHECK
#include <debugsdk/dbgheap.h>
#include <debugsdk/dbgtrace.h>

#include <SharedUtil.h>
#include <core/CFileSystemInterface.h>
#include <core/CFileSystem.h>

#include <dirtools.h>

// Include libraries.
#include "syntax.h"
#include "csv.h"
#include "ide.h"
#include "ipl.h"
#include "ini.h"

#endif //_MAIN_