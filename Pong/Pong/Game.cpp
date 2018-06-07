#include "Game.h"

Game::Game(void)
{
	GamePlay = TRUE;
	hWnd = NULL;
	ZeroMemory(keys, sizeof(keys));

	pD2DFactory = NULL;
	pRT = NULL;
	pDWFactory = NULL;
	pTF = NULL;

	srand((unsigned int)time(nullptr));
}

Game::~Game(void)
{
}

void Game::Startup(void)
{
	//TODO: Create Back Buffer

	// Initialize DirectX.
	HRESULT hr = CreateGraphics(hWnd);
	if (FAILED(hr))
	{
		return; // Return -1 to abort Window Creation.
	}

	// Samus Variables
	samusPos = D2D1::RectF(0, 100, 140, 280);
	shipPos = samusPos;
	laserPos = D2D1::RectF(0, 100, 140, 280);
	hpBar = D2D1::RectF(1, 1, 400, 20);
	hpBarFill = D2D1::RectF(1, 1, 400, 20);

	energyBar = D2D1::RectF(1, 25, 400, 45);
	energyBarFill = D2D1::RectF(1, 25, 400, 45);

	hpBarBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
	hpBarBrush->SetOpacity(1.0f);

	// Dark Samus Variables
	darkSamusPos = D2D1::RectF(pRT->GetSize().width - 150, 100, pRT->GetSize().width + 50, 300);
	darkSamusVec = D2D1::SizeF(0, 7);

	DarkHpBar = D2D1::RectF(800, 1, 1200, 20);
	DarkHpBarFill = D2D1::RectF(800, 1, 1200, 20);

	teleportFrame = 0;
	teleportTimer = 80;
	teleport = false;

	// Metroid Variables
	metroidPos = D2D1::RectF(pRT->GetSize().width - 100, 100, pRT->GetSize().width, 200);
	metroidVec = D2D1::SizeF(5, 5);
	metroidAtk = false;
	metroidCharging = false;

	metroidAtkTimer = -1;
	ultimateframe = -1;

	// Blast Variables
	blastBallpos = D2D1::RectF(40, 20, 240, 220);
	LRmoveFrame = 10;
	TBmoveFrame = 0;
	ExplosionCount = 0;

	// Missile Variables
	missilePos = samusPos;
	missileVec = D2D1::SizeF(15, 0);
	missileFrame = 0;

	dwTime = GetTickCount() + 100;

	// Initialize Particles
}

void Game::Shutdown(void)
{
	//TODO: Unload Bitmaps/Brushes/Pens/Fonts

	//TODO: Destroy Back Buffer

	// Shutdown DirectX.
	DestroyGraphics();
}

void Game::Input(void)
{
	//TODO: Read User Input
	if (keys[VK_ESCAPE] & 0x80)
	{
		PostMessage(hWnd, WM_CLOSE, 0, 0);
	}

	// Check if the W Key is pressed:
	if (keys['Q'] & 0x80)
	{
		blastBallpos = D2D1::RectF(5, 20, 205, 220);
	}
	if (keys[VK_SPACE] & 0x80 && !(animationFall || metroidAtk || animationJump || ultimate || animation || MissileShot || missileManuever))
	{
		missileFrame = 0;
		MissileShot = true;
	}
	if (keys['Z'] & 0x80 && !(animationFall || metroidAtk || animationJump || ultimate || animation) && energy == 100)
	{
		energy = 0;
		ultimateframe = 0;
		ultimate = true;
	}
	if (keys[VK_UP] & 0x80 && !(animationJump || metroidAtk || MissileShot))
	{
		jumpFrame = 0;
		animationJump = true;
	}
	else if (keys[VK_DOWN] & 0x80 && !(animationFall || metroidAtk || MissileShot))
	{
		fallFrame = 0;
		animationFall = true;
	}
	else if ((keys['W'] & 0x80) && samusPos.top - 5 > 20)
	{
		samusPos.top -= 2;
		samusPos.bottom -= 2;
	}

	else if ((keys['S'] & 0x80) && shipPos.bottom - 20 <= pRT->GetSize().height)
	{
		samusPos.top += 2;
		samusPos.bottom += 2;
	}
}

