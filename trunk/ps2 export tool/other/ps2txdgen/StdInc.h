#ifndef _MAIN_
#define _MAIN_

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#pragma warning(disable: 4996)

//#define USE_HEAP_DEBUGGING
#define USE_FULL_PAGE_HEAP
#define PAGE_HEAP_INTEGRITY_CHECK
#define PAGE_HEAP_MEMORY_STATS
//#define USE_HEAP_STACK_TRACE
#include <debugsdk/dbgheap.h>
#include <debugsdk/dbgtrace.h>

#include <SharedUtil.h>
#include <core/CFileSystemInterface.h>
#include <core/CFileSystem.h>

#include <dirtools.h>

// Include libraries.
#include <gtaconfig/include.h>

#endif //_MAIN_