/********************************************************************************************************************

                                                       main.cpp

						                    Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/VertexIJToBufferIndexMapGenerator/main.cpp#2 $

	$NoKeywords: $

 ********************************************************************************************************************/

#include <cstdio>
#include <cstring>
#include <cassert>

#define NUMBER_OF_LODS	6
#define PATCH_SIZE		33

static int	s_PatchIJToIndexMap[ PATCH_SIZE ][ PATCH_SIZE ];
static int	s_SkirtIJToIndexMap[ ( PATCH_SIZE-1 ) * 4 ][ 2 ];

void main( int argc, char * * argv )
{
	// Open the file

	FILE * fp	= fopen( "VertexIJToBufferIndexMap.cpp", "w" );
	assert( fp != 0 );

	{
		// Clear the map

		memset( s_PatchIJToIndexMap, -1, sizeof( s_PatchIJToIndexMap ) );

		// Build the map

		int index	= 0;

		for ( int lod = NUMBER_OF_LODS-1; lod >= 0; lod-- )
		{
			int const	step	= 1 << lod;

			for ( int i = 0; i < PATCH_SIZE; i += step )
			{
				for ( int j = 0; j < PATCH_SIZE; j += step )
				{
					if ( s_PatchIJToIndexMap[i][j] < 0 )
					{
						s_PatchIJToIndexMap[i][j] = index;
						++index;
					}
				}
			}
		}

		// Output the map

		fprintf( fp, "__int16 const\ts_VertexIJToBufferIndexMap[ Patch::SIZE ][ Patch::SIZE ] =\n" );
		fprintf( fp, "{\n" );

		for ( int i = 0; i < PATCH_SIZE; i++ )
		{
			fprintf( fp, "\t{ %4d", s_PatchIJToIndexMap[ i ][ 0 ] );

			for ( int j = 1; j < PATCH_SIZE; j++ )
			{
				fprintf( fp, ", %4d", s_PatchIJToIndexMap[ i ][ j ] );
			}

			fprintf( fp, " },\n" );
		}

		fprintf( fp, "};\n" );
	}

	{
		// Clear the map

		memset( s_SkirtIJToIndexMap, -1, sizeof( s_SkirtIJToIndexMap ) );

		// Build the map

		int index	= 0;

		for ( int lod = NUMBER_OF_LODS-1; lod >= 0; lod-- )
		{
			int const	step	= 1 << lod;

			for ( int i = 0; i < ( PATCH_SIZE-1 ) * 4; i += step )
			{
				for ( int j = 0; j < 2; j ++ )
				{
					if ( s_SkirtIJToIndexMap[i][j] < 0 )
					{
						s_SkirtIJToIndexMap[i][j] = index;
						++index;
					}
				}
			}
		}

		// Output the map

		fprintf( fp, "__int16 const\ts_SkirtIJToIndexMap[ ( Patch::SIZE - 1 ) * 4 ][ 2 ] =\n" );
		fprintf( fp, "{\n" );

		for ( int i = 0; i < 4; i++ )
		{
			fprintf( fp, "\t{ %3d, %3d }", s_SkirtIJToIndexMap[ i * ( PATCH_SIZE-1 ) ][ 0 ], s_SkirtIJToIndexMap[ i * ( PATCH_SIZE-1 ) ][ 1 ] );

			for ( int j = 1; j < PATCH_SIZE-1; j++ )
			{
				fprintf( fp, ", { %3d, %3d }", s_SkirtIJToIndexMap[ i * ( PATCH_SIZE-1 ) + j ][ 0 ], s_SkirtIJToIndexMap[ i * ( PATCH_SIZE-1 ) + j ][ 1 ] );
			}

			if ( i < 3 )
			{
				fprintf( fp, ",\n" );
			}
			else
			{
				fprintf( fp, "\n" );
			}
		}

		fprintf( fp, "};\n" );
	}

	fclose( fp );
}

