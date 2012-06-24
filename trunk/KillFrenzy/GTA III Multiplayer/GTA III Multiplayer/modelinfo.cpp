#include "StdInc.h"
CModelLoadInfoEx	m_modelRequests[MODELINFO_MAXENTRIES];

/*==========================
	Model functions
==========================*/

// Init environment
void			Model_Init ()
{
	//m_uiRequests=0;
	memset(m_modelRequests, 0, sizeof(m_modelRequests));
}

// Retrives a modelinterface by id
CModelInfo*		Model_GetEntry ( unsigned short usModelID )
{
	CModelInfo* pModelInfo;

	if (usModelID > MODELINFO_MAXENTRIES)
		return NULL;

	pModelInfo = *(CModelInfo**)(MEM_Table_pModelInfo+usModelID*sizeof(void*));

	return pModelInfo;
}

// Retrives a modelloadinterface by id
CModelLoadInfo*	Model_GetReference ( unsigned short usModelID )
{
	if (usModelID>MODELINFO_MAXENTRIES)
		return NULL;
	return (CModelLoadInfo*)MEM_Table_modelLoadInfo + usModelID;
}

// Rename a entry
bool			Model_Rename ( unsigned short usModelID, char *pName )
{
	CModelInfo *pModelInfo=Model_GetEntry(usModelID);

	if (!pModelInfo)
		return false;

	strncpy(pModelInfo->m_cModelName, (const char*)pName, 20);
	return true;
}

// Is model loaded?
bool			Model_IsLoaded ( unsigned short usModelID )
{
	CModelLoadInfo* pLoadInfo = (CModelLoadInfo*)MEM_Table_modelLoadInfo + usModelID;

	if (usModelID > MODELINFO_MAXENTRIES)
		return false;

	if (pLoadInfo->eLoadedState==MODEL_LOADED)
		return true;

	return false;
}

bool			Model_RequestInternal ( unsigned short usModelID )
{
	DWORD dwFunc=FUNC_RequestModel;
	DWORD modelID=usModelID;
	__asm
	{
		push 0
		push modelID
		call dwFunc
		add esp,8
	}
	return true;
}

bool			Model_RequestInternal ( unsigned short usModelID, unsigned int uiRequestID )
{
	DWORD dwFunc=FUNC_RequestModel;
	DWORD modelID=usModelID;
	__asm
	{
		push [uiRequestID]
		push modelID
		call dwFunc
		add esp,8
	}
	return true;
}

bool			Model_RequestSpecialInternal ( unsigned short usModelID, char *pSpecialName, char cRequestType )
{
	CModelLoadInfo *pLoadInfo = Model_GetReference(usModelID);
	CModelInfo *pModelInfo = Model_GetEntry(usModelID);

	if (usModelID>MODELINFO_MAXENTRIES-1)
		return false;

	if (strcmp(pModelInfo->m_cModelName, pSpecialName)==0)
	{
		Model_RequestInternal(usModelID, cRequestType);
		return true;
	}

	if (!m_pSpecialPedModelPool->GetValues(pSpecialName, &pLoadInfo->m_iUnknownID, &pLoadInfo->m_iUnknownID2))
		return false;

	if (!pModelInfo->SetByName(pSpecialName))
		return false;

	Model_Rename(usModelID, pSpecialName);
	Model_Free(usModelID);

	Model_RequestInternal(usModelID, cRequestType);
	return true;
}

void			Model_LoadRequestedModels ()
{
	DWORD dwFunc=FUNC_LoadRequestedModels;
	__asm
	{
		push 0
		call dwFunc
		add esp,4
	}
}

