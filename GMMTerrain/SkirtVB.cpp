/** @file *//********************************************************************************************************

                                                     SkirtVB.cpp

						                    Copyright 2004, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/SkirtVB.cpp#3 $

	$NoKeywords: $

 ********************************************************************************************************************/

#include "PrecompiledHeaders.h"

#include "SkirtVB.h"


// Vertex declaration

D3DVERTEXELEMENT9 const	SkirtVB::m_declaration[] = 
{
	{ 0,  0,  D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	 0 },
	{ 0, 12,  D3DDECLTYPE_FLOAT1,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
	{ 0, 16,  D3DDECLTYPE_FLOAT1,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	 1 },
	{ 0, 20,  D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	 0 },
	{ 0, 28,  D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	 1 },
	{ 0, 36,  D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	 2 },
	{ 0, 44,  D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	 3 },
	{ 0, 52,  D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	 4 },
	{ 0, 60,  D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	 5 },
	D3DDECL_END()
};
