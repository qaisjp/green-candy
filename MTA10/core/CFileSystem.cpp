/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CFileSystem.cpp
*  PURPOSE:     File management
*  DEVELOPERS:  S2Games <http://savage.s2games.com> (historical entry)
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

#include "unzip.h"
#include <list>

using namespace std;

list<CFile*>*		openFiles;
CSystemFileTranslator*	gameTranslator;
CSystemFileTranslator*	systemTranslator;

extern CFile *filenamelist_file;

#define MAX_ARCHIVE_FILES 100
#define MAX_CLAN_ICONS 100

list<CArchiveFileTranslator*>*	registeredArchives;

#ifdef SHOW_MISSING_FILES
file_t *MISSINGFILES = NULL;
#endif

GHashTable *clanHash = NULL;

extern cvar_t clan_icon_url;
extern cvar_t world_save_path;
extern cvar_t svr_pure;
extern cvar_t homedir;
extern cvar_t archive_precedence;
extern cvar_t sys_enumdir;

extern unsigned char dll_hash[MAX_HASH_SIZE];
extern int dll_hashlen;
extern unsigned char engine_hash[MAX_HASH_SIZE];
extern int engine_hashlen;

int	numOpenFiles = 0;

void	File_AddOpenFileTracking(CFile *pFile)
{
	openFiles->push_back(pFile);
}

void	File_DelOpenFileTracking(CFile *pFile)
{
	openFiles->remove(pFile);
}

/*==============================================================
	Archive Filesystem Functions

	These functions shall never be given for a mod to control.
	The core engine has to handle all loading shematics, so the mod can use the given space to initialize itself.
===============================================================*/

void	Archive_ScanDirectory(char *pDirectory, char *pWildcard, bool bRecurse, 
						void (*dirCallback)(const char *pDirectory, void *pUserdata), 
						void (*fileCallback)(const char *pFilename, void *pUserdata), 
						void *pUserdata)
{
	list <CArchiveFileTranslator*>::iterator iter = registeredArchives->begin();

	for (iter; iter != registeredArchives->end(); iter++)
	{
		(*iter)->ScanDirectory(pDirectory, pWildcard, bRecurse, dirCallback, fileCallback, pUserdata);
	}
}

bool	Archive_StatFile(const char *filename, struct stat *stats)
{
	list <CArchiveFileTranslator*>::iterator iter = registeredArchives->begin();

	for (iter; iter != registeredArchives->end(); iter++)
		if ((*iter)->Stat(filename, stats))
			return true;

	return false;
}

// Gets an archive by a absolute path
CArchiveFileTranslator*	Archive_GetArchive(const char *pPath)
{
	list<CArchiveFileTranslator*>::iterator iter = registeredArchives->begin();
	char pathBuffer[1024];

	for (iter; iter != registeredArchives->end(); iter++)
	{
		(*iter)->GetDirectory(pathBuffer, 1024);

		if (strcmp(pathBuffer, pPath) == 0)
			return *iter;
	}
	return NULL;
}

#ifdef SAVAGE_DEMO

#define SAVAGE_DEMO_CORRUPT_INSTALL_MSG "Your Savage demo installation is corrupt.  Please reinstall the Savage demo."
#define NUM_DEMO_ARCHIVES 5		//be sure to modify this whenever the demo install changes

bool	Archive_DemoHashCompare(archive_t *archive, const char *filename, const char *hashstring)
{
	char *archname = Filename_GetFilename(archive->filename);

	if (strcmp(filename, archname)!=0)
		return false;

	if (strcmp(hashstring, BinaryToHexWithXor(archive->hash, archive->hashlen, 0))!=0)
	{
		//Console_Printf("The hash of %s should be %s\n", filename, BinaryToHexWithXor(archive->hash, archive->hashlen, 0));
		return false;
	}

	return true;
}

#endif

int		Archive_UnregisterUnusedArchives()
{
	list <CArchiveFileTranslator*>::iterator iter = registeredArchives->begin();
	int count = 0;

	while (iter != registeredArchives->end())
	{
		if (!((*iter)->GetFlags() & ARCHIVE_THIS_CONNECTION_ONLY))
		{
			iter++;
			continue;
		}
		
		(*iter++)->Delete();
	}
	return count;
}

void	Archives_List_Cmd(int argc, char *argv[])
{
	list <CArchiveFileTranslator*>::iterator iter = registeredArchives->begin();
	char pathBuffer[1024];

	Console_Printf("Listing open archives, in order or priority (first = highest priority)\n");

	for (iter; iter != registeredArchives->end(); iter++)
	{
		(*iter)->GetDirectory(pathBuffer, 1024);

		Console_Printf(" %s\n", pathBuffer);
	}
}

CFile* Archive_OpenFile(const char *filename, const char *mode)
{
	list <CArchiveFileTranslator*>::iterator iter = registeredArchives->begin();
	CFile *pFile;

	for (iter; iter != registeredArchives->end(); iter++)
	{
		if ((pFile = (*iter)->Open(filename, mode)))
		{
			char pathBuffer[1024];

			(*iter)->GetDirectory(pathBuffer, 1024);

			Console_DPrintf("Opening '%s' in archive '%s'\n", filename, pathBuffer);
			return pFile;
		}
	}
	return NULL;
}

bool	Archives_FileExists(const char *filename)
{
	list <CArchiveFileTranslator*>::iterator iter = registeredArchives->begin();

	for (iter; iter != registeredArchives->end(); iter++)
	{
		if ((*iter)->Exists(filename))
			return true;
	}
	return false;
}

CArchiveFileTranslator*	Archives_FindPath(const char *pFilename)
{
	list <CArchiveFileTranslator*>::iterator iter = registeredArchives->begin();

	for (iter; iter != registeredArchives->end(); iter++)
		if ((*iter)->Exists(pFilename))
			return *iter;
	
	return NULL;
}

void	File_ScanDirectory(char *pDirectory, char *pWildcard, bool bRecurse,
					   void (*dirCallback)(const char *pDirectory, void *pUserdata),
					   void (*fileCallback)(const char *pFilename, void *pUserdata),
					   void *pUserdata)
{
	gameTranslator->ScanDirectory(pDirectory, pWildcard, bRecurse, dirCallback, fileCallback, pUserdata);
}

void	File_ListOpenFiles_Cmd(int argc, char *argv[])
{
	list<CFile*>::iterator iter = openFiles->begin();

	for (iter; iter != openFiles->end(); iter++)
		Console_Printf("%s\n", (*iter)->GetPath());
}

#ifndef DEDICATED_SERVER

void	File_FlushClanIcon(int clan_id)
{
	g_hash_table_remove(clanHash, &clan_id);
}

void	File_CacheClanIcon(int clan_id)
{
	residx_t res;
	bitmap_t image;
	CFile *f;
	int *ptr;

	if (File_GetClanIcon(clan_id) > 0)
		return;
	
	f = HTTP_OpenFileNonBlocking(fmt("%s%i.png", clan_icon_url.string, clan_id));

	if (!f)
		return; //don't have it yet

	else if (f->IsEOF()) //is it empty (i.e. the request failed?)
	{
		f->Close();
		Console_Printf("Couldn't load clan icon for clan %i\n", clan_id);

		ptr = (int*)Tag_Malloc(sizeof(int), MEM_CLANICONS);
		*ptr = clan_id;

		g_hash_table_insert(clanHash, ptr, GUINT_TO_POINTER(1));
		return;
	}
	
	if (Bitmap_LoadPNGFile(f, &image))
	{
		res = Res_LoadRawTextureFromMemoryEx(0, &image, SHD_FULL_QUALITY | SHD_NO_MIPMAPS | SHD_NO_COMPRESS);
		ptr = (int*)Tag_Malloc(sizeof(int), MEM_CLANICONS);
		*ptr = clan_id;
		g_hash_table_insert(clanHash, ptr, GUINT_TO_POINTER(res));

		if (!g_hash_table_lookup(clanHash, &clan_id))
			Console_Printf("hash table insert failed!\n");
	}
	else
	{
		ptr = (int*)Tag_Malloc(sizeof(int), MEM_CLANICONS);
		*ptr = clan_id;

		g_hash_table_insert(clanHash, ptr, GUINT_TO_POINTER(1));
	}

	f->Close();
}

