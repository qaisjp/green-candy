#ifndef _DINPUT8_
#define _DINPUT8_

class IDirectInput8DeviceProxy : public IDirectInputDevice8A
{
public:
									IDirectInput8DeviceProxy();
									~IDirectInput8DeviceProxy();

	virtual HRESULT	__stdcall		QueryInterface (REFIID riid, LPVOID *ppvObj);
	virtual ULONG	__stdcall		AddRef ();
	virtual ULONG	__stdcall		Release ();

	virtual HRESULT	__stdcall		GetCapabilities	(LPDIDEVCAPS lpDIDevCaps);
	virtual HRESULT	__stdcall		EnumObjects (LPDIENUMDEVICEOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags);
	virtual HRESULT	__stdcall		GetProperty (REFGUID rguidProp, LPDIPROPHEADER pdiph);
	virtual HRESULT	__stdcall		SetProperty (REFGUID rguidProp, LPCDIPROPHEADER pdiph);
	virtual HRESULT	__stdcall		Acquire ();
	virtual HRESULT	__stdcall		Unacquire ();
	virtual HRESULT	__stdcall		GetDeviceState (DWORD cbData, LPVOID lpvData);
	virtual HRESULT	__stdcall		GetDeviceData (DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);
	virtual HRESULT	__stdcall		SetDataFormat (LPCDIDATAFORMAT lpdf);
	virtual HRESULT	__stdcall		SetEventNotification (HANDLE hEvent);
	virtual HRESULT	__stdcall		SetCooperativeLevel (HWND hwnd, DWORD dwFlags);
	virtual HRESULT	__stdcall		GetObjectInfo (LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow);
	virtual HRESULT	__stdcall		GetDeviceInfo (LPDIDEVICEINSTANCE pdidi);
	virtual HRESULT	__stdcall		RunControlPanel (HWND hwndOwner, DWORD dwFlags);
	virtual HRESULT	__stdcall		Initialize (HINSTANCE hinst, DWORD dwVersion, REFGUID rguid);
	virtual HRESULT	__stdcall		CreateEffect (REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT * ppdeff, LPUNKNOWN punkOuter);
	virtual HRESULT	__stdcall		EnumEffects (LPDIENUMEFFECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwEffType);
	virtual HRESULT	__stdcall		GetEffectInfo (LPDIEFFECTINFO pdei, REFGUID rguid);
	virtual HRESULT	__stdcall		GetForceFeedbackState (LPDWORD pdwOut);
	virtual HRESULT	__stdcall		SendForceFeedbackCommand (DWORD dwFlags);
	virtual HRESULT	__stdcall		EnumCreatedEffectObjects (LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags);
	virtual HRESULT	__stdcall		Escape (LPDIEFFESCAPE pEscape);
	virtual HRESULT	__stdcall		Poll ();
	virtual HRESULT	__stdcall		SendDeviceData (DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);
	virtual HRESULT	__stdcall		EnumEffectsInFile (LPCSTR pFileName, LPDIENUMEFFECTSINFILECALLBACK pCallback, LPVOID pvRef, DWORD dwFlags);
	virtual HRESULT	__stdcall		WriteEffectToFile (LPCSTR pFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags);
	virtual HRESULT	__stdcall		BuildActionMap (LPDIACTIONFORMAT lpdiaf, LPCSTR pUserName, DWORD dwFlags);
	virtual HRESULT	__stdcall		SetActionMap (LPDIACTIONFORMAT lpdiActionFormat, LPCTSTR lpUserName, DWORD dwFlags);
	virtual HRESULT	__stdcall		GetImageInfo (LPDIDEVICEIMAGEINFOHEADER lpdiDevImageInfoHeader);
};

class IDirectInput8Proxy : public IDirectInput8A
{
public:
									IDirectInput8Proxy();
									~IDirectInput8Proxy();

	virtual HRESULT	__stdcall		QueryInterface (REFIID riid, LPVOID *ppvObj);
	virtual ULONG	__stdcall		AddRef ();
	virtual ULONG	__stdcall		Release ();

	virtual HRESULT	__stdcall		CreateDevice (REFGUID rguid, LPDIRECTINPUTDEVICE8 *ppDirectInputDevice, LPUNKNOWN ppUnk);
	virtual HRESULT	__stdcall		EnumDevices (DWORD dwDevType, LPDIENUMDEVICESCALLBACK pCallback, LPVOID pvRef, DWORD dwFlags);
	virtual HRESULT	__stdcall		GetDeviceStatus (REFGUID rguidInstance);
	virtual HRESULT	__stdcall		RunControlPanel (HWND hwndOwner, DWORD dwFlags);
	virtual HRESULT	__stdcall		Initialize (HINSTANCE hinst, DWORD dwVersion);
	virtual HRESULT	__stdcall		FindDevice (REFGUID rguidClass, LPCSTR pName, LPGUID pguidInstance);
	virtual HRESULT	__stdcall		EnumDevicesBySemantics (LPCSTR pUserName, LPDIACTIONFORMAT pActionFormat, LPDIENUMDEVICESBYSEMANTICSCB lpCallback, LPVOID pvRef, DWORD dwFlags);
	virtual HRESULT	__stdcall		ConfigureDevices (LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMS lpdiCDParams, DWORD dwFlags, LPVOID pvRefData);
};

void		Input_Init();

extern bool m_bInputEnabled;

#endif //_DINPUT8_