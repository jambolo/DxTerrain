/** @file *//********************************************************************************************************

                                                      SkirtVB.h

						                    Copyright 2004, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/SkirtVB.h#5 $

	$NoKeywords: $

 ********************************************************************************************************************/

#pragma once

#if defined( _DEBUG )
	#define D3D_DEBUG_INFO
#endif
#include <d3d9.h>
#include <atlcomcli.h>


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

class SkirtVB
{
public:

	//! Vertex buffer entry
	struct VBEntry
	{
		// Must match m_declaration
		D3DVECTOR	position;		//!< Vertex position
		float		maxLod;			//!< Highest LOD (lowest resolution) containing this vertex
		float		interpolatedZ;	//!< Z value of this vertex at LOD maxLod+1
		float		ul, vl;			//!< Lightmap texture uv
		float		u0, v0;			//!< Detail texture uv
		float		u1, v1;			//!< Detail texture uv
		float		u2, v2;			//!< Detail texture uv
		float		u3, v3;			//!< Detail texture uv
		float		us, vs;			//!< Detail texture selector uv
	};

	enum
	{
		STRIDE = sizeof( VBEntry )	//!< Size of a vertex buffer entry
	};

	//! Constructor
	SkirtVB( IDirect3DVertexBuffer9 * pVB = 0 ) : m_pVB( pVB ) {}

	CComPtr<IDirect3DVertexBuffer9>	m_pVB;					//!< The vertex buffer
	static D3DVERTEXELEMENT9 const	m_declaration[];		//!< The vertex declaration
};
