/** @file *//********************************************************************************************************

                                                     Sector.cpp

						                    Copyright 2004, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/Sector.cpp#6 $

	$NoKeywords: $

 ********************************************************************************************************************/

#include "PrecompiledHeaders.h"

#include "Sector.h"

#include "Patch.h"
#include "PatchVB.h"

#include "LightMapFactory.h"

#include "Dxx/Dxx.h"
#include "HeightField/HeightField.h"
#include "HeightField/HeightFieldLoader.h"
#include "Misc/Assert.h"
#include "QuadTree/QuadTree.h"
#include "Wx/Wx.h"
#include "Math/Frustum.h"

using namespace std;


namespace
{
	float const	XYSCALE				= 1.0f;
	float const	ZSCALE				= 30.0f;


int const * _GetDrawOrder( float dx, float dy )
{
	// Figure out the order to draw the nodes front to back

	static int const	drawOrderMap[4][4] =
	{
		{ 0, 2, 1, 3 },	// dx > 0, dy > 0
		{ 1, 0, 3, 2 },	// dx < 0, dy > 0
		{ 2, 3, 0, 1 },	// dx > 0, dy < 0
		{ 3, 1, 2, 0 }	// dx < 0, dy < 0
	};

	int const	doi	= ( ( dy < 0.0f ) << 1 ) + ( dx < 0.0f );

	return drawOrderMap[ doi ];
}



}

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

float const	Sector::SIZE	= 256.0f * XYSCALE;

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

