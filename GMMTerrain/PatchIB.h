/** @file *//********************************************************************************************************

                                                      PatchIB.h

						                    Copyright 2004, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/PatchIB.h#4 $

	$NoKeywords: $

 ********************************************************************************************************************/

#pragma once

#include <d3d9.h>
#include <boost/noncopyable.hpp>

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

class PatchIB : public boost::noncopyable
{
public:
	//! Constructor
	PatchIB( IDirect3DIndexBuffer9 * pIB = 0 ) : m_pIB( pIB ) {}

	//! Destructor
	~PatchIB() { if ( m_pIB != 0 ) m_pIB->Release(); }

	IDirect3DIndexBuffer9 *		m_pIB;
};


