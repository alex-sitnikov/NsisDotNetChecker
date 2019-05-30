/**
 * Original file detectfx_new.cpp is located at https://skydrive.live.com/?cid=27e6a35d1a492af7&id=27E6A35D1A492AF7%21494
 * Author: Aaron Stebner (http://blogs.msdn.com/b/astebner/archive/2009/06/16/9763379.aspx)
 * NSIS adaptation: Alexey Sitnikov
 */

#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <mscoree.h>

#include "nsis\pluginapi.h"

// In case the machine this is compiled on does not have the most recent platform SDK
// with these values defined, define them here
#ifndef SM_TABLETPC
	#define SM_TABLETPC     86
#endif

#ifndef SM_MEDIACENTER
	#define SM_MEDIACENTER  87
#endif

#define CountOf(x) sizeof(x)/sizeof(*x)

// Constants that represent registry key names and value names
// to use for detection
const TCHAR *g_szNetfx10RegKeyName = _T("Software\\Microsoft\\.NETFramework\\Policy\\v1.0");
const TCHAR *g_szNetfx10RegKeyValue = _T("3705");
const TCHAR *g_szNetfx10SPxMSIRegKeyName = _T("Software\\Microsoft\\Active Setup\\Installed Components\\{78705f0d-e8db-4b2d-8193-982bdda15ecd}");
const TCHAR *g_szNetfx10SPxOCMRegKeyName = _T("Software\\Microsoft\\Active Setup\\Installed Components\\{FDC11A6F-17D1-48f9-9EA3-9051954BAA24}");
const TCHAR *g_szNetfx11RegKeyName = _T("Software\\Microsoft\\NET Framework Setup\\NDP\\v1.1.4322");
const TCHAR *g_szNetfx20RegKeyName = _T("Software\\Microsoft\\NET Framework Setup\\NDP\\v2.0.50727");
const TCHAR *g_szNetfx30RegKeyName = _T("Software\\Microsoft\\NET Framework Setup\\NDP\\v3.0\\Setup");
const TCHAR *g_szNetfx30SpRegKeyName = _T("Software\\Microsoft\\NET Framework Setup\\NDP\\v3.0");
const TCHAR *g_szNetfx30RegValueName = _T("InstallSuccess");
const TCHAR *g_szNetfx35RegKeyName = _T("Software\\Microsoft\\NET Framework Setup\\NDP\\v3.5");
const TCHAR *g_szNetfx40ClientRegKeyName = _T("Software\\Microsoft\\NET Framework Setup\\NDP\\v4\\Client");
const TCHAR *g_szNetfx40FullRegKeyName = _T("Software\\Microsoft\\NET Framework Setup\\NDP\\v4\\Full");
const TCHAR *g_szNetfx40SPxRegValueName = _T("Servicing");
const TCHAR *g_szNetfx45RegKeyName = _T("Software\\Microsoft\\NET Framework Setup\\NDP\\v4\\Full");
const TCHAR *g_szNetfx45RegValueName = _T("Release");
const TCHAR *g_szNetfx46RegKeyName = g_szNetfx45RegKeyName;
const TCHAR *g_szNetfx46RegValueName = g_szNetfx45RegValueName;
const TCHAR *g_szNetfx47RegKeyName = g_szNetfx45RegKeyName;
const TCHAR *g_szNetfx47RegValueName = g_szNetfx45RegValueName;
const TCHAR* g_szNetfx48RegKeyName = g_szNetfx45RegKeyName;
const TCHAR* g_szNetfx48RegValueName = g_szNetfx45RegValueName;
const TCHAR *g_szNetfxStandardRegValueName = _T("Install");
const TCHAR *g_szNetfxStandardSPxRegValueName = _T("SP");
const TCHAR *g_szNetfxStandardVersionRegValueName = _T("Version");

// Version information for final release of .NET Framework 3.0
const int g_iNetfx30VersionMajor = 3;
const int g_iNetfx30VersionMinor = 0;
const int g_iNetfx30VersionBuild = 4506;
const int g_iNetfx30VersionRevision = 26;

// Version information for final release of .NET Framework 3.5
const int g_iNetfx35VersionMajor = 3;
const int g_iNetfx35VersionMinor = 5;
const int g_iNetfx35VersionBuild = 21022;
const int g_iNetfx35VersionRevision = 8;

// Version information for final release of .NET Framework 4
const int g_iNetfx40VersionMajor = 4;
const int g_iNetfx40VersionMinor = 0;
const int g_iNetfx40VersionBuild = 30319;
const int g_iNetfx40VersionRevision = 0;

// Version information for final release of .NET Framework 4.5 and 4.5.1 (4.5.1 updated as per Windows 8.1 Release value)
const int g_dwNetfx45ReleaseVersion = 378389;
const int g_dwNetfx451ReleaseVersion = 378675;

// Version information for final release of .NET Framework 4.5.2
const int g_dwNetfx452ReleaseVersion = 379893;

// Version information for final release of .NET Framework 4.6
const int g_dwNetfx46Win10ReleaseVersion = 393295;
const int g_dwNetfx46ReleaseVersion = 393297;

// Version information for final release of .NET Framework 4.6.1
const int g_dwNetfx461Win10ReleaseVersion = 394254;
const int g_dwNetfx461ReleaseVersion = 394271;

// Version information for final release of .NET Framework 4.6.2
const int g_dwNetfx462Win10ReleaseVersion = 394802;
const int g_dwNetfx462ReleaseVersion = 394806;

// Version information for final release of .NET Framework 4.7
const int g_dwNetfx47Win10ReleaseVersion = 460798;
const int g_dwNetfx47ReleaseVersion = 460805;

// Version information for final release of .NET Framework 4.7.1
const int g_dwNetfx471Win10ReleaseVersion = 461308;
const int g_dwNetfx471ReleaseVersion = 461310;

// Version information for final release of .NET Framework 4.7.2
const int g_dwNetfx472Win10ReleaseVersion = 461808;
const int g_dwNetfx472ReleaseVersion = 461814;

// Version information for final release of .NET Framework 4.8
const int g_dwNetfx48Win10ReleaseVersion = 528040;
const int g_dwNetfx48ReleaseVersion = 528049;

