/** @file *//********************************************************************************************************

                                                       main.cpp

						                    Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/main.cpp#21 $

	$NoKeywords: $

 ********************************************************************************************************************/


#include "PrecompiledHeaders.h"

#include "Hud.h"
#include "GMMTerrain/Terrain.h"

#include "DxObjects/TerrainCamera/TerrainCamera.h"
#include "DxObjects/SkyBox/SkyBox.h"
#include "Dxx/Dxx.h"
#include "Math/Constants.h"
#include "Math/FastMath.h"
#include "TgaFile/TgaFile.h"
#include "Time/Clock.h"
#include "Time/FrameRateCalculator.h"
#include "Wx/Wx.h"
#include "Console/Console.h"

using namespace std;

static LRESULT CALLBACK WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
static bool Update( HWND hWnd );
static void Display();
static void Reshape( HWND hWnd, int w, int h );
static void InitializeRendering( HWND hWnd );
static void ShutDownRendering();
static void InitializeApp();
static void ShutDownApp();
//static void DrawTeapot();
static void FixCameraPosition( __int64 time );
static void UpdateMovement( __int64 time );

static void UpdateMouse( __int64 time );
static void ShutDownMouse();
static void InitializeMouse( HINSTANCE hInstance, HWND hWnd );

static char					s_sAppName[]	= "DxTerrain";
static char					s_sTitleBar[]	= "DxTerrain";

static DxObjects::TerrainCamera *	s_pCamera;
static Dxx::Camera *				s_pDebugCamera;
static float						s_CameraSpeed				= 1.5f;
static float						s_CameraTurnSpeed			= 90.0f;
static float						s_MouseSensitivity			= 0.2f;

static Hud *				s_pHud;

static DxObjects::SkyBox *	s_pSkyBox;
static Terrain *				s_pTerrain;

static IDirect3D9 *			s_pD3d;
static IDirect3DDevice9 *	s_pD3dDevice;
static D3DCAPS9				s_Caps;

//static ID3DXMesh *			s_pTeapot;
static D3DMATERIAL9			s_TeapotMaterial	=
							{
								{ 1.0f, 1.0f, 1.0f, 1.0f },
								{ 1.0f, 1.0f, 1.0f, 1.0f },
								{ 1.0f, 1.0f, 1.0f, 1.0f },
								{ 0.0f, 0.0f, 0.0f, 0.0f },
								0.0f
							};


static Dxx::DirectionalLight *	s_pLight;

static bool		s_bFixCamera	= true;
static float	s_CameraHeight	= 2.0f;
static float	s_RecoveryRate	= 2.0f;

static LPDIRECTINPUT8		s_pDI;
static LPDIRECTINPUTDEVICE8	s_pMouseDevice;

