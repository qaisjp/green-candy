#ifndef _MODELINFO_
#define _MODELINFO_

#define MEM_Table_pModelInfo				0x0083D408
#define MEM_Table_modelLoadInfo				0x006c7090
#define FUNC_RequestModel					0x00407EA0
#define FUNC_LoadRequestedModels			0x0040A440
#define MODELINFO_MAXENTRIES				5500
#define MAX_MODEL_REQUEST_QUEUE				0xFF
#define FUNC_UnloadSceneModels				0x00408780
#define FUNC_DisableSceneBuildings			0x00408680
#define FUNC_FreeModel						0x00408830
#define FUNC_FindModel						0x00527810
#define FUNC_PushModelName					0x00527720
#define MEM_pSpecialPedModelInfo			0x0095CD48
#define FUNC_CSpecialPedModel_GetValues		0x004736E0

class CModelDimension
{
	BYTE					pad[12];
	float					m_fPlacementHeight;
	float					m_fUnknown;
	vec3_t					m_vecStartBounds;
	vec3_t					m_vecEndBounds;
	BYTE					pad2[44];
};

enum eRwType
{
	RW_ONE,
	RW_TWO
};

class CAtomicModelInfo;
class CDamageAtomicModelInfo;
class CLODAtomicModelInfo;

enum eModelType
{
	MODEL_LOL
};

class RwObject;

#define GTACALL

class CBaseModelInfo
{
public:
    virtual												~CBaseModelInfo() = 0;

    virtual CAtomicModelInfo* GTACALL				GetAtomicModelInfo() = 0;               // 4
    virtual CDamageAtomicModelInfo* GTACALL			GetDamageAtomicModelInfo() = 0;         // 8
    virtual CLODAtomicModelInfo* GTACALL				GetLODAtomicModelInfo() = 0;            // 12
    virtual eModelType GTACALL                       GetModelType() = 0;                     // 16
    virtual unsigned int GTACALL                     GetTimeInfo() = 0;                      // 20
    virtual void GTACALL                             Init() = 0;                             // 24
    virtual void GTACALL                             Shutdown() = 0;                         // 28
    virtual void GTACALL                             DeleteRwObject() = 0;                   // 32
    virtual eRwType GTACALL                          GetRwModelType() = 0;                   // 36
    virtual RwObject* GTACALL                        CreateRwObjectEx( int rwTag ) = 0;      // 40
    virtual RwObject* GTACALL                        CreateRwObject() = 0;                   // 44
    virtual void GTACALL                             SetAnimFile( const char *name ) = 0;    // 48
    virtual void GTACALL                             ConvertAnimFileIndex() = 0;             // 52
    virtual int GTACALL                              GetAnimFileIndex() = 0;  
};

class CModelInfo : public CBaseModelInfo
{
public:
	void					ResetID();
	bool					SetByName(char *pName);

	char					m_cModelName[24];		// +4
	CModelDimension*		m_pDimension;			// +28
	BYTE					pad[8];					// +32
	unsigned short			m_usInternalID;			// +40
	BYTE					pad4[4];				// +42
	char					m_cModelType;			// +46
	BYTE					pad3[7];				// +47
	char					m_cInstanceName[9];		// +54
	BYTE					pad2[26];				// +86
	int						m_iUnknown;				// +89
};

enum eModelLoadState	// I have to investigate this further, what numbers can be here
{
	MODEL_NOT_LOADED,
	MODEL_LOADED,
	MODEL_LOADING
};

class CModelLoadInfo
{
public:
	void					SetValues(int iUnk1, int iUnk2);

	char					eLoadedState;
	char					m_cUnk;
	short					m_sUnk2;
	int						m_iUnknownID;
	int						m_iUnknownID2;
	BYTE					pad[8];
};

class CSpecialPedModelInfo
{
public:
	int						m_iUnknownID;
	int						m_iUnknownID2;
	char					m_modelType[24];
};

class CSpecialPedModelPool
{
public:
	bool					GetValues(char *pModelName, int *pValue1, int *pValue2);

	CSpecialPedModelInfo*	m_pData;
	unsigned int			m_uiUnk;
	unsigned int			m_uiNumEntries;
};

#define m_pSpecialPedModelPool	(*(CSpecialPedModelPool**)(MEM_pSpecialPedModelInfo))

// Functions
void			Model_Init ();
CModelInfo*		Model_GetEntry ( unsigned short usModelID );
CModelLoadInfo*	Model_GetReference ( unsigned short usModelID );
bool			Model_Rename ( unsigned short usModelID, char *pName );
bool			Model_IsLoaded ( unsigned short usModelID );
bool			Model_RequestInternal ( unsigned short usModelID );
bool			Model_RequestInternal ( unsigned short usModelID, unsigned int uiRequestID );
bool			Model_RequestSpecialInternal ( unsigned short usModelID, char *pSpecialName, char cRequestType );
void			Model_LoadRequestedModels ();
#if 0
bool			Model_Request ( unsigned short usModelID, CEntityMP *pEntity, void (*pCallback)(CEntityMP *pEntity) );
#endif
void			Model_Free ( unsigned short usModelID );
void			Model_ManagerFrame ();

#endif //_MODELINFO_