// Constants for known .NET Framework versions used with the GetRequestedRuntimeInfo API
const TCHAR *g_szNetfx10VersionString = _T("v1.0.3705");
const TCHAR *g_szNetfx11VersionString = _T("v1.1.4322");
const TCHAR *g_szNetfx20VersionString = _T("v2.0.50727");
const TCHAR *g_szNetfx40VersionString = _T("v4.0.30319");

// Function prototypes
bool CheckNetfxBuildNumber(const TCHAR*, const TCHAR*, const int, const int, const int, const int);
bool CheckNetfxVersionUsingMscoree(const TCHAR*);
int GetNetfx10SPLevel();
int GetNetfxSPLevel(const TCHAR*, const TCHAR*);
DWORD GetProcessorArchitectureFlag();
bool IsCurrentOSTabletMedCenter();
bool IsNetfx10Installed();
bool IsNetfx11Installed();
bool IsNetfx20Installed();
bool IsNetfx30Installed();
bool IsNetfx35Installed();
bool IsNetfx40ClientInstalled();
bool IsNetfx40FullInstalled();
bool IsNetfx45Installed();
bool IsNetfx451Installed();
bool IsNetfx452Installed();
bool IsNetfx46Installed();
bool IsNetfx461Installed();
bool IsNetfx462Installed();
bool IsNetfx47Installed();
bool IsNetfx471Installed();
bool IsNetfx472Installed();
bool IsNetfx48Installed();
bool RegistryGetValue(HKEY, const TCHAR*, const TCHAR*, DWORD, LPBYTE, DWORD);


/******************************************************************
Function Name:  CheckNetfxVersionUsingMscoree
Description:    Uses the logic described in the sample code at http://msdn2.microsoft.com/library/ydh6b3yb.aspx
				to load mscoree.dll and call its APIs to determine
				whether or not a specific version of the .NET
				Framework is installed on the system
Inputs:         pszNetfxVersionToCheck - version to look for
Results:        true if the requested version is installed
				false otherwise
******************************************************************/
bool CheckNetfxVersionUsingMscoree(const TCHAR *pszNetfxVersionToCheck)
{
	bool bFoundRequestedNetfxVersion = false;
	HRESULT hr = S_OK;

	// Check input parameter
	if (NULL == pszNetfxVersionToCheck)
		return false;

	HMODULE hmodMscoree = LoadLibraryEx(_T("mscoree.dll"), NULL, 0);
	if (NULL != hmodMscoree)
	{
		typedef HRESULT (STDAPICALLTYPE *GETCORVERSION)(LPWSTR szBuffer, DWORD cchBuffer, DWORD* dwLength);
		GETCORVERSION pfnGETCORVERSION = (GETCORVERSION)GetProcAddress(hmodMscoree, "GetCORVersion");

		// Some OSs shipped with a placeholder copy of mscoree.dll. The existence of mscoree.dll
		// therefore does NOT mean that a version of the .NET Framework is installed.
		// If this copy of mscoree.dll does not have an exported function named GetCORVersion
		// then we know it is a placeholder DLL.
		if (NULL == pfnGETCORVERSION)
			goto Finish;

		typedef HRESULT (STDAPICALLTYPE *CORBINDTORUNTIME)(LPCWSTR pwszVersion, LPCWSTR pwszBuildFlavor, REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv);
		CORBINDTORUNTIME pfnCORBINDTORUNTIME = (CORBINDTORUNTIME)GetProcAddress(hmodMscoree, "CorBindToRuntime");

		typedef HRESULT (STDAPICALLTYPE *GETREQUESTEDRUNTIMEINFO)(LPCWSTR pExe, LPCWSTR pwszVersion, LPCWSTR pConfigurationFile, DWORD startupFlags, DWORD runtimeInfoFlags, LPWSTR pDirectory, DWORD dwDirectory, DWORD *dwDirectoryLength, LPWSTR pVersion, DWORD cchBuffer, DWORD* dwlength);
		GETREQUESTEDRUNTIMEINFO pfnGETREQUESTEDRUNTIMEINFO = (GETREQUESTEDRUNTIMEINFO)GetProcAddress(hmodMscoree, "GetRequestedRuntimeInfo");

		if (NULL != pfnCORBINDTORUNTIME)
		{
			TCHAR szRetrievedVersion[50];
			DWORD dwLength = CountOf(szRetrievedVersion);

			if (NULL == pfnGETREQUESTEDRUNTIMEINFO)
			{
				// Having CorBindToRuntimeHost but not having GetRequestedRuntimeInfo means that
				// this machine contains no higher than .NET Framework 1.0, but the only way to
				// 100% guarantee that the .NET Framework 1.0 is installed is to call a function
				// to exercise its functionality
				if (0 == _tcscmp(pszNetfxVersionToCheck, g_szNetfx10VersionString))
				{
					hr = pfnGETCORVERSION(szRetrievedVersion, dwLength, &dwLength);

					if (SUCCEEDED(hr))
					{
						if (0 == _tcscmp(szRetrievedVersion, g_szNetfx10VersionString))
							bFoundRequestedNetfxVersion = true;
					}

					goto Finish;
				}
			}

			// Set error mode to prevent the .NET Framework from displaying
			// unfriendly error dialogs
			UINT uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

			TCHAR szDirectory[MAX_PATH];
			DWORD dwDirectoryLength = 0;
			DWORD dwRuntimeInfoFlags = RUNTIME_INFO_DONT_RETURN_DIRECTORY | GetProcessorArchitectureFlag();

			// Check for the requested .NET Framework version
			hr = pfnGETREQUESTEDRUNTIMEINFO(NULL, pszNetfxVersionToCheck, NULL, STARTUP_LOADER_OPTIMIZATION_MULTI_DOMAIN_HOST, NULL, szDirectory, CountOf(szDirectory), &dwDirectoryLength, szRetrievedVersion, CountOf(szRetrievedVersion), &dwLength);

			if (SUCCEEDED(hr))
				bFoundRequestedNetfxVersion = true;

			// Restore the previous error mode
			SetErrorMode(uOldErrorMode);
		}
	}

Finish:
	if (hmodMscoree)
	{
		FreeLibrary(hmodMscoree);
	}

	return bFoundRequestedNetfxVersion;
}


