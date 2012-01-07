#ifndef __MAIN__
#define __MAIN__

#include <stdlib.h>
#include <stdio.h>
//#include <unistd.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>

#include <list>
#include <vector>
#include <map>

using namespace std;

#include "csv.h"
#include "ipl.h"
#include "ide.h"
#include "ini.h"
#pragma warning(disable: 4996)

typedef std::map <unsigned short, CInstance*> lodMap_t;

// Gay quats
typedef struct quat_s
{
	double w;
	double x;
	double y;
	double z;
} quat_t;

#endif //__MAIN__