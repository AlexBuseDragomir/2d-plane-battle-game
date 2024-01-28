//-----------------------------------------------------------------------------
// File: CGameApp.cpp
//
// Desc: Game Application class, this is the central hub for all app processing
//
// Original design by Adam Hoult & Gary Simmons. Modified by Mihai Popescu.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CGameApp Specific Includes
//-----------------------------------------------------------------------------
#include "CGameApp.h"
#include <algorithm>
extern HINSTANCE g_hInst;

using namespace std;

//-----------------------------------------------------------------------------
// CGameApp Member Functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CGameApp () (Constructor)
// Desc : CGameApp Class Constructor
//-----------------------------------------------------------------------------
CGameApp::CGameApp()
{
	// Reset / Clear all required values
	m_hWnd			= NULL;
	m_hIcon			= NULL;
	m_hMenu			= NULL;
	m_pBBuffer		= NULL;
	m_pPlayer		= NULL;
	m_pPlayer1		= NULL;
	m_LastFrameRate = 0;
}

//-----------------------------------------------------------------------------
// Name : ~CGameApp () (Destructor)
// Desc : CGameApp Class Destructor
//-----------------------------------------------------------------------------
CGameApp::~CGameApp()
{
	// Shut the engine down
	ShutDown();
}

//-----------------------------------------------------------------------------
// Name : InitInstance ()
// Desc : Initialises the entire Engine here.
//-----------------------------------------------------------------------------
bool CGameApp::InitInstance( LPCTSTR lpCmdLine, int iCmdShow )
{
	// Create the primary display device
	if (!CreateDisplay()) { ShutDown(); return false; }

	// Build Objects
	if (!BuildObjects()) 
	{ 
		MessageBox( 0, _T("Failed to initialize properly. Reinstalling the application may solve this problem.\nIf the problem persists, please contact technical support."), 
			_T("Fatal Error"), MB_OK | MB_ICONSTOP);
		
		ShutDown(); 
		return false; 
	}

	// Set up all required game states
	SetupGameState();

	// Success!
	return true;
}

//-----------------------------------------------------------------------------
// Name : CreateDisplay ()
// Desc : Create the display windows, devices etc, ready for rendering.
//-----------------------------------------------------------------------------
bool CGameApp::CreateDisplay()
{
	LPTSTR			WindowTitle		= _T("GameFramework");
	LPCSTR			WindowClass		= _T("GameFramework_Class");
					Width			= 1920;
					Height			= 1080;
	RECT			rc;
	WNDCLASSEX		wcex;


	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= CGameApp::StaticWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= g_hInst;
	wcex.hIcon			= LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= WindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON));

	if(RegisterClassEx(&wcex)==0)
		return false;

	// Retrieve the final client size of the window
	::GetClientRect( m_hWnd, &rc );
	m_nViewX		= rc.left;
	m_nViewY		= rc.top;
	m_nViewWidth	= rc.right - rc.left;
	m_nViewHeight	= rc.bottom - rc.top;

	m_hWnd = CreateWindow(WindowClass, WindowTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, Width, Height, NULL, NULL, g_hInst, this);

	if (!m_hWnd)
		return false;

	// Show the window
	ShowWindow(m_hWnd, SW_MAXIMIZE);
	//ShowWindow(m_hWnd, SW_MAXIMIZE);

	// Success!!
	return true;
}

//-----------------------------------------------------------------------------
// Name : BeginGame ()
// Desc : Signals the beginning of the physical post-initialisation stage.
//		From here on, the game engine has control over processing.
//-----------------------------------------------------------------------------
int CGameApp::BeginGame()
{
	MSG		msg;

	// Start main loop
	while(true) 
	{
		// Did we recieve a message, or are we idling ?
		if ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) 
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage( &msg );
			DispatchMessage ( &msg );
		} 
		else 
		{
			// Advance Game Frame.
			FrameAdvance();

		} // End If messages waiting
	
	} // Until quit message is receieved

	return 0;
}

