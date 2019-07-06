/** @file *//********************************************************************************************************

                                                 LightMapFactory.cpp

						                    Copyright 2004, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/LightMapFactory.cpp#2 $

	$NoKeywords: $

 ********************************************************************************************************************/

#include "PrecompiledHeaders.h"

#include "LightMapFactory.h"

#include "HeightField/HeightField.h"
#include "Dxx/D3d.h"
#include "Wx/Wx.h"

using namespace std;


namespace
{

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void ComputeNormals( HeightField const & hf,
					 float const xyScale,
					 HeightField const * pNorth,
					 HeightField const * pEast,
					 HeightField const * pSouth,
					 HeightField const * pWest, 
					 D3DXVECTOR3 * paVertexNormals )
{
	assert( paVertexNormals != 0 );

	int const	sx		= hf.GetSizeJ();
	int const	sy		= hf.GetSizeI();

	assert( pNorth == 0 || ( ( pNorth->GetSizeI() == sy ) && ( pNorth->GetSizeJ() == sx ) ) );
	assert( pEast == 0 || ( ( pEast->GetSizeI() == sy ) && ( pEast->GetSizeJ() == sx ) ) );
	assert( pSouth == 0 || ( ( pSouth->GetSizeI() == sy ) && ( pSouth->GetSizeJ() == sx ) ) );
	assert( pWest == 0 || ( ( pWest->GetSizeI() == sy ) && ( pWest->GetSizeJ() == sx ) ) );

	D3DXVECTOR3 *	pNormal	= paVertexNormals;

	// First row

	{
		// First column

		{
			HeightField::Vertex const * const	paV		= hf.GetData( 0, 0 );

			float const	z0	= paV[   0 ].m_Z;
			float const	z1	= paV[   1 ].m_Z;
			float const	z2	= paV[  sx ].m_Z;
			float const	z3	= ( pWest != 0 ) ? pWest->GetZ( sx-2, 0 ) : z0;
			float const	z4	= ( pNorth != 0 ) ? pNorth->GetZ( 0, sy-2 ) : z0;

			Dxx::ComputeGridNormal( z1, z2, z3, z4, xyScale, pNormal );
			++pNormal;
		}

		// Middle columns

		for ( int x = 1; x < sx-1; x++ )
		{
			HeightField::Vertex const * const	paV		= hf.GetData( x, 0 );

			float const	z0	= paV[   0 ].m_Z;
			float const	z1	= paV[   1 ].m_Z;
			float const	z2	= paV[  sx ].m_Z;
			float const	z3	= paV[  -1 ].m_Z;
			float const	z4	= ( pNorth != 0 ) ? pNorth->GetZ( x, sy-2 ) : z0;

			Dxx::ComputeGridNormal( z1, z2, z3, z4, xyScale, pNormal );
			++pNormal;
		}

		// Last column

		{
			HeightField::Vertex const * const	paV		= hf.GetData( sx-1, 0 );

			float const	z0	= paV[   0 ].m_Z;
			float const	z1	= ( pEast != 0 ) ? pEast->GetZ( 1, 0 ) : z0;
			float const	z2	= paV[  sx ].m_Z;
			float const	z3	= paV[  -1 ].m_Z;
			float const	z4	= ( pNorth != 0 ) ? pNorth->GetZ( sx-1, sy-2 ) : z0;

			Dxx::ComputeGridNormal( z1, z2, z3, z4, xyScale, pNormal );
			++pNormal;
		}
	}

	// Middle rows

	for ( int y = 1; y < sy-1; y++ )
	{
		// First column

		{
			HeightField::Vertex const * const	paV		= hf.GetData( 0, y );

			float const	z0	= paV[   0 ].m_Z;
			float const	z1	= paV[   1 ].m_Z;
			float const	z2	= paV[  sx ].m_Z;
			float const	z3	= ( pWest != 0 ) ? pWest->GetZ( sx-2, y ) : z0;
			float const	z4	= paV[ -sx ].m_Z;

			Dxx::ComputeGridNormal( z1, z2, z3, z4, xyScale, pNormal );
			++pNormal;
		}

		// Middle columns

		for ( int x = 1; x < sx-1; x++ )
		{
			HeightField::Vertex const * const	paV		= hf.GetData( x, y );

			float const	z1	= paV[   1 ].m_Z;
			float const	z2	= paV[  sx ].m_Z;
			float const	z3	= paV[  -1 ].m_Z;
			float const	z4	= paV[ -sx ].m_Z;

			Dxx::ComputeGridNormal( z1, z2, z3, z4, xyScale, pNormal );
			++pNormal;
		}

		// Last column

		{
			HeightField::Vertex const * const	paV		= hf.GetData( sx-1, y );

			float const	z0	= paV[   0 ].m_Z;
			float const	z1	= ( pEast != 0 ) ? pEast->GetZ( 1, y ) : z0;
			float const	z2	= paV[  sx ].m_Z;
			float const	z3	= paV[  -1 ].m_Z;
			float const	z4	= paV[ -sx ].m_Z;

			Dxx::ComputeGridNormal( z1, z2, z3, z4, xyScale, pNormal );
			++pNormal;
		}
	}

	// Last row

	{
		// First column

		{
			HeightField::Vertex const * const	paV		= hf.GetData( 0, sy-1 );

			float const	z0	= paV[   0 ].m_Z;
			float const	z1	= paV[   1 ].m_Z;
			float const	z2	= ( pSouth != 0 ) ? pSouth->GetZ( 0, 1 ) : z0;
			float const	z3	= ( pWest != 0 ) ? pWest->GetZ( sx-2, sy-1 ) : z0;
			float const	z4	= paV[ -sx ].m_Z;

			Dxx::ComputeGridNormal( z1, z2, z3, z4, xyScale, pNormal );
			++pNormal;
		}

		// Middle columns

		for ( int x = 1; x < sx-1; x++ )
		{
			HeightField::Vertex const * const	paV		= hf.GetData( x, sy-1 );

			float const	z0	= paV[   0 ].m_Z;
			float const	z1	= paV[   1 ].m_Z;
			float const	z2	= ( pSouth != 0 ) ? pSouth->GetZ( x, 1 ) : z0;
			float const	z3	= paV[  -1 ].m_Z;
			float const	z4	= paV[ -sx ].m_Z;

			Dxx::ComputeGridNormal( z1, z2, z3, z4, xyScale, pNormal );
			++pNormal;
		}

		// Last column

		{
			HeightField::Vertex const * const	paV		= hf.GetData( sx-1, sy-1 );

			float const	z0	= paV[   0 ].m_Z;
			float const	z1	= ( pEast != 0 ) ? pEast->GetZ( 1, sy-1 ) : z0;
			float const	z2	= ( pSouth != 0 ) ? pSouth->GetZ( sx-1, 1 ) : z0;
			float const	z3	= paV[  -1 ].m_Z;
			float const	z4	= paV[ -sx ].m_Z;

			Dxx::ComputeGridNormal( z1, z2, z3, z4, xyScale, pNormal );
			++pNormal;
		}
	}
}


} // anonymous namespace


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/


