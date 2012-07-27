/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CKeyBinds.h
*  PURPOSE:     Header file for core keybind manager class
*  DEVELOPERS:  Jax <>
*               Florian Busse <flobu@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

class CKeyBinds;

#ifndef __CKEYBINDS_H
#define __CKEYBINDS_H

#include <core/CCoreInterface.h>
#include <core/CCommandsInterface.h>

struct SDefaultCommandBind
{
    char szKey[20];
    bool bState;
    char szCommand[20];
    char szArguments[20];
};

class CKeyBinds : public CKeyBindsInterface
{
public:
                            CKeyBinds( class CCore* pCore );
                            ~CKeyBinds();

    bool                    ProcessMessage( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

protected:
    bool                    ProcessCharacter( WPARAM wChar );
    bool                    ProcessKeyStroke( const SBindableKey * pKey, bool bState );

public:
    // Basic funcs
    void                    Add( CKeyBind* pKeyBind );
    void                    Remove( CKeyBind* pKeyBind );
    void                    Clear();
    void                    RemoveDeletedBinds();
    void                    ClearCommandsAndControls();
    bool                    Call( CKeyBind* pKeyBind );
    
    std::list <CKeyBind*> ::const_iterator IterBegin()    { return m_pList->begin (); }
    std::list <CKeyBind*> ::const_iterator IterEnd()    { return m_pList->end (); }


    // Command-bind funcs
    bool                    AddCommand( const char* szKey, const char* szCommand, const char* szArguments = NULL, bool bState = true, const char* szResource = NULL, bool bAltKey = false );
    bool                    AddCommand( const SBindableKey* pKey, const char* szCommand, const char* szArguments = NULL, bool bState = true );
    bool                    RemoveCommand( const char* szKey, const char* szCommand, bool bCheckState = false, bool bState = true );
    bool                    RemoveAllCommands( const char* szKey, bool bCheckState = false, bool bState = true );
    bool                    RemoveAllCommands();
    bool                    CommandExists( const char* szKey, const char* szCommand, bool bCheckState = false, bool bState = true, const char* szArguments = NULL, const char* szResource = NULL );
    bool                    SetCommandActive( const char* szKey, const char* szCommand, bool bState, const char* szArguments, const char* szResource, bool bActive, bool checkHitState );
    void                    SetAllCommandsActive( const char* szResource, bool bActive, const char* szCommand = NULL, bool bState = true, const char* szArguments = NULL, bool checkHitState = false );
    CCommandBind*           GetBindFromCommand( const char* szCommand, const char* szArguments = NULL, bool bMatchCase = true, const char* szKey = NULL, bool bCheckHitState = false, bool bState = NULL );
    bool                    GetBoundCommands( const char* szCommand, std::list < CCommandBind * > & commandsList );
    
    // Control-bind funcs
    bool                    AddGTAControl( const char* szKey, const char* szControl );
    bool                    AddGTAControl( const char* szKey, eControllerAction action );
    bool                    AddGTAControl( const SBindableKey* pKey, SBindableGTAControl* pControl );
    bool                    RemoveGTAControl( const char* szKey, const char* szControl );
    void                    RemoveGTAControls( const char* szControl, bool bDestroy = true );
    bool                    RemoveAllGTAControls( const char* szKey );
    bool                    RemoveAllGTAControls();
    bool                    GTAControlExists( const char* szKey, const char* szControl );
    bool                    GTAControlExists( const SBindableKey* pKey, SBindableGTAControl* pControl );
    unsigned int            GTAControlsCount();
    void                    CallGTAControlBind( CGTAControlBind* pBind, bool bState );
    void                    CallAllGTAControlBinds( eControlType controlType, bool bState );
    bool                    GetBoundControls( SBindableGTAControl * pControl, std::list <CGTAControlBind*>& controlsList );

    // Control-bind extra funcs
    bool                    GetMultiGTAControlState( CGTAControlBind* pBind );
    bool                    IsControlEnabled( const char* szControl );
    bool                    SetControlEnabled( const char* szControl, bool bEnabled );
    void                    SetAllControlsEnabled( bool bGameControls, bool bMTAControls, bool bEnabled );
    void                    ResetGTAControlState( SBindableGTAControl * pControl );
    void                    ResetAllGTAControlStates();    

    // Function-bind funcs
    bool                    AddFunction( const char* szKey, KeyFunctionBindHandler Handler, bool bState = true, bool bIgnoreGUI = false );
    bool                    AddFunction( const SBindableKey* pKey, KeyFunctionBindHandler Handler, bool bState = true, bool bIgnoreGUI = false );
    bool                    RemoveFunction( const char* szKey, KeyFunctionBindHandler Handler, bool bCheckState = false, bool bState = true );
    bool                    RemoveFunction( const SBindableKey* pKey, KeyFunctionBindHandler Handler, bool bCheckState = false, bool bState = true );
    bool                    RemoveAllFunctions( KeyFunctionBindHandler Handler );
    bool                    RemoveAllFunctions();
    bool                    FunctionExists( const char* szKey, KeyFunctionBindHandler Handler, bool bCheckState = false, bool bState = true );
    bool                    FunctionExists( const SBindableKey* pKey, KeyFunctionBindHandler Handler, bool bCheckState = false, bool bState = true );

    // Function-control-bind funcs
    bool                    AddControlFunction( const char* szControl, ControlFunctionBindHandler Handler, bool bState = true );
    bool                    AddControlFunction( SBindableGTAControl* pControl, ControlFunctionBindHandler Handler, bool bState = true );
    bool                    RemoveControlFunction( const char* szControl, ControlFunctionBindHandler Handler, bool bCheckState = false, bool bState = true );
    bool                    RemoveControlFunction( SBindableGTAControl* pControl, ControlFunctionBindHandler Handler, bool bCheckState = false, bool bState = true );
    bool                    RemoveAllControlFunctions( ControlFunctionBindHandler Handler );
    bool                    RemoveAllControlFunctions();
    bool                    ControlFunctionExists( const char* szControl, ControlFunctionBindHandler Handler, bool bCheckState = false, bool bState = true );
    bool                    ControlFunctionExists( SBindableGTAControl* pControl, ControlFunctionBindHandler Handler, bool bCheckState = false, bool bState = true );

    // Key/code funcs
    char*                   GetKeyFromCode( unsigned long ulCode );
    bool                    GetCodeFromKey( const char* szKey, unsigned long& ucCode );
    const SBindableKey*     GetBindableFromKey( const char* szKey );
    const SBindableKey*     GetBindableFromGTARelative( int iGTAKey );
    bool                    IsKey( const char* szKey );
    char*                   GetKeyFromGTARelative( int iGTAKey );
    const SBindableKey*     GetBindableFromMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bState );
    void                    SetKeyStrokeHandler( KeyStrokeHandler Handler )    { m_KeyStrokeHandler = Handler; }
    void                    SetCharacterKeyHandler( CharacterKeyHandler Handler ) { m_CharacterKeyHandler = Handler; }

