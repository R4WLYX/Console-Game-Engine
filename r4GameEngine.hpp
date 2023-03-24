#ifndef _WIN32
cerr << "This game engine is not supported in your OS.";
while (true);
#endif

#include <windows.h>
#include <iostream>
#include <iterator>
#include <fstream>
#include <chrono>
#include <thread>
#include <vector>
#include <future>
#include <cmath>
#include <ios>

enum COLOUR {
	FG_BLACK		= 0x0000,
	FG_DARK_BLUE    = 0x0001,	
	FG_DARK_GREEN   = 0x0002,
	FG_DARK_CYAN    = 0x0003,
	FG_DARK_RED     = 0x0004,
	FG_DARK_MAGENTA = 0x0005,
	FG_DARK_YELLOW  = 0x0006,
	FG_GREY			= 0x0007,
	FG_DARK_GREY    = 0x0008,
	FG_BLUE			= 0x0009,
	FG_GREEN		= 0x000A,
	FG_CYAN			= 0x000B,
	FG_RED			= 0x000C,
	FG_MAGENTA		= 0x000D,
	FG_YELLOW		= 0x000E,
	FG_WHITE		= 0x000F,
	BG_BLACK		= 0x0000,
	BG_DARK_BLUE	= 0x0010,
	BG_DARK_GREEN	= 0x0020,
	BG_DARK_CYAN	= 0x0030,
	BG_DARK_RED		= 0x0040,
	BG_DARK_MAGENTA = 0x0050,
	BG_DARK_YELLOW	= 0x0060,
	BG_GREY			= 0x0070,
	BG_DARK_GREY	= 0x0080,
	BG_BLUE			= 0x0090,
	BG_GREEN		= 0x00A0,
	BG_CYAN			= 0x00B0,
	BG_RED			= 0x00C0,
	BG_MAGENTA		= 0x00D0,
	BG_YELLOW		= 0x00E0,
	BG_WHITE		= 0x00F0,
};

enum Alignment {
	ALIGN_LEFT   = 0,
	ALIGN_CENTER = 1,
	ALIGN_RIGHT  = 2
};

struct Pixel {
	int x, y;
	char c;
	short col;

	Pixel(int x, int y, char c = '#', short col = 0x000F) {
		this->x = x;
		this->y = y;
		this->c = c;
		this->col = col;
	}
};

class r4GameEngine {
protected:
	int m_nScreenWidth;
	int m_nScreenHeight;
	CHAR_INFO* m_bufScreen;
	std::wstring m_sAppName;
	HANDLE m_hOriginalConsole;
	CONSOLE_SCREEN_BUFFER_INFO m_OriginalConsoleInfo;
	HANDLE m_hConsole;
	HANDLE m_hConsoleIn;
	SMALL_RECT m_rectWindow;
	COORD m_coordBuffer;
    bool m_bRunning;
	bool m_bShowFPS;
    
    virtual bool OnLoad() = 0;
    virtual bool OnUpdate(double fElapsedTime) = 0;

	void ClearScreen(char c = ' ', short col = 0x0000) {
		for (int x = 0; x < m_nScreenWidth; x++) {
			for (int y = 0; y < m_nScreenHeight; y++) {
				m_bufScreen[y * m_nScreenWidth + x].Char.UnicodeChar = c;
				m_bufScreen[y * m_nScreenWidth + x].Attributes = col;
			}
		}
	}

    void DrawPixel(Pixel pixel) {
		if (pixel.x >= 0 && pixel.x < m_nScreenWidth && pixel.y >= 0 && pixel.y < m_nScreenHeight &&
			m_bufScreen[pixel.y * m_nScreenWidth + pixel.x].Char.UnicodeChar != pixel.c &&
			m_bufScreen[pixel.y * m_nScreenWidth + pixel.x].Attributes != pixel.col) {
			m_bufScreen[pixel.y * m_nScreenWidth + pixel.x].Char.UnicodeChar = pixel.c;
			m_bufScreen[pixel.y * m_nScreenWidth + pixel.x].Attributes = pixel.col;
		}
    }

	void DrawPixels(std::vector<Pixel> pixels) {
		for (const Pixel& pixel : pixels) {
			std::async(std::launch::async, DrawPixel, this, pixel);
		}
    }

	void DrawLine(std::pair<Pixel, Pixel> pair) {
		char c = pair.first.c;
		short col = pair.first.col;
		int x0 = pair.first.x, x1 = pair.second.x;
		int y0 = pair.first.y, y1 = pair.second.y;
		int dx = abs(x1 - x0);
    	int sx = x0 < x1 ? 1 : -1;
    	int dy = -abs(y1 - y0);
    	int sy = y0 < y1 ? 1 : -1;
    	int error = dx + dy;
		int e2;
	
    	while (true) {
    	    DrawPixel(Pixel(x0, y0, c, col));
    	    if (x0 == x1 && y0 == y1) break;
    	    e2 = 2 * error;
    	    if (e2 >= dy) {
    	        if (x0 == x1) break;
    	        error += dy;
    	        x0 += sx;
			}
    	    if (e2 <= dx) {
    	        if (y0 == y1) break;
    	        error += dx;
    	        y0 += sy;
    	    }
		}
	}

	void DrawLines(std::vector<std::pair<Pixel, Pixel>> pairs) {
		for (const std::pair<Pixel, Pixel>& pair : pairs) {
			std::async(std::launch::async, DrawLine, this, pair);
		}
	}

	void DrawText(int x, int y, std::string str, int alignment = 0, short col = 0x000F) {
		for (size_t i = 0; i < str.length(); i++) {
			int coord = int(y * m_nScreenWidth + x + (alignment != 2? i : -i) - str.length()/2*(alignment%2));
			int index = alignment != 2? i : str.length()-1 - i;
			m_bufScreen[coord].Char.UnicodeChar = str[index];
			m_bufScreen[coord].Attributes = col;
		}
	}

	void DrawTexts(int x, int y, std::vector<std::string> strVec, int alignment = 0, short col = 0x000F) {
		for (int i = 0; i < strVec.size(); i++) {
			std::async(std::launch::async, DrawText, this, x, y+i, strVec[i], alignment, col);
		}
	}

public:
    r4GameEngine() {
		m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		m_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
    }

    int ConstructConsole(int width, int height, int fontw, int fonth, bool showFPS = false) {
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

        if (!OnLoad()) m_bRunning = false;

        while(m_bRunning) {
            T1 = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsedTime = T1 - T0;
            T0 = T1;
            double fElapsedTime = elapsedTime.count();

            if(!OnUpdate(fElapsedTime)) m_bRunning = false;

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