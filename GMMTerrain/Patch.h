/********************************************************************************************************************

                                                       Patch.h

						                    Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/Patch.h#6 $

	$NoKeywords: $

 ********************************************************************************************************************/

#pragma once


#include "Dxx/Camera.h"
#include <atlcomcli.h>
#include <memory>
#include <cassert>

class HeightField;
struct IDirect3DDevice9;
struct IDirect3DIndexBuffer9;
struct IDirect3DVertexDeclaration9;
class PatchVB;
class SkirtVB;



/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

class Patch
{
public:

	enum
	{
		SIZE			= 33,						// Size of a patch in each direction
		NUMBER_OF_LODS	= 6,						// (32+1)x(32+1) -> (1+1)x(1+1)
	};

	struct PerformanceInfo
	{
		int		m_TriangleCount;
	};


	struct DebugSettings
	{
		bool	bHideSkirt;
		bool	bHideTerrain;
		bool	bDrawBoundingBox;
	};

	//! Constructor
	Patch( IDirect3DDevice9 * pD3dDevice,
		   int i, int j,
		   float x0, float y0,
		   HeightField const & hf,
		   float xyScale,
		   Dxx::Camera const & camera );
	~Patch();

	void InitializeLodControl( Dxx::Camera const & camera, HeightField const & hf );
	void Draw( int lod );
	float GetMinDistance2( int lod ) const							{ return m_aMinDistances[ lod ]; }

	static IDirect3DIndexBuffer9 * GetPatchIB()						{ return m_qPatchIB; }
	static IDirect3DIndexBuffer9 * GetSkirtIB()						{ return m_qSkirtIB; }
	static IDirect3DVertexDeclaration9 * GetVertexDeclaration()		{ return m_qVertexDeclaration; }
	// Debugging

	PerformanceInfo const & GetPerformanceInfo() const				{ return m_PerformanceInfo; }
	static DebugSettings const & GetDebugSettings()					{ return m_DebugSettings; }
	static void AddConsoleVariables();

private:

	static float const	VIEW_SPACE_TOLERANCE;

	PatchVB *										m_pPatchVB;
	static CComPtr<IDirect3DIndexBuffer9>			m_qPatchIB;
	SkirtVB *										m_pSkirtVB;
	static CComPtr<IDirect3DIndexBuffer9>			m_qSkirtIB;
	IDirect3DDevice9 *								m_pD3dDevice;
	int												m_I0, m_J0;							// Location of patch origin in the height field
	float											m_aMinDistances[ NUMBER_OF_LODS ];	// Distances at which LODs switch between i and i-1
	float											m_MaxError;							// Maximum error for any vertex in any LOD
	static CComPtr<IDirect3DVertexDeclaration9>		m_qVertexDeclaration;

	// Debugging

	PerformanceInfo					m_PerformanceInfo;					// Performance info
	static DebugSettings			m_DebugSettings;
};
