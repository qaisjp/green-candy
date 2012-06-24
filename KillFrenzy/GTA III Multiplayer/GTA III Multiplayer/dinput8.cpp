#include "StdInc.h"
typedef	HRESULT	(__stdcall*pfnDirectInput8Create)	( HINSTANCE hInst, DWORD dwVersion, REFIID riidltf, LPVOID *ppOut, LPUNKNOWN punkOuter );
pfnDirectInput8Create	m_pfnDirectInput8Create;
IDirectInput8	*m_pOrigInput;
IDirectInputDevice8		*m_pOrigInputDevice;
unsigned int	m_uiInputDeviceCount=0;
unsigned int	m_uiInputDDeviceCount=0;
bool	m_bInputEnabled=false;

/*================================
	Direct Input Device Proxy
================================*/

IDirectInput8DeviceProxy::IDirectInput8DeviceProxy()
{

}

IDirectInput8DeviceProxy::~IDirectInput8DeviceProxy()
{

}

HRESULT	__stdcall		IDirectInput8DeviceProxy::QueryInterface (REFIID riid, LPVOID *ppvObj)
{
	return m_pOrigInputDevice->QueryInterface (riid, ppvObj);
}

ULONG	__stdcall		IDirectInput8DeviceProxy::AddRef ()
{
	m_uiInputDDeviceCount++;
	return m_pOrigInputDevice->AddRef();
}