/******************************************************************
Function Name:  GetNetfx10SPLevel
Description:    Uses the detection method recommended at
				http://blogs.msdn.com/astebner/archive/2004/09/14/229802.aspx
				to determine what service pack for the
				.NET Framework 1.0 is installed on the machine
Inputs:         NONE
Results:        integer representing SP level for .NET Framework 1.0
******************************************************************/
int GetNetfx10SPLevel()
{
	TCHAR szRegValue[MAX_PATH];
	TCHAR *pszSPLevel = NULL;
	int iRetValue = -1;
	bool bRegistryRetVal = false;

	// Need to detect what OS we are running on so we know what
	// registry key to use to look up the SP level
	if (IsCurrentOSTabletMedCenter())
		bRegistryRetVal = RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx10SPxOCMRegKeyName, g_szNetfxStandardVersionRegValueName, NULL, (LPBYTE)szRegValue, MAX_PATH);
	else
		bRegistryRetVal = RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx10SPxMSIRegKeyName, g_szNetfxStandardVersionRegValueName, NULL, (LPBYTE)szRegValue, MAX_PATH);

	if (bRegistryRetVal)
	{
		// This registry value should be of the format
		// #,#,#####,# where the last # is the SP level
		// Try to parse off the last # here
		pszSPLevel = _tcsrchr(szRegValue, _T(','));
		if (NULL != pszSPLevel)
		{
			// Increment the pointer to skip the comma
			pszSPLevel++;

			// Convert the remaining value to an integer
			iRetValue = _tstoi(pszSPLevel);
		}
	}

	return iRetValue;
}


/******************************************************************
Function Name:	GetNetfxSPLevel
Description:	Determine what service pack is installed for a
				version of the .NET Framework using registry
				based detection methods documented in the
				.NET Framework deployment guides.
Inputs:         pszNetfxRegKeyName - registry key name to use for detection
				pszNetfxRegValueName - registry value to use for detection
Results:        integer representing SP level for .NET Framework
******************************************************************/
int GetNetfxSPLevel(const TCHAR *pszNetfxRegKeyName, const TCHAR *pszNetfxRegValueName)
{
	DWORD dwRegValue=0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, pszNetfxRegKeyName, pszNetfxRegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		return (int)dwRegValue;
	}

	// We can only get here if the .NET Framework is not
	// installed or there was some kind of error retrieving
	// the data from the registry
	return -1;
}


/******************************************************************
Function Name:  GetProcessorArchitectureFlag
Description:    Determine the processor architecture of the
				system (x86, x64, ia64)
Inputs:         NONE
Results:        DWORD processor architecture flag
******************************************************************/
DWORD GetProcessorArchitectureFlag()
{
	HMODULE hmodKernel32 = NULL;
	typedef void (WINAPI *PFnGetNativeSystemInfo) (LPSYSTEM_INFO);
	PFnGetNativeSystemInfo pfnGetNativeSystemInfo;

	SYSTEM_INFO sSystemInfo;
	memset(&sSystemInfo, 0, sizeof(sSystemInfo));

	bool bRetrievedSystemInfo = false;

	// Attempt to load kernel32.dll
	hmodKernel32 = LoadLibrary(_T("Kernel32.dll"));
	if (NULL != hmodKernel32)
	{
		// If the DLL loaded correctly, get the proc address for GetNativeSystemInfo
		pfnGetNativeSystemInfo = (PFnGetNativeSystemInfo) GetProcAddress(hmodKernel32, "GetNativeSystemInfo");
		if (NULL != pfnGetNativeSystemInfo)
		{
			// Call GetNativeSystemInfo if it exists
			(*pfnGetNativeSystemInfo)(&sSystemInfo);
			bRetrievedSystemInfo = true;
		}
		FreeLibrary(hmodKernel32);
	}

	if (!bRetrievedSystemInfo)
	{
		// Fallback to calling GetSystemInfo if the above failed
		GetSystemInfo(&sSystemInfo);
		bRetrievedSystemInfo = true;
	}

	if (bRetrievedSystemInfo)
	{
		switch (sSystemInfo.wProcessorArchitecture)
		{
			case PROCESSOR_ARCHITECTURE_INTEL:
				return RUNTIME_INFO_REQUEST_X86;
			case PROCESSOR_ARCHITECTURE_IA64:
				return RUNTIME_INFO_REQUEST_IA64;
			case PROCESSOR_ARCHITECTURE_AMD64:
				return RUNTIME_INFO_REQUEST_AMD64;
			default:
				return 0;
		}
	}

	return 0;
}


/******************************************************************
Function Name:	CheckNetfxBuildNumber
Description:	Retrieves the .NET Framework build number from
				the registry and validates that it is not a pre-release
				version number
Inputs:         NONE
Results:        true if the build number in the registry is greater
				than or equal to the passed in version; false otherwise
******************************************************************/
bool CheckNetfxBuildNumber(const TCHAR *pszNetfxRegKeyName, const TCHAR *pszNetfxRegKeyValue, const int iRequestedVersionMajor, const int iRequestedVersionMinor, const int iRequestedVersionBuild, const int iRequestedVersionRevision)
{
	TCHAR szRegValue[MAX_PATH];
	TCHAR *pszToken = NULL;
	TCHAR *pszNextToken = NULL;
	int iVersionPartCounter = 0;
	int iRegistryVersionMajor = 0;
	int iRegistryVersionMinor = 0;
	int iRegistryVersionBuild = 0;
	int iRegistryVersionRevision = 0;
	bool bRegistryRetVal = false;

	// Attempt to retrieve the build number registry value
	bRegistryRetVal = RegistryGetValue(HKEY_LOCAL_MACHINE, pszNetfxRegKeyName, pszNetfxRegKeyValue, NULL, (LPBYTE)szRegValue, MAX_PATH);

	if (bRegistryRetVal)
	{
		// This registry value should be of the format
		// #.#.#####.##.  Try to parse the 4 parts of
		// the version here
		pszToken = _tcstok_s(szRegValue, _T("."), &pszNextToken);
		while (NULL != pszToken)
		{
			iVersionPartCounter++;

			switch (iVersionPartCounter)
			{
			case 1:
				// Convert the major version value to an integer
				iRegistryVersionMajor = _tstoi(pszToken);
				break;
			case 2:
				// Convert the minor version value to an integer
				iRegistryVersionMinor = _tstoi(pszToken);
				break;
			case 3:
				// Convert the build number value to an integer
				iRegistryVersionBuild = _tstoi(pszToken);
				break;
			case 4:
				// Convert the revision number value to an integer
				iRegistryVersionRevision = _tstoi(pszToken);
				break;
			default:
				break;

			}

			// Get the next part of the version number
			pszToken = _tcstok_s(NULL, _T("."), &pszNextToken);
		}
	}

	// Compare the version number retrieved from the registry with
	// the version number of the final release of the .NET Framework
	// that we are checking
	if (iRegistryVersionMajor > iRequestedVersionMajor)
	{
		return true;
	}
	else if (iRegistryVersionMajor == iRequestedVersionMajor)
	{
		if (iRegistryVersionMinor > iRequestedVersionMinor)
		{
			return true;
		}
		else if (iRegistryVersionMinor == iRequestedVersionMinor)
		{
			if (iRegistryVersionBuild > iRequestedVersionBuild)
			{
				return true;
			}
			else if (iRegistryVersionBuild == iRequestedVersionBuild)
			{
				if (iRegistryVersionRevision >= iRequestedVersionRevision)
				{
					return true;
				}
			}
		}
	}

	// If we get here, the version in the registry must be less than the
	// version of the final release of the .NET Framework we are checking,
	// so return false
	return false;
}


