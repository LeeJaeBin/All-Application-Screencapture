#include "stdafx.h"
#include "AeroDrawTest.h"
#include "AeroDrawTestDlg.h"
#include "CxImage/ximage.h"
#include  <Dwmapi.h>
#include "CWindowCapture.h"
#include "SystemInfo.h"

#pragma comment(lib, "Dwmapi.lib")
#include "tlhelp32.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HWND SetFromPidtoHnd(DWORD dwPid, int nType);
int CheckRunProcess(char* filename);
bool AdjustDebugPrivilege(HANDLE hProcess, HANDLE * phProcessToken, TOKEN_PRIVILEGES* pstOld, DWORD* pdwOldCount);


extern RECT	m_rcFull;

#define NOUI			"Except"
#define NOUI2			"NoHandle"
#define DESKTOP_HANDLE 0

int g_nCount = 0;
int g_nIECount = 0;

void CAeroDrawTestDlg::SaveImageIE(HWND hnd)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	char  szBuffer[MAX_PATH] = "", szTemp[MAX_PATH] = "";
	HWND	hParent=NULL;
	HWND	myHwnd = NULL;
	char	szPath[100] = "";
	HWND	hParentHwnd = NULL;


	if(::IsWindow(hnd) == FALSE)
		return;

//	hParentHwnd = CWnd::FromHandle(hnd)->GetTopLevelParent()->GetSafeHwnd(); 
	hParentHwnd = hnd;

	sprintf(szPath, "c:\\temp\\IE-%d.bmp", g_nIECount);
	g_nIECount++;

	m_vRect.clear();
	m_rcFull.top = 0;
	m_rcFull.bottom = 0;
	m_rcFull.left = 0;
	m_rcFull.right = 0;
	//memset(m_rcFull, 0, sizeof(m_rcFull));
	::EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&m_vRect);

	CString strTemp;
	int ni = 0;
	//strTemp.Format(_T("%d번째 워터마크 rect(%d, %d, %d, %d)"), i, m_vRect[i].top, m_vRect[i].bottom, m_vRect[i].left, m_vRect[i].right);
	//strTemp.Format(_T("%d번째 워터마크 rect(%d, %d, %d, %d)"), i, m_vRect[i].top, m_vRect[i].bottom, m_vRect[i].left, m_vRect[i].right);

	m_nHeight = m_vRect[0].bottom - m_vRect[0].top;
	m_nWidth = m_vRect[0].right - m_vRect[0].left;
	HWND	CurHwnd = NULL;

	//	CurHwnd = this->m_hWnd;

	CurHwnd = hParentHwnd;
	//	hParentHwnd = CWnd::FromHandle(g_OLStr.hHwnd)->GetTopLevelParent()->GetSafeHwnd(); 
	if(CurHwnd == NULL)
	{
		char szCount [30] = "";

		sprintf(szCount, "%d",g_nCount);

		WritePrivateProfileString( NOUI, szCount, szPath, "c:\\temp\\NIui.ini");
		g_nCount++;
		return;
	}

	if(::IsWindow(CurHwnd) == FALSE || ::IsWindowVisible(CurHwnd) == FALSE)
	{
		char szCount [30] = "";

		sprintf(szCount, "%d",g_nCount);

		WritePrivateProfileString( NOUI2, szCount, szPath, "c:\\temp\\NIui.ini");
		g_nCount++;
		return;
	}

	CRect			Protectrect;
	::GetWindowRect(CurHwnd, &Protectrect);	
	if(Protectrect.Width() < 5)
	{
		char szCount [30] = "";

		sprintf(szCount, "%d",g_nCount);

		WritePrivateProfileString( NOUI2, szCount, szPath, "c:\\temp\\NIui.ini");
		g_nCount++;
		return;
	}


	TRACE("%s Top %d,%d Bottom %d %d\n",szPath, Protectrect.top, Protectrect.left, Protectrect.bottom, Protectrect.right);

	HDC hTmwDC = ::GetDC(CurHwnd);

	HDC hDC = ::GetDC(NULL);

	HDC hMemDC = ::CreateCompatibleDC(hDC);
	HBITMAP hMemBitmap = CreateCompatibleBitmap(hDC, m_nWidth, m_nHeight);
	HBITMAP hOldMemBitmap = (HBITMAP)::SelectObject(hMemDC, hMemBitmap);

	::BitBlt(hMemDC, 0, 0, m_nWidth, m_nHeight, hTmwDC, 0, 0, SRCCOPY);

	CxImage xImage;
	xImage.CreateFromHBITMAP(hMemBitmap);
	xImage.Save(szPath, CXIMAGE_SUPPORT_BMP);

	::SelectObject(hMemDC, hOldMemBitmap);
	::DeleteObject(hMemBitmap);
	::DeleteDC(hMemDC);
	::ReleaseDC(NULL, hDC);
	::ReleaseDC(CurHwnd, hTmwDC);
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    int *displayNum = (int*)dwData;
    (*displayNum)++;
    return TRUE;
}

