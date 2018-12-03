#pragma once
#define DWMACLOAKED 14
#define WINDOW_CLOAKED 2

class CWindowCapture
{
public:
	CWindowCapture();
	~CWindowCapture();

	// 인자로 받아온 HWND의 화면을 캡쳐해 HBITMAP 형식으로 반환해주는 함수
	HBITMAP Capture(HWND);

	// HWND값을 받아와 캡쳐 해야 하는 핸들인지 확인 후 Capture함수에서 반환한 HBITMAP을 AeroDrawTestDlgSub.cpp 에 전달
	HBITMAP FindBitmap(HWND, int);
	int count;
};