/******************************************************************
Function Name:  IsCurrentOSTabletMedCenter
Description:    Determine if the current OS is a Windows XP
				Tablet PC Edition or Windows XP Media Center
				Edition system
Inputs:         NONE
Results:        true if the OS is Tablet PC or Media Center
				false otherwise
******************************************************************/
bool IsCurrentOSTabletMedCenter()
{
	// Use GetSystemMetrics to detect if we are on a Tablet PC or Media Center OS
	return ( (GetSystemMetrics(SM_TABLETPC) != 0) || (GetSystemMetrics(SM_MEDIACENTER) != 0) );
}


/******************************************************************
Function Name:  IsNetfx10Installed
Description:    Uses the detection method recommended at
				http://msdn.microsoft.com/library/ms994349.aspx
				to determine whether the .NET Framework 1.0 is
				installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 1.0 is installed
				false otherwise
******************************************************************/
bool IsNetfx10Installed()
{
	TCHAR szRegValue[MAX_PATH];
	return (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx10RegKeyName, g_szNetfx10RegKeyValue, NULL, (LPBYTE)szRegValue, MAX_PATH));
}


/******************************************************************
Function Name:  IsNetfx11Installed
Description:    Uses the detection method recommended at
				http://msdn.microsoft.com/library/ms994339.aspx
				to determine whether the .NET Framework 1.1 is
				installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 1.1 is installed
				false otherwise
******************************************************************/
bool IsNetfx11Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue=0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx11RegKeyName, g_szNetfxStandardRegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (1 == dwRegValue)
			bRetValue = true;
	}

	return bRetValue;
}


/******************************************************************
Function Name:	IsNetfx20Installed
Description:	Uses the detection method recommended at
				http://msdn2.microsoft.com/library/aa480243.aspx
				to determine whether the .NET Framework 2.0 is
				installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 2.0 is installed
				false otherwise
******************************************************************/
bool IsNetfx20Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue=0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx20RegKeyName, g_szNetfxStandardRegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (1 == dwRegValue)
			bRetValue = true;
	}

	return bRetValue;
}


/******************************************************************
Function Name:	IsNetfx30Installed
Description:	Uses the detection method recommended at
				http://msdn.microsoft.com/library/aa964979.aspx
				to determine whether the .NET Framework 3.0 is
				installed on the machine
Inputs:	        NONE
Results:        true if the .NET Framework 3.0 is installed
				false otherwise
******************************************************************/
bool IsNetfx30Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue=0;

	// Check that the InstallSuccess registry value exists and equals 1
	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx30RegKeyName, g_szNetfx30RegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (1 == dwRegValue)
			bRetValue = true;
	}

	// A system with a pre-release version of the .NET Framework 3.0 can
	// have the InstallSuccess value.  As an added verification, check the
	// version number listed in the registry
	return (bRetValue && CheckNetfxBuildNumber(g_szNetfx30RegKeyName, g_szNetfxStandardVersionRegValueName, g_iNetfx30VersionMajor, g_iNetfx30VersionMinor, g_iNetfx30VersionBuild, g_iNetfx30VersionRevision));
}


/******************************************************************
Function Name:	IsNetfx35Installed
Description:	Uses the detection method recommended at
				http://msdn.microsoft.com/library/cc160716.aspx
				to determine whether the .NET Framework 3.5 is
				installed on the machine
Inputs:	        NONE
Results:        true if the .NET Framework 3.5 is installed
				false otherwise
******************************************************************/
bool IsNetfx35Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue=0;

	// Check that the Install registry value exists and equals 1
	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx35RegKeyName, g_szNetfxStandardRegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (1 == dwRegValue)
			bRetValue = true;
	}

	// A system with a pre-release version of the .NET Framework 3.5 can
	// have the Install value.  As an added verification, check the
	// version number listed in the registry
	return (bRetValue && CheckNetfxBuildNumber(g_szNetfx35RegKeyName, g_szNetfxStandardVersionRegValueName, g_iNetfx35VersionMajor, g_iNetfx35VersionMinor, g_iNetfx35VersionBuild, g_iNetfx35VersionRevision));
}


/******************************************************************
Function Name:	IsNetfx40ClientInstalled
Description:	Uses the detection method recommended at
				http://msdn.microsoft.com/library/ee942965(v=VS.100).aspx
				to determine whether the .NET Framework 4 Client is
				installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 4 Client is installed
				false otherwise
******************************************************************/
bool IsNetfx40ClientInstalled()
{
	bool bRetValue = false;
	DWORD dwRegValue=0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx40ClientRegKeyName, g_szNetfxStandardRegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (1 == dwRegValue)
			bRetValue = true;
	}

	// A system with a pre-release version of the .NET Framework 4 can
	// have the Install value.  As an added verification, check the
	// version number listed in the registry
	return (bRetValue && CheckNetfxBuildNumber(g_szNetfx40ClientRegKeyName, g_szNetfxStandardVersionRegValueName, g_iNetfx40VersionMajor, g_iNetfx40VersionMinor, g_iNetfx40VersionBuild, g_iNetfx40VersionRevision));
}