BOOL SaveBitmap(HWND CurHwnd, HBITMAP hMemBitmap){
	char	szPath[100] = "";
	sprintf(szPath, "c:\\temp\\%d.bmp", (int)CurHwnd);

	HDC hDC = ::GetDC(NULL);

	HDC hMemDC = ::CreateCompatibleDC(hDC);

	::SelectObject(hMemDC, hMemBitmap);

	CxImage xImage;
	xImage.CreateFromHBITMAP(hMemBitmap);
	xImage.Save(szPath, CXIMAGE_SUPPORT_BMP);
	return TRUE;

	::DeleteObject(hMemBitmap);
	::DeleteDC(hMemDC);
	::ReleaseDC(NULL, hDC);

	return TRUE;
}

// 모든 창을 구하기 위해 Child Process를 검색
BOOL CALLBACK EnumChildProc(HWND CurHwnd, LPARAM lParam){
	CWindowCapture pCapture;
	HBITMAP hMemBitmap = NULL;
	hMemBitmap = pCapture.FindBitmap(CurHwnd, 0);
	if(hMemBitmap != NULL)
		SaveBitmap(CurHwnd, hMemBitmap);

	if(pCapture.count>lParam){
		return FALSE;
	}
	return TRUE;
}

// Top Window만 검색
void CAeroDrawTestDlg::SaveDlgImage(int nIndex, DWORD dwPid)
{	
	int displayNum = m_ProcessCombo.GetCount();
	CWindowCapture pCapture;

	HWND	CurHwnd = NULL;
	CurHwnd = SetFromPidtoHnd(dwPid, 0);

	HBITMAP hMemBitmap = pCapture.FindBitmap(CurHwnd, 1);

	if(hMemBitmap != NULL)
		SaveBitmap(CurHwnd, hMemBitmap);

}


