#include "StdInc.h"

#define pi 3.14159265
#define round(a) ((floor(a)+0.5)<a?ceil(a):floor(a))
#define clamp(a, lo, hi) (a > hi ? hi : (a < lo ? lo : a) )

// Yup2...
inline double rad2deg(double radian)
{
	return radian * (180 / pi);
}

// Yup...
void QuatToEuler(const quat_t *quat, double *rotx,  double *roty, double *rotz)
{
	double sqw;
	double sqx;
	double sqy;
	double sqz;

	double rotxrad;
	double rotyrad;
	double rotzrad;

	sqw = quat->w * quat->w;
	sqx = quat->x * quat->x;
	sqy = quat->y * quat->y;
	sqz = quat->z * quat->z;

	rotxrad = (double)atan2l(2.0 * ( quat->y * quat->z + quat->x * quat->w ) , ( -sqx - sqy + sqz + sqw ));
	rotyrad = (double)asinl( clamp( -2.0 * ( quat->x * quat->z - quat->y * quat->w ), -1, 1 ));
	rotzrad = (double)atan2l(2.0 * ( quat->x * quat->y + quat->z * quat->w ) , (  sqx - sqy - sqz + sqw ));

	*rotx = rad2deg(rotxrad);
	*roty = rad2deg(rotyrad);
	*rotz = rad2deg(rotzrad);

	if (*rotx < 0)
		*rotx = round(360 - *rotx);

	*roty = -(*roty);

	if (*roty < 0)
		*roty = round(360 - *roty);

	if (*rotz < 0)
		*rotz = round(0 - *rotz);
	else if (*rotz > 0)
		*rotz = round(360 - *rotz);

	return;
}

// Load up some IPL file
CIPL*	LoadIPL(const char *filename)
{
	CCSV *csv = LoadCSV(filename);

	if (!csv)
		return NULL;

	return new CIPL(csv);
}

CIPL::CIPL(CCSV *csv)
{
	unsigned int n;

	for (n=0; n<10000; n++)
		m_isLOD[n] = false;

	m_csv = csv;

	// We need to catch the inst instruction
	while (csv->ReadNextRow())
	{
		const char *tag = csv->GetRowItem(0);

		if (strcmp(tag, "inst") == 0)
			ReadInstances();
	}
}

CIPL::~CIPL()
{
	instanceList_t::iterator iter;

	for (iter = m_instances.begin(); iter != m_instances.end(); iter++)
		delete *iter;

	delete m_csv;
}

void	CIPL::ReadInstances()
{
	while (m_csv->ReadNextRow())
	{
		const char **row = m_csv->GetRow();
		CInstance *inst;
		quat_t trans;

		if (strcmp(row[0], "end") == 0)
			break;

		if (m_csv->GetItemCount() < 11)
			continue;

		inst = new CInstance();

		inst->m_modelID = atoi(row[0]);

		inst->m_name = (char*)malloc(strlen(row[1]) + 1);
		memcpy(inst->m_name, row[1], strlen(row[1]) + 1);

		inst->m_interior = atoi(row[2]);

		inst->m_position[0] = atof(row[3]);
		inst->m_position[1] = atof(row[4]);
		inst->m_position[2] = atof(row[5]);

		trans.x = atof(row[6]);
		trans.y = atof(row[7]);
		trans.z = atof(row[8]);
		trans.w = atof(row[9]);

		QuatToEuler(&trans, &inst->m_rotation[0], &inst->m_rotation[1], &inst->m_rotation[2]);

		inst->m_lod = atoi(row[10]);

		if ( inst->m_lod != -1 )
			m_isLOD[inst->m_lod] = true;

		m_instances.push_back(inst);
	}
}

bool	CIPL::IsLOD(unsigned int id)
{
	if (id > 10000)
		return false;

	return m_isLOD[id];
}

CInstance*	CIPL::GetLod(CInstance *scene)
{
	if ( scene->m_lod == -1 )
		return NULL;

	return m_instances.at( scene->m_lod );
}
