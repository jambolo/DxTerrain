/** @file *//********************************************************************************************************

                                                  PatchVBFactory.cpp

						                    Copyright 2004, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/PatchVBFactory.cpp#5 $

	$NoKeywords: $

 ********************************************************************************************************************/

#include "PrecompiledHeaders.h"

#include "PatchVBFactory.h"

#include "PatchVB.h"

#include "Dxx/VertexBufferLock.h"
#include "Heightfield/Heightfield.h"
#include "Wx/Wx.h"

using namespace std;


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

// Returns the maximum LOD for a vertex at [i, j]
static int _ComputeMaxLod( int i, int j )
{
	int		lod		= 0;

	if ( ( i | j ) == 0 )
	{
		lod = numeric_limits<int>::max();
	}
	else
	{
		while ( ( ( i | j ) & ( 1 << lod ) ) == 0 )
		{
			++lod;
		}
	}

	return lod;
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

auto_ptr<PatchVB> PatchVBFactory::Create( IDirect3DDevice9 * pD3dDevice,
											   HeightField const & hf,
											   int j0, int i0,
											   float xyScale,
											   float x0, float y0,
											   __int16 const * paIJToIndexMap,
											   int size )
{
	CComPtr<IDirect3DVertexBuffer9>	pVB;

	// Create the patch VB

	HRESULT		hr;

	hr = pD3dDevice->CreateVertexBuffer( size * size * sizeof( PatchVB::VBEntry ),
										   D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED,
										   &pVB, NULL );
	assert_succeeded( hr );
	if ( FAILED( hr ) )
	{
		return auto_ptr< PatchVB >();
	}

	// Compute the lightmap UV scale and offset
	//
	// Here's how to map vertex V in a heightfield with width H to a texel T in a texture map whose width is W.
	// T = u * W - .5, so u = (T+.5)/W
	// When V=0, we want T=0, so u = .5/W at V=0
	// When V=H-1, we want T=W-1, so u = (W-.5)/W at V=H-1
	// Thus the scale u/V is ((W-.5)/W - .5/W) / (H-1) = ((W-.5) - .5) / (W * (H-1)) = (W-1) / (W * (H-1))
	// Since W = H - 1, u/V = (W-1) / (W * W)

	int const	tw			= hf.GetSizeI() - 1;
	float const	uvScale		= float( tw - 1 ) / float( tw * tw );
	float const uvOffset	= 0.5f / float( tw );

	// Initialize it

	Dxx::VertexBufferLock	lock( pVB );

	PatchVB::VBEntry *	paVertexEntries	= (PatchVB::VBEntry *)lock.GetLockedBuffer();

	for ( int i = 0; i < size; i++ )
	{
		float const	y	= y0 + i * xyScale;

		for ( int j = 0; j < size; j++ )
		{
			float const	x		= x0 + j * xyScale;
			int const	maxLod	= _ComputeMaxLod( i, j );

			PatchVB::VBEntry * const	pEntry	= &paVertexEntries[ paIJToIndexMap[ i * size + j ] ];

			pEntry->position.x		= x; 
			pEntry->position.y		= y;
			pEntry->position.z		= hf.GetZ( j0 + j, i0 + i );
			pEntry->maxLod			= (float)maxLod;
			pEntry->interpolatedZ	= hf.GetInterpolatedZ( float( j0 + j ), float( i0 + i ), 1 << ( maxLod + 1 ) );
			pEntry->ul				= ( j0 + j ) * uvScale + uvOffset;
			pEntry->vl				= ( i0 + i ) * uvScale + uvOffset;
			pEntry->u0				= (float)j;
			pEntry->v0				= (float)i;
			pEntry->u1				= (float)j;
			pEntry->v1				= (float)i;
			pEntry->u2				= (float)j;
			pEntry->v2				= (float)i;
			pEntry->u3				= (float)j;
			pEntry->v3				= (float)i;
			pEntry->us				= ( j0 + j ) * uvScale + uvOffset;
			pEntry->vs				= ( i0 + i ) * uvScale + uvOffset;
		}
	}

	return auto_ptr< PatchVB >( new PatchVB( pVB ) );
}