ULONG	__stdcall		IDirectInput8DeviceProxy::Release ()
{
	m_uiInputDDeviceCount--;

	if (m_uiInputDDeviceCount==0)
	{
		delete this;
	}
	return m_pOrigInputDevice->Release();
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::GetCapabilities	(LPDIDEVCAPS lpDIDevCaps)
{
	return m_pOrigInputDevice->GetCapabilities ( lpDIDevCaps );
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::EnumObjects (LPDIENUMDEVICEOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	return m_pOrigInputDevice->EnumObjects(lpCallback, pvRef, dwFlags);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::GetProperty (REFGUID rguidProp, LPDIPROPHEADER pdiph)
{
	return m_pOrigInputDevice->GetProperty(rguidProp, pdiph);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::SetProperty (REFGUID rguidProp, LPCDIPROPHEADER pdiph)
{
	return m_pOrigInputDevice->SetProperty(rguidProp, pdiph);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::Acquire ()
{
	return m_pOrigInputDevice->Acquire();
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::Unacquire ()
{
	return m_pOrigInputDevice->Unacquire();
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::GetDeviceState (DWORD cbData, LPVOID lpvData)
{
	if (!m_bInputEnabled)
	{
		HRESULT hRes = m_pOrigInputDevice->GetDeviceState(cbData, lpvData);

		memset(lpvData, 0, cbData);

		return hRes;
	}
	return m_pOrigInputDevice->GetDeviceState(cbData, lpvData);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::GetDeviceData (DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
	return m_pOrigInputDevice->GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::SetDataFormat (LPCDIDATAFORMAT lpdf)
{
	return m_pOrigInputDevice->SetDataFormat(lpdf);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::SetEventNotification (HANDLE hEvent)
{
	return m_pOrigInputDevice->SetEventNotification(hEvent);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::SetCooperativeLevel (HWND hwnd, DWORD dwFlags)
{
	return m_pOrigInputDevice->SetCooperativeLevel(hwnd, dwFlags);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::GetObjectInfo (LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow)
{
	return m_pOrigInputDevice->GetObjectInfo(pdidoi, dwObj, dwHow);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::GetDeviceInfo (LPDIDEVICEINSTANCE pdidi)
{
	return m_pOrigInputDevice->GetDeviceInfo(pdidi);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::RunControlPanel (HWND hwndOwner, DWORD dwFlags)
{
	return m_pOrigInputDevice->RunControlPanel(hwndOwner, dwFlags);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::Initialize (HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
{
	return m_pOrigInputDevice->Initialize(hinst, dwVersion, rguid);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::CreateEffect (REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT * ppdeff, LPUNKNOWN punkOuter)
{
	return m_pOrigInputDevice->CreateEffect(rguid, lpeff, ppdeff, punkOuter);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::EnumEffects (LPDIENUMEFFECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwEffType)
{
	return m_pOrigInputDevice->EnumEffects(lpCallback, pvRef, dwEffType);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::GetEffectInfo (LPDIEFFECTINFO pdei, REFGUID rguid)
{
	return m_pOrigInputDevice->GetEffectInfo(pdei, rguid);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::GetForceFeedbackState (LPDWORD pdwOut)
{
	return m_pOrigInputDevice->GetForceFeedbackState(pdwOut);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::SendForceFeedbackCommand (DWORD dwFlags)
{
	return m_pOrigInputDevice->SendForceFeedbackCommand(dwFlags);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::EnumCreatedEffectObjects (LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	return m_pOrigInputDevice->EnumCreatedEffectObjects(lpCallback, pvRef, dwFlags);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::Escape (LPDIEFFESCAPE pEscape)
{
	return m_pOrigInputDevice->Escape(pEscape);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::Poll ()
{
	return m_pOrigInputDevice->Poll();
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::SendDeviceData (DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
	return m_pOrigInputDevice->SendDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::EnumEffectsInFile (LPCSTR pFileName, LPDIENUMEFFECTSINFILECALLBACK pCallback, LPVOID pvRef, DWORD dwFlags)
{
	return m_pOrigInputDevice->EnumEffectsInFile(pFileName, pCallback, pvRef, dwFlags);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::WriteEffectToFile (LPCSTR pFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags)
{
	return m_pOrigInputDevice->WriteEffectToFile(pFileName, dwEntries, rgDiFileEft, dwFlags);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::BuildActionMap (LPDIACTIONFORMAT lpdiaf, LPCSTR pUserName, DWORD dwFlags)
{
	return m_pOrigInputDevice->BuildActionMap(lpdiaf, pUserName, dwFlags);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::SetActionMap (LPDIACTIONFORMAT lpdiActionFormat, LPCTSTR lpUserName, DWORD dwFlags)
{
	return m_pOrigInputDevice->SetActionMap(lpdiActionFormat, lpUserName, dwFlags);
}

HRESULT	__stdcall		IDirectInput8DeviceProxy::GetImageInfo (LPDIDEVICEIMAGEINFOHEADER lpdiDevImageInfoHeader)
{
	return m_pOrigInputDevice->GetImageInfo(lpdiDevImageInfoHeader);
}

/*================================
	Direct Input Proxy
================================*/

IDirectInput8Proxy::IDirectInput8Proxy()
{

}

IDirectInput8Proxy::~IDirectInput8Proxy()
{

}

HRESULT	__stdcall		IDirectInput8Proxy::QueryInterface (REFIID riid, LPVOID *ppvObj)
{
	return m_pOrigInput->QueryInterface(riid, ppvObj);
}

ULONG	__stdcall		IDirectInput8Proxy::AddRef ()
{
	m_uiInputDeviceCount++;
	return m_pOrigInput->AddRef();
}

ULONG	__stdcall		IDirectInput8Proxy::Release ()
{
	m_uiInputDeviceCount--;

	if (m_uiInputDeviceCount==0)
	{
		delete this;
	}
	return m_pOrigInput->Release();
}

HRESULT	__stdcall		IDirectInput8Proxy::CreateDevice (REFGUID rguid, LPDIRECTINPUTDEVICE8 *ppDirectInputDevice, LPUNKNOWN ppUnk)
{
	HRESULT hRes;

	hRes = m_pOrigInput->CreateDevice(rguid, &m_pOrigInputDevice, ppUnk);

	if (hRes==S_OK)
	{
		*ppDirectInputDevice = new IDirectInput8DeviceProxy();
	}
	return hRes;
}

HRESULT	__stdcall		IDirectInput8Proxy::EnumDevices (DWORD dwDevType, LPDIENUMDEVICESCALLBACK pCallback, LPVOID pvRef, DWORD dwFlags)
{
	return m_pOrigInput->EnumDevices(dwDevType, pCallback, pvRef, dwFlags);
}

HRESULT	__stdcall		IDirectInput8Proxy::GetDeviceStatus (REFGUID rguidInstance)
{
	return m_pOrigInput->GetDeviceStatus(rguidInstance);
}

HRESULT	__stdcall		IDirectInput8Proxy::RunControlPanel (HWND hwndOwner, DWORD dwFlags)
{
	return m_pOrigInput->RunControlPanel(hwndOwner, 0);
}

HRESULT	__stdcall		IDirectInput8Proxy::Initialize (HINSTANCE hinst, DWORD dwVersion)
{
	return m_pOrigInput->Initialize(hinst, dwVersion);
}

HRESULT	__stdcall		IDirectInput8Proxy::FindDevice (REFGUID rguidClass, LPCSTR pName, LPGUID pguidInstance)
{
	return m_pOrigInput->FindDevice(rguidClass, pName, pguidInstance);
}

HRESULT	__stdcall		IDirectInput8Proxy::EnumDevicesBySemantics (LPCSTR pUserName, LPDIACTIONFORMAT pActionFormat, LPDIENUMDEVICESBYSEMANTICSCB pCallback, LPVOID pvRef, DWORD dwFlags)
{
	return m_pOrigInput->EnumDevicesBySemantics(pUserName, pActionFormat, pCallback, pvRef, dwFlags);
}

HRESULT	__stdcall		IDirectInput8Proxy::ConfigureDevices (LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMS lpdiCDParams, DWORD dwFlags, LPVOID pvRefData)
{
	return m_pOrigInput->ConfigureDevices(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
}

///////////////////////////////////////////////

HRESULT __stdcall		Hook_DirectInput8Create ( HINSTANCE hInst, DWORD dwVersion, REFIID riidltf, LPVOID *ppOut, LPUNKNOWN punkOuter )
{
	HRESULT hRes = m_pfnDirectInput8Create ( hInst, dwVersion, riidltf, (LPVOID*)&m_pOrigInput, punkOuter );

	if (hRes==S_OK)
	{
		IDirectInput8Proxy *pInterface = new IDirectInput8Proxy();
		*ppOut = pInterface;
	}
	return hRes;
}

// Installing hooks, etc
void		Input_Init()
{
	m_bInputEnabled=true;

	// Hook creation of dinput8
	m_pfnDirectInput8Create = (pfnDirectInput8Create)DetourFunction(DetourFindFunction("dinput8.dll", "DirectInput8Create"), (PBYTE)Hook_DirectInput8Create);
}