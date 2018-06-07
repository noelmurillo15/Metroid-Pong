#pragma once

#include <Windows.h> // Include the Win32 API
#include <tchar.h> // Include support for UNICODE
#include <sstream> // Include wostringstream support
#include <time.h>
#include <stdlib.h>     
using std::wostringstream;

#include <d2d1.h> // Include the Direct2D API.
#pragma comment(lib, "d2d1.lib") // Connect to the Direct2D Import Library.

#include <dwrite.h> // Include the DirectWrite API.
#pragma comment(lib, "dwrite.lib") // Connect to the DirectWrite Import Library.

#include <mmsystem.h> // Include the Multimedia header file.
#pragma comment(lib, "winmm.lib") // Connect to the Multimedia import library.


#include <wincodec.h>
#pragma comment (lib, "windowscodecs.lib")

#define NUM_PARTICLES 1000

class Game
{
	HWND hWnd;

	template <class T> void SafeRelease(T **ppT)
	{
		if (*ppT)
		{
			(*ppT)->Release();
			*ppT = NULL;
		}
	}

	// Programmer defined macro to make using the known colors easier.
	#define D2DColor(clr) D2D1::ColorF(D2D1::ColorF::clr)

	// DirectX Interfaces:
	ID2D1Factory* pD2DFactory;
	ID2D1HwndRenderTarget* pRT;

	// DirectWrite Interfaces:
	IDWriteFactory* pDWFactory;
	IDWriteTextFormat* pTF;

	// WIC Interfaces
	IWICImagingFactory* pWICFactory;
	ID2D1Bitmap* backgnd;
	
	// Particle Arrays
	ID2D1Bitmap* particle;
	D2D1_POINT_2F particlePos[NUM_PARTICLES];
	D2D1_SIZE_F particleVel[NUM_PARTICLES];
	float particlelife[NUM_PARTICLES];

	// Samus Variables
	ID2D1Bitmap* spriteIdle;
	ID2D1Bitmap* blastStance;
	ID2D1Bitmap* blastCharging;
	ID2D1Bitmap* smallBlastCharge;
	ID2D1Bitmap* bigBlastCharge;

	ID2D1Bitmap* jump;
	ID2D1Bitmap* tuck1;
	ID2D1Bitmap* screw1;
	ID2D1Bitmap* screw2;
	ID2D1Bitmap* land;

	ID2D1Bitmap* morph1;
	ID2D1Bitmap* morph2;
	ID2D1Bitmap* fallStance;

	ID2D1Bitmap* ultStance;
	ID2D1Bitmap* ult1;
	ID2D1Bitmap* ult2;
	ID2D1Bitmap* ult3;
	ID2D1Bitmap* ultBeam;

	ID2D1Bitmap* m1;
	ID2D1Bitmap* m2;
	ID2D1Bitmap* m3;
	ID2D1Bitmap* m4;
	ID2D1Bitmap* m5;

	ID2D1Bitmap* ship;

	D2D1_RECT_F samusPos;	
	D2D1_RECT_F laserPos;
	D2D1_RECT_F shipPos;
	D2D1_RECT_F hpBar;
	D2D1_RECT_F hpBarFill;
	ID2D1SolidColorBrush *hpBarBrush;

	D2D1_RECT_F energyBar;
	D2D1_RECT_F energyBarFill;

	float maxHealth = 100;
	float currHealth = 100;
	float energy = 0;

	// Dark Samus Variables
	ID2D1Bitmap* darkSamus;
	ID2D1Bitmap* eb1;
	ID2D1Bitmap* eb2;
	ID2D1Bitmap* eb3;
	ID2D1Bitmap* eb4;
	ID2D1Bitmap* eb5;
	ID2D1Bitmap* eb6;

	ID2D1Bitmap* tele1;
	ID2D1Bitmap* tele2;
	ID2D1Bitmap* tele3;
	ID2D1Bitmap* tele4;
	ID2D1Bitmap* tele5;
	ID2D1Bitmap* tele6;

	D2D1_RECT_F darkSamusPos;
	D2D1_SIZE_F darkSamusVec;
	D2D1_RECT_F DarkHpBar;
	D2D1_RECT_F DarkHpBarFill;

	float darkCurrHealth = 100;

	// Metroid Variables
	ID2D1Bitmap* metroid;
	ID2D1Bitmap* metroidAttack;
	ID2D1Bitmap* metroidCharge;
	ID2D1Bitmap* metroidFall;

	D2D1_RECT_F metroidPos;
	D2D1_SIZE_F metroidVec;


	// Blast Variables
	ID2D1Bitmap* blastBall;
	ID2D1Bitmap* blastBallsmall;
	ID2D1Bitmap* blastBallmed;
	ID2D1Bitmap* blastBallhigh;

	D2D1_RECT_F blastBallpos;
	
	int ExplosionCount;

	// Missile Variables
	ID2D1Bitmap* missiles;
	D2D1_RECT_F missilePos;
	D2D1_SIZE_F missileVec;

	// Animations
	bool animation = true;
	bool animationJump = false;
	bool animationFall = false;
	bool darkAnimation = false;
	bool teleport = false;
	bool metroidAtk = false;
	bool metroidCharging = false;
	bool ultimate = false;
	bool MissileShot = false;
	bool missileManuever = false;
	bool metroidDead = false;
	bool dmStunned = false;
	
	// Frames per sec
	DWORD dwTime;
	float LRmoveFrame;
	float TBmoveFrame;
	float blastFrameWidth;

	int ultimateframe;
	int frame;
	int jumpFrame;
	int fallFrame;
	int teleportFrame;
	int teleportTimer;
	int metroidAtkTimer;
	int missileFrame;
	int telePosx = 0;

public:
	BOOL GamePlay;		// Is the game running?
	HANDLE hGameThread; // Handle to the GameMain thread.
	BYTE keys[256];		// Current state of the keyboard.

	Game(void);
	virtual ~Game(void);

	float GetHealth() const { return currHealth; }
	float GetEnergy() const { return energy; }
	float GetDarkHealth() const { return darkCurrHealth; }

	void SetHealth(float hp){ if (hp <= maxHealth && currHealth >= 0){ currHealth = hp; } }
	void SetEnergy(float newEnergy){ if (energy < 100 && energy >= 0){ energy = newEnergy; } }
	void SetDarkHealth(float hp){ if (hp <= maxHealth && darkCurrHealth >= 0){ darkCurrHealth = hp; } }

	void Startup(void);
	void Shutdown(void);
	void Input(void);
	void Simulate(void);
	void Render(void);
	void Update(void);
	void EndGame(void);
	void SetHwnd(HWND _hWnd) {hWnd = _hWnd;}
	void DestroyGraphics(void);

	HRESULT LoadBitmapFromFile(LPCTSTR strFileName, ID2D1Bitmap** ppBitmap);
	HRESULT CreateGraphics(HWND hWnd);	
};