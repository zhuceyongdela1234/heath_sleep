

#include "StdAfx.h"
#include <time.h>
void Init(); 
BOOL IsInstalled(); 
BOOL Install(); 
BOOL Uninstall(); 
void LogEvent(LPCTSTR pszFormat, ...); 
void WINAPI ServiceMain(); 
void WINAPI ServiceStrl(DWORD dwOpcode); 
 
TCHAR szServiceName[] = _T("Health_Sleep"); 
BOOL bInstall; 
SERVICE_STATUS_HANDLE hServiceStatus; 
SERVICE_STATUS status; 
DWORD dwThreadID; 
 
void __cdecl _tmain(int argc, TCHAR *argv[]) 
{ 
    Init(); 
 
    dwThreadID = ::GetCurrentThreadId(); 
 
    SERVICE_TABLE_ENTRY st[] = 
    { 
        { szServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain }, 
        { NULL, NULL } 
    }; 
 
    if (strcmp(argv[1], "/install") == 0) 
        { 
           Install(); 
        } 
    else if (strcmp(argv[1], "/uninstall") == 0) 
        { 
            Uninstall(); 
        } 
    else 
        { 
         if (!::StartServiceCtrlDispatcher(st)) 
        { 
            LogEvent(_T("Register Service Main Function Error!")); 
        } 
      } 
 
    //return 0; 
} 
//*********************************************************  
//Functiopn:            Init  
//Description:          ��ʼ��  
//Calls:                main          
//          <author>niying <time>2006-8-10      <version>     <desc>  
//*********************************************************  
void Init() 
{ 
    hServiceStatus = NULL; 
    status.dwServiceType = SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS; 
    status.dwCurrentState = SERVICE_START_PENDING; 
    status.dwControlsAccepted = SERVICE_ACCEPT_STOP; 
    status.dwWin32ExitCode = 0; 
    status.dwServiceSpecificExitCode = 0; 
    status.dwCheckPoint = 0; 
    status.dwWaitHint = 0; 
} 
 
//*********************************************************  
//Functiopn:            ServiceMain  
//Description:          ��������������������п��ƶԷ�����Ƶ�ע��  
//          <author>niying <time>2006-8-10      <version>     <desc>  
//*********************************************************  
int myshut( void )

{

    time_t t = time( 0 );

    char tmp[64];

    //strftime( tmp, sizeof(tmp), "%Y/%m/%d %X %A �����%j�� %z",localtime(&t) ); 
	//�����ļ�
	//��ȡ�ļ� 
	HANDLE hFile;  
    DWORD nBytes;
	char line[256]={0};  
    BOOL bResult;
	BOOL sleepneed=0;
	int i;
    hFile = CreateFileA(("c:\\time.ini"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS,  0,NULL );  
    if( hFile != INVALID_HANDLE_VALUE )  
    {  
         
        bResult = ReadFile(hFile,   
            line,   
            sizeof(line),   
            &nBytes,   
            NULL) ;   
	} 
	CloseHandle(hFile);  
	strftime( tmp, sizeof(tmp), "%X",localtime(&t) );//21:07:36
	for(i=0;i<4;i++){
		if(line[i]!=tmp[i])
		{
			sleepneed=0;
			break;
		}
	}
	if(i==5)
	{
		sleepneed=1;
	}
	if(sleepneed==1)
	{
		system("shutdown /s /t 45");
	}	
    puts( tmp ); 

    return 0;

}
//
void WINAPI ServiceMain() 
{ 
    // Register the control request handler  
    status.dwCurrentState = SERVICE_START_PENDING; 
    status.dwControlsAccepted = SERVICE_ACCEPT_STOP; 
 
    //ע��������  
    hServiceStatus = RegisterServiceCtrlHandler(szServiceName, ServiceStrl); 
    if (hServiceStatus == NULL) 
    { 
        LogEvent(_T("Handler not installed")); 
        return; 
    } 
    SetServiceStatus(hServiceStatus, &status); 
 
    status.dwWin32ExitCode = S_OK; 
    status.dwCheckPoint = 0; 
    status.dwWaitHint = 0; 
    status.dwCurrentState = SERVICE_RUNNING; 
    SetServiceStatus(hServiceStatus, &status); 
 
    //ģ���������С�Ӧ��ʱ����Ҫ������ڴ˼���  
        //���ڴ�д�Ϸ�����Ҫִ�еĴ��룬һ��Ϊ��ѭ��  
      while(1) 
    { 
        myshut();
		Sleep(20); //ѭ����ʲô  
     }     
    status.dwCurrentState = SERVICE_STOPPED; 
    SetServiceStatus(hServiceStatus, &status); 
    OutputDebugString(_T("Service stopped")); 
} 
 
//*********************************************************  
//Functiopn:            ServiceStrl  
//Description:          �������������������ʵ�ֶԷ���Ŀ��ƣ�  
//                      ���ڷ����������ֹͣ����������ʱ���������д˴�����  
//Calls:  
//Called By:  
//Table Accessed:  
//Table Updated:  
//Input:                dwOpcode�����Ʒ����״̬  
//Output:  
//Return:  
//Others:  
//History:  
//          <author>niying <time>2006-8-10      <version>     <desc>  
//*********************************************************  
void WINAPI ServiceStrl(DWORD dwOpcode) 
{ 
    switch (dwOpcode) 
    { 
    case SERVICE_CONTROL_STOP: 
        status.dwCurrentState = SERVICE_STOP_PENDING; 
        SetServiceStatus(hServiceStatus, &status); 
        PostThreadMessage(dwThreadID, WM_CLOSE, 0, 0); 
        break; 
    case SERVICE_CONTROL_PAUSE: 
        break; 
    case SERVICE_CONTROL_CONTINUE: 
        break; 
    case SERVICE_CONTROL_INTERROGATE: 
        break; 
    case SERVICE_CONTROL_SHUTDOWN: 
        break; 
    default: 
        LogEvent(_T("Bad service request")); 
        OutputDebugString(_T("Bad service request")); 
    } 
} 
//*********************************************************  
//Functiopn:            IsInstalled  
//Description:          �жϷ����Ƿ��Ѿ�����װ  
//          <author>niying <time>2006-8-10      <version>     <desc>  
//*********************************************************  
BOOL IsInstalled() 
{ 
    BOOL bResult = FALSE; 
 
    //�򿪷�����ƹ�����  
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS); 
 
    if (hSCM != NULL) 
    { 
        //�򿪷���  
        SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_QUERY_CONFIG); 
        if (hService != NULL) 
        { 
            bResult = TRUE; 
            ::CloseServiceHandle(hService); 
        }    
        ::CloseServiceHandle(hSCM); 
    } 
    return bResult; 
} 
 