Sector::Sector( float x0, float y0,
				IDirect3DDevice9 * pD3dDevice,
				Dxx::DirectionalLight const & light,
				Dxx::Camera const & camera )
	: m_pD3dDevice( pD3dDevice ),
	m_pHeightField( 0 ),
	m_pLightMap( 0 ),
	m_pEffect( 0 ),
	m_X0( x0 ), m_Y0( y0 )
{
	pD3dDevice->AddRef();

	try
	{
		// Load the height field

		LoadHeightField( "res/hf.tga" );

		// Generate the light map now the height field is loaded

		GenerateLightMap( light );

		// Divide the sector up into patches

		CreatePatches( x0, y0, camera );

		// Load sector textures

		LoadTextures( "res/256.tga" );

		// Load the shaders

		LoadShaders( "res/patch.fxo" );
	}
	catch ( ... )
	{
		this->~Sector();	// Need to delete a whole bunch of crap if the constructor fails
		throw;
	}
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

Sector::~Sector()
{
	Wx::SafeRelease( m_pEffect );

	for ( int i = 0; i < MAX_DETAIL_TEXTURES; i++ )
	{
		Wx::SafeRelease( m_apDetailTextures[i] );
	}

	Wx::SafeRelease( m_pDetailSelector );
	Wx::SafeRelease( m_pLightMap );

	m_QuadTree.ForEachLeaf( &NodeInfoCleaner );

	delete m_pHeightField;

	Wx::SafeRelease( m_pD3dDevice );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Sector::LoadHeightField( char const * sFileName )
{
	m_pHeightField = HeightFieldLoader::LoadTga( sFileName, ZSCALE ).release();
	if ( m_pHeightField == 0 ) throw bad_alloc();

	assert( m_pHeightField->GetSizeI() == m_pHeightField->GetSizeJ() );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Sector::CreatePatches( float x0, float y0, Dxx::Camera const & camera )
{
	// Make sure the height field size is a multiple of the patch size
	assert( ( m_pHeightField->GetSizeI() - 1 ) % ( Patch::SIZE - 1 ) == 0 );

	// Create the patch quadtree

	float const			minZ	= m_pHeightField->GetMinZ();
	float const			maxZ	= m_pHeightField->GetMaxZ();
	NodeInfo * const	pInfo	= &m_QuadTree.m_RootNode.m_NodeData;

	pInfo->m_X0 				= x0;
	pInfo->m_Y0 				= y0;
	pInfo->m_Size				= ( m_pHeightField->GetSizeI()-1 ) * XYSCALE;
	pInfo->m_nVerticesPerSide	= m_pHeightField->GetSizeI();
	pInfo->m_nPatchesPerSide	= ( m_pHeightField->GetSizeI() - 1 ) / ( Patch::SIZE - 1 );
	pInfo->m_BoundingBox		= AABox( Vector3( x0, y0, minZ ),
										 Vector3( pInfo->m_Size, pInfo->m_Size, maxZ - minZ ) );
	pInfo->m_pPatch				= 0;

	if ( pInfo->m_nPatchesPerSide > 1 )
	{
		int const		nPatchesPerSubSide	= pInfo->m_nPatchesPerSide / 2;
		float const		subSize				= pInfo->m_Size * 0.5f;

		m_QuadTree.m_RootNode.Subdivide( auto_ptr< PatchNode >( CreatePatchNode( 0,                  0,                  x0,           y0,           nPatchesPerSubSide, camera ) ),
										 auto_ptr< PatchNode >( CreatePatchNode( 0,                  nPatchesPerSubSide, x0 + subSize, y0,           nPatchesPerSubSide, camera ) ),
										 auto_ptr< PatchNode >( CreatePatchNode( nPatchesPerSubSide, 0,                  x0,           y0 + subSize, nPatchesPerSubSide, camera ) ),
										 auto_ptr< PatchNode >( CreatePatchNode( nPatchesPerSubSide, nPatchesPerSubSide, x0 + subSize, y0 + subSize, nPatchesPerSubSide, camera ) ) );
	}
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

Sector::PatchNode * Sector::CreatePatchNode( int i, int j, float x0, float y0, int nPatchesPerSide, Dxx::Camera const & camera )
{
	int const		nVertexesPerSide	= nPatchesPerSide * ( Patch::SIZE-1 ) + 1;
	float const		minZ				= m_pHeightField->GetMinZ( i, j, nVertexesPerSide, nVertexesPerSide );
	float const		maxZ				= m_pHeightField->GetMaxZ( i, j, nVertexesPerSide, nVertexesPerSide );
	float const		size				= ( nVertexesPerSide - 1 ) * XYSCALE;

	NodeInfo		info( x0, y0, size,
						  nVertexesPerSide,
						  nPatchesPerSide,
						  AABox( Vector3( x0, y0, minZ ), Vector3( size, size, maxZ - minZ ) ),
						  0 );

	PatchNode *	pNode;

	if ( nPatchesPerSide == 1 )
	{
		info.m_pPatch = new Patch( m_pD3dDevice,
								   i * (Patch::SIZE-1), j * (Patch::SIZE-1),
								   x0, y0,
								   *m_pHeightField,
								   XYSCALE,
								   camera );
		if ( info.m_pPatch == 0 ) throw bad_alloc();

		pNode = new PatchNode( info );
		if ( pNode == 0 ) throw bad_alloc();
	}
	else
	{
		int const	nPatchesPerSubSide	= nPatchesPerSide / 2;
		float const	subSize				= size * 0.5f;

		pNode = new PatchNode( info,
							   auto_ptr< PatchNode >( CreatePatchNode( i,                    j,                    x0,           y0,           nPatchesPerSubSide, camera ) ),
							   auto_ptr< PatchNode >( CreatePatchNode( i,                    j+nPatchesPerSubSide, x0 + subSize, y0,           nPatchesPerSubSide, camera ) ),
							   auto_ptr< PatchNode >( CreatePatchNode( i+nPatchesPerSubSide, j,                    x0,           y0 + subSize, nPatchesPerSubSide, camera ) ),
							   auto_ptr< PatchNode >( CreatePatchNode( i+nPatchesPerSubSide, j+nPatchesPerSubSide, x0 + subSize, y0 + subSize, nPatchesPerSubSide, camera ) ) );
		if ( pNode == 0 ) throw bad_alloc();
	}

	return pNode;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Sector::LoadTextures( char const * sFileName )
{
	HRESULT		hr;

	memset( m_apDetailTextures, 0, sizeof( m_apDetailTextures ) );
	m_pDetailSelector = 0;

	hr = D3DXCreateTextureFromFileEx( m_pD3dDevice, sFileName, D3DX_DEFAULT, D3DX_DEFAULT,
									  D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
									  D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL,
									  &m_apDetailTextures[0]);
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Sector::GenerateLightMap( Dxx::DirectionalLight const & light )
{
	float const		AMBIENT		= 0.3f;

	m_pLightMap = LightMapFactory::Create( m_pD3dDevice, *m_pHeightField, XYSCALE, light, 0, 0, 0, 0, AMBIENT ).Detach();
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Sector::LoadShaders( char const * sFileName )
{
	HRESULT		hr;

	// Load the effects file

	{
		ID3DXBuffer *	pErrorMsgs;
		hr = D3DXCreateEffectFromFile( m_pD3dDevice, sFileName, NULL, NULL, 0, NULL, &m_pEffect, &pErrorMsgs );
//		hr = D3DXCreateEffectFromResource( m_pD3dDevice, NULL, "IDR_PATCHEFFECT", NULL, NULL, 0, NULL, &m_pEffect, &pErrorMsgs );
		if ( FAILED( hr ) )
		{
#if defined( _DEBUG )
			if ( pErrorMsgs != 0 && pErrorMsgs->GetBufferPointer() != 0 )
			{
				OutputDebugString( (char *)pErrorMsgs->GetBufferPointer() );
			}
#endif // defined( _DEBUG )
		}
		assert_succeeded( hr );

		Wx::SafeRelease( pErrorMsgs );
	}

	D3DXHANDLE hTechnique;
	hr = m_pEffect->FindNextValidTechnique( NULL, &hTechnique );
	assert_succeeded( hr );

	m_pEffect->SetTechnique( hTechnique );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Sector::Update( __int64 t )
{
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Sector::Draw( Dxx::Camera const & camera )
{
	HRESULT		hr;

	m_PerformanceInfo.m_TriangleCount		= 0;
	m_PerformanceInfo.m_PatchCount			= 0;

	D3DXMATRIX	viewProjection	= camera.GetViewProjectionMatrix();
	m_pEffect->SetMatrix( "WorldViewProjectionMatrix", &viewProjection );
	m_pEffect->SetTexture("LightMapTexture", m_pLightMap );
	m_pEffect->SetTexture("DetailTexture0", m_apDetailTextures[0] );
//	m_pEffect->SetTexture("DetailTexture1", m_apDetailTextures[1] );
//	m_pEffect->SetTexture("DetailTexture2", m_apDetailTextures[2] );
//	m_pEffect->SetTexture("DetailTexture3", m_apDetailTextures[3] );
//	m_pEffect->SetTexture("DetailSelectorTexture", m_pDetailSelector );

	hr = m_pD3dDevice->SetVertexDeclaration( Patch::GetVertexDeclaration() );
	assert_succeeded( hr );

	{
		UINT	nPasses;

		m_pEffect->Begin( &nPasses, 0 );
		
		for ( UINT pass = 0; pass < nPasses; ++pass )
		{
			m_pEffect->BeginPass( pass );
			DrawPartiallyVisibleQuadTreeNode( &m_QuadTree.m_RootNode, camera );
			m_pEffect->EndPass();
		}
        
		m_pEffect->End();
	}
}



/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Sector::DrawPartiallyVisibleQuadTreeNode( PatchNode const * pNode, Dxx::Camera const & camera )
{
	NodeInfo const &	info	= pNode->m_NodeData;

	// Compute the directed distance along each axis from the camera to the center of the patch
	// This is used to determine patch drawing order.

	float const	dx	= info.m_X0 + info.m_Size * 0.5f - camera.GetPosition().x;
	float const dy	= info.m_Y0 + info.m_Size * 0.5f - camera.GetPosition().y;

	int const *	paDrawOrderMap	=	_GetDrawOrder( dx, dy );

	// For each sub-node, determine if it is fully, partially, or not in view, and draw appropriately

	for ( int k = 0; k < 4; k++ )
	{
		PatchNode const * const	pSubNode	= pNode->m_apSubNodes[ paDrawOrderMap[k] ];	// Convenience

		Intersectable::IntersectionClass	ic	= camera.GetViewFrustum().Intersects( pSubNode->m_NodeData.m_BoundingBox );

		if ( ic != Intersectable::NO_INTERSECTION )
		{
			if ( ic == Intersectable::ENCLOSES || info.m_nPatchesPerSide / 2 == 1 )
			{
				DrawFullyVisibleQuadTreeNode( pSubNode, camera );
			}
			else
			{
				DrawPartiallyVisibleQuadTreeNode( pSubNode, camera );
			}
		}
	}
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Sector::DrawFullyVisibleQuadTreeNode( PatchNode const * pNode, Dxx::Camera const & camera )
{
	NodeInfo const &	info	= pNode->m_NodeData;

	// Compute the directed distance along each axis from the camera to the center of the patch
	// This is used to determine patch drawing order or to determine the LOD level.

	float const	dx	= info.m_X0 + info.m_Size * 0.5f - camera.GetPosition().x;
	float const dy	= info.m_Y0 + info.m_Size * 0.5f - camera.GetPosition().y;

	// If we are down to a single patch, then draw it. Otherwise, draw each of its four subnodes.

	if ( info.m_nPatchesPerSide == 1 )
	{
		Patch * const	pPatch	= info.m_pPatch;		// Convenience
		assert( pPatch != 0 );

		// Compute the distance from the center of the patch to the camera

		float const	distance2	= D3DXVec2LengthSq( &D3DXVECTOR2 ( dx, dy ) );

		// Determine which LOD to draw

		int		lod	= 0;

		while (    lod < Patch::NUMBER_OF_LODS-1
				&& distance2 >= pPatch->GetMinDistance2( lod+1 ) )
		{
			++lod;
		}

		// Set the lod vertex constant for tweening

		if ( lod < Patch::NUMBER_OF_LODS-1 )
		{
			float const		lodDistance1	= pPatch->GetMinDistance2( lod+1 );
			float const		lodDistance0	= pPatch->GetMinDistance2( lod );

			float const		lodTween	= lod + ( distance2 - lodDistance0 ) / ( lodDistance1 - lodDistance0 );
			m_pEffect->SetFloat( "Lod", lodTween );
		}
		else
		{
			m_pEffect->SetFloat( "Lod", (float)lod );
		}

//		D3DXVECTOR4	LightMapTextureOffset( 0.0f, 0.0f, 0.0f, 0.0f );
//		D3DXVECTOR4	LightMapTextureScale( m_LightMapUVScale, m_LightMapUVScale, 0.0f, 0.0f );
//		D3DXVECTOR4	DetailTexture0Offset( 0.0f, 0.0f, 0.0f, 0.0f );
//		D3DXVECTOR4	DetailTexture0Scale( 1.0f, 1.0f, 0.0f, 0.0f );
//		D3DXVECTOR4	DetailTexture1Offset( 0.0f, 0.0f, 0.0f, 0.0f );
//		D3DXVECTOR4	DetailTexture1Scale( 1.0f, 1.0f, 0.0f, 0.0f );
//		D3DXVECTOR4	DetailTexture2Offset( 0.0f, 0.0f, 0.0f, 0.0f );
//		D3DXVECTOR4	DetailTexture2Scale( 1.0f, 1.0f, 0.0f, 0.0f );
//		D3DXVECTOR4	DetailTexture3Offset( 0.0f, 0.0f, 0.0f, 0.0f );
//		D3DXVECTOR4	DetailTexture3Scale( 1.0f, 1.0f, 0.0f, 0.0f );
//		D3DXVECTOR4	DetailSelectorTextureOffset( 0.0f, 0.0f, 0.0f, 0.0f );
//		D3DXVECTOR4	DetailSelectorTextureScale( 1.0f, 1.0f, 0.0f, 0.0f );
//
//
//		m_pEffect->SetVector( "LightMapTextureOffset", &LightMapTextureOffset );
//		m_pEffect->SetVector( "LightMapTextureScale", &LightMapTextureScale );
//		m_pEffect->SetVector( "DetailTexture0Offset", &DetailTexture0Offset );
//		m_pEffect->SetVector( "DetailTexture0Scale", &DetailTexture0Scale );
//		m_pEffect->SetVector( "DetailTexture1Offset", &DetailTexture1Offset );
//		m_pEffect->SetVector( "DetailTexture1Scale", &DetailTexture1Scale );
//		m_pEffect->SetVector( "DetailTexture2Offset", &DetailTexture2Offset );
//		m_pEffect->SetVector( "DetailTexture2Scale", &DetailTexture2Scale );
//		m_pEffect->SetVector( "DetailTexture3Offset", &DetailTexture3Offset );
//		m_pEffect->SetVector( "DetailTexture3Scale", &DetailTexture3Scale );
//		m_pEffect->SetVector( "DetailSelectorTextureOffset", &DetailSelectorTextureOffset );
//		m_pEffect->SetVector( "DetailSelectorTextureScale", &DetailSelectorTextureScale );

		// Draw finally

		pPatch->Draw( lod );

		m_PerformanceInfo.m_PatchCount += 1;
		m_PerformanceInfo.m_TriangleCount += pPatch->GetPerformanceInfo().m_TriangleCount;
	}
	else
	{
		int const *	paDrawOrderMap	=	_GetDrawOrder( dx, dy );

		DrawFullyVisibleQuadTreeNode( pNode->m_apSubNodes[ paDrawOrderMap[0] ], camera );
		DrawFullyVisibleQuadTreeNode( pNode->m_apSubNodes[ paDrawOrderMap[1] ], camera );
		DrawFullyVisibleQuadTreeNode( pNode->m_apSubNodes[ paDrawOrderMap[2] ], camera );
		DrawFullyVisibleQuadTreeNode( pNode->m_apSubNodes[ paDrawOrderMap[3] ], camera );
	}
}

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

float Sector::GetZ( float x, float y ) const
{
	NodeInfo const * const	pInfo	= &m_QuadTree.m_RootNode.m_NodeData;

	x = ( x - pInfo->m_X0 ) / XYSCALE;
	y = ( y - pInfo->m_Y0 ) / XYSCALE;

	if ( x < 0.0f || x > m_pHeightField->GetSizeJ()-1 || y < 0.0f || y > m_pHeightField->GetSizeI()-1 )
	{
		return 0.0f;
	}

	return m_pHeightField->GetInterpolatedZ( x, y );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Sector::AddConsoleVariables()
{
	Patch::AddConsoleVariables();
}
