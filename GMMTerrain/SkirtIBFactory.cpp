/** @file *//********************************************************************************************************

                                                  SkirtIBFactory.cpp

						                    Copyright 2004, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/SkirtIBFactory.cpp#3 $

	$NoKeywords: $

 ********************************************************************************************************************/

#include "PrecompiledHeaders.h"

#include "SkirtIBFactory.h"

#include "Dxx/VertexBufferLock.h"
#include "Wx/Wx.h"


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

CComPtr<IDirect3DIndexBuffer9>  SkirtIBFactory::Create( IDirect3DDevice9 * pD3dDevice,
								    					int nIndexes, int nLods, int size,
														__int16 const * paIJToIndexMap )
{
	HRESULT							hr;
	CComPtr<IDirect3DIndexBuffer9>	pIB;

	hr = pD3dDevice->CreateIndexBuffer( nIndexes * sizeof( __int16 ),
										D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED,
										&pIB, NULL );
	assert_succeeded( hr );
	if ( pIB != 0 )
	{
		Dxx::IndexBufferLock	lock( pIB );

		__int16 *	pIndex	= (__int16 *)lock.GetLockedBuffer();

		for ( int lod = nLods-1; lod >= 0; lod-- )
		{
			int const	step	= 1 << lod;

			for ( int i = 0; i < 4; i ++ )
			{
				__int16 const * const	paIJToIndexMapI	= &paIJToIndexMap[ i * (size-1) * 2 ];

				for ( int j = 0; j < size-1; j += step )
				{
					__int16 const * const	paIJToIndexMapIJ	= &paIJToIndexMapI[ j * 2 ];

					*pIndex++ = paIJToIndexMapIJ[ 0 ];
					*pIndex++ = paIJToIndexMapIJ[ 1 ];
				}
			}

			*pIndex++ = paIJToIndexMap[ 0 ];
			*pIndex++ = paIJToIndexMap[ 1 ];
		}

		assert( pIndex - (__int16 *)lock.GetLockedBuffer() == nIndexes );
	}

	return pIB;
}
