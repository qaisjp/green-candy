// Functions to handle .cvs files
#include "main.h"

/*======================
	Init
======================*/

// Inits .csv files
CCSV*	LoadCSV(const char *filename)
{
	CCSV *csv;
	FILE *file = fopen(filename, "r");

	if (!file)
		return NULL;

	csv = new CCSV();

	csv->m_currentLine = 0;
	csv->m_numItems = 0;
	csv->m_file = file;
	csv->m_row = NULL;
	return csv;
}

CCSV::CCSV()
{

}

CCSV::~CCSV()
{
	// Freeing memory used by row
	FreeRow();

	fclose(m_file);
}

/*======================
	CSV Functions
======================*/

unsigned int	CCSV::GetItemCount()
{
	return m_numItems;
}

unsigned int	CCSV::GetCurrentLine()
{
	return m_currentLine;
}

// Gets a line
bool	CCSV::ReadNextRow()
{
	unsigned int n;
	unsigned int seek = 0;
	unsigned int numSep = 1;
	unsigned int buffpos = 0;
	char itembuff[65537];
	char linebuff[65537];
	char lastchr = 1;

	m_numItems = 0;

	// Get a complete line, until a NULL termination appears, and check, how many seperation are in this line
	while (seek < 65536 && lastchr != 0 && lastchr != '\n' && lastchr != -1)
	{
		if (!fread(&lastchr, 1, 1, m_file))
			return false;

		linebuff[seek] = lastchr;

		if (lastchr == CSV_SEPERATION)
			numSep++;

		seek++;
	}

	linebuff[seek++] = 0;

	if (lastchr == -1 && seek == 2)
		return false;

	m_currentLine++;

	// Malloc first memory
	m_row = (char**)calloc(numSep, sizeof(long int));

	// Filter all things from the linebuff
	for (n=0; n<seek; n++)
	{
		lastchr = linebuff[n];

		switch(lastchr)
		{
		case '	':
		case 13:
		case 10:
		case ' ':
			break;
		case 0:
		case CSV_SEPERATION:
			itembuff[buffpos] = 0;
			buffpos = 0;

			m_row[m_numItems] = (char*)malloc(strlen(itembuff) + 1);
			memcpy(m_row[m_numItems], itembuff, strlen(itembuff) + 1);

			m_numItems++;
			break;
		default:
			itembuff[buffpos++] = lastchr;
			break;
		}
	}

	return true;
}

const char*	CCSV::GetRowItem(unsigned int id)
{
	if (!m_row)
		return NULL;

	if (id > m_numItems)
		return NULL;

	return m_row[id];
}

const char**	CCSV::GetRow()
{
	return (const char**)m_row;
}

void	CCSV::FreeRow()
{
	unsigned int n;

	if (!m_row)
		return;

	for (n=0; n < m_numItems; n++)
		free(m_row[n]);

	free(m_row);

	m_numItems = 0;
	m_row = NULL;
}