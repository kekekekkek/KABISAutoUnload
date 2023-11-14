#include "Include.h"

bool bCanExit = true;
HWND hKabisWnd = NULL;
HWND hCatalogWnd = NULL;
LONG lOrigWndProc = NULL;
LONG lOrigDialogProc = NULL;

LRESULT WINAPI MyDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_SHOWWINDOW)
	{
		//����� ���� "�������� ������": uMsg = 273; wParam = 10, lParam = 0.
		SendMessageA(hWnd, 273, 10, 0);

		/*����� ����� ���-���� ������� �� ����, ��� ����� ������� ���������, ��������
		����������� ��������� ���� � ����������� �������. (��� ����� ��� ���������� �����
		��������� ��� �� ������� CreateFileA, ��� ��� ��� ���������� ��� �������� �����
		.htm-�����, � �� ��������� "lpFileName" ������� ��� ����. ���� ��������� ���
		�� ������� WriteFile � ��� � ��� �������� ����� ������ � � ����� ���� ������������)*/

		ExitProcess(TRUE);
	}

	return CallWindowProcA((WNDPROC)lOrigDialogProc, hWnd, uMsg, wParam, lParam);
}

LRESULT WINAPI MyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_SYSCOMMAND
		&& wParam == SC_CLOSE)
	{
		//����� ���� "��������": uMsg = 273; wParam = 57, lParam = 0.
		SendMessageA(hWnd, 273, 57, 0);
		bCanExit = false;

		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	}

	if (!IsWindow(hCatalogWnd))
	{
		hCatalogWnd = FindWindowA("ThunderRT6FormDC", "��������");

		lOrigDialogProc = GetWindowLongA(hCatalogWnd, GWL_WNDPROC);
		SetWindowLongA(hCatalogWnd, GWL_WNDPROC, (LONG_PTR)MyDialogProc);
	}
	
	return CallWindowProcA((WNDPROC)lOrigWndProc, hWnd, uMsg, wParam, lParam);
}

typedef BOOL(WINAPI* PeekMessageFn)(LPMSG msMsg, HWND hWnd, UINT uMsgFilterMin, UINT uMsgFilterMax, UINT uMsgRemove);
PeekMessageFn OrigPeekMessage;

BOOL WINAPI PeekMessageHook(LPMSG msMsg, HWND hWnd, UINT uMsgFilterMin, UINT uMsgFilterMax, UINT uMsgRemove)
{
	if (!bCanExit)
	{
		msMsg->message = NULL;
		msMsg->wParam = NULL;
		msMsg->lParam = NULL;

		bCanExit = true;
	}

	return OrigPeekMessage(msMsg, hWnd, uMsgFilterMin, uMsgFilterMax, uMsgRemove);
}

typedef int (WINAPI* rtcShellFn)(int a1, string a2);
rtcShellFn OrigrtcShell;

int WINAPI rtcShellHook(int a1, string a2) {
	return NULL;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReasonForCall, LPVOID lpReserved)
{
	if (dwReasonForCall == DLL_PROCESS_ATTACH)
	{
		OrigrtcShell = (rtcShellFn)DetourFunction((PBYTE)GetProcAddress(GetModuleHandleA("msvbvm60.dll"), "rtcShell"), (PBYTE)rtcShellHook);
		OrigPeekMessage = (PeekMessageFn)DetourFunction((PBYTE)PeekMessageA, (PBYTE)PeekMessageHook);

		/*���������� ���� ����� �������� � ���� "PeekMessageHook" � ��������� �������� ���� ��� ������
		�� ������� ������, ������� ������ � ���������� ������� ����.*/

		hKabisWnd = FindWindowA("ThunderRT6FormDC", "�����.Catalog");
		lOrigWndProc = GetWindowLongA(hKabisWnd, GWL_WNDPROC);

		SetWindowLongA(hKabisWnd, GWL_WNDPROC, ((LONG_PTR)MyWndProc));
	}

	return TRUE;
}