//-----------------------------------------------------------------------------
// Name : ShutDown ()
// Desc : Shuts down the game engine, and frees up all resources.
//-----------------------------------------------------------------------------
bool CGameApp::ShutDown()
{
	// Release any previously built objects
	ReleaseObjects ( );
	
	// Destroy menu, it may not be attached
	if (m_hMenu)
	{
		DestroyMenu(m_hMenu);
	}

	m_hMenu		 = NULL;

	// Destroy the render window
	SetMenu( m_hWnd, NULL );

	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
	}

	m_hWnd		  = NULL;
	
	// Shutdown Success
	return true;
}

//-----------------------------------------------------------------------------
// Name : StaticWndProc () (Static Callback)
// Desc : This is the main messge pump for ALL display devices, it captures
//		the appropriate messages, and routes them through to the application
//		class for which it was intended, therefore giving full class access.
// Note : It is VITALLY important that you should pass your 'this' pointer to
//		the lpParam parameter of the CreateWindow function if you wish to be
//		able to pass messages back to that app object.
//-----------------------------------------------------------------------------
LRESULT CALLBACK CGameApp::StaticWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	// If this is a create message, trap the 'this' pointer passed in and store it within the window.
	if ( Message == WM_CREATE ) SetWindowLong( hWnd, GWL_USERDATA, (LONG)((CREATESTRUCT FAR *)lParam)->lpCreateParams);

	// Obtain the correct destination for this message
	CGameApp *Destination = (CGameApp*)GetWindowLong( hWnd, GWL_USERDATA );
	
	// If the hWnd has a related class, pass it through
	if (Destination) return Destination->DisplayWndProc( hWnd, Message, wParam, lParam );
	
	// No destination found, defer to system...
	return DefWindowProc( hWnd, Message, wParam, lParam );
}

//-----------------------------------------------------------------------------
// Name : DisplayWndProc ()
// Desc : The display devices internal WndProc function. All messages being
//		passed to this function are relative to the window it owns.
//-----------------------------------------------------------------------------
LRESULT CGameApp::DisplayWndProc( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam )
{
	static UINT			fTimer;	

	// Determine message type
	switch (Message)
	{
		case WM_CREATE:
			break;
		
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		
		case WM_SIZE:
			if ( wParam == SIZE_MINIMIZED )
			{
				// App is inactive
				m_bActive = false;
			
			} // App has been minimized
			else
			{
				// App is active
				m_bActive = true;

				// Store new viewport sizes
				m_nViewWidth  = LOWORD( lParam );
				m_nViewHeight = HIWORD( lParam );
		
			
			} // End if !Minimized

			break;

		case WM_LBUTTONDOWN:
			// Capture the mouse
			SetCapture( m_hWnd );
			GetCursorPos( &m_OldCursorPos );
			break;

		case WM_LBUTTONUP:
			// Release the mouse
			ReleaseCapture( );
			break;

		case WM_KEYDOWN:
			switch(wParam)
			{
			case VK_ESCAPE:
				PostQuitMessage(0);
				break;
			case VK_RETURN:
				fTimer = SetTimer(m_hWnd, 1, 70, NULL);
				m_pPlayer->Explode();
				m_pPlayer->m_pSprite->mVelocity = Vec2(0, 0);
				m_pPlayer->Position() = Vec2(100, 900);
				break;
			case 'Q':
				fTimer = SetTimer(m_hWnd, 1, 70, NULL);
				m_pPlayer1->Explode();
				m_pPlayer1->m_pSprite->mVelocity = Vec2(0, 0);
				m_pPlayer1->Position() = Vec2(1800, 900);
				break;
			case VK_SPACE:
				m_pPlayer->Shoot();
				break;
			case VK_CONTROL:
				m_pPlayer1->Shoot();
				break;
			case VK_F1:
				Save_game();
				break;
			case VK_F2:
				fTimer = SetTimer(m_hWnd, 1, 70, NULL);
				Load_game();
				break;
			}
			

			break;

		case WM_TIMER:
			switch(wParam)
			{
			case 1:
				if(!m_pPlayer->AdvanceExplosion())
					KillTimer(m_hWnd, 1);
				if (!m_pPlayer1->AdvanceExplosion())
					KillTimer(m_hWnd, 1);
			}
			break;

		case WM_COMMAND:
			break;

		default:
			return DefWindowProc(hWnd, Message, wParam, lParam);

	} // End Message Switch
	
	return 0;
}