static HINSTANCE	s_hInstance;
static HWND			s_hWnd;

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow )
{
	Console::Instance()->AddVariable( "cameraspeed", &s_CameraSpeed );
	Console::Instance()->AddVariable( "fixcamera", &s_bFixCamera );
	Console::Instance()->AddVariable( "cameraheight", &s_CameraHeight );
	Console::Instance()->AddVariable( "recoveryrate", &s_RecoveryRate );
	Console::Instance()->AddVariable( "cameraturnspeed", &s_CameraTurnSpeed );
	Terrain::AddConsoleVariables();

	s_hInstance = hInstance;

	if ( Wx::RegisterWindowClass(	CS_OWNDC,
									( WNDPROC )WindowProc,
									hInstance,
									s_sAppName,
									LoadCursor( NULL, IDC_ARROW )
								) == NULL )
	{
		MessageBox( NULL, "Wx::RegisterWindowClass() failed.", "Error", MB_OK );
		exit( 1 );
	}

	HWND hWnd = CreateWindowEx( 0,
								s_sAppName,
								s_sTitleBar,
								WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
								0, 0, 800, 600,
								NULL,
								NULL,
								hInstance,
								NULL );

	if ( hWnd == NULL )
	{
		MessageBox( NULL, "CreateWindowEx() failed.", "Error", MB_OK );
		exit( 1 );
	}

	s_hWnd		= hWnd;

	ShowWindow( hWnd, nCmdShow );
	ShowCursor( FALSE );

	return Wx::MessageLoop( hWnd, Update );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

static LRESULT CALLBACK WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_CREATE:
		OutputDebugString( "***WM_CREATE\n" );
		InitializeRendering( hWnd );
		InitializeMouse( s_hInstance, hWnd );
		InitializeApp();
		return 0;

	case WM_SIZE:
		OutputDebugString( "***WM_SIZE\n" );
		Reshape( hWnd, LOWORD( lParam ), HIWORD( lParam ) );
		return 0;

//	case WM_PAINT:
//		Display();
//		ValidateRect( hWnd, NULL );
//		return 0;

	case WM_CHAR:
	{
		if ( s_pHud->ConsoleIsDisplayed() )
		{
			switch ( wParam )
			{
			case VK_RETURN:
				Console::Instance()->Execute();
				break;

			case VK_ESCAPE:
			case '`':
				s_pHud->DisplayConsole( false );
				break;

			default:
				Console::Instance()->Input( wParam );
				break;
			}
		}
		else
		{
			switch ( wParam )
			{
			case VK_ESCAPE:	// Quit
				PostMessage( hWnd, WM_CLOSE, 0, 0 );
				break;

			case '`':
				s_pHud->DisplayConsole( true );
				break;
			}
		}

		return 0;
	}

	case WM_CLOSE:
		ShutDownApp();
		ShutDownMouse();
		ShutDownRendering();
		DestroyWindow( hWnd );
		return 0;

	case WM_DESTROY:
		PostQuitMessage( 0 );
		return 0;
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam ); 
} 


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

