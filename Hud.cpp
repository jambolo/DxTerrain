/** @file *//********************************************************************************************************

                                                       Hud.cpp

						                    Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/Hud.cpp#13 $

	$NoKeywords: $

 ********************************************************************************************************************/

#include "PrecompiledHeaders.h"

#include "Hud.h"

#include "GMMTerrain/Terrain.h"

#include "Console/Console.h"
#include "Time/FrameRateCalculator.h"
#include "Wx/Wx.h"

using namespace std;

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

Hud::Hud( IDirect3DDevice9 * pD3dDevice )
	: m_pD3dDevice( pD3dDevice ),
	m_bDisplayConsole( false )
{
	pD3dDevice->AddRef();

	Console::Instance()->AddVariable( "consoledisplay", &m_bDisplayConsole );

	InitializeFonts( pD3dDevice );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

Hud::~Hud()
{
	ShutDownFonts();

	Wx::SafeRelease( m_pD3dDevice );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Hud::Update( __int64 t )
{
	m_FrameRateCalculator.Update( t );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Hud::DisplayConsole( bool turnOn )
{
	m_bDisplayConsole = turnOn;

	if ( !turnOn )
	{
		Console::Instance()->ClearCommandLine();
	}
}

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Hud::Draw( Terrain::PerformanceInfo const & info )
{
	char	buffer[ 256 ];
	RECT	rect			= { 0, 0, 320, 16 };

	sprintf( buffer, "x: %7.1f  y: %7.1f  z: %7.1f", info.m_Position.x, info.m_Position.y, info.m_Position.z );

	m_pFont->DrawText( NULL, buffer, -1, &rect, 0, D3DCOLOR_XRGB( 255, 255, 0 )  );
	rect.top += 16;
	rect.bottom += 16;

	sprintf( buffer, "Frame rate: %6.2f fps (%6.2f fps average)",
			 m_FrameRateCalculator.GetFrameRate(),
			 m_FrameRateCalculator.GetAverageFrameRate() );

	m_pFont->DrawText( NULL, buffer, -1, &rect, 0, D3DCOLOR_XRGB( 255, 255, 0 )  );
	rect.top += 16;
	rect.bottom += 16;

	sprintf( buffer, "Triangles: %7d", info.m_TriangleCount );

	m_pFont->DrawText( NULL, buffer, -1, &rect, 0, D3DCOLOR_XRGB( 255, 255, 0 )  );
	rect.top += 16;
	rect.bottom += 16;

	sprintf( buffer, "Patches: %3d", info.m_PatchCount );

	m_pFont->DrawText( NULL, buffer, -1, &rect, 0, D3DCOLOR_XRGB( 255, 255, 0 )  );
	rect.top += 16;
	rect.bottom += 16;

	sprintf( buffer, "Sectors: %3d", info.m_SectorCount );

	m_pFont->DrawText( NULL, buffer, -1, &rect, 0, D3DCOLOR_XRGB( 255, 255, 0 )  );
	rect.top += 16;
	rect.bottom += 16;

	if ( m_bDisplayConsole )
	{
		rect.top += 16;
		rect.bottom += 16;
		for ( Console::History::const_iterator psLine = Console::Instance()->GetHistory().begin();
			  psLine != Console::Instance()->GetHistory().end();
			  ++psLine )
		{
			m_pFont->DrawText( NULL, *psLine, -1, &rect, 0, D3DCOLOR_XRGB( 255, 255, 0 )  );
			rect.top += 16;
			rect.bottom += 16;
		}

		string	commandLine	= Console::Instance()->GetCommandLine() + '_';
		m_pFont->DrawText( NULL, commandLine.c_str(), -1, &rect, 0, D3DCOLOR_XRGB( 255, 127, 0 )  );
	}
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Hud::InitializeFonts( IDirect3DDevice9 * pD3dDevice )
{
	OutputDebugString( "***InitializeFonts\n" );

	HRESULT		hr;

	static D3DXFONT_DESC	desc;

	hr = D3DXCreateFontIndirect( pD3dDevice, &desc, &m_pFont );
	assert_succeeded( hr );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Hud::ShutDownFonts()
{
	OutputDebugString( "***ShutDownFonts\n" );

	Wx::SafeRelease( m_pFont );
}