//-----------------------------------------------------------------------------
// Name : BuildObjects ()
// Desc : Build our demonstration meshes, and the objects that instance them
//-----------------------------------------------------------------------------
bool CGameApp::BuildObjects()
{
	m_pBBuffer = new BackBuffer(m_hWnd, m_nViewWidth, m_nViewHeight);
	m_pPlayer = new CPlayer(m_pBBuffer);
	m_pPlayer1 = new CPlayer(m_pBBuffer);

	Enemy enemy_plane(m_pBBuffer);
	Enemy enemy_plane1(m_pBBuffer);
	Enemy enemy_plane2(m_pBBuffer);

	enemyOnScreen.push_back(enemy_plane);
	enemyOnScreen.push_back(enemy_plane1);
	enemyOnScreen.push_back(enemy_plane2);
	
	if (!m_imgBackground_2.LoadBitmapFromFile("data/background-2.bmp", GetDC(m_hWnd)))
	{
		return false;
	}

	if (!m_imgBackground_1.LoadBitmapFromFile("data/background-1.bmp", GetDC(m_hWnd)))
	{
		return false;
	}
		
	if (!m_imgBackground0.LoadBitmapFromFile("data/background0.bmp", GetDC(m_hWnd)))
	{
		return false;
	}
	
	if (!m_imgBackground1.LoadBitmapFromFile("data/background1.bmp", GetDC(m_hWnd)))
	{
		return false;
	}
	
	if (!m_imgBackground2.LoadBitmapFromFile("data/background2.bmp", GetDC(m_hWnd)))
	{
		return false;
	}
		
	// Success!
	return true;
}

//-----------------------------------------------------------------------------
// Name : SetupGameState ()
// Desc : Sets up all the initial states required by the game.
//-----------------------------------------------------------------------------
void CGameApp::SetupGameState()
{
	m_pPlayer->Position() = Vec2(100, 900);
	m_pPlayer1->Position() = Vec2(1800, 900);
}

//-----------------------------------------------------------------------------
// Name : ReleaseObjects ()
// Desc : Releases our objects and their associated memory so that we can
//		rebuild them, if required, during our applications life-time.
//-----------------------------------------------------------------------------
void CGameApp::ReleaseObjects( )
{
	if(m_pPlayer != NULL)
	{
		delete m_pPlayer;
		m_pPlayer = NULL;
	}

	if(m_pBBuffer != NULL)
	{
		delete m_pBBuffer;
		m_pBBuffer = NULL;
	}
}

//-----------------------------------------------------------------------------
// Name : FrameAdvance () (Private)
// Desc : Called to signal that we are now rendering the next frame.
//-----------------------------------------------------------------------------
void CGameApp::FrameAdvance()
{
	static TCHAR FrameRate[ 50 ];
	static TCHAR TitleBuffer[ 255 ];

	// Advance the timer
	m_Timer.Tick( );

	// Skip if app is inactive
	if ( !m_bActive ) return;
	
	// Get / Display the framerate
	if ( m_LastFrameRate != m_Timer.GetFrameRate() )
	{
		m_LastFrameRate = m_Timer.GetFrameRate( FrameRate, 50 );
		sprintf_s( TitleBuffer, _T("2D Plane Battle Game : %s"), FrameRate );
		SetWindowText( m_hWnd, TitleBuffer );

	} // End if Frame Rate Altered

	// Poll & Process input devices
	ProcessInput();

	// Animate the game objects
	AnimateObjects();

	// Drawing the game objects
	DrawObjects();
}

