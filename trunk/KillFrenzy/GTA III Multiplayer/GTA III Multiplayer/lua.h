#ifndef _LUA_
#define _LUA_

void Lua_Init();
void Lua_Start();
void Lua_Frame();
bool Lua_ProcessCommand( const std::string& cmdName, const std::vector <std::string>& args );
void Lua_Destroy();

extern class CLuaManager *lua_manager;
extern class CResourceManager *resMan;

#endif //_LUA_