void Game::Simulate(void)
{
#pragma region Intersect
	RECT rIntersect;

	RECT rMissile = { (LONG)missilePos.left, (LONG)missilePos.top, (LONG)missilePos.right, (LONG)missilePos.bottom };
	RECT rBlastBall = { (LONG)blastBallpos.left, (LONG)blastBallpos.top, (LONG)blastBallpos.right, (LONG)blastBallpos.bottom };
	RECT rSamus = { (LONG)samusPos.left, (LONG)samusPos.top + 80, 6, (LONG)samusPos.bottom - 80 };
	RECT rDarkSamus = { (LONG)darkSamusPos.left + 80, (LONG)darkSamusPos.top, (LONG)darkSamusPos.right, (LONG)darkSamusPos.bottom };
	RECT rMetroid = { (LONG)metroidPos.left, (LONG)metroidPos.top + 20, (LONG)metroidPos.right, (LONG)metroidPos.bottom };
	RECT rLaser = { (LONG)laserPos.left, (LONG)laserPos.top, (LONG)laserPos.right, (LONG)laserPos.bottom };


	if (IntersectRect(&rIntersect, &rLaser, &rDarkSamus) && ultimate)
	{
		SetDarkHealth(GetDarkHealth() - 1);
	}
	if (IntersectRect(&rIntersect, &rMissile, &rDarkSamus) && missileManuever)
	{
		missileManuever = false;
		darkAnimation = false;
		
		SetDarkHealth(GetDarkHealth() - 5);
	}
	rDarkSamus = { (LONG)darkSamusPos.left + 120, (LONG)darkSamusPos.top + 130, (LONG)darkSamusPos.right, (LONG)darkSamusPos.bottom - 69 };
	if (IntersectRect(&rIntersect, &rBlastBall, &rDarkSamus) && !(darkAnimation))
	{		
		frame = 0;
		LRmoveFrame = -(LRmoveFrame);
		if (!teleport)
		darkAnimation = true;
		
	}
	else if (IntersectRect(&rIntersect, &rBlastBall, &rSamus) && !(animation || metroidAtk))
	{
		SetEnergy(GetEnergy() + 5);
		frame = 0;		
		if (animationJump && jumpFrame < 4){
			TBmoveFrame = -2.5;
		}
		else if (animationFall && fallFrame < 4){
			TBmoveFrame = 2.5;
		}
		else
		TBmoveFrame = 0;

		LRmoveFrame = -(LRmoveFrame);
		if (LRmoveFrame < 40)
			LRmoveFrame += 0.5f;

		animation = true;		
	}
	else if (IntersectRect(&rIntersect, &rMetroid, &rSamus))
	{		
		animation = false;
		metroidAtk = true;
		frame = 0;		
	}
	else if (IntersectRect(&rIntersect, &rMissile, &rMetroid) && !metroidCharging && !metroidAtk && !metroidDead){
		missileManuever = false;
		MissileShot = false;
		missilePos = D2D1::RectF(0, 0, 0, 0);
		metroidDead = true;
	}

#pragma endregion

	Update();
}

void Game::Render(void)
{
	pRT->BeginDraw();

	pRT->DrawBitmap(backgnd, D2D1::RectF(0, 0, pRT->GetSize().width, pRT->GetSize().height));

	hpBarFill = D2D1::RectF(1, 1, GetHealth() * 4, 20);
	DarkHpBarFill = D2D1::RectF(800, 1, 800 + GetDarkHealth() * 4, 20);
	energyBarFill = D2D1::RectF(1, 25, GetEnergy() * 4, 45);

	hpBarBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
	pRT->DrawRectangle(hpBar, hpBarBrush, 2.0f, NULL);
	pRT->DrawRectangle(DarkHpBar, hpBarBrush, 2.0f, NULL);
	pRT->DrawRectangle(energyBar, hpBarBrush);

	hpBarBrush->SetColor(D2D1::ColorF(D2D1::ColorF::LimeGreen));
	pRT->FillRectangle(hpBarFill, hpBarBrush);
	hpBarBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Yellow));
	pRT->FillRectangle(energyBarFill, hpBarBrush);
	hpBarBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Purple));
	pRT->FillRectangle(DarkHpBarFill, hpBarBrush);

	if (!(animationJump)){
		shipPos.top = samusPos.top + 75;
		shipPos.bottom = shipPos.top + 100;
	}
	pRT->DrawBitmap(ship, shipPos);