//-----------------------------------------------------------------------------
// Name : ProcessInput () (Private)
// Desc : Simply polls the input devices and performs basic input operations
//-----------------------------------------------------------------------------
void CGameApp::ProcessInput( )
{
	static UCHAR pKeyBuffer[ 256 ];
	ULONG		Direction = 0;
	ULONG		Direction1 = 0;
	POINT		CursorPos;
	float		X = 0.0f, Y = 0.0f;

	// Retrieve keyboard state
	if ( !GetKeyboardState( pKeyBuffer ) ) return;

	// Check the relevant keys
	if ( pKeyBuffer[ VK_UP	] & 0xF0 ) Direction |= CPlayer::DIR_FORWARD;
	if ( pKeyBuffer[ VK_DOWN  ] & 0xF0 ) Direction |= CPlayer::DIR_BACKWARD;
	if ( pKeyBuffer[ VK_LEFT  ] & 0xF0 ) Direction |= CPlayer::DIR_LEFT;
	if ( pKeyBuffer[ VK_RIGHT ] & 0xF0 ) Direction |= CPlayer::DIR_RIGHT;

	if (pKeyBuffer[ 'W' ] & 0xF0) Direction1 |= CPlayer::DIR_FORWARD;
	if (pKeyBuffer[ 'S' ] & 0xF0) Direction1 |= CPlayer::DIR_BACKWARD;
	if (pKeyBuffer[ 'A' ] & 0xF0) Direction1 |= CPlayer::DIR_LEFT;
	if (pKeyBuffer[ 'D' ] & 0xF0) Direction1 |= CPlayer::DIR_RIGHT;

	
	// Move the player
	
	m_pPlayer->Move(Direction);
	m_pPlayer1->Move(Direction1);

	// Now process the mouse (if the button is pressed)
	if ( GetCapture() == m_hWnd )
	{
		// Hide the mouse pointer
		SetCursor( NULL );

		// Retrieve the cursor position
		GetCursorPos( &CursorPos );

		// Reset our cursor position so we can keep going forever :)
		SetCursorPos( m_OldCursorPos.x, m_OldCursorPos.y );

	} // End if Captured
}


// Method that tests if two entities collide
// We calculate a frame for our two objects by adding their 
bool CGameApp :: Sprite_Collide(Sprite *entity1, Sprite *entity2) 
{

	double left1, left2;
	double right1, right2;
	double top1, top2;
	double bottom1, bottom2;


	left1 = entity1->mPosition.x - (entity1->width() / 2);
	left2 = entity2->mPosition.x - (entity2->width() / 2);

	right1 = left1 + entity1->width();
	right2 = left2 + entity2->width();

	top1 = entity1->mPosition.y - (entity1->height() / 2);
	top2 = entity2->mPosition.y - (entity2->height() / 2);

	bottom1 = top1 + entity1->height();
	bottom2 = top2 + entity2->height();

	if (bottom1 < top2)
	{
		return false;
	}

	else if (top1 > bottom2)
	{
		return false;
	}

	else if (right1 < left2) 
	{
		return false;
	}
	
	else if (left1 > right2) 
	{
		return false;
	}

	else
	{
		return true;
	}
};

//-----------------------------------------------------------------------------
// Name : AnimateObjects () (Private)
// Desc : Animates the objects we currently have loaded.
//-----------------------------------------------------------------------------
void CGameApp::AnimateObjects()
{
	m_pPlayer->Update(m_Timer.GetTimeElapsed());
	m_pPlayer1->Update(m_Timer.GetTimeElapsed());
}