residx_t	File_GetClanIcon(int clan_id)
{
	return GPOINTER_TO_UINT(g_hash_table_lookup(clanHash, &clan_id));
}

void	File_ClanIconInit()
{
	if (clanHash)
	{
		g_hash_table_destroy(clanHash);
	}
	Tag_FreeAll(MEM_CLANICONS);
	clanHash = g_hash_table_new(g_int_hash, g_int_equal);
}

#endif //DEDICATED_SERVER

void	File_GetHashStrings(char *hashstring, int size, int xorbits)
{
	char *publicHash;	
	list <CArchiveFileTranslator*>::iterator iter = registeredArchives->begin();

	for (iter; iter != registeredArchives->end(); iter++)
	{
		if (((*iter)->GetFlags() & ARCHIVE_OFFICIAL) || ((*iter)->GetFlags() & ARCHIVE_THIS_CONNECTION_ONLY))
		{
			char hash[MAX_HASH_SIZE];
			int iHashLength;
			char pathBuffer[1024];

			(*iter)->GetHash(hash, &iHashLength);
			(*iter)->GetDirectory(pathBuffer, 1024);

			publicHash = BinaryToHexWithXor(hash, iHashLength, xorbits);
			ST_SetState(hashstring, Filename_GetFilename(pathBuffer), publicHash, size);
			//Console_Printf("Sending archive hash %s\n", publicHash);
		}
	}

	//also include the game dll hash
	publicHash = BinaryToHexWithXor((char*)dll_hash, dll_hashlen, xorbits);
#ifdef _WIN32
	ST_SetState(hashstring, "dll", publicHash, size);
#else
	ST_SetState(hashstring, "so", publicHash, size);
#endif
	//Console_Printf("Sending dll hash %s\n", publicHash);

	//also include the engine hash
	publicHash = BinaryToHexWithXor((char*)engine_hash, engine_hashlen, xorbits);
#ifdef _WIN32
	ST_SetState(hashstring, "win32engine", publicHash, size);
#else
	ST_SetState(hashstring, "linuxengine", publicHash, size);
#endif
	//Console_Printf("Sending engine hash %s\n", publicHash);
}

bool	File_DLLHashOkay(const char *filename, const char *hash)
{
	CFile *pFile;
	char line[512];
	char *marker;

	pFile = File_Open("hashes.txt", "r");
	if (!pFile)
		return true; //why bother with pure when we have no hashes?

	while (pFile->GetString(line, 512))
	{
		if ((marker = strchr(line, ':')))
		{
			marker[0] = 0;
			marker++;
			if (stricmp(filename, line) == 0)
			{
				Console_Printf("Comparing the file hash %s with %s\n", marker, hash);
				if (strcmp(hash, marker) == 0)
				{
					Console_Printf("Match!\n");
					pFile->Close();
					return true;
				}
			}
		}
	}
	
	pFile->Close();
	return false;
}

bool    File_CompareHashStrings(char *hashString, int xorbits)
{
	int len;
	int numClient, numServer = 0;
	char binaryHash[MAX_HASH_SIZE+1];
	char *hash;
	const char *s;
	bool success = true;
	list <CArchiveFileTranslator*>::iterator iter;
		
	Console_Printf("the client's xorbits are %i\n", xorbits);
	
	numClient = ST_ForeachState(hashString, NULL);

	hash = ST_GetState(hashString, "dll");
	if (hash[0])
	{
		len = HexToBinary(hash, binaryHash, MAX_HASH_SIZE);
		hash = BinaryToHexWithXor(binaryHash, len, xorbits);
		if (!File_DLLHashOkay("dll", hash))
		{
			Console_Printf(".dll hash (%s) doesn't match acceptable list\n", hash);
			if (svr_pure.integer > 1)
				success = false;
		}
		numServer++;
	}
	
	hash = ST_GetState(hashString, "so");
	if (hash[0])
	{
		len = HexToBinary(hash, binaryHash, MAX_HASH_SIZE);
		hash = BinaryToHexWithXor(binaryHash, len, xorbits);
		if (!File_DLLHashOkay("so", hash))
		{
			Console_Printf(".so hash (%s) doesn't match acceptable list\n", hash);
			if (svr_pure.integer > 1)
				success = false;
		}
		numServer++;
	}
	
	hash = ST_GetState(hashString, "win32engine");
	if (hash[0])
	{
		len = HexToBinary(hash, binaryHash, MAX_HASH_SIZE);
		hash = BinaryToHexWithXor(binaryHash, len, xorbits);
		if (!File_DLLHashOkay("win32engine", hash))
		{
			Console_Printf("win32 binary hash (%s) doesn't match acceptable list\n", hash);
			if (svr_pure.integer > 1)
				success = false;
		}
		numServer++;
	}
	
	hash = ST_GetState(hashString, "linuxengine");
	if (hash[0])
	{
		len = HexToBinary(hash, binaryHash, MAX_HASH_SIZE);
		hash = BinaryToHexWithXor(binaryHash, len, xorbits);
		if (!File_DLLHashOkay("linuxengine", hash))
		{
			Console_Printf("linux binary hash (%s) doesn't match acceptable list\n", hash);
			if (svr_pure.integer > 1)
				success = false;
		}
		numServer++;
	}

	iter = registeredArchives->begin();
	
	for (iter; iter != registeredArchives->end(); iter++)
	{
		if (((*iter)->GetFlags() & ARCHIVE_OFFICIAL))
		{
			char pathBuffer[1024];

			(*iter)->GetDirectory(pathBuffer, 1024);

			s = Filename_GetFilename(pathBuffer);
			hash = ST_GetState(hashString, s);

			len = HexToBinary(hash, binaryHash, MAX_HASH_SIZE);
			hash = BinaryToHexWithXor(binaryHash, len, xorbits);

			if (!File_DLLHashOkay(s, hash))
			{
				Console_Printf("The client file %s differs from ours (they have hash %s)\n", Filename_GetFilename(pathBuffer), hash);
				success = false; //client doesn't match server
			}
			numServer++;
		}
	}
		
	if (numClient != numServer)
	{
		if (numClient > numServer)
			Console_Printf("the client has %i more files than the server\n", numClient - numServer);
		else
			Console_Printf("the client has %i less files than the server\n", numServer - numClient);

		success = false; //client has more files than the server
	}
		
	return success;
}

void	Archive_RegisterOfficialArchives()
{
	unsigned int i;
	char nameBuffer[1024];
	char pathBuffer[1024];

	gameTranslator->GetDirectory(pathBuffer, 1024, true);

	i = 0;
	while (i < MAX_ARCHIVE_FILES)
	{
		snprintf(nameBuffer, 1023, "%sopt%u.s2z", pathBuffer, i);

		if (!File_Exists(nameBuffer))
			break;

		Archive_CreateTranslator(nameBuffer, ACCESS_GAME, ARCHIVE_OFFICIAL);
		i++;
	}

	i = 0;
	while (i < MAX_ARCHIVE_FILES)
	{
		snprintf(nameBuffer, 1023, "%ssounds%u.s2z", pathBuffer, i);

		if (!File_Exists(nameBuffer))
			break;

		Archive_CreateTranslator(nameBuffer, ACCESS_GAME, ARCHIVE_NORMAL);  //don't make these official so people can modify them
		i++;
	}
	
	i = 0;
	while (i < MAX_ARCHIVE_FILES)
	{
		snprintf(nameBuffer, 1023, "%sgui%u.s2z", pathBuffer, i);

		if (!File_Exists(nameBuffer))
			break;

		Archive_CreateTranslator(nameBuffer, ACCESS_GAME, ARCHIVE_NORMAL); //don't make these official so people can modify them
		i++;
	}

	i = 0;
	while (i < MAX_ARCHIVE_FILES)
	{
		snprintf(nameBuffer, 1023, "%simages%u.s2z", pathBuffer, i);

		if (!File_Exists(nameBuffer))
			break;

		Archive_CreateTranslator(nameBuffer, ACCESS_GAME, ARCHIVE_OFFICIAL);
		i++;
	}

	i = 0;
	while (i < MAX_ARCHIVE_FILES)
	{
		snprintf(nameBuffer, 1023, "%ssavage%u.s2z", pathBuffer, i);

		if (!File_Exists(nameBuffer))
			break;

		Archive_CreateTranslator(nameBuffer, ACCESS_GAME, ARCHIVE_OFFICIAL);
		i++;
	}

#ifdef SAVAGE_DEMO
	{
		int num_archives;
		i = 0;
		num_archives = 0;
		while (i < MAX_ARCHIVE_FILES)
		{
			if (archives[i].active)
				num_archives++;
			i++;
		}/*
		if (num_archives != NUM_DEMO_ARCHIVES)
			System_Error(fmt("%s [%i %i]", SAVAGE_DEMO_CORRUPT_INSTALL_MSG, i, num_archives));*/
	}
#endif
}

