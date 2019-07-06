/** @file *//********************************************************************************************************

                                                     Terrain.cpp

						                    Copyright 2004, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/Terrain.cpp#4 $

	$NoKeywords: $

 ********************************************************************************************************************/

#include "PrecompiledHeaders.h"

#include "Terrain.h"

#include "Sector.h"
#include "Dxx/Camera.h"
#include "Dxx/Light.h"
#include "Wx/Wx.h"
#include "Console/Console.h"

using namespace std;


namespace
{

// A class used to sort the sector list from closest to farthest
class SectorListSorter
{
public:
	SectorListSorter( float x, float y ) : m_X( x ), m_Y( y ) {}

	// Returns true if a is closer than b
	bool operator()( Sector const * pA, Sector const *pB )
	{
		float da2	= ( pA->m_X0 - m_X ) * ( pA->m_X0 - m_X ) + ( pA->m_Y0 - m_Y ) * ( pA->m_Y0 - m_Y );
		float db2	= ( pB->m_X0 - m_X ) * ( pB->m_X0 - m_X ) + ( pB->m_Y0 - m_Y ) * ( pB->m_Y0 - m_Y );

		return ( da2 < db2 );
	}

	float	m_X;
	float	m_Y;
};


} // anonymous namespace

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

Terrain::DebugSettings	Terrain::m_DebugSettings	= { false, true };

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

Terrain::Terrain( float x0, float y0,
				  IDirect3DDevice9 * pD3dDevice,
				  Dxx::DirectionalLight const & light,
				  Dxx::Camera const & camera )
	: m_X0( x0 ), m_Y0( y0 ),
	m_pD3dDevice( pD3dDevice )
{
	m_pD3dDevice->AddRef();

	try
	{
		for ( int i = 0; i < 5; i++ )
		{
			float const		y	= y0 + i * Sector::SIZE;

			for ( int j = 0; j < 5; j++ )
			{
				float const		x	= x0 + j * Sector::SIZE;

				Sector * const	pSector = new Sector( x, y, pD3dDevice, light, camera );
				if ( pSector == 0 ) throw bad_alloc();

				m_Sectors.push_back( pSector );
			}
		}
	}
	catch ( ... )
	{
		this->~Terrain();
		throw;
	}
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

Terrain::~Terrain()
{
	for ( SectorList::iterator ppSector = m_Sectors.begin(); ppSector != m_Sectors.end(); ++ppSector )
	{
		delete *ppSector;
	}

	Wx::SafeRelease( m_pD3dDevice );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Terrain::Update( __int64 t )
{
	for ( SectorList::iterator ppSector = m_Sectors.begin(); ppSector != m_Sectors.end(); ++ppSector )
	{
		Sector * const	pSector		= *ppSector;

		pSector->Update( t );
	}
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Terrain::Draw( Dxx::Camera const & camera )
{
	HRESULT		hr;

	m_PerformanceInfo.m_PatchCount		= 0;
	m_PerformanceInfo.m_TriangleCount	= 0;
	m_PerformanceInfo.m_SectorCount		= 0;
	m_PerformanceInfo.m_Position		= camera.GetPosition();

	if ( m_DebugSettings.bSortSectors )
	{
		m_Sectors.sort( SectorListSorter( camera.GetPosition().x, camera.GetPosition().y ) );
	}

	if ( m_DebugSettings.bDrawWireframe )
	{
		hr = m_pD3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
		assert_succeeded( hr );
	}

	for ( SectorList::iterator ppSector = m_Sectors.begin(); ppSector != m_Sectors.end(); ++ppSector )
	{
		Sector * const	pSector		= *ppSector;

		pSector->Draw( camera );

		m_PerformanceInfo.m_PatchCount		+= pSector->GetPerformanceInfo().m_PatchCount;
		m_PerformanceInfo.m_TriangleCount	+= pSector->GetPerformanceInfo().m_TriangleCount;
		m_PerformanceInfo.m_SectorCount		+= 1;
	}

	if ( m_DebugSettings.bDrawWireframe )
	{
		hr = m_pD3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
		assert_succeeded( hr );
	}
}

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

float Terrain::GetZ( float x, float y ) const
{
	for ( SectorList::const_iterator ppSector = m_Sectors.begin(); ppSector != m_Sectors.end(); ++ppSector )
	{
		Sector * const	pSector		= *ppSector;

		if (    x >= pSector->m_X0
			 && x < pSector->m_X0 + Sector::SIZE
			 && y >= pSector->m_Y0
			 && y < pSector->m_Y0 + Sector::SIZE )
		{
			return pSector->GetZ( x, y );
		}
	}

	return 0.0f;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Terrain::AddConsoleVariables()
{
	Console::Instance()->AddVariable( "wireframe", &m_DebugSettings.bDrawWireframe );
	Console::Instance()->AddVariable( "sortsectors", &m_DebugSettings.bSortSectors );
	Sector::AddConsoleVariables();
}
