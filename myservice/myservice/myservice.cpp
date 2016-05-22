#include "StdAfx.h"
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <time.h>

#pragma comment(lib, "advapi32.lib")

#define SVCNAME TEXT("Go_to_bed")

SERVICE_STATUS          gSvcStatus; 
SERVICE_STATUS_HANDLE   gSvcStatusHandle; 
HANDLE                  ghSvcStopEvent = NULL;

VOID SvcInstall(void);
bool SvcunInstall(void);
VOID WINAPI SvcCtrlHandler( DWORD ); 
VOID WINAPI SvcMain( DWORD, LPTSTR * ); 

VOID ReportSvcStatus( DWORD, DWORD, DWORD );
VOID SvcInit( DWORD, LPTSTR * ); 
VOID SvcReportEvent( LPTSTR );


//
// Purpose: 
//   Entry point for the process
//
// Parameters:
//   None
// 
// Return value:
//   None
//
void __cdecl _tmain(int argc, TCHAR *argv[]) 
{ 
    // If command-line parameter is "install", install the service. 
    // Otherwise, the service is probably being started by the SCM.

    if( lstrcmpi( argv[1], TEXT("install")) == 0 )
    {
        SvcInstall();
        return;
    }
	if( lstrcmpi( argv[1], TEXT("uninstall")) == 0 )
    {
        SvcunInstall();
        return;
    }

    // TO_DO: Add any additional services for the process to this table.
    SERVICE_TABLE_ENTRY DispatchTable[] = 
    { 
        { SVCNAME, (LPSERVICE_MAIN_FUNCTION) SvcMain }, 
        { NULL, NULL } 
    }; 
 
    // This call returns when the service has stopped. 
    // The process should simply terminate when the call returns.

    if (!StartServiceCtrlDispatcher( DispatchTable )) 
    { 
        SvcReportEvent(TEXT("StartServiceCtrlDispatcher")); 
    } 
} 

//
// Purpose: 
//   Installs a service in the SCM database
//
// Parameters:
//   None
// 
// Return value:
//   None
//
BOOL IsInstalled() 
{ 
    BOOL bResult = FALSE; 
 
    //打开服务控制管理器  
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS); 
 
    if (hSCM != NULL) 
    { 
        //打开服务  
        SC_HANDLE hService = OpenService(hSCM, SVCNAME, SERVICE_QUERY_CONFIG); 
        if (hService != NULL) 
        { 
            bResult = TRUE; 
            CloseServiceHandle(hService); 
        }    
        CloseServiceHandle(hSCM); 
    } 
    return bResult; 
} 
VOID SvcInstall()
{
	if (IsInstalled()) 
        return ; 
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szPath[MAX_PATH];

    if( !GetModuleFileName( NULL, szPath, MAX_PATH ) )
    {
        printf("Cannot install service (%d)\n", GetLastError());
        return;
    }

    // Get a handle to the SCM database. 
 
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager) 
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Create the service

    schService = CreateService( 
        schSCManager,              // SCM database 
        SVCNAME,                   // name of service 
        SVCNAME,                   // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_AUTO_START,      // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        szPath,                    // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 
 
    if (schService == NULL) 
    {
        printf("CreateService failed (%d)\n", GetLastError()); 
        CloseServiceHandle(schSCManager);
        return;
    }
    else printf("Service installed successfully\n"); 

    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
}