//*********************************************************  
//Functiopn:            Install  
//Description:          ��װ������  
//          <author>niying <time>2006-8-10      <version>     <desc>  
//*********************************************************  
BOOL Install() 
{ 
    if (IsInstalled()) 
        return TRUE; 
 
    //�򿪷�����ƹ�����  
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS); 
    if (hSCM == NULL) 
    { 
        MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK); 
        return FALSE; 
    } 
 
    // Get the executable file path  
    TCHAR szFilePath[MAX_PATH]; 
    ::GetModuleFileName(NULL, szFilePath, MAX_PATH); 
 
    //��������  
    SC_HANDLE hService = ::CreateService(hSCM, szServiceName, szServiceName, 
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS ,SERVICE_AUTO_START , SERVICE_ERROR_NORMAL, 
        szFilePath, NULL, NULL, _T(""), NULL, NULL); 
 
    if (hService == NULL) 
    { 
        ::CloseServiceHandle(hSCM); 
        MessageBox(NULL, _T("Couldn't create service"), szServiceName, MB_OK); 
        return FALSE; 
    } 
 
    ::CloseServiceHandle(hService); 
    ::CloseServiceHandle(hSCM); 
    return TRUE; 
} 
 
//*********************************************************  
//Functiopn:            Uninstall  
//Description:          ɾ��������  
//          <author>niying <time>2006-8-10      <version>     <desc>  
//*********************************************************  
BOOL Uninstall() 
{ 
    if (!IsInstalled()) 
        return TRUE; 
 
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS); 
 
    if (hSCM == NULL) 
    { 
        MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK); 
        return FALSE; 
    } 
 
    SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_STOP | DELETE); 
 
    if (hService == NULL) 
    { 
        ::CloseServiceHandle(hSCM); 
        MessageBox(NULL, _T("Couldn't open service"), szServiceName, MB_OK); 
        return FALSE; 
    } 
    SERVICE_STATUS status; 
    ::ControlService(hService, SERVICE_CONTROL_STOP, &status); 
 
    //ɾ������  
    BOOL bDelete = ::DeleteService(hService); 
    ::CloseServiceHandle(hService); 
    ::CloseServiceHandle(hSCM); 
 
    if (bDelete) 
        return TRUE; 
 
    LogEvent(_T("Service could not be deleted")); 
    return FALSE; 
} 
 
//*********************************************************  
//Functiopn:            LogEvent  
//Description:          ��¼�����¼�  
//          <author>niying <time>2006-8-10      <version>     <desc>  
//*********************************************************  
void LogEvent(LPCTSTR pFormat, ...) 
{ 
    TCHAR    chMsg[256]; 
    HANDLE  hEventSource; 
    LPTSTR  lpszStrings[1]; 
    va_list pArg; 
 
    va_start(pArg, pFormat); 
    _vstprintf(chMsg, pFormat, pArg); 
    va_end(pArg); 
 
    lpszStrings[0] = chMsg; 
     
    hEventSource = RegisterEventSource(NULL, szServiceName); 
    if (hEventSource != NULL) 
    { 
        ReportEvent(hEventSource, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL); 
        DeregisterEventSource(hEventSource); 
    } 
} 