CComPtr<IDirect3DTexture9> LightMapFactory::Create( IDirect3DDevice9 * pD3dDevice,
													HeightField const & hf,
													float const xyScale,
													Dxx::DirectionalLight const & light,
													HeightField * pNorth,
													HeightField * pEast,
													HeightField * pSouth,
													HeightField * pWest, 
													float ambient )
{
	float const	fDiffuse	= 1.0f - ambient;	// Diffuse factor

	int const	hsj		= hf.GetSizeJ();	// Size of the heightmap
	int const	hsi		= hf.GetSizeI();	// Size of the heightmap
	int const	hsj1	= hsj - 1;			// Last vertex in the heightmap
	int const	hsi1	= hsi - 1;			// Last vertex in the heightmap
	int const	tsx		= hsj - 1;			// Size of the light map
	int const	tsy		= hsi - 1;			// Size of the light map
	int const	tsx1	= tsx - 1;			// Last texel in the light map
	int const	tsy1	= tsy - 1;			// Last texel in the light map

	CComPtr<IDirect3DTexture9>	pLightMap;

	// Compute the vertex normals

	D3DXVECTOR3 * const	paVertexNormals	= new D3DXVECTOR3[ hsj * hsi ];
	if ( paVertexNormals == 0 ) throw bad_alloc();

	ComputeNormals( hf, xyScale, pNorth, pEast, pSouth, pWest, paVertexNormals );

	HRESULT			hr;
	D3DLOCKED_RECT	locked;

	// Create the texture

	hr = D3DXCreateTexture( pD3dDevice, tsx, tsy, D3DX_DEFAULT, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_L8, D3DPOOL_MANAGED, &pLightMap );
	assert_succeeded( hr );

	// Compute the brightness of the texel using L dot N + ambient

	hr = pLightMap->LockRect( 0, &locked, NULL, 0 );
	assert_succeeded( hr );

	D3DXVECTOR3			L		=	-D3DXVECTOR3( light.GetDirection() );
	D3DXVec3Normalize( &L, &L );

	unsigned __int8 *	pRow	= (unsigned __int8 *)locked.pBits;

	// Set the brightness of each texel in the lightmap. The tsx x tsy lightmap texels are mapped to
	// hsj x hsi vertexes. Because the sizes are different, the normals must be interpolated.
	// Note: the last row and column of the lightmap are special because we want to avoid
	// floating point inaccuracies in these texels and the interpolation is a special case.

	for ( int y = 0; y < tsy1; y++ )
	{
		unsigned __int8 *	pData	= pRow;
		int const			i0		= y * hsi1 / tsy1;
		float const			dy		= float( y ) / float( tsy1 ) * float( hsi1 ) - float( i0 );

		for ( int x = 0; x < tsx1; x++ )
		{
			int const	j0	= x * hsj1 / tsx1;
			float const	dx	= float( x ) / float( tsx1 ) * float( hsj1 ) - float( j0 );

			D3DXVECTOR3	const	n00	= paVertexNormals[ i0         * hsj + j0         ] * ( ( 1 - dy ) * ( 1 - dx ) );
			D3DXVECTOR3	const	n01	= paVertexNormals[ i0         * hsj + ( j0 + 1 ) ] * ( ( 1 - dy ) * dx );
			D3DXVECTOR3	const	n10	= paVertexNormals[ ( i0 + 1 ) * hsj + j0         ] * ( dy         * ( 1 - dx ) );
			D3DXVECTOR3	const	n11	= paVertexNormals[ ( i0 + 1 ) * hsj + ( j0 + 1 ) ] * ( dy         * dx );
			D3DXVECTOR3			N	= n00 + n01 + n10 + n11;
			D3DXVec3Normalize( &N, &N );

			float const	diffuse	= fDiffuse * max( 0.0f, D3DXVec3Dot( &L, &N ) );

			*pData++ = (int)( ( ambient + diffuse ) * 255.0f + 0.5f );
		}

		// Last column.

		{
			D3DXVECTOR3	const	n00	= paVertexNormals[ i0         * hsj + hsj1 ] * ( 1 - dy );
			D3DXVECTOR3	const	n10	= paVertexNormals[ ( i0 + 1 ) * hsj + hsj1 ] * dy;
			D3DXVECTOR3			N	= n00 + n10;
			D3DXVec3Normalize( &N, &N );

			float const	diffuse	= fDiffuse * max( 0.0f, D3DXVec3Dot( &L, &N ) );

			*pData = (int)( ( ambient + diffuse ) * 255.0f + 0.5f );
		}

		pRow += locked.Pitch;
	}

	// Last row.

	{
		unsigned __int8 *	pData	= pRow;

		for ( int x = 0; x < tsx-1; x++ )
		{
			int const	j0	= x * hsj1 / tsx1;
			float const	dx	= float( x ) / float( tsx1 ) * float( hsj1 ) - float( j0 );

			D3DXVECTOR3	const	n00	= paVertexNormals[ hsi1 * hsj + j0         ] * ( 1 - dx );
			D3DXVECTOR3	const	n01	= paVertexNormals[ hsi1 * hsj + ( j0 + 1 ) ] * dx;
			D3DXVECTOR3			N	= n00 + n01;
			D3DXVec3Normalize( &N, &N );

			float const	diffuse	= fDiffuse * max( 0.0f, D3DXVec3Dot( &L, &N ) );

			*pData++ = (int)( ( ambient + diffuse ) * 255.0f + 0.5f );
		}

		// Last column.

		{
			D3DXVECTOR3 const	N	= paVertexNormals[ hsi * hsj - 1 ];

			float const	diffuse	= fDiffuse * max( 0.0f, D3DXVec3Dot( &L, &N ) );

			*pData = (int)( ( ambient + diffuse ) * 255.0f + 0.5f );
		}
	}

	hr = pLightMap->UnlockRect( 0 );
	assert_succeeded( hr );

	// Done with the normals

	delete[] paVertexNormals;

	// Generate the mipmap levels

//	pLightMap->GenerateMipSubLevels();

	return pLightMap;
}
