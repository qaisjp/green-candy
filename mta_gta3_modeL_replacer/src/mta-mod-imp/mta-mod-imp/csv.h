/*	CSV reading, MYSQL type environment (as these files are MYSQL database backups)
	General Notice: Always close .csv files if you dont need them anymore.
*/

#ifndef _CSV_
#define _CSV_

#define CSVERR_FILEOPEN 0

#define CSV_SEPERATION 44

typedef unsigned char **CSV_ROW;

class CCSV
{
	friend CCSV*	LoadCSV(const char *filename);

public:
					CCSV();
					~CCSV();

	unsigned int	GetCurrentLine();

	bool			ReadNextRow();
	unsigned int	GetItemCount();
	const char*		GetRowItem(unsigned int id);
	const char**	GetRow();
	void			FreeRow();

private:
	FILE*			m_file;
	unsigned int	m_currentLine;
	unsigned int	m_numItems;
	char**			m_row;
};

#endif