#pragma region Samus
	if (!(metroidAtk)){
		if (frame == 0 && animation){
			pRT->DrawBitmap(bigBlastCharge, samusPos);
		}
		else if (frame == 1 && animation){
			pRT->DrawBitmap(blastStance, samusPos);
		}
		else if (frame == 2 && animation){
			pRT->DrawBitmap(blastCharging, samusPos);
		}
		else if (frame == 3 && animation){
			pRT->DrawBitmap(smallBlastCharge, samusPos);
		}
		else if (frame == 4 && animation){
			pRT->DrawBitmap(bigBlastCharge, samusPos);
		}
		else if (!(animationJump) && !(animationFall) && !ultimate && !MissileShot)
			pRT->DrawBitmap(spriteIdle, samusPos);
	}
	else{
		pRT->DrawBitmap(metroidAttack, samusPos);
	}
	
	if (MissileShot){
		if (missileFrame == 0){
			pRT->DrawBitmap(ultStance, samusPos);
		}
		else if (missileFrame == 1){
			pRT->DrawBitmap(m1, samusPos);
		}
		else if (missileFrame == 2){
			pRT->DrawBitmap(m2, samusPos);
		}
		else if (missileFrame == 3){
			pRT->DrawBitmap(m3, samusPos);
		}		
		else if (missileFrame == 4){
			pRT->DrawBitmap(m5, samusPos);
			missilePos.top = samusPos.top + 55;
			missilePos.bottom = missilePos.top + 36;
			missilePos.left = samusPos.left + 60;
			missilePos.right = missilePos.left + 36;
		}
		else{
			MissileShot = false;
			missileManuever = true;
		}
	}

	if (missileManuever){
		pRT->DrawBitmap(missiles, missilePos);
	}

	if (ultimate){
		if (ultimateframe <= 3){
			pRT->DrawBitmap(ultStance, samusPos);
		}
		else if (ultimateframe <= 4){
			pRT->DrawBitmap(ult1, samusPos);
		}
		else if (ultimateframe <= 5){
			pRT->DrawBitmap(ult2, samusPos);
		}
		else if (ultimateframe <= 6){
			pRT->DrawBitmap(ult3, samusPos);
			laserPos = samusPos;
			laserPos.top = samusPos.top + 25;
			laserPos.bottom = laserPos.top + 100;
			laserPos.left = samusPos.left + 30;
		}
		else{

			if (laserPos.left < pRT->GetSize().width){
				pRT->DrawBitmap(ultStance, samusPos);
				pRT->DrawBitmap(ultBeam, laserPos);
			}
			else{
				ultimate = false;
				frame = 0;
				animation = true;
			}
		}
	}
#pragma endregion