/******************************************************************
Function Name:	IsNetfx40FullInstalled
Description:	Uses the detection method recommended at
				http://msdn.microsoft.com/library/ee942965(v=VS.100).aspx
				to determine whether the .NET Framework 4 Full is
				installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 4 Full is installed
				false otherwise
******************************************************************/
bool IsNetfx40FullInstalled()
{
	bool bRetValue = false;
	DWORD dwRegValue=0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx40FullRegKeyName, g_szNetfxStandardRegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (1 == dwRegValue)
			bRetValue = true;
	}

	// A system with a pre-release version of the .NET Framework 4 can
	// have the Install value.  As an added verification, check the
	// version number listed in the registry
	return (bRetValue && CheckNetfxBuildNumber(g_szNetfx40FullRegKeyName, g_szNetfxStandardVersionRegValueName, g_iNetfx40VersionMajor, g_iNetfx40VersionMinor, g_iNetfx40VersionBuild, g_iNetfx40VersionRevision));
}


/******************************************************************
Function Name:	IsNetfx45Installed
Description:	Uses the detection method recommended at
				http://msdn.microsoft.com/en-us/library/ee942965(v=vs.110).aspx
				to determine whether the .NET Framework 4.5 is
				installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 4.5 is installed
				false otherwise
******************************************************************/
bool IsNetfx45Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue=0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx45RegKeyName, g_szNetfx45RegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (g_dwNetfx45ReleaseVersion <= dwRegValue)
			bRetValue = true;
	}

	return bRetValue;
}

/******************************************************************
Function Name:	IsNetfx46Installed
Description:	Uses the detection method recommended at
http://msdn.microsoft.com/en-us/library/ee942965(v=vs.110).aspx
to determine whether the .NET Framework 4.6 is
installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 4.6 is installed
false otherwise
******************************************************************/
bool IsNetfx46Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue = 0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx46RegKeyName, g_szNetfx46RegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (g_dwNetfx46ReleaseVersion <= dwRegValue || g_dwNetfx46Win10ReleaseVersion <= dwRegValue)
			bRetValue = true;
	}

	return bRetValue;
}


/******************************************************************
Function Name:	IsNetfx451Installed
Description:	Uses the detection method recommended at
				http://msdn.microsoft.com/en-us/library/ee942965(v=vs.110).aspx
				and
				http://blogs.msdn.com/b/astebner/archive/2013/11/11/10466402.aspx
				to determine whether the .NET Framework 4.5.1 is
				installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 4.5.1 is installed
				false otherwise
******************************************************************/
bool IsNetfx451Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue=0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx45RegKeyName, g_szNetfx45RegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (g_dwNetfx451ReleaseVersion <= dwRegValue)
			bRetValue = true;
	}

	return bRetValue;
}



/******************************************************************
Function Name: IsNetfx452Installed
Description: Uses the detection method recommended at
http://msdn.microsoft.com/en-us/library/ee942965(v=vs.110).aspx
to determine whether the .NET Framework 4.5.2 is
installed on the machine
Inputs: NONE
Results: true if the .NET Framework 4.5.2 is installed
false otherwise
******************************************************************/
bool IsNetfx452Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue = 0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx45RegKeyName, g_szNetfx45RegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (g_dwNetfx452ReleaseVersion <= dwRegValue)
			bRetValue = true;
	}

	return bRetValue;
}


/******************************************************************
Function Name:	IsNetfx461Installed
Description:	Uses the detection method recommended at
http://msdn.microsoft.com/en-us/library/ee942965(v=vs.110).aspx
to determine whether the .NET Framework 4.6.1 is
installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 4.6.1 is installed
false otherwise
******************************************************************/
bool IsNetfx461Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue = 0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx46RegKeyName, g_szNetfx46RegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (g_dwNetfx461ReleaseVersion <= dwRegValue || g_dwNetfx461Win10ReleaseVersion <= dwRegValue)
			bRetValue = true;
	}

	return bRetValue;
}


/******************************************************************
Function Name:	IsNetfx462Installed
Description:	Uses the detection method recommended at
http://msdn.microsoft.com/en-us/library/ee942965(v=vs.110).aspx
to determine whether the .NET Framework 4.6.2 is
installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 4.6.2 is installed
false otherwise
******************************************************************/
bool IsNetfx462Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue = 0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx46RegKeyName, g_szNetfx46RegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (g_dwNetfx462ReleaseVersion <= dwRegValue || g_dwNetfx462Win10ReleaseVersion <= dwRegValue)
			bRetValue = true;
	}

	return bRetValue;
}

/******************************************************************
Function Name:	IsNetfx47Installed
Description:	Uses the detection method recommended at
http://msdn.microsoft.com/en-us/library/ee942965(v=vs.110).aspx
to determine whether the .NET Framework 4.7 is
installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 4.7 is installed
false otherwise
******************************************************************/
bool IsNetfx47Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue = 0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx47RegKeyName, g_szNetfx47RegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (g_dwNetfx47ReleaseVersion <= dwRegValue || g_dwNetfx47Win10ReleaseVersion <= dwRegValue)
			bRetValue = true;
	}

	return bRetValue;
}


/******************************************************************
Function Name:	IsNetfx471Installed
Description:	Uses the detection method recommended at
http://msdn.microsoft.com/en-us/library/ee942965(v=vs.110).aspx
to determine whether the .NET Framework 4.7.1 is
installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 4.7.1 is installed
false otherwise
******************************************************************/
bool IsNetfx471Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue = 0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx47RegKeyName, g_szNetfx47RegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (g_dwNetfx471ReleaseVersion <= dwRegValue || g_dwNetfx471Win10ReleaseVersion <= dwRegValue)
			bRetValue = true;
	}

	return bRetValue;
}