    // Control/action funcs
    bool                    GetControlState( eBindableControl control ) const;
    char*                   GetControlFromAction( eControllerAction action );
    bool                    GetActionFromControl( const char* szControl, eControllerAction& action );
    SBindableGTAControl*    GetBindableFromControl( const char* szControl );
    SBindableGTAControl*    GetBindableFromAction( eControllerAction action );
    SBindableGTAControl*    GetBindable( eBindableControl cntrl );
    bool                    GetBindableIndex( const std::string& name, eBindableControl& cntrl );
    bool                    IsControl( const char* szControl );
    void                    SetAllControls( bool bState );
    void                    SetAllFootControls( bool bState );
    void                    SetAllVehicleControls( bool bState );

    void                    SetAllBindStates( bool bState, eKeyBindType onlyType = KEY_BIND_UNDEFINED );

    unsigned int            Count( eKeyBindType bindType = KEY_BIND_UNDEFINED );

    void                    DoPreFramePulse();
    void                    DoPostFramePulse();

    bool                    LoadFromXML( CXMLNode* pMainNode );
    bool                    SaveToXML( CXMLNode* pMainNode );
    void                    LoadDefaultBinds();
    void                    LoadDefaultControls();
    void                    LoadDefaultCommands( bool bForce );
    void                    LoadControlsFromGTA();

    void                    BindCommand( const char* szCmdLine );
    void                    UnbindCommand( const char* szCmdLine );
    void                    PrintBindsCommand( const char* szCmdLine );

    static bool             IsFakeCtrl_L( UINT message, WPARAM wParam, LPARAM lParam );

    void                    AddSection( char* szSection );
    void                    RemoveSection( char* szSection );
    void                    RemoveAllSections();

private:    
    CCore*                      m_pCore;

    std::list <CKeyBind*>*      m_pList;
    char*                       m_szFileName;
    bool                        m_bMouseWheel;
    bool                        m_bInVehicle;
    CCommandBind*               m_pChatBoxBind;
    bool                        m_bProcessingKeyStroke;
    KeyStrokeHandler            m_KeyStrokeHandler;
    CharacterKeyHandler         m_CharacterKeyHandler;
    bool                        m_bWaitingToLoadDefaults;
};

#endif