//-----------------------------------------------------------------------------
// Name : DrawObjects () (Private)
// Desc : Draws the game objects
//-----------------------------------------------------------------------------
void CGameApp::DrawObjects()
{
	static UINT			fTimer;
	m_pBBuffer->reset();

	if (this->plane_lives == 2 && this->enemy_lives != -1)
	{
		m_imgBackground2.Paint(m_pBBuffer->getDC(), 0, 0);
	}

	else if (this->plane_lives == 1 && this->enemy_lives != -1)
	{
		m_imgBackground1.Paint(m_pBBuffer->getDC(), 0, 0);
	}

	else if (this->plane_lives == 0 && this->enemy_lives != -1)
	{
		m_imgBackground0.Paint(m_pBBuffer->getDC(), 0, 0);
	}

	else if(this->plane_lives == -1 && this->enemy_lives != -1)
	{
		m_imgBackground_1.Paint(m_pBBuffer->getDC(), 0, 0);
	}

	else if (enemy_lives == -1)
	{
		m_imgBackground_2.Paint(m_pBBuffer->getDC(), 0, 0);
	}

	// Draw the players if no one has won yet
	if (plane_lives != -1 && enemy_lives != -1)
	{
		m_pPlayer->Draw();
		m_pPlayer1->Draw();
	}
	
	//If there is no enemy, we create an enemy and save it
	if (enemyOnScreen.size() == 0 && enemy_lives != -1)
	{
		if (enemy_lives != -1)
		{
			Enemy enemy_plane(m_pBBuffer);
			Enemy enemy_plane1(m_pBBuffer);
			Enemy enemy_plane2(m_pBBuffer);

			enemy_plane.m_pSprite->mPosition = Vec2(950, 70);
			enemy_plane1.m_pSprite->mPosition = Vec2(350, 70);
			enemy_plane2.m_pSprite->mPosition = Vec2(1550, 70);


			enemyOnScreen.push_back(enemy_plane);
			enemyOnScreen.push_back(enemy_plane1);
			enemyOnScreen.push_back(enemy_plane2);
		}
	}

	// we update the enemy position by searching for them in the container
	for(auto &it : enemyOnScreen) 
	{
		// turn on the enemy plane if he has lives left and the player is still alive
		if (enemy_lives != -1 && plane_lives != -1)
		{
			it.shootCooldown--;
			it.move();
			it.m_pSprite->draw();
			it.Shoot();
		}

		// if planes get too close, our plane will explode (the enemy wins)
		if (Sprite_Collide(it.m_pSprite, m_pPlayer->m_pSprite))
		{
			fTimer = SetTimer(m_hWnd, 1, 70, NULL);
			m_pPlayer->Explode();
			m_pPlayer->m_pSprite->mVelocity = Vec2(0, 0);
			m_pPlayer->Position() = Vec2(100, 900);
		}

		if (Sprite_Collide(it.m_pSprite, m_pPlayer1->m_pSprite))
		{
			fTimer = SetTimer(m_hWnd, 1, 70, NULL);
			m_pPlayer1->Explode();
			m_pPlayer1->m_pSprite->mVelocity = Vec2(0, 0);
			m_pPlayer1->Position() = Vec2(1800, 900);
		}
	}

	// we do things like above for the bullets in the container
	for (auto &it : bulletsOnScreen)
	{
		it.Move();
		it.m_pSprite->draw();
		
		// we get an iterator to the first and currently only enemy plane
		// we also get iterators to the second and third enemy planes
		auto enemy_it = enemyOnScreen.begin();
		auto enemy_it1 = std::next(enemy_it);
		auto enemy_it2 = std::next(enemy_it1);

		// like for the planes, the enemies have 3 lives
		if (Sprite_Collide(it.m_pSprite, enemy_it->m_pSprite) && this->enemy_lives > 0 && it.owner == "player")
		{
			it.m_pSprite->mPosition = Vec2(1070, 0);
			it.m_pSprite->mVelocity = Vec2(0, 0);
			
			enemy_it->hit = true;
			
			this->enemy_lives--;
		}

		if (Sprite_Collide(it.m_pSprite, enemy_it1->m_pSprite) && this->enemy_lives > 0 && it.owner == "player")
		{
			it.m_pSprite->mPosition = Vec2(1070, 0);
			it.m_pSprite->mVelocity = Vec2(0, 0);
			
			enemy_it1->hit = true;
			
			this->enemy_lives--;
		}

		if (Sprite_Collide(it.m_pSprite, enemy_it2->m_pSprite) && this->enemy_lives > 0 && it.owner == "player")
		{
			it.m_pSprite->mPosition = Vec2(1070, 0);
			it.m_pSprite->mVelocity = Vec2(0, 0);
			
			enemy_it2->hit = true;
			
			this->enemy_lives--;
		}

		// if enemies dont have lives left and one of them gets hit, we win the game
		else if ( (Sprite_Collide(it.m_pSprite, enemy_it->m_pSprite) || Sprite_Collide(it.m_pSprite, enemy_it1->m_pSprite) || 
			Sprite_Collide(it.m_pSprite, enemy_it2->m_pSprite)) && this->enemy_lives == 0 && it.owner == "player")
		{
			fTimer = SetTimer(m_hWnd, 1, 70, NULL);

			enemy_it->m_pSprite->mPosition = Vec2(950, 70);
			enemy_it->m_pSprite->mVelocity = Vec2(0, 0);

			it.m_pSprite->mVelocity = Vec2(0, 0);
			it.m_pSprite->mPosition = Vec2(1070, 0);

			this->enemy_lives = -1;
		}
		
		// if the bullets hit the players for 3 times, they will lose
		if (Sprite_Collide(it.m_pSprite, m_pPlayer->m_pSprite) && this->plane_lives > 0 && it.owner == "enemy")
		{
			it.m_pSprite->mPosition = Vec2(1070, 0);
			it.m_pSprite->mVelocity = Vec2(0, 0);
			this->plane_lives--;
		}

		else if (Sprite_Collide(it.m_pSprite, m_pPlayer->m_pSprite) && this->plane_lives == 0  && it.owner == "enemy")
		{
			fTimer = SetTimer(m_hWnd, 1, 70, NULL);
			
			m_pPlayer->Explode();
			
			m_pPlayer->m_pSprite->mVelocity = Vec2(0, 0);
			m_pPlayer->m_pSprite->mPosition = Vec2(100, 900);
			
			it.m_pSprite->mVelocity = Vec2(0, 0);
			it.m_pSprite->mPosition = Vec2(1070, 0);
			
			this->plane_lives = -1;
		}

		if (Sprite_Collide(it.m_pSprite, m_pPlayer1->m_pSprite) && this->plane_lives > 0 && it.owner == "enemy")
		{
			it.m_pSprite->mPosition = Vec2(1070, 0);
			it.m_pSprite->mVelocity = Vec2(0, 0);
			this->plane_lives--;
		}

		else if (Sprite_Collide(it.m_pSprite, m_pPlayer1->m_pSprite) && this->plane_lives == 0 && it.owner == "enemy") 
		{
			fTimer = SetTimer(m_hWnd, 1, 70, NULL);
			
			m_pPlayer1->Explode();
			
			m_pPlayer1->m_pSprite->mVelocity = Vec2(0, 0);
			m_pPlayer1->m_pSprite->mPosition = Vec2(1800, 900);
			
			it.m_pSprite->mPosition = Vec2(1070, 0);
			it.m_pSprite->mVelocity = Vec2(0, 0);
			
			this->plane_lives = -1;
		}
	}

	// Remove a bullet if it gets close to the margin of the screen
	// A lambda function from STL that checks if the bullet is close to the limit of the screen
	// If true, the bullet from the container will be removed by the remove_if algorithm

	bulletsOnScreen.remove_if([=](Bullet bullet) 
	{ return (bullet.m_pSprite->mPosition.y < 35 || bullet.m_pSprite->mPosition.y > 960
		|| plane_lives == -1 || enemy_lives == -1) ? true : false;
	});

	// Remove an enemy if it gets close to the margin of the screen
	// A lambda function from STL that checks if the enemy is close to the limit of the screen
	// If true, the enemy from the container will be removed by the remove_if algorithm
	// In the main case we will have just an enemy object, so the container will contain
	// At most one element in the base case
	
	enemyOnScreen.remove_if([=](Enemy enemy_plane){
		return (enemy_plane.m_pSprite->mPosition.y < 35 || 
			enemy_plane.m_pSprite->mPosition.y > 960) ? true : false;
	});

	m_pBBuffer->present();
}

