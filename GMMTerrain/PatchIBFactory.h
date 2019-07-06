/** @file *//********************************************************************************************************

                                                   PatchIBFactory.h

						                    Copyright 2004, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/PatchIBFactory.h#4 $

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

//! A class that creates patch IBs

class PatchIBFactory
{
public:
	//! Creates a patch IB from the height field. Returns 0 if it fails.
	static CComPtr<IDirect3DIndexBuffer9> Create( IDirect3DDevice9 * pD3dDevice,
												  int nIndexes, int nLods, int size,
												  __int16 const * paIJToIndexMap );

	//! Loads a patch IB from a file. Returns 0 if it fails.
	static CComPtr<IDirect3DIndexBuffer9> Load( IDirect3DDevice9 * pD3dDevice, char const * sFileName );

	//! Writes a patch IB to a file
	static bool Write( char const * sFileName, IDirect3DIndexBuffer9 const * pIB );
};
