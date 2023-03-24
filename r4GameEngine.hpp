#ifndef _WIN32
cerr << "This game engine is not supported in your OS.";
#endif

#include <windows.h>
#include <iostream>
#include <iterator>
#include <fstream>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <vector>
#include <future>
#include <cstdio>
#include <cmath>
#include <ios>

#define FG_BLACK 0x0000
#define FG_DARK_BLUE 0x0001
#define FG_DARK_GREEN 0x0002
#define FG_DARK_CYAN 0x0003
#define FG_DARK_RED 0x0004
#define FG_DARK_MAGENTA 0x0005
#define FG_DARK_YELLOW 0x0006
#define FG_GREY 0x0007
#define FG_DARK_GREY 0x0008
#define FG_BLUE 0x0009
#define FG_GREEN 0x000A
#define FG_CYAN 0x000B
#define FG_RED 0x000C
#define FG_MAGENTA 0x000D
#define FG_YELLOW 0x000E
#define FG_WHITE 0x000F
#define BG_BLACK 0x0000
#define BG_DARK_BLUE 0x0010
#define BG_DARK_GREEN 0x0020
#define BG_DARK_CYAN 0x0030
#define BG_DARK_RED 0x0040
#define BG_DARK_MAGENTA 0x0050
#define BG_DARK_YELLOW 0x0060
#define BG_GREY 0x0070
#define BG_DARK_GREY 0x0080
#define BG_BLUE 0x0090
#define BG_GREEN 0x00A0
#define BG_CYAN 0x00B0
#define BG_RED 0x00C0
#define BG_MAGENTA 0x00D0
#define BG_YELLOW 0x00E0
#define BG_WHITE 0x00F0

#define LEFT 0
#define CENTER 1
#define RIGHT 2

struct Pixel {
    int x, y;
    char c;
    short col;

    Pixel(const int& x = 0, const int& y = 0, const char& c = '#', const short& col = 0x000F) :
        x(x), y(y), c(c), col(col) {}
};

class r4GameEngine {
protected:
	int m_nScreenWidth;
	int m_nScreenHeight;
	CHAR_INFO* m_bufScreen;
	std::wstring m_sAppName;
	HANDLE m_hOriginalConsole;
	HANDLE m_hConsole;
	HANDLE m_hConsoleIn;
	SMALL_RECT m_rectWindow;
	COORD m_coordBuffer;
    bool m_bRunning;
	bool m_bShowFPS;
    
    virtual bool OnUserCreate() = 0;
    virtual bool OnUserUpdate(double fElapsedTime) = 0;

	void ClearScreen(const char& c = ' ', const short& col = 0x0000) {
		for (int x = 0; x < m_nScreenWidth; x++) {
			for (int y = 0; y < m_nScreenHeight; y++) {
				m_bufScreen[y * m_nScreenWidth + x].Char.UnicodeChar = c;
				m_bufScreen[y * m_nScreenWidth + x].Attributes = col;
			}
		}
	}

    void DrawPixel(const Pixel& pixel) {
		if (pixel.x >= 0 && pixel.x < m_nScreenWidth && pixel.y >= 0 && pixel.y < m_nScreenHeight &&
			m_bufScreen[pixel.y * m_nScreenWidth + pixel.x].Char.UnicodeChar != pixel.c &&
			m_bufScreen[pixel.y * m_nScreenWidth + pixel.x].Attributes != pixel.col) {
			m_bufScreen[pixel.y * m_nScreenWidth + pixel.x].Char.UnicodeChar = pixel.c;
			m_bufScreen[pixel.y * m_nScreenWidth + pixel.x].Attributes = pixel.col;
		}
    }

	void DrawPixels(const std::vector<Pixel>& pixels) {
		#pragma omp parallel for
    	for (int i = 0; i < pixels.size(); i++) {
    	    DrawPixel(pixels[i]);
    	}
    }

	void DrawLine(const std::pair<Pixel,Pixel>& pair) {
		char c = pair.first.c;
		short col = pair.first.col;
		int x0 = pair.first.x, x1 = pair.second.x;
		int y0 = pair.first.y, y1 = pair.second.y;
		bool steep = abs(y1 - y0) > abs(x1 - x0);
    	if (steep) {
    	    std::swap(x0, y0);
    	    std::swap(x1, y1);
    	}
    	if (x0 > x1) {
    	    std::swap(x0, x1);
    	    std::swap(y0, y1);
    	}
    	int dx = x1 - x0;
    	int dy = abs(y1 - y0);
    	float err = dx / 2.0f;
    	int ystep = (y0 < y1) ? 1 : -1;
    	int y = y0;
		#pragma omp parallel for
    	for (int x = x0; x <= x1; x++) {
    	    if (steep) {
    	        DrawPixel(Pixel(y, x, c, col));
    	    }
    	    else {
    	        DrawPixel(Pixel(x, y, c, col));
    	    }
    	    err -= dy;
    	    if (err < 0) {
    	        y += ystep;
    	        err += dx;
    	    }
    	}
	}

