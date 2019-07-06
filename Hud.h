/** @file *//********************************************************************************************************

                                                        Hud.h

						                    Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/Hud.h#9 $

	$NoKeywords: $

 ********************************************************************************************************************/

#pragma once

#include "GMMTerrain/Terrain.h"
#include "Time/FrameRateCalculator.h"

struct IDirect3DDevice9;
struct ID3DXFont;


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

class Hud
{
public:

	Hud( IDirect3DDevice9 * pD3dDevice );
	~Hud();

	void Update( __int64 t );

	void Draw( Terrain::PerformanceInfo const & info );

	void DisplayConsole( bool turnOn );
	bool ConsoleIsDisplayed() const		{ return m_bDisplayConsole; }

private:

	void InitializeFonts( IDirect3DDevice9 * pD3dDevice );
	void ShutDownFonts();

	IDirect3DDevice9 *		m_pD3dDevice;
	FrameRateCalculator		m_FrameRateCalculator;
	ID3DXFont *				m_pFont;
	bool					m_bDisplayConsole;
};
