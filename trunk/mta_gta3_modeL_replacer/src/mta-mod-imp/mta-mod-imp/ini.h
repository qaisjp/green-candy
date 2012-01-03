#ifndef _INI_
#define _INI_

#include "syntax.h"

class CINI
{
					CINI(const char *buffer);
					~CINI();

public:
	friend CINI*	LoadINI(const char *filename);

	class Entry
	{
	public:
		class Setting
		{
		public:
			~Setting()
			{
				free(key);
				free(value);
			}

			char *key;
			char *value;
		};

		Entry(char *name)
		{
			m_name = name;
		}

		~Entry()
		{
			settingList_t::iterator iter;

			for (iter = settings.begin(); iter != settings.end(); iter++)
				delete *iter;

			free(m_name);
		}

		Setting*	Find(const char *key)
		{
			settingList_t::iterator iter;

			for (iter = settings.begin(); iter != settings.end(); iter++)
			{
				if (strcmp((*iter)->key, key) == 0)
					return *iter;
			}

			return NULL;
		}

		void		Set(const char *key, const char *value)
		{
			Setting *set = Find(key);

			if (set)
			{
				free(set->value);

				set->value = _strdup(value);
				return;
			}

			set = new Setting();

			set->key = _strdup(key);
			set->value = _strdup(value);

			settings.push_back( set );
		}

		inline const char*	Get(const char *key)
		{
			Setting *set = Find(key);

			if (!set)
				return NULL;

			return set->value;
		}

		inline int GetInt(const char *key)
		{
			const char *value = Get(key);

			if (!value)
				return 0;

			return atoi(value);
		}

		inline double GetFloat(const char *key)
		{
			const char *value = Get(key);

			if (!value)
				return 0;

			return atof(value);
		}

		inline bool GetBool(const char *key)
		{
			const char *value = Get(key);

			if (!value)
				return false;

			return (strcmp(value, "true") == 0) || (unsigned int)atoi(value) != 0;
		}

		char*		m_name;

	private:
		typedef list <Setting*> settingList_t;

		settingList_t settings;
	};

	Entry*			GetEntry(const char *entry);

	const char*		Get(const char *entry, const char *key);
	int				GetInt(const char *entry, const char *key);
	double			GetFloat(const char *entry, const char *key);

private:
	typedef list <Entry*> entryList_t;

	entryList_t		entries;
};

#endif //_INI_