/******************************************************************
Function Name:	IsNetfx472Installed
Description:	Uses the detection method recommended at
http://msdn.microsoft.com/en-us/library/ee942965(v=vs.110).aspx
to determine whether the .NET Framework 4.7.2 is
installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 4.7.2 is installed
false otherwise
******************************************************************/
bool IsNetfx472Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue = 0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx47RegKeyName, g_szNetfx47RegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (g_dwNetfx472ReleaseVersion <= dwRegValue || g_dwNetfx472Win10ReleaseVersion <= dwRegValue)
			bRetValue = true;
	}

	return bRetValue;
}


/******************************************************************
Function Name:	IsNetfx48Installed
Description:	Uses the detection method recommended at
http://msdn.microsoft.com/en-us/library/ee942965(v=vs.110).aspx
to determine whether the .NET Framework 4.8 is
installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 4.8 is installed
false otherwise
******************************************************************/
bool IsNetfx48Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue = 0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx48RegKeyName, g_szNetfx48RegValueName, NULL, (LPBYTE)& dwRegValue, sizeof(DWORD)))
	{
		if (g_dwNetfx48ReleaseVersion <= dwRegValue || g_dwNetfx48Win10ReleaseVersion <= dwRegValue)
			bRetValue = true;
	}

	return bRetValue;
}


/******************************************************************
Function Name:  RegistryGetValue
Description:    Get the value of a reg key
Inputs:         HKEY hk - The hk of the key to retrieve
				TCHAR *pszKey - Name of the key to retrieve
				TCHAR *pszValue - The value that will be retrieved
				DWORD dwType - The type of the value that will be retrieved
				LPBYTE data - A buffer to save the retrieved data
				DWORD dwSize - The size of the data retrieved
Results:        true if successful, false otherwise
******************************************************************/
bool RegistryGetValue(HKEY hk, const TCHAR * pszKey, const TCHAR * pszValue, DWORD dwType, LPBYTE data, DWORD dwSize)
{
	HKEY hkOpened;

	// Try to open the key
	if (RegOpenKeyEx(hk, pszKey, 0, KEY_READ, &hkOpened) != ERROR_SUCCESS)
	{
		return false;
	}

	// If the key was opened, try to retrieve the value
	if (RegQueryValueEx(hkOpened, pszValue, 0, &dwType, (LPBYTE)data, &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hkOpened);
		return false;
	}

	// Clean up
	RegCloseKey(hkOpened);

	return true;
}

//********************************************* NSIS Plugin Functions ****************************************************************************

//***************************************************** .NET 4.8 *********************************************************************************

extern "C"
void __declspec(dllexport) IsDotNet48Installed(HWND hwndParent, int string_size, TCHAR* variables, stack_t** stacktop, extra_parameters* extra)
{
	EXDLL_INIT();
	pushstring((IsNetfx48Installed()) ? L"true" : L"false");
}

extern "C"
void __declspec(dllexport) GetDotNet48ServicePack(HWND hwndParent, int string_size, TCHAR* variables, stack_t** stacktop, extra_parameters* extra)
{
	EXDLL_INIT();

	int iNetfx48SPLevel = -1;
	bool bNetfx48Installed = (IsNetfx48Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx40VersionString));
	TCHAR szMessage[MAX_PATH];
	TCHAR szOutputString[MAX_PATH * 20];

	if (bNetfx48Installed)
	{
		iNetfx48SPLevel = GetNetfxSPLevel(g_szNetfx46RegKeyName, g_szNetfx40SPxRegValueName);

		if (iNetfx48SPLevel > 0)
			pushint(iNetfx48SPLevel);
		else
			pushint(-1);
	}
	else
	{
		pushint(-2);
	}
}


//***************************************************** .NET 4.7.2 *******************************************************************************

extern "C"
void __declspec(dllexport) IsDotNet472Installed(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();
	pushstring((IsNetfx472Installed()) ? L"true" : L"false");
}

extern "C"
void __declspec(dllexport) GetDotNet472ServicePack(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	int iNetfx472SPLevel = -1;
	bool bNetfx472Installed = (IsNetfx472Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx40VersionString));
	TCHAR szMessage[MAX_PATH];
	TCHAR szOutputString[MAX_PATH * 20];

	if (bNetfx472Installed)
	{
		iNetfx472SPLevel = GetNetfxSPLevel(g_szNetfx46RegKeyName, g_szNetfx40SPxRegValueName);

		if (iNetfx472SPLevel > 0)
			pushint(iNetfx472SPLevel);
		else
			pushint(-1);
	}
	else
	{
		pushint(-2);
	}
}

//***************************************************** .NET 4.7.1 *******************************************************************************

extern "C"
void __declspec(dllexport) IsDotNet471Installed(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();
	pushstring((IsNetfx471Installed()) ? L"true" : L"false");
}

extern "C"
void __declspec(dllexport) GetDotNet471ServicePack(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	int iNetfx471SPLevel = -1;
	bool bNetfx471Installed = (IsNetfx471Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx40VersionString));
	TCHAR szMessage[MAX_PATH];
	TCHAR szOutputString[MAX_PATH * 20];

	if (bNetfx471Installed)
	{
		iNetfx471SPLevel = GetNetfxSPLevel(g_szNetfx46RegKeyName, g_szNetfx40SPxRegValueName);

		if (iNetfx471SPLevel > 0)
			pushint(iNetfx471SPLevel);
		else
			pushint(-1);
	}
	else
	{
		pushint(-2);
	}
}

//***************************************************** .NET 4.7 *********************************************************************************

extern "C"
void __declspec(dllexport) IsDotNet47Installed(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();
	pushstring((IsNetfx47Installed()) ? L"true" : L"false");
}

extern "C"
void __declspec(dllexport) GetDotNet47ServicePack(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	int iNetfx47SPLevel = -1;
	bool bNetfx47Installed = (IsNetfx47Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx40VersionString));
	TCHAR szMessage[MAX_PATH];
	TCHAR szOutputString[MAX_PATH * 20];

	if (bNetfx47Installed)
	{
		iNetfx47SPLevel = GetNetfxSPLevel(g_szNetfx47RegKeyName, g_szNetfx40SPxRegValueName);

		if (iNetfx47SPLevel > 0)
			pushint(iNetfx47SPLevel);
		else
			pushint(-1);
	}
	else
	{
		pushint(-2);
	}
}

//***************************************************** .NET 4.6.2 *******************************************************************************