#pragma region Dark Samus	
	if (teleportFrame % 100 == teleportTimer && !darkAnimation){
  		teleport = true;

		teleportFrame = 0;
	}
	if (frame == 0 && darkAnimation){
		pRT->DrawBitmap(eb6, darkSamusPos);
	}
	else if (frame == 1 && darkAnimation){
		pRT->DrawBitmap(eb5, darkSamusPos);
	}
	else if (frame == 2 && darkAnimation){
		pRT->DrawBitmap(eb4, darkSamusPos);
	}
	else if (frame == 3 && darkAnimation){
		pRT->DrawBitmap(eb3, darkSamusPos);
	}
	else if (frame == 4 && darkAnimation){
		pRT->DrawBitmap(eb2, darkSamusPos);
	}
	else if (frame == 5 && darkAnimation){
		pRT->DrawBitmap(eb1, darkSamusPos);
	}
	else if (teleport && !darkAnimation){
		if (teleportFrame == 0){
			pRT->DrawBitmap(tele1, darkSamusPos);
		}
		else if (teleportFrame == 1){
			pRT->DrawBitmap(tele2, darkSamusPos);
		}
		else if (teleportFrame == 2){
			pRT->DrawBitmap(tele3, darkSamusPos);
		}
		else if (teleportFrame == 3){
			pRT->DrawBitmap(tele4, darkSamusPos);
		}
		else if (teleportFrame == 4){
			pRT->DrawBitmap(tele5, darkSamusPos);
		}
		else if (teleportFrame == 5){
			pRT->DrawBitmap(tele6, darkSamusPos);
		}
		else if (teleportFrame == 6){
			telePosx = rand() % 601;			
			darkSamusPos.top = (float)telePosx;
			darkSamusPos.bottom = darkSamusPos.top + 200;

			pRT->DrawBitmap(tele6, darkSamusPos);
			if (teleportTimer > 20)
				teleportTimer -= 5;
			teleport = false;
		}
	}
	else
		pRT->DrawBitmap(darkSamus, darkSamusPos);
#pragma endregion

#pragma region Jump & Fall
	if (animationJump || animationFall){
		bool tmp = false;
		if (samusPos.top - 20 < 0 || samusPos.bottom + 20 > pRT->GetSize().height){
			tmp = true;
		}
		if (jumpFrame == 0 && animationJump){
			pRT->DrawBitmap(jump, samusPos);
		}
		else if (jumpFrame == 1 && animationJump){
			samusPos.top -= 5;
			samusPos.bottom -= 5;
			pRT->DrawBitmap(tuck1, samusPos);
		}
		else if (jumpFrame == 2 && animationJump){
			samusPos.top -= 5;
			samusPos.bottom -= 5;
			pRT->DrawBitmap(screw1, samusPos);
		}
		else if (jumpFrame == 3 && animationJump){			
			if (tmp){
				samusPos.top += 5;
				samusPos.bottom += 5;
			}
			else{
				samusPos.top -= 5;
				samusPos.bottom -= 5;
			}
			pRT->DrawBitmap(screw2, samusPos);
		}
		else if (jumpFrame == 4 && animationJump){			
			if (tmp){
				samusPos.top += 5;
				samusPos.bottom += 5;
			}
			else{
				samusPos.top -= 5;
				samusPos.bottom -= 5;
			}
			pRT->DrawBitmap(tuck1, samusPos);
		}
		else if (jumpFrame == 5 && animationJump){
			pRT->DrawBitmap(land, samusPos);
		}
		else if (jumpFrame == 6 && animationJump){
			animationJump = false;
		}
////////////////////////////////////////////////////////////////////
		if (fallFrame == 0){
			pRT->DrawBitmap(tuck1, samusPos);
		}
		else if (fallFrame == 1){
			samusPos.top += 5;
			samusPos.bottom += 5;
			pRT->DrawBitmap(morph1, samusPos);
		}
		else if (fallFrame == 2){
			samusPos.top += 5;
			samusPos.bottom += 5;
			pRT->DrawBitmap(morph2, samusPos);
		}
		else if (fallFrame == 3){
			if (!tmp){
				samusPos.top += 5;
				samusPos.bottom += 5;
			}
			else{
				samusPos.top -= 5;
				samusPos.bottom -= 5;
			}
			pRT->DrawBitmap(morph1, samusPos);
		}
		else if (fallFrame == 4){
			if (!tmp){
				samusPos.top += 5;
				samusPos.bottom += 5;
			}
			else{
				samusPos.top -= 5;
				samusPos.bottom -= 5;
			}
			pRT->DrawBitmap(fallStance, samusPos);
		}
		else if (fallFrame == 5){
			pRT->DrawBitmap(land, samusPos);
		}
		else if (fallFrame == 6){
			animationFall = false;
		}
	}
