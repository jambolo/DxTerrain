/** @file *//********************************************************************************************************

                                                  SkirtVBFactory.cpp

						                    Copyright 2004, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/SkirtVBFactory.cpp#5 $

	$NoKeywords: $

 ********************************************************************************************************************/

#include "PrecompiledHeaders.h"

#include "SkirtVBFactory.h"

#include "SkirtVB.h"

#include "Heightfield/Heightfield.h"
#include "Wx/Wx.h"
#include "Dxx/VertexBufferLock.h"

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

auto_ptr<SkirtVB> SkirtVBFactory::Create( IDirect3DDevice9 * pD3dDevice,
											   HeightField const & hf,
											   int j0, int i0,
											   float xyScale,
											   float x0, float y0,
											   __int16 const * paIJToIndexMap,
											   int size,
											   float maxError )
{
	__int16 const * const	paIJToIndexMap0	= &paIJToIndexMap[ 0 * ( size-1 ) * 2 ];
	__int16 const * const	paIJToIndexMap1	= &paIJToIndexMap[ 1 * ( size-1 ) * 2 ];
	__int16 const * const	paIJToIndexMap2	= &paIJToIndexMap[ 2 * ( size-1 ) * 2 ];
	__int16 const * const	paIJToIndexMap3	= &paIJToIndexMap[ 3 * ( size-1 ) * 2 ];

	CComPtr<IDirect3DVertexBuffer9>	pVB;

	// Create the skirt VB

	HRESULT		hr;

	hr = pD3dDevice->CreateVertexBuffer( 4 * (size-1) * 2 * sizeof( SkirtVB::VBEntry ),
										 D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED,
										 &pVB, NULL );
	assert_succeeded( hr );
	if ( FAILED( hr ) )
	{
		return auto_ptr<SkirtVB>();
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

	SkirtVB::VBEntry *	paVertexEntries	= (SkirtVB::VBEntry *)lock.GetLockedBuffer();
	SkirtVB::VBEntry *	pEntry;

	int		i, j;
	float	x, y;
	int		maxLod;

	i = 0;
	y = y0 + i * xyScale;

	for ( j = 0; j < size-1; j++ )
	{
		__int16 const * const	paIJToIndexMap0j	= &paIJToIndexMap0[ j * 2 ];

		maxLod	= _ComputeMaxLod( i, j );
		x		= x0 + j * xyScale;
		pEntry	= &paVertexEntries[ paIJToIndexMap0j[0] ];

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

		pEntry	= &paVertexEntries[ paIJToIndexMap0j[1] ];

		pEntry->position.x		= x; 
		pEntry->position.y		= y;
		pEntry->position.z		= hf.GetZ( j0 + j, i0 + i ) - maxError;
		pEntry->maxLod			= (float)numeric_limits<int>::max();	// No interpolation
		pEntry->interpolatedZ	= pEntry->position.z;						// No interpolation
		pEntry->ul				= ( j0 + j ) * uvScale + uvOffset;
		pEntry->vl				= ( i0 + i ) * uvScale + uvOffset;
		pEntry->u0				= (float)j;
		pEntry->v0				= (float)i - 1.0f;
		pEntry->u1				= (float)j;
		pEntry->v1				= (float)i - 1.0f;
		pEntry->u2				= (float)j;
		pEntry->v2				= (float)i - 1.0f;
		pEntry->u3				= (float)j;
		pEntry->v3				= (float)i - 1.0f;
		pEntry->us				= ( j0 + j ) * uvScale + uvOffset;
		pEntry->vs				= ( i0 + i ) * uvScale + uvOffset;
	}

//	j = size - 1;
	x = x0 + j * xyScale;

	for ( i = 0; i < size-1; i++ )
	{
		__int16 const * const	paIJToIndexMap1i	= &paIJToIndexMap1[ i * 2 ];

		maxLod	= _ComputeMaxLod( i, j );
		y		= y0 + i * xyScale;
		pEntry	= &paVertexEntries[ paIJToIndexMap1i[0] ];

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

		pEntry	= &paVertexEntries[ paIJToIndexMap1i[1] ];

		pEntry->position.x		= x; 
		pEntry->position.y		= y;
		pEntry->position.z		= hf.GetZ( j0 + j, i0 + i ) - maxError;
		pEntry->maxLod			= (float)numeric_limits<int>::max();	// No interpolation
		pEntry->interpolatedZ	= pEntry->position.z;						// No interpolation
		pEntry->ul				= ( j0 + j ) * uvScale + uvOffset;
		pEntry->vl				= ( i0 + i ) * uvScale + uvOffset;
		pEntry->u0				= (float)j + 1.0f;
		pEntry->v0				= (float)i;
		pEntry->u1				= (float)j + 1.0f;
		pEntry->v1				= (float)i;
		pEntry->u2				= (float)j + 1.0f;
		pEntry->v2				= (float)i;
		pEntry->u3				= (float)j + 1.0f;
		pEntry->v3				= (float)i;
		pEntry->us				= ( j0 + j ) * uvScale + uvOffset;
		pEntry->vs				= ( i0 + i ) * uvScale + uvOffset;

		++pEntry;
	}

//	i = size - 1;
	y = y0 + i * xyScale;

	for ( j = size-1; j > 0; j-- )
	{
		__int16 const * const	paIJToIndexMap2j	= &paIJToIndexMap2[ ( size-1 - j ) * 2 ];

		maxLod	= _ComputeMaxLod( i, j );
		x		= x0 + j * xyScale;
		pEntry	= &paVertexEntries[ paIJToIndexMap2j[0] ];

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

		pEntry	= &paVertexEntries[ paIJToIndexMap2j[1] ];

		pEntry->position.x		= x; 
		pEntry->position.y		= y;
		pEntry->position.z		= hf.GetZ( j0 + j, i0 + i ) - maxError;
		pEntry->maxLod			= (float)numeric_limits<int>::max();	// No interpolation
		pEntry->interpolatedZ	= pEntry->position.z;						// No interpolation
		pEntry->ul				= ( j0 + j ) * uvScale + uvOffset;
		pEntry->vl				= ( i0 + i ) * uvScale + uvOffset;
		pEntry->u0				= (float)j;
		pEntry->v0				= (float)i + 1.0f;
		pEntry->u1				= (float)j;
		pEntry->v1				= (float)i + 1.0f;
		pEntry->u2				= (float)j;
		pEntry->v2				= (float)i + 1.0f;
		pEntry->u3				= (float)j;
		pEntry->v3				= (float)i + 1.0f;
		pEntry->us				= ( j0 + j ) * uvScale + uvOffset;
		pEntry->vs				= ( i0 + i ) * uvScale + uvOffset;

		++pEntry;
	}

//	j = 0;
	x = x0 + j * xyScale;

	for ( i = size-1; i > 0; i-- )
	{
		__int16 const * const	paIJToIndexMap3i	= &paIJToIndexMap3[ ( size-1 - i ) * 2 ];

		maxLod	= _ComputeMaxLod( i, j );
		y		= y0 + i * xyScale;
		pEntry	= &paVertexEntries[ paIJToIndexMap3i[0] ];

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

		pEntry	= &paVertexEntries[ paIJToIndexMap3i[1] ];

		pEntry->position.x		= x; 
		pEntry->position.y		= y;
		pEntry->position.z		= hf.GetZ( j0 + j, i0 + i ) - maxError;
		pEntry->maxLod			= (float)numeric_limits<int>::max();	// No interpolation
		pEntry->interpolatedZ	= pEntry->position.z;						// No interpolation
		pEntry->ul				= ( j0 + j ) * uvScale + uvOffset;
		pEntry->vl				= ( i0 + i ) * uvScale + uvOffset;
		pEntry->u0				= (float)j - 1.0f;
		pEntry->v0				= (float)i;
		pEntry->u1				= (float)j - 1.0f;
		pEntry->v1				= (float)i;
		pEntry->u2				= (float)j - 1.0f;
		pEntry->v2				= (float)i;
		pEntry->u3				= (float)j - 1.0f;
		pEntry->v3				= (float)i;
		pEntry->us				= ( j0 + j ) * uvScale + uvOffset;
		pEntry->vs				= ( i0 + i ) * uvScale + uvOffset;

		++pEntry;
	}

	return auto_ptr<SkirtVB>( new SkirtVB( pVB ) );
}
