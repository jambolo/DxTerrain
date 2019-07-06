/** @file *//********************************************************************************************************

                                                   SkirtIBFactory.h

						                    Copyright 2004, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/SkirtIBFactory.h#3 $

	$NoKeywords: $

 ********************************************************************************************************************/

#pragma once

#include <atlcomcli.h>
#include <memory>

struct IDirect3DIndexBuffer9;
struct IDirect3DDevice9;


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

//! A class that creates skirt IBs

class SkirtIBFactory
{
public:
	//! Creates a skirt IB from the height field. Returns true if successful.
	static CComPtr<IDirect3DIndexBuffer9> Create( IDirect3DDevice9 * pD3dDevice,
												  int nIndexes, int nLods, int size,
												  __int16 const * paIJToIndexMap );

	//! Loads a skirt IB from a file
	static CComPtr<IDirect3DIndexBuffer9> Load( IDirect3DDevice9 * pD3dDevice, char const * sFileName );

	//! Writes a skirt IB to a file
	static bool Write( char const * sFileName, CComPtr<IDirect3DIndexBuffer9> const * pIB );
};
