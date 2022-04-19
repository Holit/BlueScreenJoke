#include <Windows.h>
#ifndef _NT_HDRS_
#define _NT_HDRS_

//typedef __success(return >= 0) LONG NTSTATUS;
typedef NTSTATUS* PNTSTATUS;

#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)

typedef struct _LSA_UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} LSA_UNICODE_STRING, * PLSA_UNICODE_STRING, UNICODE_STRING, * PUNICODE_STRING;

typedef enum _HARDERROR_RESPONSE_OPTION {
	OptionAbortRetryIgnore,
	OptionOk,
	OptionOkCancel,
	OptionRetryCancel,
	OptionYesNo,
	OptionYesNoCancel,
	OptionShutdownSystem
} HARDERROR_RESPONSE_OPTION, * PHARDERROR_RESPONSE_OPTION;

typedef enum _HARDERROR_RESPONSE {
	ResponseReturnToCaller,
	ResponseNotHandled,
	ResponseAbort,
	ResponseCancel,
	ResponseIgnore,
	ResponseNo,
	ResponseOk,
	ResponseRetry,
	ResponseYes
} HARDERROR_RESPONSE, * PHARDERROR_RESPONSE;


#endif

HINSTANCE hInst;


typedef UINT(CALLBACK* NTRAISEHARDERROR)(NTSTATUS, ULONG, PUNICODE_STRING, PVOID, HARDERROR_RESPONSE_OPTION, PHARDERROR_RESPONSE);
typedef UINT(CALLBACK* RTLADJUSTPRIVILEGE)(ULONG, BOOL, BOOL, PINT);

HWND GetConsoleHandle()
{
#define CONSOLE_WINDOW_TITLE_BUFSIZE 1024 // Buffer size for console window titles.
	HWND hwndFound;         // This is what is returned to the caller.
	WCHAR pszNewWindowTitle[CONSOLE_WINDOW_TITLE_BUFSIZE]; // Contains fabricated
										  // WindowTitle.
	WCHAR pszOldWindowTitle[CONSOLE_WINDOW_TITLE_BUFSIZE]; // Contains original
										// WindowTitle.
	// Fetch current window title.
	GetConsoleTitle(pszOldWindowTitle, CONSOLE_WINDOW_TITLE_BUFSIZE);
	// Format a "unique" NewWindowTitle.
	wsprintf(pszNewWindowTitle, TEXT("%d/%d"),
		GetTickCount(),
		GetCurrentProcessId());
	// Change current window title.
	SetConsoleTitle(pszNewWindowTitle);
	// Ensure window title has been updated.
	Sleep(40);
	// Look for NewWindowTitle.
	hwndFound = FindWindow(NULL, pszNewWindowTitle);
	// Restore original window title.
	SetConsoleTitle(pszOldWindowTitle);
	return(hwndFound);
}

int main(int argc, char* argv[])
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);

	if (argc != 2 && argv[1] != "-s")
	{
		int uBtn = MessageBox(0, L"Press OK to envoke the Bluescreen fatal\nUsing startup option '-s' to prevent this dialog.", L"Warning", MB_ICONWARNING | MB_OKCANCEL);
		if (uBtn == IDCANCEL)
		{
			MessageBox(0, L"Operation cancelled by user.", L"Information", MB_ICONINFORMATION | MB_OK);
			return 0;
		}
	}
	HINSTANCE hDLL = LoadLibrary(TEXT("ntdll.dll"));
	NTRAISEHARDERROR NtRaiseHardError;
	RTLADJUSTPRIVILEGE RtlAdjustPrivilege;
	int nEn = 0;
	HARDERROR_RESPONSE reResponse;
	if (hDLL != NULL)
	{
		NtRaiseHardError = (NTRAISEHARDERROR)GetProcAddress(hDLL, "NtRaiseHardError");
		RtlAdjustPrivilege = (RTLADJUSTPRIVILEGE)GetProcAddress(hDLL, "RtlAdjustPrivilege");
		if (!NtRaiseHardError)
		{
			// handle the error
			FreeLibrary(hDLL);
			return 0;
		}
		if (!RtlAdjustPrivilege)
		{
			// handle the error
			FreeLibrary(hDLL);
			return 0;
		}
		RtlAdjustPrivilege(0x13, TRUE, FALSE, &nEn);
		//0x13 = SeShutdownPrivilege
		NtRaiseHardError(0xC000021A, 0, 0, 0, OptionShutdownSystem, &reResponse);
	}
	return 1;
}