#pragma endregion

	if (!metroidCharging && !metroidAtk && !metroidDead)
		pRT->DrawBitmap(metroid, metroidPos);
	else if (!metroidAtk && !metroidDead)
		pRT->DrawBitmap(metroidCharge, metroidPos);
	else if (metroidDead){
		pRT->DrawBitmap(metroidFall, metroidPos);
	}
	
	if (!(animation || ultimate)){
		if (ExplosionCount < 2)
			pRT->DrawBitmap(blastBall, blastBallpos);

		else if (ExplosionCount < 4)
			pRT->DrawBitmap(blastBallsmall, blastBallpos);

		else if (ExplosionCount >= 5)
			pRT->DrawBitmap(blastBallmed, blastBallpos);

		else 
			pRT->DrawBitmap(blastBallhigh, blastBallpos);
	}

	HRESULT hr = pRT->EndDraw();
	if (hr == D2DERR_RECREATE_TARGET)
	{
		//TODO: Destroy Direct2D.
		DestroyGraphics();
		//TODO: Recreate Direct2D.
		CreateGraphics(hWnd);
	}
}

HRESULT Game::CreateGraphics(HWND hWnd)
{
#pragma region Misc
	// Initialize the Direct2D Factory.
	HRESULT hr;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the Direct2D Factory."),
			_T("Direct2D Error"), MB_OK | MB_ICONERROR);
		return hr;
	}


	// Get the dimensions of the client.
	RECT rc;
	GetClientRect(hWnd, &rc);

	// Initialize a Direct2D Size Structure.
	D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);


	// Create the Direct2D Render Target.
	hr = pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(hWnd, size), &pRT);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the Direct2D Render Target."),
			_T("Direct2D Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	pRT->SetDpi(96.0f, 96.0f);


	// Create the Direct2D Solid Color Brush.
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the Direct2D Solid Color Brush."),
			_T("Direct2D Error"), MB_OK | MB_ICONERROR);
		return hr;
	}


	// Initialize the DirectWrite Factory.
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
		(IUnknown**)&pDWFactory);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the DirectWrite Factory."),
			_T("DirectWrite Error"), MB_OK | MB_ICONERROR);
		return hr;
	}


	// Create the DirectWrite Text Format.
	hr = pDWFactory->CreateTextFormat(_T("Veranda"), NULL,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		24, _T(""), &pTF);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the DirectWrite Text Format."),
			_T("DirectWrite Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWICFactory));
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	// Create the Direct2D Solid Color Brush.
	hr = pRT->CreateSolidColorBrush(D2D1::ColorF(0x0), &hpBarBrush);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create the Direct2D Solid Color Brush."),
			_T("Direct2D Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

#pragma endregion

	// Load Background ////////////////////////////////////////////////////////////////////////////////////////
	hr = LoadBitmapFromFile(L"MetroidBackgroundSpawn1.jpg", &backgnd);
	if (FAILED(hr)){
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	// Load Sprite

#pragma region Samus
	hr = LoadBitmapFromFile(L"Ship.png", &ship);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"Idle.png", &spriteIdle);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}	
	hr = LoadBitmapFromFile(L"BlastStance.png", &blastStance);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"BlastCharging.png", &blastCharging);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"SmallBlastCharge.png", &smallBlastCharge);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"SamusBlast.png", &bigBlastCharge);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"Jump.png", &jump);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"Tuck.png", &tuck1);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"Screw1.png", &screw1);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"Screw2.png", &screw2);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"Land.png", &land);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"FallStance.png", &fallStance);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"Morphball1.png", &morph1);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"Morphball2.png", &morph2);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"M5.png", &m5);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"M1.png", &m1);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"M2.png", &m2);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"M3.png", &m3);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"M4.png", &m4);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
#pragma endregion

#pragma region Weapons
	hr = LoadBitmapFromFile(L"Blast.png", &blastBall);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"BlastSmall.png", &blastBallsmall);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"BlastMed.png", &blastBallmed);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"BlastHigh.png", &blastBallhigh);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"Missile.png", &missiles);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