static bool Update( HWND hWnd )
{
	__int64 const	t	= Clock::Instance()->GetTime();

	s_pTerrain->Update( t );

	s_pHud->Update( t );
	
	UpdateMovement( t );

	// Display the world

	Display();

	return true;	// Call this function as often as possible
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

static void Display()
{
	HRESULT		hr;

	// Clear the viewport

	hr = s_pD3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, D3DCOLOR_XRGB( 64, 64, 64 ), 1.0f, 0 );
	assert_succeeded( hr );

	// Begin the scene

	hr = s_pD3dDevice->BeginScene();
	assert_succeeded( hr );

	// Place the camera

	s_pCamera->Look();

	// Draw the skybox

	s_pSkyBox->Draw( *s_pCamera );

	// Lights

	s_pLight->Enable();

	// Draw the terrain

	hr = s_pD3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
	assert_succeeded( hr );

	hr = s_pD3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	assert_succeeded( hr );

	hr = s_pD3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	assert_succeeded( hr );

	s_pTerrain->Draw( *s_pCamera );

//	DrawTeapot();

	// Draw the hud

	s_pHud->Draw( s_pTerrain->GetPerformanceInfo() );

	hr = s_pD3dDevice->EndScene();
	assert_succeeded( hr );

	hr = s_pD3dDevice->Present(	NULL, NULL,	NULL, NULL );
	assert_succeeded( hr );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/


static void Reshape( HWND hWnd, int w, int h )
{
//	HRESULT		hr;
//
//	OutputDebugString( "***ResetD3dWindowedDevice\n" );
//	hr = Dxx::ResetD3dWindowedDevice( s_pD3dDevice, D3DFMT_D24S8 );
//	assert_succeeded( hr );

	OutputDebugString( "***s_pCamera->Reshape\n" );
	s_pCamera->SetAspectRatio( float( w ), float( h ) );
	s_pCamera->Reshape();
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

static void InitializeRendering( HWND hWnd )
{
	OutputDebugString( "***InitializeRendering\n" );

	HRESULT		hr;

	// Create the D3D and D3DDevice objects

	s_pD3d = Direct3DCreate9( D3D_SDK_VERSION );
	hr = Dxx::CreateD3dWindowedDevice( s_pD3d, hWnd, &s_pD3dDevice, D3DFMT_D24S8, D3DADAPTER_DEFAULT, D3DPRESENT_INTERVAL_IMMEDIATE );
//	hr = Dxx::CreateD3dWindowedDevice( s_pD3d, hWnd, &s_pD3dDevice, D3DFMT_D24S8, D3DADAPTER_DEFAULT, D3DPRESENT_INTERVAL_IMMEDIATE, D3DDEVTYPE_REF );
//	hr = Dxx::CreateD3dFullScreenDevice( s_pD3d, hWnd, 1280, 1024, D3DFMT_X8R8G8B8, &s_pD3dDevice, D3DFMT_D24S8, D3DADAPTER_DEFAULT, D3DPRESENT_INTERVAL_IMMEDIATE );//, D3DDEVTYPE_REF  );
	assert_succeeded( hr );
	assert_succeeded( hr != S_FALSE );

	// Query the device's caps

	s_pD3dDevice->GetDeviceCaps( &s_Caps );

	// Make sure the necssary caps are supported

	assert( s_Caps.VertexShaderVersion >= D3DVS_VERSION( 1, 1 ) );	// vs 1.1
	assert( s_Caps.PixelShaderVersion >= D3DVS_VERSION( 1, 4 ) );	// ps 1.4

	hr = s_pD3dDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_COLORVALUE( 0.25, 0.27, 0.30, 1.0 ) );
	assert_succeeded( hr );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

static void ShutDownRendering()
{
	OutputDebugString( "***ShutDownRendering\n" );

	Wx::SafeRelease( s_pD3dDevice );
	Wx::SafeRelease( s_pD3d );
}



/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

static void InitializeApp()
{
	OutputDebugString( "***InitializeApp\n" );

	Clock::Instantiate();

	s_pHud = new Hud( s_pD3dDevice );

	// Create the camera

	s_pCamera	= new DxObjects::TerrainCamera( s_pD3dDevice, 60.0f, 0.1f, 1000.0f, 800.0f / 600.0f );

	// Create a light

	s_pLight = new Dxx::DirectionalLight( s_pD3dDevice, 0,
										  Dxx::Sunlight(),
										  Dxx::NoColor(),
										  Dxx::NoColor(),
										  D3DXVECTOR3( -0.6f, -0.4f, -0.6f ) );

	s_pSkyBox = new DxObjects::SkyBox( s_pD3dDevice, "res/Skybox", ".tga" );
	if ( s_pSkyBox == 0 ) throw bad_alloc();

	s_pTerrain = new Terrain( 0.0f, 0.0f, s_pD3dDevice, *s_pLight, *s_pCamera );
	if ( s_pTerrain == 0 ) throw bad_alloc();

//	// Create the reference models
//
//	hr = D3DXCreateTeapot( s_pD3dDevice, &s_pTeapot, NULL );
//	assert_succeeded( hr );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

static void ShutDownApp()
{
	OutputDebugString( "***ShutDownApp\n" );

//	SAFE_RELEASE( s_pTeapot );

	delete s_pSkyBox;
	s_pSkyBox = 0;

	delete s_pTerrain;
	s_pTerrain = 0;

	delete s_pLight;
	s_pLight = 0;

	delete s_pCamera;
	s_pCamera = 0;

	delete s_pHud;
	s_pHud = 0;
}


///********************************************************************************************************************/
///*																													*/
///********************************************************************************************************************/
//
//static void DrawTeapot()
//{
//	HRESULT		hr;
//	D3DXMATRIX	world;
//
//	// By default, the teapot's up is down the Y-axis. Rotate it PI/2 around the X-axis to fix that.
//
//	D3DXMatrixRotationX( &world, (float)Math::PI_OVER_2 );
//
//	hr = s_pD3dDevice->SetTransform( D3DTS_WORLD, &world );
//	assert_succeeded( hr );
//
//	hr = s_pD3dDevice->SetMaterial( &s_TeapotMaterial );
//	assert_succeeded( hr );
//
//	hr = s_pD3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
//	assert_succeeded( hr );
//
//	hr = s_pD3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
//	assert_succeeded( hr );
//
//	hr = s_pTeapot->DrawSubset( 0 );
//	assert_succeeded( hr );
//}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

static void FixCameraPosition( __int64 time )
{
	static __int64	last_time	= Clock::Instance()->GetTime();

	float const		delta_time		= (float)Clock::ToSeconds( time - last_time );

	D3DXVECTOR3	position	= s_pCamera->GetPosition();
	float		terrainZ	= s_pTerrain->GetZ( position.x, position.y );

	position.z = terrainZ + s_CameraHeight;
//	position.z += ( terrainZ + s_CameraHeight - position.z ) * delta_time * s_RecoveryRate;

	s_pCamera->SetPosition( position );

	last_time = time;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

static void UpdateMovement( __int64 time )
{
	static __int64	last_time	= Clock::Instance()->GetTime();

	float const		delta_time	= (float)Clock::ToSeconds( time - last_time );

	UpdateMouse( time );

	if ( !s_pHud->ConsoleIsDisplayed() )
	{
		if ( GetAsyncKeyState( VK_SPACE ) < 0 )	// Forward
		{
			s_pCamera->Move( Dxx::Vector3ZAxis() * s_CameraSpeed * delta_time );
		}

		if ( GetAsyncKeyState( 'S' ) < 0 )	// Backwards
		{
			s_pCamera->Move( -Dxx::Vector3ZAxis() * s_CameraSpeed * delta_time );
		}

		if ( GetAsyncKeyState( 'D' ) < 0 )	// Strafe right
		{
			s_pCamera->Move( Dxx::Vector3XAxis() * s_CameraSpeed * delta_time );
		}

		if ( GetAsyncKeyState( 'A' ) < 0 )	// Strafe left
		{
			s_pCamera->Move( -Dxx::Vector3XAxis() * s_CameraSpeed * delta_time );
		}

		if ( GetAsyncKeyState( 'W' ) < 0 )	// Strafe up
		{
			s_pCamera->Move( Dxx::Vector3YAxis() * s_CameraSpeed * delta_time );
		}

		if ( GetAsyncKeyState( 'X' ) < 0 )	// Strafe down
		{
			s_pCamera->Move( -Dxx::Vector3YAxis() * s_CameraSpeed * delta_time );
		}

		if ( GetAsyncKeyState( VK_PRIOR ) < 0 )
		{
			s_pCamera->Roll( -s_CameraTurnSpeed * delta_time );
		}

		if ( GetAsyncKeyState( VK_INSERT ) < 0 )
		{
			s_pCamera->Roll( s_CameraTurnSpeed * delta_time );
		}
	}

	// Make sure the camera is exactly 2 meters over the ground

	if ( s_bFixCamera )
	{
		FixCameraPosition( time );
	}

	last_time = time;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

static void InitializeMouse( HINSTANCE hInstance, HWND hWnd )
{
	OutputDebugString( "***InitializeMouse\n" );

    HRESULT hr;

    hr = DirectInput8Create( hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&s_pDI, NULL );
	assert_succeeded( hr );
    
	hr = s_pDI->CreateDevice( GUID_SysMouse, &s_pMouseDevice, NULL );
	assert_succeeded( hr );
    
    hr = s_pMouseDevice->SetDataFormat( &c_dfDIMouse2 );
	assert_succeeded( hr );
    
    hr = s_pMouseDevice->SetCooperativeLevel( hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND );
	assert_succeeded( hr );

    s_pMouseDevice->Acquire();

	Console::Instance()->AddVariable( "mousesensitivity", &s_MouseSensitivity );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

static void ShutDownMouse()
{
	OutputDebugString( "***ShutDownMouse\n" );

	if( s_pMouseDevice )
	{
        s_pMouseDevice->Unacquire();
	}
    
    Wx::SafeRelease( s_pMouseDevice );
    Wx::SafeRelease( s_pDI );
}

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

static void UpdateMouse( __int64 time )
{
    HRESULT       hr;
    DIMOUSESTATE2 dims2;      // DirectInput mouse state structure

    if ( s_pMouseDevice == NULL )
	{
        return;
	}
    
    memset( &dims2, 0, sizeof(dims2) );
    hr = s_pMouseDevice->GetDeviceState( sizeof(DIMOUSESTATE2), &dims2 );
    if ( FAILED( hr ) ) 
    {
        hr = s_pMouseDevice->Acquire();
        while ( hr == DIERR_INPUTLOST )
		{
            hr = s_pMouseDevice->Acquire();
		}
        return; 
    }

	s_pCamera->Yaw( s_MouseSensitivity * dims2.lX );
	s_pCamera->Pitch( s_MouseSensitivity * dims2.lY );
	s_pCamera->Roll( s_MouseSensitivity * dims2.lZ );
}