// Function that will write the current plane coordinates into a text file in order to retrieve them
// later by pressing the 'L' - Load buttom
void CGameApp::Save_game()
{
	// we empty the text file every time we save
	ofstream fout;
	fout.open("game_data.txt", ofstream::out | ofstream::trunc);

	// we get an iterator to the first and currently only enemy plane
	// we also get iterators to the second and third enemy planes
	auto enemy_it = enemyOnScreen.begin();
	auto enemy_it1 = std::next(enemy_it);
	auto enemy_it2 = std::next(enemy_it1);
	
	// we save in the file the positions of the two players and the positions of the three enemy planes
	fout << "Plane1:" << " " <<  m_pPlayer->m_pSprite->mPosition.x << " " <<  m_pPlayer->m_pSprite->mPosition.y << endl;
	fout << "Plane2:" << " " << m_pPlayer1->m_pSprite->mPosition.x << " " << m_pPlayer1->m_pSprite->mPosition.y << endl;
	fout << "Enemy1:" << " " << enemy_it->m_pSprite->mPosition.x << " " << enemy_it->m_pSprite->mPosition.y << endl;
	fout << "Enemy2:" << " " << enemy_it1->m_pSprite->mPosition.x << " " << enemy_it1->m_pSprite->mPosition.y << endl;
	fout << "Enemy3:" << " " << enemy_it2->m_pSprite->mPosition.x << " " << enemy_it2->m_pSprite->mPosition.y << endl;

	// we also save the number of lives of friendly and enemy planes
	fout << "FriendlyLives:" << " " << plane_lives << endl;
	fout << "EnemyLives:" << " " << enemy_lives;

	// close the file
	fout.close();
}

