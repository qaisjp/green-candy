/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CCommandFile.h
*  PURPOSE:     Command file parser class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CCOMMANDFILE_H
#define __CCOMMANDFILE_H

#include "stdio.h"
class CConsole;
class CClient;

class CCommandFile
{
public:
                        CCommandFile            ( const char* szFilename, CConsole& Console, CClient& Client );
                        ~CCommandFile           ();

    bool                Run                     ();

private:
    bool                Parse                   ( char* szLine );

    static char*        SkipWhitespace          ( char* szLine );
    static void         TrimRightWhitespace     ( char* szLine );
    static inline bool  IsWhitespace            ( char cChar )      { return cChar == 32 ||
                                                                             cChar == 9 ||
                                                                             cChar == 10 ||
                                                                             cChar == 13; };

    CConsole&           m_Console;
    CClient&            m_Client;

    CFile*              m_file;
    bool                m_bEcho;
};

#endif
