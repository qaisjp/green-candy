// Functions to handle .cvs files
#include "StdInc.h"

/*======================
	Init
======================*/

CCSV*   CreateCSV( const char *filename )
{
    FILE *file = fopen( filename, "w" );

    if ( !file )
        return NULL;

    return new CCSV( file );
}

// Inits .csv files
CCSV*	LoadCSV(const char *filename)
{
	FILE *file = fopen(filename, "r");

	if (!file)
		return NULL;

    return new CCSV( file );
}

CCSV::CCSV( FILE *ioptr )
{
    m_currentLine = 0;
    m_numItems = 0;
    m_file = ioptr;
    m_row = NULL;
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

// Writes a line
void    CCSV::WriteNextRow( const std::vector <std::string>& items )
{
    std::vector <std::string>::const_iterator iter = items.begin();

    while ( iter != items.end() )
    {
        const std::string& item = *iter;
        fwrite( item.c_str(), 1, item.size(), m_file );

        iter++;

        if ( iter != items.end() )
            fputc( CSV_SEPERATION, m_file );
    }

    fputc( '\n', m_file );

	m_currentLine++;
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
