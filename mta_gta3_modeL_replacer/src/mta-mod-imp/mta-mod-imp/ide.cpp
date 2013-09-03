#include "StdInc.h"

// Get all possible ids from the IDE files
CIDE*	LoadIDE(const char *filename)
{
	CCSV *csv = LoadCSV(filename);

	if (!csv)
		return NULL;

	return new CIDE(csv);
}

CIDE::CIDE(CCSV *csv)
{
	m_csv = csv;

	while (csv->ReadNextRow())
	{
		const char *token = csv->GetRowItem(0);

		if (strcmp(token, "objs") == 0)
			ReadObjects();
	}
}

CIDE::~CIDE()
{
	objectList_t::iterator iter;

	for (iter = m_objects.begin(); iter != m_objects.end(); iter++)
		delete *iter;
}

void	CIDE::ReadObjects()
{
	while (m_csv->ReadNextRow())
	{
		const char **row = m_csv->GetRow();
		CObject *object;

		if (strcmp(row[0], "end") == 0)
			break;

		if (m_csv->GetItemCount() < 5)
			continue;

		object = new CObject();

		object->m_modelID = atoi(row[0]);
		object->m_modelName = row[1];
		object->m_textureName = row[2];
		object->m_drawDistance = atof(row[3]);
		object->m_flags = atol(row[4]);

		m_objects.push_back(object);
	}
}