#pragma endregion

#pragma region Dark Samus
	hr = LoadBitmapFromFile(L"DarkSamus.png", &darkSamus);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"BeginEnergyBlast.png", &eb1);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"TinyEnergyBlast.png", &eb2);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"SmallEnergyBlast.png", &eb3);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"DarkEnergyBlast.png", &eb4);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"DissapatingEnergyBlast.png", &eb5);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"EndEnergyBlast.png", &eb6);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}

	hr = LoadBitmapFromFile(L"tele6.png", &tele1);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"tele5.png", &tele2);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"tele4.png", &tele3);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"tele3.png", &tele4);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"tele2.png", &tele5);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"tele1.png", &tele6);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
#pragma endregion

#pragma region Metroid
	hr = LoadBitmapFromFile(L"BabyMetroid.png", &metroid);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"BabyMetroidAttack.png", &metroidAttack);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"BabyMetroidCharging.png", &metroidCharge);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"BabyMetroidHurt.png", &metroidFall);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
#pragma endregion

#pragma region Ultimate
	hr = LoadBitmapFromFile(L"UltimateStance.png", &ultStance);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"Ultimate1.png", &ult1);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"Ultimate2.png", &ult2);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"Ultimate3.png", &ult3);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
	hr = LoadBitmapFromFile(L"UltimateLaser.png", &ultBeam);
	if (FAILED(hr))
	{
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to Create WIC Interface."),
			_T("WIC Error"), MB_OK | MB_ICONERROR);
		return hr;
	}
#pragma endregion

	return S_OK; // Success!
}

void Game::DestroyGraphics(void)
{
	//Release WIC Stuff
	SafeRelease(&pWICFactory);
	SafeRelease(&backgnd);
	// Samus
	SafeRelease(&spriteIdle);
	SafeRelease(&blastStance);
	SafeRelease(&blastCharging);
	SafeRelease(&smallBlastCharge);
	SafeRelease(&bigBlastCharge);
	SafeRelease(&screw1);
	SafeRelease(&screw2);
	SafeRelease(&tuck1);
	SafeRelease(&land);
	SafeRelease(&fallStance);
	SafeRelease(&morph1);
	SafeRelease(&morph2);
	// Ball
	SafeRelease(&blastBall);
	SafeRelease(&blastBallsmall);
	SafeRelease(&blastBallmed);
	SafeRelease(&blastBallhigh);
	// Metroid
	SafeRelease(&metroid);
	SafeRelease(&metroidAttack);
	// Dark Samus
	SafeRelease(&darkSamus);
	SafeRelease(&eb1);
	SafeRelease(&eb2);
	SafeRelease(&eb3);
	SafeRelease(&eb4);
	SafeRelease(&eb5);
	SafeRelease(&eb6);

	SafeRelease(&tele1);
	SafeRelease(&tele2);
	SafeRelease(&tele3);
	SafeRelease(&tele4);
	SafeRelease(&tele5);
	SafeRelease(&tele6);

	// Misc
	SafeRelease(&hpBarBrush);
	SafeRelease(&pTF);
	SafeRelease(&pDWFactory);
	SafeRelease(&pRT);
	// Release the Direct2D Factory.
	SafeRelease(&pD2DFactory);
}

HRESULT Game::LoadBitmapFromFile(LPCTSTR strFileName, ID2D1Bitmap** ppBitmap)
{
	HRESULT hr;

	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICFormatConverter *pConverter = NULL;


	hr = pWICFactory->CreateDecoderFromFilename(
		strFileName,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
		);
	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}
	if (SUCCEEDED(hr))
	{

		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = pWICFactory->CreateFormatConverter(&pConverter);
	}
	if (SUCCEEDED(hr))
	{
		hr = pConverter->Initialize(
			pSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeMedianCut
			);
	}
	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRT->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
			);
	}

	SafeRelease(&pDecoder);
	SafeRelease(&pSource);
	SafeRelease(&pConverter);

	return hr;
}

