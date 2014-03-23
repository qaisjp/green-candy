#ifndef _IPL_
#define _IPL_

class CInstance
{
	friend class CIPL;

public:
	unsigned int	m_modelID;
	char*			m_name;

	unsigned int	m_interior;

	double			m_position[3];
	double			m_rotation[3];

	long			m_flags;

	int				m_lod;
	int				m_lodID;
};

typedef	std::vector <CInstance*> instanceList_t;

class CIPL
{
	friend CIPL*	LoadIPL(const char *filename);
public:
					CIPL(CCSV *csv);
					~CIPL();

	bool			IsLOD(unsigned int id);
	CInstance*		GetLod(CInstance *obj);

	instanceList_t	m_instances;
private:
	void			ReadInstances();

	CCSV*			m_csv;

    typedef std::list <int> lodList_t;

    lodList_t       m_isLod;
};

CIPL*	LoadIPL(const char *filename);

#endif
