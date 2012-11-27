/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CIMGManagerSA.h
*  PURPOSE:     Image accelerated archive management
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*  INSPIRATION: fastman92 <http://fastman92.tk>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _IMAGE_MANAGER_SA_
#define _IMAGE_MANAGER_SA_

void IMG_Initialize();
void IMG_Shutdown();

enum eLoadingState
{
    MODEL_UNAVAILABLE,
    MODEL_LOADED,
    MODEL_LOADING,
    MODEL_LOD,    // Perhaps
    MODEL_RELOAD
};

class CModelLoadInfoSA  // size: 20
{
public:
    unsigned short  m_lodModelID;       // 0
    unsigned short  m_unknown4;         // 2
    unsigned short  m_lastID;           // 4
    unsigned char   m_flags;            // 6
    unsigned char   m_imgID;            // 7
    unsigned int    m_blockOffset;      // 8
    unsigned int    m_blockCount;       // 12
    eLoadingState   m_eLoading;         // 16

    bool __thiscall             GetOffset( unsigned int& offset, unsigned int& blockCount );
    void __thiscall             SetOffset( unsigned int offset, unsigned int blockCount );
    unsigned int __thiscall     GetStreamOffset() const;
};

#define MAX_GTA_IMG_ARCHIVES    512

struct IMGFile
{
    char            name[40];               // 0
    bool            isNotPlayerImg;         // 40
    // ALIGNMENT: 3bytes
    unsigned int    handle;                 // 44
};

extern IMGFile imgArchives[MAX_GTA_IMG_ARCHIVES];

#endif //_IMAGE_MANAGER_SA_