void	Archive_Init()
{
#ifdef SHOW_MISSING_FILES
	MISSINGFILES = File_Open("missingfiles.txt", "w");
#endif

	registeredArchives = new list<CArchiveFileTranslator*>();
}

void	Archive_Shutdown()
{
	while (!registeredArchives->empty())
		(*registeredArchives->begin())->Delete();
#ifdef SHOW_MISSING_FILES
	File_Close(MISSINGFILES);
	MISSINGFILES = NULL;
#endif
}

void	File_GetFullPath(const char *pPath, char *pBuffer, size_t sMaxBuffer)
{
	gameTranslator->GetFullPath(pPath, pBuffer, sMaxBuffer);
}

extern	scriptBufferEntry_t scriptBuffer[SCRIPT_BUFFER_SIZE];

CFile*	File_CreateBuffered(CFile *pFile)
{
	CBufferedFile *pBufferedFile = new CBufferedFile();
	pFile->Seek(0, SEEK_SET);

	pBufferedFile->m_pBuffer = (char*)Tag_Malloc(pFile->GetSize(), MEM_FILE);
	
	if (!pFile->Read(pBufferedFile->m_pBuffer, pFile->GetSize(), 1))
	{
		Tag_Free(pBufferedFile->m_pBuffer);

		delete pBufferedFile;
		return NULL;
	}

	pBufferedFile->m_sSize = pFile->GetSize();
	pBufferedFile->m_iSeek = 0;
	pBufferedFile->m_pPath = (char*)Tag_Strdup(pFile->GetPath(), MEM_FILE);
	return pBufferedFile;
}

CFile*	File_ImportBuffered(char *pPath, void *pBuffer, size_t sBuffer)
{
	CBufferedFile *pBufferedFile;

	if (!pBuffer || sBuffer == 0)
		return NULL;

	pBufferedFile = new CBufferedFile();

	pBufferedFile->m_pBuffer = (char*)Tag_Malloc(sBuffer, MEM_FILE);
	Mem_Copy(pBufferedFile->m_pBuffer, pBuffer, sBuffer);
	pBufferedFile->m_sSize = sBuffer;
	pBufferedFile->m_iSeek = 0;
	pBufferedFile->m_pPath = (char*)Tag_Strdup(pPath, MEM_FILE);
	return pBufferedFile;
}

/*=====================================================================
 *
 * File_OpenAbsolute
 *
 * Opens a system file.
 * The direct access to this function is not allowed due to security reasons.
 * If you want to pierce the directory, create a new translator with a specified root!
 *
=======================================================================*/

CFile*	File_OpenAbsolute(const char *filename, const char *mode)
{
	void *pSystemFile;
	CRawFile *pFile;
	DWORD dwAccess = 0;
	DWORD dwCreate = 0;
	const char *pModeIter = mode;

	while (*pModeIter)
	{
		switch (*pModeIter)
		{
		case 'w':
			dwCreate = CREATE_ALWAYS;
			dwAccess |= GENERIC_WRITE;
			break;
		case 'r':
			dwCreate = OPEN_EXISTING;
			dwAccess |= GENERIC_READ;
			break;
		case 'a':
			Console_DPrintf("Silverback does not support file append mode, ignoring (%s)\n", filename);
			break;
		case 't':
			Console_DPrintf("Silverback does not support text mode, ignoring (%s)\n", filename);
			break;
		case '+':
			if (dwAccess & GENERIC_READ)
				dwAccess |= GENERIC_WRITE;
			else if (dwAccess & GENERIC_WRITE)
				dwAccess |= GENERIC_READ;
			break;
		default:
			Console_DPrintf("Unknown file mode descriptor '%c' (%s)\n", *pModeIter, filename);
			break;
		}

		pModeIter++;
	}
	if (!dwAccess)
	{
		Console_DPrintf("Opening of file '%s' failed, no mode specified\n", filename);
		return NULL;
	}
	
	if (strlen(filename) == 0 || filename[strlen(filename)-1] == '/')
	{
		Console_DPrintf("weird filename %s, mode %s\n", filename, mode);
	}

	pSystemFile = CreateFile(filename, dwAccess, FILE_SHARE_READ, NULL, dwCreate, 0, NULL);
	
	if (pSystemFile == INVALID_HANDLE_VALUE)
		return NULL;

	pFile = new CRawFile();
	pFile->m_pFile = pSystemFile;
	pFile->m_pMode = mode;
	pFile->m_pPath = (char*)Tag_Malloc(strlen(filename)+1, MEM_FILE);
	strcpy(pFile->m_pPath, filename);

	openFiles->push_back(pFile);
	return pFile;
}

CFile*	File_Open(const char *filename, const char *mode)
{
	return gameTranslator->Open(filename, mode);
}

CFile*	File_OpenBuffered(const char *pFilename, const char *pMode)
{
	CFile *pFile = File_Open(pFilename, pMode);
	CFile *pBufferedFile;

	if (!pFile)
		return NULL;
	pBufferedFile = File_CreateBuffered(pFile);
	pFile->Close();

	return pBufferedFile;
}

bool    File_StatAbsolute(const char *filename, struct stat *stats)
{
	return (stat(filename, stats) == 0);
}

bool    File_Stat(const char *filename, struct stat *stats)
{
	return gameTranslator->Stat(filename, stats);
}

bool	File_Delete(const char *filename)
{
	if (remove(filename) == 0)
		return true;

	return false;
}

bool	File_IsDirectoryAbsolute(const char *pPath)
{
#ifdef WIN32
	DWORD dwAttributes = GetFileAttributes(pPath);

	if (dwAttributes == INVALID_FILE_ATTRIBUTES)
		return false;
	return (dwAttributes & FILE_ATTRIBUTE_DIRECTORY);
#else
	return false;
#endif
}

//current_dir always begins with a / and ends with a /.  the first / denotes the root directory of the game.
void	File_ChangeDir(const char *dir)
{
	OVERHEAD_INIT;

	if (gameTranslator->ChangeDirectory(dir))
	{
		Console_DPrintf("changed to directory %s\n", dir);
	}
	else
	{
		Console_DPrintf("Invalid directory %s\n", dir);
	}

	//Console_Printf("current dir: %s\n", current_dir);
	OVERHEAD_COUNT(OVERHEAD_IO);
}

void	File_GetCurrentDir(char *pBuffer, size_t sMaxBuffer)
{
	gameTranslator->GetDirectory(pBuffer, sMaxBuffer, false);
}

bool	_File_ParseRelativePath(const char *pPath, char dirTree[][MAX_PATH], unsigned int uiMaxTreeDepth, unsigned int *uiTreeDepth, bool *bFile)
{
	size_t sPathCursor=0;

	if (uiMaxTreeDepth == 0)
		return false;

	*uiTreeDepth = 0;

	while (*pPath)
	{
		switch (*pPath)
		{
		case '\\':
		case '/':
			if (sPathCursor == 0)
				break;

			if (sPathCursor == 1 && dirTree[*uiTreeDepth][0] == '.')
			{
				// We ignore this current path indicator
				sPathCursor = 0;
				break;
			}
			else if (sPathCursor == 2 && dirTree[*uiTreeDepth][0] == '.' && dirTree[*uiTreeDepth][1] == '.')
			{
				if (*uiTreeDepth == 0)
					return false;	// We cannot go deeper than the root

				(*uiTreeDepth)--;
				sPathCursor = 0;
				break;
			}

			dirTree[*uiTreeDepth][sPathCursor] = 0;

			(*uiTreeDepth)++;
			sPathCursor = 0;
			break;
		case ':':
			if (sPathCursor == 1 && *uiTreeDepth == 0)
				return false;	// Drive root does not work out
			return false;
#ifdef _WIN32
		case '<':
		case '>':
		case '"':
		case '|':
		case '?':
		case '*':
			return false;
#endif
		default:
			if (sPathCursor == MAX_PATH)
				return false;
			if (*uiTreeDepth == uiMaxTreeDepth)
				return false;

			dirTree[*uiTreeDepth][sPathCursor++] = *pPath;
			break;
		}

		pPath++;
	}

	if (sPathCursor != 0)
	{
		dirTree[*uiTreeDepth][sPathCursor] = 0;
		(*uiTreeDepth)++;

		*bFile = true;
	}
	else
		*bFile = false;

	return true;
}

