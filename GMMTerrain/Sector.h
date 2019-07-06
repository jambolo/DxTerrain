/** @file *//********************************************************************************************************

                                                      Sector.h

						                    Copyright 2004, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/Sector.h#5 $

	$NoKeywords: $

 ********************************************************************************************************************/

#pragma once

#include "Patch.h"
#include "Dxx/Camera.h"
#include "Dxx/Light.h"
#include "Math/Box.h"
#include "QuadTree/QuadTree.h"

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#if defined( _DEBUG )
	#define D3D_DEBUG_INFO
#endif
#include <d3d9.h>
#include <d3dx9.h>

class HeightField;


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

class Sector
{
public:

	struct PerformanceInfo
	{
		int		m_PatchCount;
		int		m_TriangleCount;
	};

	Sector( float x0, float y0,
			IDirect3DDevice9 * pD3dDevice,
			Dxx::DirectionalLight const & light,
			Dxx::Camera const & camera );
	~Sector();

	void Update( __int64 t );

	void Draw( Dxx::Camera const & camera );

	float GetZ( float x, float y ) const;

	PerformanceInfo const & GetPerformanceInfo() const		{ return m_PerformanceInfo; }
	static void AddConsoleVariables();

	static float const	SIZE/*	= 256.0f * XYSCALE*/;
	float const			m_X0;
	float const			m_Y0;

private:

	enum { MAX_DETAIL_TEXTURES = 4 };

	// Quad tree node

	struct NodeInfo
	{
		float	m_X0, m_Y0;				// Base location
		float	m_Size;					// Size in each direction (in world coordinates)
		int		m_nVerticesPerSide;		// Number of vertices in each direction
		int		m_nPatchesPerSide;		// Number of patches in each direction
		AABox	m_BoundingBox;			// AA bounding box for the node
		Patch *	m_pPatch;

		NodeInfo()
		{
		}

		NodeInfo( float x0, float y0, float size, int nv, int np, AABox const & bbox, Patch * pPatch )
			: m_X0( x0 ), m_Y0( y0 ), m_Size( size ),
			m_nVerticesPerSide( nv ),
			m_nPatchesPerSide( np ),
			m_BoundingBox( bbox ),
			m_pPatch( pPatch )
		{
		}
	};


	typedef QuadTree< NodeInfo >		PatchQuadTree;
	typedef PatchQuadTree::Node			PatchNode;
	
	void LoadHeightField( char const * sFileName );
	void CreatePatches( float x, float y, Dxx::Camera const & camera );
	PatchNode * CreatePatchNode( int i, int j, float x0, float y0, int size, Dxx::Camera const & camera  );
	void LoadTextures( char const * sFileName );
	void LoadShaders( char const * sFileName );
	void GenerateLightMap( Dxx::DirectionalLight const & light );
	void DrawPartiallyVisibleQuadTreeNode( PatchNode const * pNode, Dxx::Camera const & camera );
	void DrawFullyVisibleQuadTreeNode( PatchNode const * pNode, Dxx::Camera const & camera );
	static void NodeInfoCleaner( NodeInfo * pNodeInfo ) { delete pNodeInfo->m_pPatch; }

	HeightField *					m_pHeightField;
	IDirect3DDevice9 *				m_pD3dDevice;
	IDirect3DTexture9 *				m_apDetailTextures[ MAX_DETAIL_TEXTURES ];
	IDirect3DTexture9 *				m_pDetailSelector;
	IDirect3DTexture9 *				m_pLightMap;
	PatchQuadTree					m_QuadTree;				// Quadtree containing the patches
	Dxx::Camera const *				m_pCamera;
	ID3DXEffect *					m_pEffect;
	float							m_LightMapUVScale;

	// Performance info
	PerformanceInfo			m_PerformanceInfo;
};
