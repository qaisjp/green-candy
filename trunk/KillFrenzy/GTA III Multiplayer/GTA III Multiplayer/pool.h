#ifndef _POOL_
#define _POOL_

#define MEM_PTRNODES_POOL					0x009431FC
#define MEM_ENTRYINFOSTRUCTS_POOL			0x00941600
#define MEM_PEDPOOL							0x008F2D14
#define MEM_VEHPOOL							0x00943294
#define MEM_BUILDINGSPOOL					0x008F2CB8
#define MEM_TREADABLESPOOL					0x008F261C
#define MEM_OBJECTSPOOL						0x00880DD8
#define MEM_DUMMIESPOOL						0x008F2CCC
#define	MEM_SHUTDOWNPOOL					0x008F1C20
#define FUNC_InitPointerNodesPool			0x004A3240
#define FUNC_InitEntryInfoStructsPool		0x004A33B0
#define FUNC_InitPedPool					0x004A3510
#define FUNC_InitVehPool					0x004A3670
#define FUNC_InitBuildingsPool				0x004A37D0
#define FUNC_InitTreadablesPool				0x004A3940
#define FUNC_InitObjectsPool				0x004A3AA0
#define FUNC_InitDummiesPool				0x004A3C00
#define FUNC_InitShutdownPool				0x004A3D60
#define MAX_PTRNODES						30000
#define MAX_ENTRYINFOSTRUCTS				5400
#define MAX_PEDS							1024
#define MAX_VEHICLES						512
#define MAX_BUILDINGS						5500
#define MAX_TREADABLES						1214
#define MAX_OBJECTS							2048
#define MAX_DUMMIES							2802
#define MAX_SHUTDOWN						256

#define POOL_SLOTEMPTY	0x80

class CPool
{
public:
	void *m_pPoolHeap;
	char *m_pSlots;
	unsigned int m_uiMaxItems;
	unsigned int m_uiActiveItems;
};

extern CPool**	m_ppPointerNodesPool;
extern CPool**	m_ppEntryInfoStructsPool;
extern CPool**	m_ppPedPool;
extern CPool**	m_ppVehPool;
extern CPool**	m_ppBuildingsPool;
extern CPool**	m_ppTreadablesPool;
extern CPool**	m_ppObjectsPool;
extern CPool**	m_ppDummiesPool;
extern CPool**	m_ppShutdownPool;

#define m_pPedPool			(*m_ppPedPool)
#define m_pVehPool			(*m_ppVehPool)
#define m_pObjectsPool		(*m_ppObjectsPool)

// Pool Functions
void	Pool_Init();

#endif //_POOL_