bool SvcunInstall()
{
    if (!IsInstalled()) 
        return TRUE; 
 
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS); 
 
    if (hSCM == NULL) 
    { 
        MessageBox(NULL, _T("Couldn't open service manager"), SVCNAME, MB_OK); 
        return FALSE; 
    } 
 
    SC_HANDLE hService = OpenService(hSCM, SVCNAME, SERVICE_STOP | DELETE); 
 
    if (hService == NULL) 
    { 
        CloseServiceHandle(hSCM); 
        MessageBox(NULL, _T("Couldn't open service"), SVCNAME, MB_OK); 
        return FALSE; 
    } 
    SERVICE_STATUS status; 
    ControlService(hService, SERVICE_CONTROL_STOP, &status); 
 
    //删除服务  
    BOOL bDelete = DeleteService(hService); 
    CloseServiceHandle(hService); 
    CloseServiceHandle(hSCM); 
 
    if (bDelete) 
        return TRUE; 
 
    //LogEvent(_T("Service could not be deleted")); 
    return FALSE; 
}
//
// Purpose: 
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None.
//
VOID WINAPI SvcMain( DWORD dwArgc, LPTSTR *lpszArgv )
{
    // Register the handler function for the service

    gSvcStatusHandle = RegisterServiceCtrlHandler( 
        SVCNAME, 
        SvcCtrlHandler);

    if( !gSvcStatusHandle )
    { 
        SvcReportEvent(TEXT("RegisterServiceCtrlHandler")); 
        return; 
    } 

    // These SERVICE_STATUS members remain as set here

    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
    gSvcStatus.dwServiceSpecificExitCode = 0;    

    // Report initial status to the SCM

    ReportSvcStatus( SERVICE_START_PENDING, NO_ERROR, 3000 );

    // Perform service-specific initialization and work.

    SvcInit( dwArgc, lpszArgv );
}

//
// Purpose: 
//   The service code
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None
//
int myshut( void )

{

    time_t t = time( 0 );

    char tmp[64];

    //strftime( tmp, sizeof(tmp), "%Y/%m/%d %X %A 本年第%j天 %z",localtime(&t) ); 
	//配置文件
	//读取文件 
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
	if(i==4)
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

VOID SvcInit( DWORD dwArgc, LPTSTR *lpszArgv)
{
    // TO_DO: Declare and set any required variables.
    //   Be sure to periodically call ReportSvcStatus() with 
    //   SERVICE_START_PENDING. If initialization fails, call
    //   ReportSvcStatus with SERVICE_STOPPED.


    ReportSvcStatus( SERVICE_RUNNING, NO_ERROR, 0 );

    // TO_DO: Perform work until service stops.
	
	

    while(1)
    {
        // Check whether to stop the service.

        myshut();
		Sleep(2000);
        
    }
	
	return;
}

//
// Purpose: 
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation, 
//     in milliseconds
// 
// Return value:
//   None
//
VOID ReportSvcStatus( DWORD dwCurrentState,
                      DWORD dwWin32ExitCode,
                      DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ( (dwCurrentState == SERVICE_RUNNING) ||
           (dwCurrentState == SERVICE_STOPPED) )
        gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus( gSvcStatusHandle, &gSvcStatus );
}

//
// Purpose: 
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
// 
// Return value:
//   None
//
VOID WINAPI SvcCtrlHandler( DWORD dwCtrl )
{
   // Handle the requested control code. 

   switch(dwCtrl) 
   {  
      case SERVICE_CONTROL_STOP: 
         ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

         
         ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
         
         return;
 
      case SERVICE_CONTROL_INTERROGATE: 
         break; 
 
      default: 
         break;
   } 
   
}

//
// Purpose: 
//   Logs messages to the event log
//
// Parameters:
//   szFunction - name of function that failed
// 
// Return value:
//   None
//
// Remarks:
//   The service must have an entry in the Application event log.
//
VOID SvcReportEvent(LPTSTR szFunction) 
{ 
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];
    TCHAR Buffer[80];

    hEventSource = RegisterEventSource(NULL, SVCNAME);

    if( NULL != hEventSource )
    {
        StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

        lpszStrings[0] = SVCNAME;
        lpszStrings[1] = Buffer;

        ReportEvent(hEventSource,        // event log handle
                    EVENTLOG_ERROR_TYPE, // event type
                    0,                   // event category
                    0,           // event identifier
                    NULL,                // no security identifier
                    2,                   // size of lpszStrings array
                    0,                   // no binary data
                    lpszStrings,         // array of strings
                    NULL);               // no binary data

        DeregisterEventSource(hEventSource);
    }
}