	void DrawLines(const std::vector<std::pair<Pixel,Pixel>>& pairs) {
		#pragma omp parallel for
    	for (int i = 0; i < pairs.size(); i++) {
    	    DrawLine(pairs[i]);
    	}
	}

	void DrawText(const int& x, const int& y, const std::string& str, const int& alignment = 0, const short& col = 0x000F) {
		#pragma omp parallel for
		for (size_t i = 0; i < str.length(); i++) {
			int coord = int(y * m_nScreenWidth + x + (alignment != 2? i : -i) - str.length()/2*(alignment%2));
			int index = alignment != 2? i : str.length()-1 - i;
			m_bufScreen[coord].Char.UnicodeChar = str[index];
			m_bufScreen[coord].Attributes = col;
		}
	}

	void DrawTexts(const int& x, const int& y, const std::vector<std::string>& strVec, const int& alignment = 0, const short& col = 0x000F) {
		#pragma omp parallel for
		for (int i = 0; i < strVec.size(); i++) {
			DrawText(x, y+i, strVec[i], alignment, col);
		}
	}

public:
    r4GameEngine() {
		m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		m_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
    }

    int ConstructConsole(const int& width, const int& height, const int& fontw, const int& fonth, const bool& showFPS = false) {
		m_nScreenWidth = width;
		m_nScreenHeight = height;

		m_rectWindow = {0, 0, 1, 1};
		SetConsoleWindowInfo(m_hConsole, TRUE, &m_rectWindow);

		COORD coord = {(short)m_nScreenWidth, (short)m_nScreenHeight};
		SetConsoleScreenBufferSize(m_hConsole, coord);

        SetConsoleActiveScreenBuffer(m_hConsole);

        CONSOLE_FONT_INFOEX cfi;
        cfi.cbSize = sizeof(cfi);
        cfi.nFont = 0;
        cfi.dwFontSize.X = fontw;
        cfi.dwFontSize.Y = fonth;
        cfi.FontFamily = FF_DONTCARE;
        cfi.FontWeight = FW_NORMAL;

		wcscpy(cfi.FaceName, L"Consolas");
        SetCurrentConsoleFontEx(m_hConsole, false, &cfi);

        m_rectWindow = {0, 0, short(m_nScreenWidth - 1), short(m_nScreenHeight - 1)};
		SetConsoleWindowInfo(m_hConsole, TRUE, &m_rectWindow);

		SetConsoleMode(m_hConsoleIn, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

        m_bufScreen = new CHAR_INFO[m_nScreenWidth*m_nScreenHeight];
		memset(m_bufScreen, 0, sizeof(CHAR_INFO) * m_nScreenWidth*m_nScreenHeight);

		m_bShowFPS = showFPS;

		SetConsoleCtrlHandler((PHANDLER_ROUTINE)CloseHandle, TRUE);
		return 1;
    }

    void Start() {
        m_bRunning = true;
        std::thread t = std::thread(&r4GameEngine::GameThread, this);
        t.join();
    }

    int ScreenWidth() {return m_nScreenWidth;}
    int ScreenHeight() {return m_nScreenHeight;}

private:
    void GameThread() {
        auto T0 = std::chrono::system_clock::now();
        auto T1 = std::chrono::system_clock::now();

		SetConsoleTitleW(L"R4WLYX Game Engine");

        if (!OnUserCreate()) m_bRunning = false;

        while(m_bRunning) {
            T1 = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsedTime = T1 - T0;
            T0 = T1;
            double fElapsedTime = elapsedTime.count();

            if(!OnUserUpdate(fElapsedTime)) m_bRunning = false;

            wchar_t s[256];
			if (!m_bShowFPS)
				swprintf_s(s, 256, L"%s", m_sAppName.c_str());
			else
				swprintf_s(s, 256, L"%s - FPS:%8.2f", m_sAppName.c_str(), float(1.0f/fElapsedTime));
			SetConsoleTitleW(s);
			WriteConsoleOutput(m_hConsole, m_bufScreen, {(short)m_nScreenWidth, (short)m_nScreenHeight}, {0, 0}, &m_rectWindow);
        }
    }
};