// Parse a path so it will not turn out illegal
bool	File_ParsePath(const char *pPath, char *pBuffer, size_t sMaxBuffer)
{
	unsigned int n;
	unsigned int uiTreeDepth;
	char dirTree[32][MAX_PATH];
	bool bFile;

	if (sMaxBuffer == 0)
		return false;

	// We have to handle absolute path, too
	if (strlen(pPath) > 2 && pPath[1] == ':')
	{
		if (sMaxBuffer < 4)
			return false;

		if (!_File_ParseRelativePath(pPath + 3, dirTree, 32, &uiTreeDepth, &bFile))
			return false;

		pBuffer[0] = pPath[0];
		pBuffer[1] = ':';
		pBuffer[2] = '/';
		pBuffer[3] = 0;
	}
	else
	{
		if (!_File_ParseRelativePath(pPath, dirTree, 32, &uiTreeDepth, &bFile))
			return false;

		pBuffer[0] = 0;
	}

	if (bFile)
	{
		for (n=0; n<uiTreeDepth-1; n++)
			strncat(pBuffer, fmt("%s/", dirTree[n]), sMaxBuffer);

		strncat(pBuffer, dirTree[uiTreeDepth-1], sMaxBuffer);
	}
	else
	{
		if (uiTreeDepth == 0)
		{
			pBuffer[0] = 0;
			return true;
		}

		for (n=0; n<uiTreeDepth; n++)
			strncat(pBuffer, fmt("%s/", dirTree[n]), sMaxBuffer);
	}
	return true;
}

// Get the directory correctly parsed
bool	File_ParseDirectory(const char *pPath, char *pBuffer, size_t sMaxBuffer)
{
	unsigned int n;
	unsigned int uiTreeDepth;
	char dirTree[32][MAX_PATH];
	bool bFile;

	if (sMaxBuffer == 0)
		return false;

	// We have to handle absolute path, too
	if (strlen(pPath) > 2 && pPath[1] == ':')
	{
		if (sMaxBuffer < 4)
			return false;

		if (!_File_ParseRelativePath(pPath + 3, dirTree, 32, &uiTreeDepth, &bFile))
			return false;

		pBuffer[0] = pPath[0];
		pBuffer[1] = ':';
		pBuffer[2] = '/';
		pBuffer[3] = 0;
	}
	else
	{
		if (!_File_ParseRelativePath(pPath, dirTree, 32, &uiTreeDepth, &bFile))
			return false;

		pBuffer[0] = 0;
	}

	if (bFile)
	{
		if (uiTreeDepth == 1)
		{
			pBuffer[0] = 0;
			return true;
		}

		uiTreeDepth--;
	}
	else if (uiTreeDepth == 0)
	{
		pBuffer[0] = 0;
		return true;
	}

	for (n=0; n<uiTreeDepth; n++)
		strncat(pBuffer, fmt("%s/", dirTree[n]), sMaxBuffer);

	return true;
}

// Try to create a relative version of a path
bool	_File_ParsePathToRelative(const char *pPath, const char rootTree[][MAX_PATH], unsigned int uiRootTreeDepth, char *pBuffer, size_t sMaxBuffer)
{
	unsigned int uiTreeDepth;
	char dirTree[32][MAX_PATH];
	bool bFile;
	unsigned int n;

	if (sMaxBuffer == 0)
		return false;

	if (!_File_ParseRelativePath(pPath + 3, dirTree, 32, &uiTreeDepth, &bFile))
		return false;

	if (uiTreeDepth <= uiRootTreeDepth)
		return false;

	for (n=0; n<uiRootTreeDepth; n++)
	{
		if (strcmp(rootTree[n], dirTree[n]) != 0)
			return false;
	}
	pBuffer[0] = 0;

	if (bFile)
	{
		for (n; n<uiTreeDepth-1; n++)
			strncat(pBuffer, fmt("%s/", dirTree[n]), sMaxBuffer);

		strncat(pBuffer, dirTree[n], sMaxBuffer);
	}
	else
	{
		for (n; n<uiTreeDepth; n++)
			strncat(pBuffer, fmt("%s/", dirTree[n]), sMaxBuffer);
	}
	return true;
}

// Create a relative directory path
bool	_File_ParseDirectoryToRelative(const char *pPath, const char rootTree[][MAX_PATH], unsigned int uiRootTreeDepth, char *pBuffer, size_t sMaxBuffer)
{
	unsigned int uiTreeDepth;
	char dirTree[32][MAX_PATH];
	bool bFile;
	unsigned int n;

	if (sMaxBuffer == 0)
		return false;

	if (!_File_ParseRelativePath(pPath + 3, dirTree, 32, &uiTreeDepth, &bFile))
		return false;

	if (uiTreeDepth < uiRootTreeDepth)
		return false;

	for (n=0; n<uiRootTreeDepth; n++)
	{
		if (strcmp(rootTree[n], dirTree[n]) != 0)
			return false;
	}
	pBuffer[0] = 0;

	if (bFile)
	{
		if (uiTreeDepth - uiRootTreeDepth < 2)
		{
			pBuffer[0] = 0;
			return true;
		}

		uiTreeDepth--;
	}
	else if (uiTreeDepth == uiRootTreeDepth)
	{
		pBuffer[0] = 0;
		return true;
	}

	for (n; n<uiTreeDepth; n++)
		strncat(pBuffer, fmt("%s/", dirTree[n]), sMaxBuffer);
	return true;
}

void	File_ResetDir()
{
	File_ChangeDir("/");
}

void	File_SeekToNewLine(CFile *pFile)
{
	int c;
	OVERHEAD_INIT;

	while (!pFile->IsEOF())
	{
		c = pFile->ReadByte();
		if (c=='\n')
		{
			OVERHEAD_COUNT(OVERHEAD_IO);
			return;
		}
	}
	OVERHEAD_COUNT(OVERHEAD_IO);
}

#ifdef unix
int		File_SizeAbsolute(const char *filename)
{
	STRUCT_STAT fstats;
	OVERHEAD_INIT;
	
	if (!filename[0])
		return 0;

	if (File_StatAbsolute(filename, &fstats))
		return fstats.st_size;
	else
		return 0;

	OVERHEAD_COUNT(OVERHEAD_IO);
}
#endif

bool	File_Exists(const char *pPath)
{
	return gameTranslator->Exists(pPath);
}

size_t	File_Size(const char *filename)
{
	return gameTranslator->Size(filename);
}

eAccessLevel	File_GetPathAccessLevel(const char *pPath)
{
	return gameTranslator->GetPathAccessLevel(pPath);
}

char *File_GetNextFileIncrement(int num_digits, const char *basename, const char *ext, char *filename, int size)
{
	//const int int_size = 5;
	int i,limit;
	char fname[1024];
	const char *format;

	if (num_digits > 6)
		num_digits = 6;

	switch(num_digits)
	{
	case 1:
		format = "%s%01d.%s";
		limit = 9;
		break;
	case 2:
		format = "%s%02d.%s";
		limit = 99;
		break;
	case 3:
		format = "%s%03d.%s";
		limit = 999;
		break;
	case 4:
		format = "%s%04d.%s";
		limit = 9999;
		break;
	case 5:
		format = "%s%05d.%s";
		limit = 99999;
		break;
	case 6:
		format = "%s%06d.%s";
		limit = 999999;
		break;
	default:
		format = "%s%04d.%s";
		limit = 9999;
		break;
	}
	
	i = 0;
	
	do
	{		
		BPrintf(fname, sizeof(fname), format, basename, i, ext);
		i++;
	} while (File_Exists(fname) && i <= limit);

	strncpySafe(filename, fname, size);

	return filename;
}