// Function that will read data from a text file and initialize the game with the last coordinates
// of the planes (before the moment of save)
void CGameApp::Load_game()
{
	PlaySound("data/Song.wav", NULL, SND_FILENAME | SND_ASYNC);
	
	ifstream fin("game_data.txt");

	// the coordinates of the 2 friendly planes and the 3 enemy planes
	double p1x, p2x, e1x, e2x, e3x;
	double p1y, p2y, e1y, e2y, e3y;
	double pLives, eLives;

	// we get an iterator to the first and currently only enemy plane
	// we also get iterators to the second and third enemy planes
	auto enemy_it = enemyOnScreen.begin();
	auto enemy_it1 = std::next(enemy_it);
	auto enemy_it2 = std::next(enemy_it1);

	// we want to get rid of the non useful text between the plane positions ("Plane x position =" and " ")
	string garbage;

	// we load the old data regarding the position
	fin >> garbage >> p1x >> p1y;
	fin >> garbage >> p2x >> p2y;
	fin >> garbage >> e1x >> e1y;
	fin >> garbage >> e2x >> e2y;
	fin >> garbage >> e3x >> e3y;

	m_pPlayer->m_pSprite->mVelocity = Vec2(0, 0);
	m_pPlayer->Position() = Vec2(p1x, p1y);
	
	m_pPlayer1->m_pSprite->mVelocity = Vec2(0, 0);
	m_pPlayer1->Position() = Vec2(p2x, p2y);
	
	enemy_it->m_pSprite->mVelocity = Vec2(0, 0);
	enemy_it->m_pSprite->mPosition = Vec2(e1x, e1y);
	
	enemy_it1->m_pSprite->mVelocity = Vec2(0, 0);
	enemy_it1->m_pSprite->mPosition = Vec2(e2x, e2y);
	
	enemy_it2->m_pSprite->mVelocity = Vec2(0, 0);
	enemy_it2->m_pSprite->mPosition = Vec2(e3x, e3y);

	// we load the lives of friendly and enemy planes
	fin >> garbage >> pLives;
	fin >> garbage >> eLives;

	// we update the plane lives
	plane_lives = pLives;
	enemy_lives = eLives;

	// close the file 
	fin.close();
}
