/** @file *//********************************************************************************************************

                                                   PatchVBFactory.h

						                    Copyright 2004, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/PatchVBFactory.h#4 $

	$NoKeywords: $

 ********************************************************************************************************************/

#pragma once

#include <memory>

class PatchVB;
struct IDirect3DDevice9;
class HeightField;


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! A class that creates patch VBs

class PatchVBFactory
{
public:
	//! Creates a patch VB from the height field. Returns 0 if it fails.
	static std::auto_ptr<PatchVB> Create( IDirect3DDevice9 * pD3dDevice,
							   			  HeightField const & hf,
										  int j0, int i0,
							   			  float xyScale,
							   			  float x0, float y0,
										  __int16 const * paIJToIndexMap,
										  int size );

	//! Loads a PatchVB from a file. Returns 0 if it fails.
	static std::auto_ptr<PatchVB> Load( char const * sFileName );

	//! Writes a PatchVB to a file
	static bool Write( char const * sFileName, PatchVB const & pvb );
};