/*==========================

  File_AllocBlockList

  allocate a new blocklist structure and fill it in

  the 'filename' field is ONLY used for outputting debugging info
  the file should have been read in and put into 'buf' prior to
  File_AllocBlockList

 ==========================*/

blockList_t *File_AllocBlockList(const void *buf, int buflen, const char *filename_debug)
{
	blockList_t *blocklist;
	int n = 0;
	int curpos = 0;
	char *cbuf = (char *)buf;

	blocklist = (blockList_t*)Tag_Malloc(sizeof(blockList_t), MEM_FILESYSTEM);
	//allocate blocks in increments of 64
	blocklist->blocks = (block_t*)Tag_Malloc(sizeof(block_t) * 64, MEM_FILESYSTEM);
	blocklist->_num_allocated = 64;	

	while (curpos+8 < buflen)
	{
		int length;

		blocklist->blocks[n].name[0] = cbuf[curpos++];
		blocklist->blocks[n].name[1] = cbuf[curpos++];
		blocklist->blocks[n].name[2] = cbuf[curpos++];
		blocklist->blocks[n].name[3] = cbuf[curpos++];
		blocklist->blocks[n].name[4] = '\0';
		
		length = blocklist->blocks[n].length = LittleInt(*(int *)(&cbuf[curpos]));
		if (curpos + length > buflen)
		{
			System_Error("File_BufParseBlocks: Invalid block\n");
			Tag_Free(blocklist->blocks);
			Tag_Free(blocklist);
			return NULL;
		}
		
		curpos+=4;
		blocklist->blocks[n].pos = curpos;
		blocklist->blocks[n].data = (byte*)&cbuf[curpos];
		curpos+=length;

		n++;
		if (n >= blocklist->_num_allocated)
		{
			//allocate some more blocks
			blocklist->blocks = (block_t*)Tag_Realloc(blocklist->blocks, sizeof(block_t) * (blocklist->_num_allocated + 64), MEM_FILESYSTEM);
			blocklist->_num_allocated += 64;
		}		
	}

	if (curpos != buflen)
	{
		Console_DPrintf("File_AllocBlockList: bad filesize in %s\n", filename_debug);
		Tag_Free(blocklist->blocks);
		Tag_Free(blocklist);
		return NULL;
	}

	blocklist->num_blocks = n;

	return blocklist;
}

void	File_FreeBlockList(blockList_t *blocklist)
{
	if (!blocklist)
		return;

	Tag_Free(blocklist->blocks);
	Tag_Free(blocklist);
}

void	File_Init()
{
	char pathBuffer[1024];

	openFiles = new list<CFile*>();

	snprintf(pathBuffer, 1023, "%s/", mod.string);

	systemTranslator = File_CreateTranslator("./", ACCESS_SYSTEM);
	gameTranslator = File_CreateTranslator(pathBuffer, ACCESS_GAME);

	if (!gameTranslator)
		System_Error("Could not register the FileTranslator for the mod's path!\n");
}

void	File_Shutdown()
{
	Tag_FreeAll(MEM_FILESYSTEM);

	delete openFiles;
}


/*===================================================
	CRawFile

	This class represents a file on the system.
	As long as it is present, the file is opened.

	fixme: Port to unix and mac
===================================================*/

size_t		CRawFile::Read(void *pBuffer, size_t sElement, long iNumElements)
{
#ifdef _WIN32
	DWORD dwBytesRead;

	if (sElement == 0 || iNumElements == 0)
		return 0;

	ReadFile(m_pFile, pBuffer, sElement * iNumElements, &dwBytesRead, NULL);
	return dwBytesRead / sElement;
#endif
}

size_t		CRawFile::Write(void *pBuffer, size_t sElement, long iNumElements)
{
#ifdef _WIN32
	DWORD dwBytesWritten;

	if (sElement == 0 || iNumElements == 0)
		return 0;

	WriteFile(m_pFile, pBuffer, sElement * iNumElements, &dwBytesWritten, NULL);
	return dwBytesWritten / sElement;
#endif
}

int			CRawFile::Seek(long iOffset, int iType)
{
#ifdef _WIN32
	if (SetFilePointer(m_pFile, iOffset, NULL, iType) == INVALID_SET_FILE_POINTER)
		return -1;
	return 0;
#endif
}

long		CRawFile::Tell()
{
#ifdef _WIN32
	return SetFilePointer(m_pFile, 0, NULL, FILE_CURRENT);
#endif
}

bool		CRawFile::IsEOF()
{
#ifdef _WIN32
	return (SetFilePointer(m_pFile, 0, NULL, FILE_CURRENT) == GetFileSize(m_pFile, NULL));
#endif
}

bool		CRawFile::Stat(struct stat *pFileStats)
{
#ifdef _WIN32
	BY_HANDLE_FILE_INFORMATION info;
	if (!GetFileInformationByHandle(m_pFile, &info))
		return false;

	pFileStats->st_size = (long)info.nFileSizeLow >> 32 | info.nFileSizeHigh;
	pFileStats->st_dev = (long)info.nFileIndexLow >> 32 | info.nFileSizeHigh;
	pFileStats->st_atime = (long)info.ftLastAccessTime.dwLowDateTime >> 32 | info.ftLastAccessTime.dwHighDateTime;
	pFileStats->st_ctime = (long)info.ftCreationTime.dwLowDateTime >> 32 | info.ftCreationTime.dwHighDateTime;
	pFileStats->st_mtime = (long)info.ftLastWriteTime.dwLowDateTime >> 32 | info.ftLastWriteTime.dwLowDateTime;
	pFileStats->st_dev = info.dwVolumeSerialNumber;
	pFileStats->st_mode = 0;
	pFileStats->st_nlink = info.nNumberOfLinks;
	pFileStats->st_rdev = 0;
	pFileStats->st_gid = 0;
	return true;
#endif
}

size_t		CRawFile::GetSize()
{
#ifdef _WIN32
	return GetFileSize(m_pFile, NULL);
#endif
}

void		CRawFile::Flush()
{
#ifdef _WIN32
	FlushFileBuffers(m_pFile);
#endif
}

const char*	CRawFile::GetPath()
{
	return m_pPath;
}

void		CRawFile::Close()
{
	Tag_Free(m_pPath);
	
#ifdef _WIN32
	CloseHandle(m_pFile);
#endif

	openFiles->remove(this);
	delete this;
}

int			CRawFile::ReadInt()
{
	int iBuffer = 0;

	Read((char*)&iBuffer, sizeof(int), 1);
	return iBuffer;
}

short		CRawFile::ReadShort()
{
	short sBuffer = 0;

	Read((char*)&sBuffer, sizeof(short), 1);
	return sBuffer;
}

char		CRawFile::ReadByte()
{
	char cBuffer = 0;

	Read((char*)&cBuffer, sizeof(char), 1);
	return cBuffer;
}

size_t		CRawFile::WriteInt(int iInt)
{
	return Write((char*)&iInt, sizeof(int), 1);
}

size_t		CRawFile::WriteShort(short iShort)
{
	return Write((char*)&iShort, sizeof(short), 1);
}

size_t		CRawFile::WriteByte(char cByte)
{
	return Write((char*)&cByte, sizeof(char), 1);
}

size_t		CRawFile::Printf(const char *pFormat, ...)
{
	va_list args;
	char cBuffer[1024];

	va_start(args, pFormat);
	_vsnprintf(cBuffer, 1023, pFormat, args);
	va_end(args);

	return Write(cBuffer, 1, strlen(cBuffer));
}

size_t		CRawFile::GetString(char *pBuffer, size_t sMaxLength)
{
	int i = 0;

	if (sMaxLength == 0)
		return 0;
	
	while (i < sMaxLength - 1)
	{
		pBuffer[i] = ReadByte();
		if (!pBuffer[i] || pBuffer[i] == '\n')
			break;
		//only increment i if we have a valid character
		if (pBuffer[i] != '\r')
			i++;
	}
	pBuffer[i] = 0;
	return i;
}

/*=========================================
	CBufferedFile

	Loads a file at open and keeps it in a managed buffer.
=========================================*/