extern "C"
void __declspec(dllexport) IsDotNet462Installed(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();
	pushstring((IsNetfx462Installed()) ? L"true" : L"false");
}

extern "C"
void __declspec(dllexport) GetDotNet462ServicePack(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	int iNetfx462SPLevel = -1;
	bool bNetfx462Installed = (IsNetfx462Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx40VersionString));
	TCHAR szMessage[MAX_PATH];
	TCHAR szOutputString[MAX_PATH * 20];

	if (bNetfx462Installed)
	{
		iNetfx462SPLevel = GetNetfxSPLevel(g_szNetfx46RegKeyName, g_szNetfx40SPxRegValueName);

		if (iNetfx462SPLevel > 0)
			pushint(iNetfx462SPLevel);
		else
			pushint(-1);
	}
	else
	{
		pushint(-2);
	}
}

//***************************************************** .NET 4.6.1 *******************************************************************************

extern "C"
void __declspec(dllexport) IsDotNet461Installed(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();
	pushstring((IsNetfx461Installed()) ? L"true" : L"false");
}

extern "C"
void __declspec(dllexport) GetDotNet461ServicePack(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	int iNetfx461SPLevel = -1;
	bool bNetfx461Installed = (IsNetfx461Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx40VersionString));
	TCHAR szMessage[MAX_PATH];
	TCHAR szOutputString[MAX_PATH * 20];

	if (bNetfx461Installed)
	{
		iNetfx461SPLevel = GetNetfxSPLevel(g_szNetfx46RegKeyName, g_szNetfx40SPxRegValueName);

		if (iNetfx461SPLevel > 0)
			pushint(iNetfx461SPLevel);
		else
			pushint(-1);
	}
	else
	{
		pushint(-2);
	}
}

//***************************************************** .NET 4.6 *********************************************************************************

extern "C"
void __declspec(dllexport) IsDotNet46Installed(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();
	pushstring((IsNetfx46Installed()) ? L"true" : L"false");
}

extern "C"
void __declspec(dllexport) GetDotNet46ServicePack(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	int iNetfx46SPLevel = -1;
	bool bNetfx46Installed = (IsNetfx46Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx40VersionString));
	TCHAR szMessage[MAX_PATH];
	TCHAR szOutputString[MAX_PATH * 20];

	if (bNetfx46Installed)
	{
		iNetfx46SPLevel = GetNetfxSPLevel(g_szNetfx46RegKeyName, g_szNetfx40SPxRegValueName);

		if (iNetfx46SPLevel > 0)
			pushint(iNetfx46SPLevel);
		else
			pushint(-1);
	}
	else
	{
		pushint(-2);
	}
}

//***************************************************** .NET 4.5.2 *******************************************************************************

extern "C"
void __declspec(dllexport) IsDotNet452Installed(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();
	pushstring((IsNetfx452Installed()) ? L"true" : L"false");
}

extern "C"
void __declspec(dllexport) GetDotNet452ServicePack(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	int iNetfx452SPLevel = -1;
	bool bNetfx452Installed = (IsNetfx452Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx40VersionString));
	TCHAR szMessage[MAX_PATH];
	TCHAR szOutputString[MAX_PATH * 20];

	if (bNetfx452Installed)
	{
		iNetfx452SPLevel = GetNetfxSPLevel(g_szNetfx45RegKeyName, g_szNetfx40SPxRegValueName);

		if (iNetfx452SPLevel > 0)
			pushint(iNetfx452SPLevel);
		else
			pushint(-1);
	}
	else
	{
		pushint(-2);
	}
}

//***************************************************** .NET 4.5.1 *******************************************************************************

extern "C"
void __declspec(dllexport) IsDotNet451Installed(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();
	pushstring((IsNetfx451Installed()) ? L"true" : L"false");
}

extern "C"
void __declspec(dllexport) GetDotNet451ServicePack(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	int iNetfx451SPLevel = -1;
	bool bNetfx451Installed = (IsNetfx451Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx40VersionString));
	TCHAR szMessage[MAX_PATH];
	TCHAR szOutputString[MAX_PATH*20];

	if (bNetfx451Installed)
	{
		iNetfx451SPLevel = GetNetfxSPLevel(g_szNetfx45RegKeyName, g_szNetfx40SPxRegValueName);

		if (iNetfx451SPLevel > 0)
			pushint(iNetfx451SPLevel);
		else
			pushint(-1);
	}
	else
	{
		pushint(-2);
	}
}

//************************************************* .NET 4.5 *************************************************************************************
extern "C"
void __declspec(dllexport) IsDotNet45Installed(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();
	pushstring(IsNetfx45Installed() ? L"true" : L"false");
}

extern "C"
void __declspec(dllexport) GetDotNet45ServicePack(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	int iNetfx45SPLevel = -1;
	bool bNetfx45Installed = (IsNetfx45Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx40VersionString));
	TCHAR szMessage[MAX_PATH];
	TCHAR szOutputString[MAX_PATH*20];

	if (bNetfx45Installed)
	{
		iNetfx45SPLevel = GetNetfxSPLevel(g_szNetfx45RegKeyName, g_szNetfx40SPxRegValueName);

		if (iNetfx45SPLevel > 0)
			pushint(iNetfx45SPLevel);
		else
			pushint(-1);
	}
	else
	{
		pushint(-2);
	}
}

//************************************************* .NET 4.0 Full ********************************************************************************
extern "C"
void __declspec(dllexport) IsDotNet40FullInstalled(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();
	pushstring((IsNetfx40FullInstalled() && CheckNetfxVersionUsingMscoree(g_szNetfx40VersionString)) ? L"true" : L"false");
}

extern "C"
void __declspec(dllexport) GetDotNet40FullServicePack(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	int iNetfx40FullSPLevel = -1;
	bool bNetfx40FullInstalled = (IsNetfx40FullInstalled() && CheckNetfxVersionUsingMscoree(g_szNetfx40VersionString));
	TCHAR szMessage[MAX_PATH];
	TCHAR szOutputString[MAX_PATH*20];

	if (bNetfx40FullInstalled)
	{
		iNetfx40FullSPLevel = GetNetfxSPLevel(g_szNetfx40FullRegKeyName, g_szNetfx40SPxRegValueName);

		if (iNetfx40FullSPLevel > 0)
			pushint(iNetfx40FullSPLevel);
		else
			pushint(-1);
	}
	else
	{
		pushint(-2);
	}
}

