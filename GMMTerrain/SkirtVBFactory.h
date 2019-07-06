/** @file *//********************************************************************************************************

                                                   SkirtVBFactory.h

						                    Copyright 2004, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/SkirtVBFactory.h#4 $

	$NoKeywords: $

 ********************************************************************************************************************/

#pragma once

#include <memory>

class SkirtVB;
struct IDirect3DDevice9;
class HeightField;


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! A class that creates patch VBs

class SkirtVBFactory
{
public:

	enum
	{
		PATCH_SIZE = 33
	};

	//! Creates a patch VB from the height field. Returns true if successful.
	static std::auto_ptr<SkirtVB> Create( IDirect3DDevice9 * pD3dDevice,
							   			  HeightField const & hf,
										  int j0, int i0,
							   			  float xyScale,
							   			  float x0, float y0,
										  __int16 const * aIJToIndexMap,
										  int size,
										  float maxError );

	//! Loads a SkirtVB from a file
	static std::auto_ptr<SkirtVB> Load( char const * sFileName );

	//! Writes a SkirtVB to a file
	static bool Write( char const * sFileName, SkirtVB const & svb );

//	static __int16 const	m_aSkirtIJToIndexMap[ PATCH_SIZE ][ PATCH_SIZE ];
};