#if 0
// Request a model to be loaded, enchanced
bool			Model_Request ( unsigned short usModelID, CEntityMP *pEntity, void (*pCallback)(CEntityMP *pEntity) )
{
	unsigned int refID;

	if (Model_IsLoaded(usModelID))
	{
		pCallback(pEntity);
		return true;
	}

	// Give the engine a notice that we need the model
	Model_RequestInternal ( usModelID );

	// Fill out request structs
	refID=m_modelRequests[usModelID].m_uiRefs;
	assert( refID != MAX_MODEL_REQUEST_QUEUE );
	m_modelRequests[usModelID].m_ppOnLoad[refID]=pCallback;
	m_modelRequests[usModelID].m_ppEntities[refID]=pEntity;
	m_modelRequests[usModelID].m_uiRefs++;
	return true;
}
#endif

// Free a model
void			Model_Free ( unsigned short usModelID )
{
	DWORD dwFreeModel=FUNC_FreeModel;
	CModelLoadInfo *pInfo = Model_GetReference(usModelID);

	if (pInfo->eLoadedState != MODEL_LOADED && pInfo->eLoadedState != MODEL_LOADING)
		return;

#if 0
	// Clear request structs
	m_modelRequests[usModelID].m_uiRefs=0;
	//todo: Maybe install a handler for objects which dont load?
#endif

	__asm
	{
		lea eax,usModelID
		push [eax]
		call dwFreeModel
		pop ecx
	}
}

// Handle our model requests
void			Model_ManagerFrame ()
{
#if 0
	unsigned int n;
	bool bNeedLoad=false;

	for (n=0; n<MODELINFO_MAXENTRIES; n++)
	{
		// Check whether we need a model to load or unload
		CModelLoadInfoEx *pEntry=&m_modelRequests[n];
		//CModelLoadInfo *pInternalEntry=(CModelLoadInfo*)(MEM_Table_modelLoadInfo + sizeof(CModelLoadInfo)*n);

		if (pEntry->m_uiRefs)
		{
			if (Model_IsLoaded(n)==true)
			{
				unsigned int i;

				for (i=0; i<pEntry->m_uiRefs; i++)
					pEntry->m_ppOnLoad[i](pEntry->m_ppEntities[i]);
				pEntry->m_uiRefs=0;
			}
			else
			{
				bNeedLoad=true;
				Model_RequestInternal(n);
			}
		}
	}
	if (bNeedLoad)
		Model_LoadRequestedModels();
#endif
}

/*========================================
	ModelInfo Class
========================================*/

void	CModelInfo::ResetID ()
{
	m_usInternalID |= 0xFFFF;
}

bool	CModelInfo::SetByName (char *pName)
{
	DWORD dwFindModel=FUNC_FindModel;
	DWORD dwPushModel=FUNC_PushModelName;
	unsigned short usID;

	__asm
	{
		lea eax,pName
		push [eax]
		call dwFindModel
		add esp,4
		mov usID,ax
	}
	if (usID!=0xFFFF)
		m_usInternalID=usID;
	else
	{
		__asm
		{
			lea eax,pName
			push [eax]
			call dwPushModel
			add esp,4
		}
	}
}

/*=========================================
	Model Load Info Class
=========================================*/

void	CModelLoadInfo::SetValues(int iUnk1, int iUnk2)
{
	m_iUnknownID = iUnk1;
	m_iUnknownID2 = iUnk2;
}

/*=========================================
	Special Ped Models

	Alright, the core seems to have a special memory allocation for special ped models.
	To load these models you also have to init specific values into the CModelLoadInfo class.
	I have no idea what these are, yet, but I just recode how the engine behaves,
	and that is loading specific data from a internal table.
=========================================*/

bool	CSpecialPedModelPool::GetValues(char *pModelName, int *pValue1, int *pValue2)
{
	unsigned int n;

	for (n=0; n<m_uiNumEntries; n++)
	{
		CSpecialPedModelInfo *pInfo = m_pData + n;

		if (strcmp(pInfo->m_modelType, pModelName)==0)
		{
			*pValue1 = pInfo->m_iUnknownID;
			*pValue2 = pInfo->m_iUnknownID2;
			return true;
		}
	}
	return false;
}