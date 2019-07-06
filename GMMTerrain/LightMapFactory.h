/** @file *//********************************************************************************************************

                                                  LightMapFactory.h

						                    Copyright 2004, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/LightMapFactory.h#3 $

	$NoKeywords: $

 ********************************************************************************************************************/

#pragma once

#include "Dxx/Light.h"
#include <atlcomcli.h>

struct IDirect3DTexture9;
class HeightField;

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

class LightMapFactory
{
public:

	//! Creates a light map for a heightfield using a directional light
	static CComPtr<IDirect3DTexture9> LightMapFactory::Create( IDirect3DDevice9 * pD3dDevice,
															   HeightField const & hf,
															   float const xyScale,
															   Dxx::DirectionalLight const & light,
															   HeightField * pNorth,
															   HeightField * pEast,
															   HeightField * pSouth,
															   HeightField * pWest, 
															   float ambient );
};
