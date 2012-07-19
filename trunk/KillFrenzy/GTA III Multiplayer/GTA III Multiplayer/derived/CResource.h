/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CResource.h
*  PURPOSE:     Lua Resource environment
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RESOURCE_
#define _RESOURCE_

class CResource : public Resource
{
public:
                               CResource( LuaMain& main, unsigned short id, const filePath& name, CFileTranslator& root );
                               ~CResource();


};

#endif //_RESOURCE_