BOOL AdhustDebugPriviege(HANDLE hProcess, HANDLE *phProcessToken, TOKEN_PRIVILEGES *pstOld, DWORD *pdwOldCount)
{
	TOKEN_PRIVILEGES stNew;

	if (OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, phProcessToken) == FALSE) 
	{
		return FALSE;
	}

	if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &(stNew.Privileges[0].Luid)) == FALSE)
	{
		return FALSE;
	}

	stNew.PrivilegeCount = 1;
	stNew.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (AdjustTokenPrivileges(*phProcessToken, FALSE, &stNew, sizeof(TOKEN_PRIVILEGES), pstOld, pdwOldCount) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL RestorePrivilege(HANDLE hProcessToken, TOKEN_PRIVILEGES *pstOld, DWORD pdwOldCount)
{
	if (AdjustTokenPrivileges(hProcessToken, FALSE, pstOld, pdwOldCount, NULL, NULL) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

char *GetProcessUsername(HANDLE hProcess, BOOL bIncDomain) 
{
	HANDLE tok = 0;	
	DWORD nlen, dlen;
	int iUse;
	char name[MAX_PATH] = {0, }, dom[MAX_PATH] = {0, }, tubuf[MAX_PATH] = {0, };

	if (hProcess == NULL)
	{
		return NULL;
	}

	HANDLE ProcessToken = NULL;
	TOKEN_PRIVILEGES oldTokenPrivileges = { 0 };
	DWORD dwOldTPLen;

	if (AdhustDebugPriviege(GetCurrentProcess(), &ProcessToken, &oldTokenPrivileges, &dwOldTPLen) == FALSE)
	{
		return NULL;
	}

	//open the processes token
	if (OpenProcessToken(hProcess, TOKEN_QUERY, &tok) == FALSE) 
	{
		return NULL;
	}

	//get the SID of the token
	TOKEN_USER *ptu = (TOKEN_USER*)tubuf;
	if (!GetTokenInformation(tok, TokenUser, ptu, MAX_PATH, &nlen)) 
	{
		return NULL;
	}

	//get the account/domain name of the SID
	dlen = MAX_PATH;
	nlen = MAX_PATH;
	if (!LookupAccountSid(0, ptu->User.Sid, name, &nlen, dom, &dlen, (PSID_NAME_USE)&iUse)) 
	{
		return NULL;
	}

	char *sname = new char[MAX_PATH];
	ZeroMemory(sname, MAX_PATH);

	//copy info to our static buffer
	if (dlen && bIncDomain) 
	{
		//	strcpy(sname,dom);
		//	strcat(sname,"");

		strcat(sname,name);
	}
	else
	{
		strcpy(sname,name);
	}

	//set our return variable

	if (tok != NULL)
	{
		CloseHandle(tok);
	}

	RestorePrivilege(ProcessToken, &oldTokenPrivileges, dwOldTPLen);
	return sname;
}



void CAeroDrawTestDlg::SetProcess() 
{
	// TODO: Add your control notification handler code here
	LPPROCESSINFO pTemp = NULL;
	for (int i = 0 ; i < m_arrProcessInfo.GetSize() ; ++i)
	{
		pTemp = m_arrProcessInfo.GetAt(i);
		delete pTemp;
		pTemp = NULL;
	}
	m_arrProcessInfo.RemoveAll();

	BOOL bIsSystem = FALSE;

	LoadProcessInfo(bIsSystem);

	SortProcessInfo();

	m_ProcessCombo.ResetContent();
	for (int i = 0 ; i < m_arrProcessInfo.GetSize() ; ++i)
	{
		m_ProcessCombo.AddString(m_arrProcessInfo[i]->szName);
	}

	int displayNum = m_arrProcessInfo.GetSize();

	m_ProcessCombo.SetCurSel(m_arrProcessInfo.GetSize() - 1);
	CString strPID;
	strPID.Format(_T("%d"), m_arrProcessInfo.GetAt(m_arrProcessInfo.GetSize() - 1)->nPid);

	// Desktop의 Child 전체를 검색
	::EnumChildWindows(DESKTOP_HANDLE, EnumChildProc, displayNum);
}

void CAeroDrawTestDlg::LoadProcessInfo(BOOL bIsSystem)
{
	// 실행 중인 프로세스 정보를 구하기 위한 스냅샷 HANDLE을 가져옴
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);


	LPPROCESSINFO pProcessInfo = NULL;

	if (hSnapshot)
	{
		// 첫 프로세스 검색
		PROCESSENTRY32 ProcessEntry32;
		ProcessEntry32.dwSize = sizeof(PROCESSENTRY32);
		BOOL bProcessFound=Process32First(hSnapshot, &ProcessEntry32);
		BOOL bIsAdd = TRUE;
		while (bProcessFound)
		{
			if (bIsSystem == FALSE)
			{
				// 프로세스 아이디를 이용하여 해당 프로세스 핸들 값을 얻는다.
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, ProcessEntry32.th32ProcessID);

				char *szUserName = GetProcessUsername(hProcess, FALSE);
				if (szUserName == NULL ||
					strcmp(szUserName, "LOCAL SERVICE")		== 0 ||
					strcmp(szUserName, "NETWORK SERVICE")	== 0 ||
					strcmp(szUserName, "SYSTEM")			== 0)
				{
					bIsAdd = FALSE;	
				}
				delete []szUserName;
			}

			if (bIsAdd == TRUE)
			{
				pProcessInfo = new PROCESSINFO;
				memset(pProcessInfo, 0x00, sizeof(PROCESSINFO));

				pProcessInfo->nPid = ProcessEntry32.th32ProcessID;
				memcpy(pProcessInfo->szName, ProcessEntry32.szExeFile, strlen(ProcessEntry32.szExeFile));

				m_arrProcessInfo.Add(pProcessInfo);
				pProcessInfo = NULL;
			}
			bIsAdd = TRUE;

			// 다음 프로세스 검색
			bProcessFound = Process32Next(hSnapshot, &ProcessEntry32);
		}

		// 스냅샷 핸들 종료
		CloseHandle(hSnapshot);
	}
}


void CAeroDrawTestDlg::SortProcessInfo()
{
	int nFind = -1;
	CString strMin;
	CString strFileName;
	// 파일 정렬
	for (int i = 0 ; i < m_arrProcessInfo.GetSize() - 1 ; ++i)
	{
		strMin.Format(_T("%s"), m_arrProcessInfo[i]->szName);

		for (int j = i+1 ; j < m_arrProcessInfo.GetSize() ; ++j)
		{
			strFileName.Format(_T("%s"), m_arrProcessInfo[j]->szName);
			if (strMin.CompareNoCase(strFileName) > 0)
			{
				strMin = strFileName;
				nFind = j;
			}
		}

		if (nFind > -1)
		{
			LPPROCESSINFO pTemp = m_arrProcessInfo[i];
			m_arrProcessInfo[i] = m_arrProcessInfo[nFind];
			m_arrProcessInfo[nFind] = pTemp;
			nFind = -1;
		}
	}
}


int CheckRunProcess(char* filename)
{
	HANDLE         hProcessSnap = NULL; 
	BOOL           bRet      = FALSE; 
	PROCESSENTRY32 pe32      = {0};


	//  Take a snapshot of all processes in the system. 

#if 1
	TOKEN_PRIVILEGES stOld;
	ZeroMemory(&stOld,sizeof(TOKEN_PRIVILEGES));
	HANDLE hProcessToken = NULL;
	DWORD dwOldCount = 0;

	if(!AdjustDebugPrivilege(GetCurrentProcess(), &hProcessToken, &stOld, &dwOldCount))
	{
		//		AfxMessageBox(_T("Failed Elevation to OpenProcess"));
		CloseHandle( hProcessSnap );
		hProcessSnap = NULL;
		CloseHandle(hProcessToken);
		hProcessToken = NULL;
		return bRet;
	}

	//RestorePrivilege(hProcessToken, &stOld, dwOldCount);

	CloseHandle(hProcessToken);
	hProcessToken = NULL;
#endif 	


	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 

	if (hProcessSnap == INVALID_HANDLE_VALUE) 
		return bRet; 

	//  Fill in the size of the structure before using it. 

	pe32.dwSize = sizeof(PROCESSENTRY32); 

	//  Walk the snapshot of the processes, and for each process, 
	//  display information. 

	if (Process32First(hProcessSnap, &pe32)) 
	{ 
		DWORD         dwPriorityClass = 0; 
		BOOL          bGotModule = FALSE; 
		MODULEENTRY32 me32       = {0}; 

		do 
		{ 
			//bGotModule = GetProcessFullPath (
			//	pe32.th32ProcessID, 
			//             pe32.th32ModuleID,
			//	&me32,
			//	sizeof(MODULEENTRY32) ); 


			CString strname = pe32.szExeFile;
			strname.MakeLower();
			if(strname == (CString)filename)
			{
				CloseHandle (hProcessSnap); 
				return pe32.th32ProcessID;//ERR_RUN_CHECK_PROCESS;
			}

		} 
		while (Process32Next(hProcessSnap, &pe32)); 
	} 
	else 
		bRet = ERR_NONE;    // could not walk the list of processes 

	// Do not forget to clean up the snapshot object. 
	CloseHandle (hProcessSnap); 
	return bRet; 

}

HWND SetFromPidtoHnd(DWORD dwPid, int nType)
{
	HWND	hParent=NULL;
	HWND	myHwnd = NULL;

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	BOOL fForceIconic = TRUE;
	BOOL fHasIconicBitmap = TRUE;


	if(dwPid)
	{
		SystemWindowInformation wi( dwPid, TRUE );

		if ( wi.m_WindowInfos.GetHeadPosition() == NULL )
		{
			return NULL;
		}

		for ( POSITION pos = wi.m_WindowInfos.GetHeadPosition(); pos != NULL; )
		{
			SystemWindowInformation::WINDOW_INFO& w = wi.m_WindowInfos.GetNext(pos);

			//TRACE("caption %s \n", w.Caption);
			if(dwPid == wi.m_processId)
			{
				myHwnd = w.hWnd;
				if(myHwnd == NULL)
					continue;


				if(IsWindow(myHwnd) == FALSE || ::IsWindowVisible(myHwnd ) == FALSE)
					continue;

				hParent = ::GetParent(myHwnd);

				if(hParent == NULL)
					hParent = myHwnd;

				if(nType == 1)
				{
					// Aero 가 나타날때 해당 윈도우의 이벤트 전달 요청 등록
					DwmSetWindowAttribute(hParent, 7, &fForceIconic, sizeof(fForceIconic)); // DWMWA_FORCE_ICONIC_REPRESENTATION
					DwmSetWindowAttribute(hParent, 10, &fHasIconicBitmap, sizeof(fHasIconicBitmap)); // DWMWA_HAS_ICONIC_BITMAP
				}

				break;
			}
		}
	}
	return hParent;
}


bool AdjustDebugPrivilege(HANDLE hProcess, HANDLE * phProcessToken, TOKEN_PRIVILEGES* pstOld, DWORD* pdwOldCount)
{
	TOKEN_PRIVILEGES stNew;
	if(!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, phProcessToken))
	{
		//LOG_ERROR2(0, ::GetLastError());
		return FALSE;
	}

	if(!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &(stNew.Privileges[0].Luid)))
	{
		//LOG_ERROR2(0, ::GetLastError());
		return FALSE;
	}

	stNew.PrivilegeCount = 1;
	stNew.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if(!AdjustTokenPrivileges(*phProcessToken, FALSE, &stNew, sizeof(TOKEN_PRIVILEGES), pstOld, pdwOldCount))
	{
		//LOG_ERROR2(0, ::GetLastError());
		return FALSE;
	} 
	return TRUE;
}