size_t		CBufferedFile::Read(void *pBuffer, size_t sElement, long iNumElements)
{
	long iReadElements = MIN((m_sSize - m_iSeek) / sElement, iNumElements);
	size_t sRead = iReadElements * sElement;

	if (iNumElements <= 0)
		return 0;
	
	Mem_Copy(pBuffer, m_pBuffer + m_iSeek, sRead);
	m_iSeek += sRead;
	return iReadElements;
}

size_t		CBufferedFile::Write(void *pBuffer, size_t sElement, long iNumElements)
{
	return 0;
}

int			CBufferedFile::Seek(long iOffset, int iType)
{
	switch (iType)
	{
	case SEEK_SET:
		m_iSeek = 0;
		break;
	case SEEK_END:
		m_iSeek = m_sSize;
		break;
	}
	m_iSeek = MAX(0, MIN(m_iSeek + iOffset, m_sSize));

	return 0;
}

long		CBufferedFile::Tell()
{
	return m_iSeek;
}

bool		CBufferedFile::IsEOF()
{
	return (m_iSeek == m_sSize);
}

bool		CBufferedFile::Stat(struct stat *pFileStats)
{
	pFileStats->st_dev = -1;
	pFileStats->st_ino = -1;
	pFileStats->st_mode = -1;
	pFileStats->st_nlink = -1;
	pFileStats->st_uid = -1;
	pFileStats->st_gid = -1;
	pFileStats->st_rdev = -1;
	pFileStats->st_size = m_sSize;
	pFileStats->st_atime = 0;
	pFileStats->st_ctime = 0;
	pFileStats->st_mtime = 0;
	return 0;
}

size_t		CBufferedFile::GetSize()
{
	return m_sSize;
}

void		CBufferedFile::Flush()
{
	return;
}

const char*	CBufferedFile::GetPath()
{
	return m_pPath;
}

void		CBufferedFile::Close()
{
	Tag_Free(m_pPath);
	Tag_Free(m_pBuffer);
	delete this;
}

int			CBufferedFile::ReadInt()
{
	int iResult;

	if ((m_sSize - m_iSeek) < sizeof(int))
		return 0;

	iResult = *(int*)(m_pBuffer + m_iSeek);
	m_iSeek += sizeof(int);
	return iResult;
}

short		CBufferedFile::ReadShort()
{
	short iResult;

	if ((m_sSize - m_iSeek) < sizeof(short))
		return 0;

	iResult = *(short*)(m_pBuffer + m_iSeek);
	m_iSeek += sizeof(short);
	return iResult;
}

char		CBufferedFile::ReadByte()
{
	if (m_sSize == m_iSeek)
		return 0;
	return *(m_pBuffer + m_iSeek++);
}

size_t		CBufferedFile::GetString(char *pBuffer, size_t sMaxLength)
{
	int i = 0;

	if (sMaxLength == 0)
		return 0;
	
	while (i < sMaxLength - 1)
	{
		pBuffer[i] = ReadByte();
		if (!pBuffer[i] || pBuffer[i] == '\n')
			break;
		//only increment i if we have a valid character
		if (pBuffer[i] != '\r')
			i++;
	}
	pBuffer[i] = 0;
	return i;
}

/*=======================================
	CArchiveFileTranslator

	Translator to manage files in archives.
=======================================*/

CArchiveFileTranslator*	Archive_CreateTranslator(const char *pPath, enum eAccessLevel eAccess, int iFlags)
{
	CArchiveFileTranslator *pTranslator;
	char pathBuffer[1024];
	char fullBuffer[1024];
	unsigned char hash[MAX_HASH_SIZE];
	int iHashSize;
	const char *pExtension = Filename_GetExtension(pPath);
	unsigned int n;
	char dirTree[32][MAX_PATH];
	unsigned int uiTreeDepth;
	bool bFile;
	unzFile pArchive;

	if ((pTranslator = Archive_GetArchive(pPath)))
		return pTranslator;

	if (strcmp(pExtension, "s2z") != 0)
		return NULL;	// We only accept .s2z archives

	// We have to handle absolute path, too
	if (strlen(pPath) > 2 && pPath[1] == ':')
	{
		if (!_File_ParseRelativePath(pPath + 3, dirTree, 32, &uiTreeDepth, &bFile))
			return NULL;

		pathBuffer[0] = pPath[0];
		pathBuffer[1] = ':';
		pathBuffer[2] = '/';
		pathBuffer[3] = 0;
	}
	else
	{
		strncpy(pathBuffer, System_GetRootDir(), 1023);
		strncat(pathBuffer, pPath, 1023);

		if (!_File_ParseRelativePath(pathBuffer + 3, dirTree, 32, &uiTreeDepth, &bFile))
			return NULL;

		pathBuffer[3] = 0;
	}

	if (!bFile)
		return NULL;

	if (uiTreeDepth < 2)
		return NULL;

	uiTreeDepth--;

	for (n=0; n<uiTreeDepth; n++)
		strncat(pathBuffer, fmt("%s/", dirTree[n]), 1024);

	strcpy(fullBuffer, pathBuffer);
	strncat(fullBuffer, dirTree[n], 1023);

	pArchive = ZIP_Open(fullBuffer, hash, &iHashSize);

	if (!pArchive)
	{
		Console_Printf("Invalid archive '%s' - loading failed\n", fullBuffer);
		return false;
	}
	pTranslator = new CArchiveFileTranslator();

	pTranslator->m_pPath = Tag_Strdup(fullBuffer, MEM_FILE);
	pTranslator->m_pRoot = Tag_Strdup(pathBuffer, MEM_FILE);
	pTranslator->m_pArchive = pArchive;
	pTranslator->m_eAccessPriviledge = eAccess;
	pTranslator->m_iFlags = iFlags;

	memcpy(pTranslator->m_hash, hash, MAX_HASH_SIZE);
	pTranslator->m_iHashLength = iHashSize;

	memcpy(pTranslator->m_rootTree, dirTree, sizeof(pTranslator->m_rootTree));
	pTranslator->m_uiTreeDepth = uiTreeDepth;

	registeredArchives->push_front(pTranslator);

#ifdef SAVAGE_DEMO
	//Console_Printf("filename = %s, hash = %s\n", archives[i].filename, BinaryToHexWithXor(archives[i].hash, archives[i].hashlen, 0));

	if (
		!Archive_DemoHashCompare(&archives[i], "savage0.s2z", "cb484349925878d0aa0464b1cc1faa37e19ffaa8") &&
		!Archive_DemoHashCompare(&archives[i], "images0.s2z", "1a2292e0a54fd37b247326a191783900cae3c5f8") &&
		!Archive_DemoHashCompare(&archives[i], "gui0.s2z", "fdd9de5822066ee644567a97178d84d470aea69f") &&			
		!Archive_DemoHashCompare(&archives[i], "sounds0.s2z", "28fd0b29d77206a3e75c60a0002e350920abc07b") &&
		!Archive_DemoHashCompare(&archives[i], "eden2.s2z", "e6070a1994375aae91f7847ffb8088bf12fb88b7") &&
		!Archive_DemoHashCompare(&archives[i], "crossroads.s2z", "5fab63e4236dd3c043144e814a1d1f2f98a1d7bf"))
		System_Error(SAVAGE_DEMO_CORRUPT_INSTALL_MSG);
#endif //SAVAGE_DEMO

	return pTranslator;
}

CFile*	CArchiveFileTranslator::Open(const char *pPath, const char *pMode)
{
	char pathBuffer[1024];

	if (!*pPath)
		return NULL;

	if (pPath[1] == ':')
	{
		if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
			return NULL;
	}
	else if (!File_ParsePath(pPath, pathBuffer, 1024))
		return NULL;

	return ZIP_OpenFileBuffered(m_pArchive, pathBuffer, pMode);
}

bool	CArchiveFileTranslator::Exists(const char *pPath)
{
	char pathBuffer[1024];

	if (!*pPath)
		return false;

	if (pPath[1] == ':')
	{
		if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
			return false;
	}
	else if (!File_ParsePath(pPath, pathBuffer, 1024))
		return false;

	return ZIP_FileExists(m_pArchive, pathBuffer);
}

size_t	CArchiveFileTranslator::Size(const char *pPath)
{
	char pathBuffer[1024];
	struct stat fileStats;

	if (!*pPath)
		return 0;

	if (pPath[1] == ':')
	{
		if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
			return 0;
	}
	else if (!File_ParsePath(pPath, pathBuffer, 1024))
		return 0;

	if (!ZIP_StatFile(m_pArchive, pathBuffer, &fileStats))
		return 0;

	return fileStats.st_size;
}