//************************************************* .NET 4.0 Client ******************************************************************************
extern "C"
void __declspec(dllexport) IsDotNet40ClientInstalled(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();
	pushstring((IsNetfx40ClientInstalled() && CheckNetfxVersionUsingMscoree(g_szNetfx40VersionString)) ? L"true" : L"false");
}

extern "C"
void __declspec(dllexport) GetDotNet40ClientServicePack(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	int iNetfx40ClientSPLevel = -1;
	bool bNetfx40ClientInstalled = (IsNetfx40ClientInstalled() && CheckNetfxVersionUsingMscoree(g_szNetfx40VersionString));
	TCHAR szMessage[MAX_PATH];
	TCHAR szOutputString[MAX_PATH*20];

	if (bNetfx40ClientInstalled)
	{
		iNetfx40ClientSPLevel = GetNetfxSPLevel(g_szNetfx40FullRegKeyName, g_szNetfx40SPxRegValueName);

		if (iNetfx40ClientSPLevel > 0)
			pushint(iNetfx40ClientSPLevel);
		else
			pushint(-1);
	}
	else
	{
		pushint(-2);
	}
}

//***************************************************** .NET 3.5 *********************************************************************************
extern "C"
void __declspec(dllexport) IsDotNet35Installed(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	// The .NET Framework 3.5 is an add-in that installs
	// on top of the .NET Framework 2.0 and 3.0.  For this version
	// check, validate that 2.0, 3.0 and 3.5 are installed.
	pushstring((IsNetfx20Installed() && IsNetfx30Installed() && IsNetfx35Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx20VersionString)) ? L"true" : L"false");
}

extern "C"
void __declspec(dllexport) GetDotNet35ServicePack(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	int iNetfx35SPLevel = -1;
	bool bNetfx35Installed = (IsNetfx20Installed() && IsNetfx30Installed() && IsNetfx35Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx20VersionString));
	TCHAR szMessage[MAX_PATH];
	TCHAR szOutputString[MAX_PATH*20];

	if (bNetfx35Installed)
	{
		iNetfx35SPLevel = GetNetfxSPLevel(g_szNetfx35RegKeyName, g_szNetfxStandardSPxRegValueName);

		if (iNetfx35SPLevel > 0)
			pushint(iNetfx35SPLevel);
		else
			pushint(-1);
	}
	else
	{
		pushint(-2);
	}
}

//***************************************************** .NET 3.0 *********************************************************************************
extern "C"
void __declspec(dllexport) IsDotNet30Installed(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	// The .NET Framework 3.0 is an add-in that installs
	// on top of the .NET Framework 2.0.  For this version
	// check, validate that both 2.0 and 3.0 are installed.
	pushstring((IsNetfx20Installed() && IsNetfx30Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx20VersionString)) ? L"true" : L"false");
}

extern "C"
void __declspec(dllexport) GetDotNet30ServicePack(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	int iNetfx30SPLevel = -1;
	bool bNetfx30Installed = (IsNetfx20Installed() && IsNetfx30Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx20VersionString));
	TCHAR szMessage[MAX_PATH];
	TCHAR szOutputString[MAX_PATH*20];

	if (bNetfx30Installed)
	{
		iNetfx30SPLevel = GetNetfxSPLevel(g_szNetfx30SpRegKeyName, g_szNetfxStandardSPxRegValueName);

		if (iNetfx30SPLevel > 0)
			pushint(iNetfx30SPLevel);
		else
			pushint(-1);
	}
	else
	{
		pushint(-2);
	}
}

//***************************************************** .NET 2.0 *********************************************************************************
extern "C"
void __declspec(dllexport) IsDotNet20Installed(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();
	pushstring((IsNetfx20Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx20VersionString)) ? L"true" : L"false");
}

extern "C"
void __declspec(dllexport) GetDotNet20ServicePack(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	int iNetfx20SPLevel = -1;
	bool bNetfx20Installed = (IsNetfx20Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx20VersionString));
	TCHAR szMessage[MAX_PATH];
	TCHAR szOutputString[MAX_PATH*20];

	if (bNetfx20Installed)
	{
		iNetfx20SPLevel = GetNetfxSPLevel(g_szNetfx20RegKeyName, g_szNetfxStandardSPxRegValueName);

		if (iNetfx20SPLevel > 0)
			pushint(iNetfx20SPLevel);
		else
			pushint(-1);
	}
	else
	{
		pushint(-2);
	}
}

//***************************************************** .NET 1.1 *********************************************************************************
extern "C"
void __declspec(dllexport) IsDotNet11Installed(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();
	pushstring((IsNetfx11Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx11VersionString)) ? L"true" : L"false");
}

extern "C"
void __declspec(dllexport) GetDotNet11ServicePack(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	int iNetfx11SPLevel = -1;
	bool bNetfx11Installed = (IsNetfx11Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx11VersionString));
	TCHAR szMessage[MAX_PATH];
	TCHAR szOutputString[MAX_PATH*20];

	if (bNetfx11Installed)
	{
		iNetfx11SPLevel = GetNetfxSPLevel(g_szNetfx11RegKeyName, g_szNetfxStandardSPxRegValueName);

		if (iNetfx11SPLevel > 0)
			pushint(iNetfx11SPLevel);
		else
			pushint(-1);
	}
	else
	{
		pushint(-2);
	}
}

//***************************************************** .NET 1.0 *********************************************************************************
extern "C"
void __declspec(dllexport) IsDotNet10Installed(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();
	pushstring((IsNetfx11Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx11VersionString)) ? L"true" : L"false");
}

extern "C"
void __declspec(dllexport) GetDotNet10ServicePack(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	EXDLL_INIT();

	int iNetfx10SPLevel = -1;
	bool bNetfx10Installed = (IsNetfx10Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx10VersionString));
	TCHAR szMessage[MAX_PATH];
	TCHAR szOutputString[MAX_PATH*20];

	if (bNetfx10Installed)
	{
		iNetfx10SPLevel = GetNetfx10SPLevel();

		if (iNetfx10SPLevel > 0)
			pushint(iNetfx10SPLevel);
		else
			pushint(-1);
	}
	else
	{
		pushint(-2);
	}
}
