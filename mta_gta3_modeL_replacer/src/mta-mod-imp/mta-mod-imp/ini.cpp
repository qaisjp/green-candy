#include "StdInc.h"
#include <sys/stat.h>

CINI*	LoadINI(const char *filename)
{
	FILE *stream = fopen(filename, "rb");
	char *buffer;
	struct stat stats;
	CINI *config;

	if (!stream)
		return NULL;

	fstat(fileno(stream), &stats);

	buffer = (char*)malloc(stats.st_size + 1);

	fread(buffer, 1, stats.st_size, stream);
	buffer[stats.st_size] = 0;

	config = new CINI(buffer);

	free(buffer);

	return config;
}

CINI::CINI(const char *buffer)
{
	CSyntax *syntax = new CSyntax(buffer, strlen(buffer));
	Entry *section = NULL;

	do
	{
		size_t len;
		const char *token = syntax->ParseToken( &len );

		if ( len == 1 )
		{
			if ( *token == '[' )
			{
				char *name;
				unsigned long offset = syntax->GetOffset();

				// Read section and select it
				if ( !syntax->ScanCharacterEx( ']', true, true, false ) )
					continue;

				size_t tokLen = syntax->GetOffset() - offset - 1;

				name = (char*)malloc( tokLen + 1 );
				strncpy( name, token + 1, tokLen );

				name[len] = 0;

				section = new Entry( name );

				entries.push_back( section );
			}
			else if ( *token == ';' )
			{
				syntax->ScanCharacter( '\n' );
				syntax->Seek( -1 );
				continue;
			}
			else
				goto sectionDo;
		}
		else if ( section )
		{
sectionDo:
			char name[256];
			char value[256];
			const char *set;

			len = std::min(len, (size_t)255);

			strncpy(name, token, len );
			name[len] = 0;

			set = syntax->ParseToken( &len );

			if ( len != 1 || *set != '=' )
				continue;

			syntax->GetToken( value, 256 );

			section->Set( name, value );
		}

	} while ( syntax->GotoNewLine() );

	delete syntax;
}

CINI::~CINI()
{
	entryList_t::iterator iter;

	for (iter = entries.begin(); iter != entries.end(); iter++)
		delete *iter;
}

CINI::Entry* CINI::GetEntry(const char *entry)
{
	entryList_t::iterator iter;

	for (iter = entries.begin(); iter != entries.end(); iter++)
	{
		if (strcmp((*iter)->m_name, entry) == 0)
			return *iter;
	}

	return NULL;
}

const char* CINI::Get(const char *entry, const char *key)
{
	Entry *section = GetEntry(entry);

	if (!section)
		return NULL;

	return section->Get( key );
}

int CINI::GetInt(const char *entry, const char *key)
{
	const char *value = Get(entry, key);

	if (!value)
		return 0;

	return atoi(value);
}

double CINI::GetFloat(const char *entry, const char *key)
{
	const char *value = Get(entry, key);

	if (!value)
		return 0.0f;

	return atof(value);
}
