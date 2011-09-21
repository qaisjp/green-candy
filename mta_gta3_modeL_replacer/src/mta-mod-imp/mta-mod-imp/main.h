#ifndef __MAIN__
#define __MAIN__

#include <stdlib.h>
#include <stdio.h>
//#include <unistd.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>

#include <list>

using namespace std;

#include "csv.h"
#include "ipl.h"
#include "ide.h"
#pragma warning(disable: 4996)

// IPL inst entry
typedef struct ipl_inst_s
{
	unsigned short usModelID;
	unsigned char *p_ucName;
	unsigned char ucUnknown;
	float fPosX;
	float fPosY;
	float fPosZ;
	double fRotX;
	double fRotY;
	double fRotZ;
	double fRotW;
	char ucIsLOD;
} ipl_inst_t;

// Gay quats
typedef struct quat_s
{
	double w;
	double x;
	double y;
	double z;
} quat_t;

#endif //__MAIN__