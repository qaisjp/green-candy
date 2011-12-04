/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CModuleLoader.h
*  PURPOSE:     Header file for module loader class
*  DEVELOPERS:  Derek Abdine <>
*               Alberto Alonso <rydencillo@gmail.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CMODULELOADER_H
#define __CMODULELOADER_H

class CModuleLoader
{
public:
                    CModuleLoader       ( const filePath& ModuleName );
                    CModuleLoader       ();
                    ~CModuleLoader      (); 

    bool            LoadModule          ( const filePath& ModuleName ); 
    void*           GetFunctionPointer  ( const std::string& FunctionName );
    void            UnloadModule        ();

private:
    HMODULE     m_hLoadedModule;
    SString     m_strLastError;
};

#endif