void Game::Update(){

	if (GetTickCount() > dwTime){
		frame++;
		jumpFrame++;
		fallFrame++;
		ultimateframe++;
		missileFrame++;
		teleportFrame++;
		if (metroidAtkTimer >= 0){
			metroidAtkTimer--;
		}
		else{
			metroidAtkTimer = -1;
		}

		if (ultimate && ultimateframe > 6 && laserPos.right < pRT->GetSize().width + 100){	
			laserPos.left -= 20;
			laserPos.right += 60;
			laserPos.top -= 4;
			laserPos.bottom += 4;
		}
		else if (ultimate && ultimateframe > 6 && laserPos.left < pRT->GetSize().width){
			laserPos.right += 20;
			laserPos.left += 50;
		}
		dwTime = GetTickCount() + 100;
	}

	if (frame == 4){
		animation = false;
	}
	else if (frame == 15 && metroidAtk){	
		SetHealth(GetHealth() - 2);
		metroidAtkTimer = 60;
		metroidAtk = false;
	}
	else if (frame == 6){
		darkAnimation = false;
	}
	if (metroidAtkTimer == 0){
		metroidCharging = false;
		metroidPos = D2D1::RectF(darkSamusPos.left, darkSamusPos.top, metroidPos.left + 100, metroidPos.top + 100);
	}

// Samus Update
	if (!ultimate){
		if (!(animation)){
			blastBallpos.left += LRmoveFrame;
			blastBallpos.right += LRmoveFrame;
			blastBallpos.top += TBmoveFrame;
			blastBallpos.bottom += TBmoveFrame;
			if (blastBallpos.right - 50 > pRT->GetSize().width)
			{
				ExplosionCount++;
				LRmoveFrame = -(LRmoveFrame);
			}
			else if (blastBallpos.left + 80 < 0)
			{
				

				if (ExplosionCount < 2){
					SetHealth(GetHealth() - 2);
				}
				else if (ExplosionCount < 4){
					SetHealth(GetHealth() - 5);
				}
				else{
					SetHealth(GetHealth() - 10);
				}

				ExplosionCount++;
				LRmoveFrame = -(LRmoveFrame);
			}

			if (blastBallpos.bottom - 70 > pRT->GetSize().height)
			{
				TBmoveFrame = -TBmoveFrame;
			}
			else if (blastBallpos.top + 70 < 0)
			{
				TBmoveFrame = -TBmoveFrame;
			}
		}
		// Blast Ball Update
		if (animation){
			blastBallpos.top = samusPos.top;
			blastBallpos.bottom = blastBallpos.top + 200;

			blastBallpos.left = samusPos.left + 2;
			blastBallpos.right = samusPos.left + 200;
		}

		// Missile
		if (missileManuever && missilePos.right < pRT->GetSize().width){
			missilePos.left += missileVec.width;
			missilePos.right += missileVec.width;
		}
		else{
			missileManuever = false;
		}
		// Metroid Update
		if (!metroidCharging && !metroidDead){
			metroidPos.left -= 1.75;
			metroidPos.right -= 1.75;
		}
		else if (metroidDead && !metroidAtk && !metroidCharging){
			metroidPos.top += 5;
			metroidPos.bottom += 5;
			if (metroidPos.top > pRT->GetSize().height){
				metroidPos.left = -100;
				metroidPos.right = -10;
				metroidAtkTimer = 50;
				metroidDead = false;
			}
		}
		else{
			if (metroidPos.right > 0){
				metroidPos.left -= 6;
				metroidPos.right -= 6;
			}
			if (metroidPos.top < samusPos.top){
				metroidPos.top += 8;
			}
			else if (metroidPos.top > samusPos.top){
				metroidPos.top -= 8;
			}
			else{

			}
			metroidPos.bottom = metroidPos.top + 100;
		}

		if (metroidPos.left <= samusPos.right + 225 && !metroidCharging){
			metroidCharging = true;
		}
	}
}

void Game::EndGame(){
	if (darkCurrHealth < 1 || currHealth < 1){
		GamePlay = false;
	}
}