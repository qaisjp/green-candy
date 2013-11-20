/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CFileUtilsSA.cpp
*  PURPOSE:     File parsing utilities used by R* Games
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

/*=========================================================
    FileMgr::GetConfigLine

    Arguments:
        file - source stream to read from
    Purpose:
        Reads a line from the stream character by character
        and parses it for sscanf compatibility. Returns the
        string it processed. If there was nothing to read,
        it returns NULL.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00536F80
=========================================================*/
static char tempBuffer[512];

const char* FileMgr::GetConfigLine( CFile *file )
{
    if ( !file->GetString( tempBuffer, sizeof( tempBuffer ) ) )
        return NULL;
    
    char *buf = tempBuffer;

    for ( buf = tempBuffer; *buf; buf++ )
    {
        const char c = *buf;

        if ( c < ' ' || c == ',' )
            *buf = ' ';
    }
    
    // Offset until we start at valid offset
    for ( buf = tempBuffer; *buf && *buf <= ' '; buf++ );

    return buf;
}