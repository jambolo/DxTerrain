/** @file *//********************************************************************************************************

                                                      Terrain.h

						                    Copyright 2004, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/Terrain.h#2 $

	$NoKeywords: $

 ********************************************************************************************************************/

#pragma once

#include "Dxx/Light.h"
#include "Dxx/Camera.h"
#include <d3dx9math.h>		// for PerformanceInfo::m_Position
#include <list>

class Sector;
struct IDirect3DDevice9;


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

class Terrain
{
public:

	struct PerformanceInfo
	{
		int			m_PatchCount;
		int			m_TriangleCount;
		int			m_SectorCount;
		D3DXVECTOR3	m_Position;
	};

	struct DebugSettings
	{
		bool	bDrawWireframe;
		bool	bSortSectors;
	};

	Terrain( float x, float y,
			 IDirect3DDevice9 * pD3dDevice,
			 Dxx::DirectionalLight const & light,
			 Dxx::Camera const & camera );

	~Terrain();

	void Update( __int64 t );

	void Draw( Dxx::Camera const & camera );

	float GetZ( float x, float y ) const;

	// Debugging

	PerformanceInfo const & GetPerformanceInfo() const		{ return m_PerformanceInfo; }
	static void AddConsoleVariables();
	static DebugSettings	m_DebugSettings;

private:

	typedef std::list< Sector * >	SectorList;
	SectorList						m_Sectors;

	float							m_X0, m_Y0;
	IDirect3DDevice9 *				m_pD3dDevice;
	PerformanceInfo					m_PerformanceInfo;
};