void	CArchiveFileTranslator::GetDirectory(char *pBuffer, size_t sMaxBuffer)
{
	memcpy(pBuffer, m_pPath, min(strlen(m_pPath)+1, sMaxBuffer));
}

void	CArchiveFileTranslator::ScanDirectory(char *pDirectory, char *pWildcard, bool bRecurse, 
						void (*dirCallback)(const char *pDirectory, void *pUserdata), 
						void (*fileCallback)(const char *pFilename, void *pUserdata), 
						void *pUserdata)
{
	char pathBuffer[1024];

	if (!*pDirectory)
		return;

	if (pDirectory[1] == ':')
	{
		if (!_File_ParseDirectoryToRelative(pDirectory, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
			return;
	}
	else if (!File_ParseDirectory(pDirectory, pathBuffer, 1024))
		return;

	ZIP_ScanDirectory(m_pArchive, m_pRoot, pathBuffer, pWildcard, bRecurse, dirCallback, fileCallback, pUserdata);
}

bool	CArchiveFileTranslator::Stat(const char *pPath, struct stat *pStats)
{
	char pathBuffer[1024];

	if (!*pPath)
		return false;

	if (pPath[1] == ':')
	{
		if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
			return false;
	}
	else if (!File_ParsePath(pPath, pathBuffer, 1024))
		return false;

	return ZIP_StatFile(m_pArchive, pathBuffer, pStats);
}

eAccessLevel	CArchiveFileTranslator::GetAccessLevel()
{
	return m_eAccessPriviledge;
}

int		CArchiveFileTranslator::GetFlags()
{
	return m_iFlags;
}

void	CArchiveFileTranslator::GetHash(char pHash[MAX_HASH_SIZE], int *pHashLength)
{
	memcpy(pHash, m_hash, m_iHashLength);

	*pHashLength = m_iHashLength;
}

void	CArchiveFileTranslator::Delete()
{
	ZIP_Close(m_pArchive);

	Tag_Free(m_pPath);
	Tag_Free(m_pRoot);

	registeredArchives->remove(this);
	delete this;
}

/*=======================================
	CSystemFileTranslator

	Default file translator
=======================================*/

CSystemFileTranslator*	File_CreateTranslator(const char *pRootPath, enum eAccessLevel eAccess)
{
	CSystemFileTranslator *pTranslator;
	char pathBuffer[1024];
	char dirTree[32][MAX_PATH];
	unsigned int uiTreeDepth;
	bool bFile;
	unsigned int n;

	// We have to handle absolute path, too
	if (strlen(pRootPath) > 2 && pRootPath[1] == ':')
	{
		if (!_File_ParseRelativePath(pRootPath + 3, dirTree, 32, &uiTreeDepth, &bFile))
			return NULL;

		if (uiTreeDepth == 0)
			return NULL;

		pathBuffer[0] = pRootPath[0];
		pathBuffer[1] = ':';
		pathBuffer[2] = '/';
		pathBuffer[3] = 0;
	}
	else
	{
		strncpy(pathBuffer, System_GetRootDir(), 1023);
		strncat(pathBuffer, pRootPath, 1023);

		if (!_File_ParseRelativePath(pathBuffer + 3, dirTree, 32, &uiTreeDepth, &bFile))
			return NULL;

		pathBuffer[3] = 0;
	}

	if (bFile)
		return NULL;

	for (n=0; n<uiTreeDepth; n++)
		strncat(pathBuffer, fmt("%s/", dirTree[n]), 1024);

	if (!File_IsDirectoryAbsolute(pathBuffer))
		return NULL;

	pTranslator = new CSystemFileTranslator();
	pTranslator->m_eAccessPriviledge = eAccess;

	strcpy(pTranslator->m_root, pathBuffer);

	memcpy(pTranslator->m_rootTree, dirTree, sizeof(pTranslator->m_rootTree));
	pTranslator->m_uiTreeDepth = uiTreeDepth;
	pTranslator->m_currentDir[0] = 0;
	return pTranslator;
}

CFile*	CSystemFileTranslator::Open(const char *pPath, const char *pMode)
{
	char pathBuffer[1024];
	char full_system_path[1024];
	bool bWriting = false;
	CFile *pFile;
	OVERHEAD_INIT;

	if (!*pPath)
		return NULL;

	if (pPath[0] == '/')
	{
		if (!File_ParsePath(pPath + 1, pathBuffer, 1024))
			return NULL;

		snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
	}
	else
	{
		// Open HTTP file
		if (strncmp(pPath, "http:", 5) == 0)
		{
			OVERHEAD_COUNT(OVERHEAD_IO);
			return HTTP_OpenFile((char *)pPath);
		}

		if (pPath[1] == ':')
		{
			if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
				return NULL;

			snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
		}
		else
		{
			snprintf(pathBuffer, 1023, "%s%s", m_currentDir, pPath);

			if (!File_ParsePath(pathBuffer, pathBuffer, 1024))
				return false;

			snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
		}
	}

	//if we are writing, check to see if the file is in the script buffer and knock it out
	if (pMode[0] == 'w')
	{
		int index;

		for (index = 0; index < SCRIPT_BUFFER_SIZE; index++)
		{
			if (!stricmp(full_system_path, scriptBuffer[index].scriptName))
			{
				Tag_Free(scriptBuffer[index].buffer);
				scriptBuffer[index].buffer = NULL;
				scriptBuffer[index].lastRun = 0;
				scriptBuffer[index].length = 0;
				scriptBuffer[index].scriptName[0] = 0;
			}
		}
		bWriting = true;
	}

	//if archives override files on the hard drive, load them here
	if (archive_precedence.integer == 1)
	{
		pFile = Archive_OpenFile(full_system_path, pMode);

		if (pFile)
		{
			OVERHEAD_COUNT(OVERHEAD_IO);
			return pFile;
		}
	}

	pFile = File_OpenAbsolute(full_system_path, pMode);

#if 0
	if (!pFile)
	{
		//check the root, debug only
		char systempath[1024];

		snprintf(systempath, 1023, "%s%s", m_root, Filename_GetFilename(pPath));		
		pFile = File_OpenAbsolute(systempath, pMode);
	}
#endif

#ifdef unix
	//fixme: Perform this through another file translator!
	if (!f)
	{
		f = File_OpenAbsolute(fmt("%s%s", homedir.string, Filename_GetFilename((char *)filename)), mode);
	}
#endif
	
	//if we want to load archives second, try them here
	if (!pFile && archive_precedence.integer != 1)
	{
		pFile = Archive_OpenFile(full_system_path, pMode);
	}

	OVERHEAD_COUNT(OVERHEAD_IO);
	return pFile;
}

bool	CSystemFileTranslator::Exists(const char *pPath)
{
	STRUCT_STAT tmp;
	bool bExists = false;
	char pathBuffer[1024];
	char full_system_path[1024];

	if (!*pPath)
		return false;

	if (pPath[0] == '/')
	{
		if (!File_ParsePath(pPath + 1, pathBuffer, 1024))
			return NULL;

		snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
	}
	else
	{
		if (pPath[1] == ':')
		{
			if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
				return NULL;

			snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
		}
		else
		{
			snprintf(pathBuffer, 1023, "%s%s", m_currentDir, pPath);

			if (!File_ParsePath(pathBuffer, pathBuffer, 1024))
				return false;

			snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
		}
	}

	if (archive_precedence.integer == 1)
		bExists = Archives_FileExists(full_system_path);

	if (!bExists)
		bExists = (stat(full_system_path, &tmp) == 0);

	if (!bExists && archive_precedence.integer != 1)
		bExists = Archives_FileExists(full_system_path);

	return bExists;
}

void	CSystemFileTranslator::GetDirectory(char *pBuffer, size_t sMaxBuffer, bool bSystem)
{
	if (bSystem)
		snprintf(pBuffer, sMaxBuffer, "%s%s", m_root, m_currentDir);
	else
		snprintf(pBuffer, sMaxBuffer, "/%s", m_currentDir);
}

bool	CSystemFileTranslator::ChangeDirectory(const char *pPath)
{
	char pathBuffer[1024];

	if (pPath[0] == '/')
	{
		if (!File_ParseDirectory(pPath + 1, pathBuffer, 1024))
			return false;
	}
	else
	{
		if (pPath[1] == ':')
		{
			if (!_File_ParseDirectoryToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
				return false;
		}
		else
		{
			snprintf(pathBuffer, 1023, "%s%s", m_currentDir, pPath);

			if (!File_ParseDirectory(pathBuffer, pathBuffer, 1024))
				return false;
		}
	}

	memcpy(m_currentDir, pathBuffer, 1024);
	return true;
}

bool	CSystemFileTranslator::Stat(const char *pPath, struct stat *pStats)
{
	bool ret = false;
	char pathBuffer[1024];
	char full_system_path[1024];
	OVERHEAD_INIT;

	if (!*pPath)
		return false;

	if (pPath[0] == '/')
	{
		if (!File_ParsePath(pPath + 1, pathBuffer, 1024))
			return NULL;

		snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
	}
	else
	{
		if (pPath[1] == ':')
		{
			if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
				return NULL;

			snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
		}
		else
		{
			snprintf(pathBuffer, 1023, "%s%s", m_currentDir, pPath);

			if (!File_ParsePath(pathBuffer, pathBuffer, 1024))
				return false;

			snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
		}
	}
		
	if (archive_precedence.integer == 1)
		ret = Archive_StatFile(full_system_path, pStats);
	
	if (!ret)
		ret = (stat(full_system_path, pStats) == 0);

	if (!ret && archive_precedence.integer != 1)
		ret = Archive_StatFile(full_system_path, pStats);
	
	OVERHEAD_COUNT(OVERHEAD_IO);
	return ret;
}

size_t	CSystemFileTranslator::Size(const char *pPath)
{
	STRUCT_STAT fstats;
	char pathBuffer[1024];
	char full_system_path[1024];
	OVERHEAD_INIT;

	if (!*pPath)
		return 0;

	if (pPath[0] == '/')
	{
		if (!File_ParsePath(pPath + 1, pathBuffer, 1024))
			return 0;
	}
	else
	{
		if (pPath[1] == ':')
		{
			if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
				return 0;
		}
		else
		{
			snprintf(pathBuffer, 1023, "%s%s", m_currentDir, pPath);

			if (!File_ParsePath(pathBuffer, pathBuffer, 1024))
				return 0;
		}
	}
	snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);

	if (archive_precedence.integer == 1 && Archive_StatFile(full_system_path, &fstats))
	{
		OVERHEAD_COUNT(OVERHEAD_IO);
		return fstats.st_size;
	}

	if (File_Stat(pathBuffer, &fstats))
	{
		OVERHEAD_COUNT(OVERHEAD_IO);
		return fstats.st_size;
	}

	if (archive_precedence.integer != 1 && Archive_StatFile(full_system_path, &fstats))
	{
		OVERHEAD_COUNT(OVERHEAD_IO);
		return fstats.st_size;
	}

	OVERHEAD_COUNT(OVERHEAD_IO);
	return 0;
}

bool	CSystemFileTranslator::GetFullPath(const char *pPath, char *pBuffer, size_t sMaxBuffer)
{
	char pathBuffer[1024];

	if (!*pPath)
		return false;

	if (sMaxBuffer == 0)
		return false;
	sMaxBuffer--;

	if (pPath[0] == '/')
	{
		if (!File_ParsePath(pPath + 1, pathBuffer, 1024))
			return false;

		snprintf(pBuffer, sMaxBuffer, "%s%s", m_root, pathBuffer);
	}
	else
	{
		snprintf(pathBuffer, 1023, "%s%s", m_currentDir, pPath);

		if (!File_ParsePath(pathBuffer, pathBuffer, 1024))
			return false;

		snprintf(pBuffer, sMaxBuffer, "%s%s", m_root, pathBuffer);
	}
	return true;
}

void	CSystemFileTranslator::ScanDirectory(char *pDirectory, char *pWildcard, bool bRecurse, 
						void (*dirCallback)(const char *pDirectory, void *pUserdata), 
						void (*fileCallback)(const char *pFilename, void *pUserdata), 
						void *pUserdata)
{
	WIN32_FIND_DATA		finddata;
	HANDLE				handle;
	char				searchstring[1024];
	char				pathBuffer[1024];
	char				wcard[256];
	char				fullDirectory[1024];

	if (!*pDirectory)
		return;

	if (pDirectory[0] == '/')
	{
		if (!File_ParseDirectory(pDirectory + 1, pathBuffer, 1024))
			return;
	}
	else
	{
		if (pDirectory[1] == ':')
		{
			if (!_File_ParseDirectoryToRelative(pDirectory, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
				return;
		}
		else
		{
			snprintf(pathBuffer, 1023, "%s%s", m_currentDir, pDirectory);

			if (!File_ParseDirectory(pathBuffer, pathBuffer, 1024))
				return;
		}
	}
	snprintf(fullDirectory, 1023, "%s%s", m_root, pathBuffer);

	Archive_ScanDirectory(fullDirectory, pWildcard, bRecurse, dirCallback, fileCallback, pUserdata);

	if (!pWildcard)
		strcpy(wcard, "*");
	else
		strncpy(wcard, pWildcard, 255);

	//first search for files only
	if (fileCallback)
	{
		BPrintf(searchstring, 1023, "%s%s", fullDirectory, wcard);
		
		Cvar_SetVar(&sys_enumdir, fullDirectory);

		handle = FindFirstFile(searchstring, &finddata);

		if (handle != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (!(finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) &&
					!(finddata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
					!(finddata.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) &&
					!(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					snprintf(pathBuffer, 1023, "%s%s", fullDirectory, finddata.cFileName);

					fileCallback(pathBuffer, pUserdata);		
				}
			} while (FindNextFile(handle, &finddata));
		}

		FindClose(handle);
	}

	//next search for subdirectories only
	BPrintf(searchstring, 1023, "%s*", fullDirectory);
	searchstring[1023] = 0;

	handle = FindFirstFile(searchstring, &finddata);

	if (handle == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if (!(finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) &&
			!(finddata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
			!(finddata.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY))
		{
			if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{					
	 			if (strcmp(finddata.cFileName, ".") && strcmp(finddata.cFileName, "..") && strcmp(finddata.cFileName, "CVS"))
				{
					BPrintf(pathBuffer, 1023, "%s%s/", fullDirectory, finddata.cFileName);

					Cvar_SetVar(&sys_enumdir, pathBuffer);

					if (dirCallback)
						dirCallback(pathBuffer, pUserdata);

					if (bRecurse)
					{
						ScanDirectory(pathBuffer, wcard, true, dirCallback, fileCallback, pUserdata);
					}			
				}
			}
		}
	} while (FindNextFile(handle, &finddata));

	FindClose(handle);

	Cvar_SetVar(&sys_enumdir, "");
}

eAccessLevel	CSystemFileTranslator::GetAccessLevel()
{
	return m_eAccessPriviledge;
}

eAccessLevel	CSystemFileTranslator::GetPathAccessLevel(const char *pPath)
{
	STRUCT_STAT stats;
	CArchiveFileTranslator *pArchive;
	char pathBuffer[1024];
	char full_system_path[1024];

	if (pPath[0] == '/')
	{
		if (!File_ParsePath(pPath + 1, pathBuffer, 1024))
			return ACCESS_GUEST;

		snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
	}
	else
	{
		if (pPath[1] == ':')
		{
			if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
				return ACCESS_GUEST;

			snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
		}
		else
		{
			snprintf(pathBuffer, 1023, "%s%s", m_currentDir, pPath);

			if (!File_ParsePath(pathBuffer, pathBuffer, 1024))
				return ACCESS_GUEST;

			snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
		}
	}

	if (archive_precedence.integer == 1 && (pArchive = Archives_FindPath(full_system_path)))
		return pArchive->GetAccessLevel();
	if (stat(full_system_path, &stats) == 0)
		return ACCESS_GAME;
	if (archive_precedence.integer != 1 && (pArchive = Archives_FindPath(full_system_path)))
		return pArchive->GetAccessLevel();
	return ACCESS_GUEST;
}