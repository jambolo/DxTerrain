/********************************************************************************************************************

                                                      Patch.cpp

						                    Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: //depot/DxTerrain/GMMTerrain/Patch.cpp#8 $

	$NoKeywords: $

 ********************************************************************************************************************/

#include "PrecompiledHeaders.h"

#include "Patch.h"

#include "PatchIBFactory.h"
#include "PatchVB.h"
#include "PatchVBFactory.h"
#include "SkirtIBFactory.h"
#include "SkirtVB.h"
#include "SkirtVBFactory.h"

#include "Console/Console.h"
#include "Dxx/Dxx.h"
#include "HeightField/HeightField.h"
#include "Misc/Assert.h"
#include "Wx/Wx.h"

using namespace std;


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

namespace
{

struct LodParameters
{
	int m_IndexBase;			// First index of LOD in IB
	int m_PatchSize;			// Number of vertices in each direction
	int m_PrimitiveCount;		// Number of primitives in each strip
};

LodParameters const	s_aPatchLodParameters[ Patch::NUMBER_OF_LODS ] =
{
	//					m_IndexBase						 m_PatchSize m_PrimitiveCount
	{ 1*1*2*3 + 2*2*2*3 + 4*4*2*3 + 8*8*2*3 + 16*16*2*3,	33,			32*32*2		},	// 0 - 33 x 33
	{ 1*1*2*3 + 2*2*2*3 + 4*4*2*3 + 8*8*2*3,				17,			16*16*2		},	// 1 - 17 x 17
	{ 1*1*2*3 + 2*2*2*3 + 4*4*2*3,							 9,			 8* 8*2		},	// 2 - 9 x 9
	{ 1*1*2*3 + 2*2*2*3,									 5,			 4* 4*2		},	// 3 - 5 x 5
	{ 1*1*2*3,												 3,			 2* 2*2		},	// 4 - 3 x 3
	{ 0,													 2,			 1* 1*2		}	// 5 - 2 x 2
};

LodParameters const	s_aSkirtLodParameters[ Patch::NUMBER_OF_LODS ] =
{
	//					m_IndexBase						m_PatchSize	m_PrimitiveCount
	{ 1*2*4+2 + 2*2*4+2 + 4*2*4+2 + 8*2*4+2 + 16*2*4+2,		33,			32*2*4		},	// 0 - 33 x 2 x 4
	{ 1*2*4+2 + 2*2*4+2 + 4*2*4+2 + 8*2*4+2,				17,			16*2*4		},	// 1 - 17 x 2 x 4
	{ 1*2*4+2 + 2*2*4+2 + 4*2*4+2,							 9,			 8*2*4		},	// 2 - 9 x 2 x 4
	{ 1*2*4+2 + 2*2*4+2,									 5,			 4*2*4		},	// 3 - 5 x 2 x 4
	{ 1*2*4+2,												 3,			 2*2*4		},	// 4 - 3 x 2 x 4
	{ 0,													 2,			 1*2*4		}	// 5 - 2 x 2 x 4
};


__int16 const	s_aPatchIJToIndexMap[ Patch::SIZE ][ Patch::SIZE ] =
{
	{    0,  289,   81,  290,   25,  291,   82,  292,    9,  293,   83,  294,   26,  295,   84,  296,    4,  297,   85,  298,   27,  299,   86,  300,   10,  301,   87,  302,   28,  303,   88,  304,    1 },
	{  305,  306,  307,  308,  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,  330,  331,  332,  333,  334,  335,  336,  337 },
	{   89,  338,   90,  339,   91,  340,   92,  341,   93,  342,   94,  343,   95,  344,   96,  345,   97,  346,   98,  347,   99,  348,  100,  349,  101,  350,  102,  351,  103,  352,  104,  353,  105 },
	{  354,  355,  356,  357,  358,  359,  360,  361,  362,  363,  364,  365,  366,  367,  368,  369,  370,  371,  372,  373,  374,  375,  376,  377,  378,  379,  380,  381,  382,  383,  384,  385,  386 },
	{   29,  387,  106,  388,   30,  389,  107,  390,   31,  391,  108,  392,   32,  393,  109,  394,   33,  395,  110,  396,   34,  397,  111,  398,   35,  399,  112,  400,   36,  401,  113,  402,   37 },
	{  403,  404,  405,  406,  407,  408,  409,  410,  411,  412,  413,  414,  415,  416,  417,  418,  419,  420,  421,  422,  423,  424,  425,  426,  427,  428,  429,  430,  431,  432,  433,  434,  435 },
	{  114,  436,  115,  437,  116,  438,  117,  439,  118,  440,  119,  441,  120,  442,  121,  443,  122,  444,  123,  445,  124,  446,  125,  447,  126,  448,  127,  449,  128,  450,  129,  451,  130 },
	{  452,  453,  454,  455,  456,  457,  458,  459,  460,  461,  462,  463,  464,  465,  466,  467,  468,  469,  470,  471,  472,  473,  474,  475,  476,  477,  478,  479,  480,  481,  482,  483,  484 },
	{   11,  485,  131,  486,   38,  487,  132,  488,   12,  489,  133,  490,   39,  491,  134,  492,   13,  493,  135,  494,   40,  495,  136,  496,   14,  497,  137,  498,   41,  499,  138,  500,   15 },
	{  501,  502,  503,  504,  505,  506,  507,  508,  509,  510,  511,  512,  513,  514,  515,  516,  517,  518,  519,  520,  521,  522,  523,  524,  525,  526,  527,  528,  529,  530,  531,  532,  533 },
	{  139,  534,  140,  535,  141,  536,  142,  537,  143,  538,  144,  539,  145,  540,  146,  541,  147,  542,  148,  543,  149,  544,  150,  545,  151,  546,  152,  547,  153,  548,  154,  549,  155 },
	{  550,  551,  552,  553,  554,  555,  556,  557,  558,  559,  560,  561,  562,  563,  564,  565,  566,  567,  568,  569,  570,  571,  572,  573,  574,  575,  576,  577,  578,  579,  580,  581,  582 },
	{   42,  583,  156,  584,   43,  585,  157,  586,   44,  587,  158,  588,   45,  589,  159,  590,   46,  591,  160,  592,   47,  593,  161,  594,   48,  595,  162,  596,   49,  597,  163,  598,   50 },
	{  599,  600,  601,  602,  603,  604,  605,  606,  607,  608,  609,  610,  611,  612,  613,  614,  615,  616,  617,  618,  619,  620,  621,  622,  623,  624,  625,  626,  627,  628,  629,  630,  631 },
	{  164,  632,  165,  633,  166,  634,  167,  635,  168,  636,  169,  637,  170,  638,  171,  639,  172,  640,  173,  641,  174,  642,  175,  643,  176,  644,  177,  645,  178,  646,  179,  647,  180 },
	{  648,  649,  650,  651,  652,  653,  654,  655,  656,  657,  658,  659,  660,  661,  662,  663,  664,  665,  666,  667,  668,  669,  670,  671,  672,  673,  674,  675,  676,  677,  678,  679,  680 },
	{    5,  681,  181,  682,   51,  683,  182,  684,   16,  685,  183,  686,   52,  687,  184,  688,    6,  689,  185,  690,   53,  691,  186,  692,   17,  693,  187,  694,   54,  695,  188,  696,    7 },
	{  697,  698,  699,  700,  701,  702,  703,  704,  705,  706,  707,  708,  709,  710,  711,  712,  713,  714,  715,  716,  717,  718,  719,  720,  721,  722,  723,  724,  725,  726,  727,  728,  729 },
	{  189,  730,  190,  731,  191,  732,  192,  733,  193,  734,  194,  735,  195,  736,  196,  737,  197,  738,  198,  739,  199,  740,  200,  741,  201,  742,  202,  743,  203,  744,  204,  745,  205 },
	{  746,  747,  748,  749,  750,  751,  752,  753,  754,  755,  756,  757,  758,  759,  760,  761,  762,  763,  764,  765,  766,  767,  768,  769,  770,  771,  772,  773,  774,  775,  776,  777,  778 },
	{   55,  779,  206,  780,   56,  781,  207,  782,   57,  783,  208,  784,   58,  785,  209,  786,   59,  787,  210,  788,   60,  789,  211,  790,   61,  791,  212,  792,   62,  793,  213,  794,   63 },
	{  795,  796,  797,  798,  799,  800,  801,  802,  803,  804,  805,  806,  807,  808,  809,  810,  811,  812,  813,  814,  815,  816,  817,  818,  819,  820,  821,  822,  823,  824,  825,  826,  827 },
	{  214,  828,  215,  829,  216,  830,  217,  831,  218,  832,  219,  833,  220,  834,  221,  835,  222,  836,  223,  837,  224,  838,  225,  839,  226,  840,  227,  841,  228,  842,  229,  843,  230 },
	{  844,  845,  846,  847,  848,  849,  850,  851,  852,  853,  854,  855,  856,  857,  858,  859,  860,  861,  862,  863,  864,  865,  866,  867,  868,  869,  870,  871,  872,  873,  874,  875,  876 },
	{   18,  877,  231,  878,   64,  879,  232,  880,   19,  881,  233,  882,   65,  883,  234,  884,   20,  885,  235,  886,   66,  887,  236,  888,   21,  889,  237,  890,   67,  891,  238,  892,   22 },
	{  893,  894,  895,  896,  897,  898,  899,  900,  901,  902,  903,  904,  905,  906,  907,  908,  909,  910,  911,  912,  913,  914,  915,  916,  917,  918,  919,  920,  921,  922,  923,  924,  925 },
	{  239,  926,  240,  927,  241,  928,  242,  929,  243,  930,  244,  931,  245,  932,  246,  933,  247,  934,  248,  935,  249,  936,  250,  937,  251,  938,  252,  939,  253,  940,  254,  941,  255 },
	{  942,  943,  944,  945,  946,  947,  948,  949,  950,  951,  952,  953,  954,  955,  956,  957,  958,  959,  960,  961,  962,  963,  964,  965,  966,  967,  968,  969,  970,  971,  972,  973,  974 },
	{   68,  975,  256,  976,   69,  977,  257,  978,   70,  979,  258,  980,   71,  981,  259,  982,   72,  983,  260,  984,   73,  985,  261,  986,   74,  987,  262,  988,   75,  989,  263,  990,   76 },
	{  991,  992,  993,  994,  995,  996,  997,  998,  999, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023 },
	{  264, 1024,  265, 1025,  266, 1026,  267, 1027,  268, 1028,  269, 1029,  270, 1030,  271, 1031,  272, 1032,  273, 1033,  274, 1034,  275, 1035,  276, 1036,  277, 1037,  278, 1038,  279, 1039,  280 },
	{ 1040, 1041, 1042, 1043, 1044, 1045, 1046, 1047, 1048, 1049, 1050, 1051, 1052, 1053, 1054, 1055, 1056, 1057, 1058, 1059, 1060, 1061, 1062, 1063, 1064, 1065, 1066, 1067, 1068, 1069, 1070, 1071, 1072 },
	{    2, 1073,  281, 1074,   77, 1075,  282, 1076,   23, 1077,  283, 1078,   78, 1079,  284, 1080,    8, 1081,  285, 1082,   79, 1083,  286, 1084,   24, 1085,  287, 1086,   80, 1087,  288, 1088,    3 },
};


__int16 const	s_aSkirtIJToIndexMap[ 4 ][ Patch::SIZE-1 ][ 2 ] =
{
	{ {   0,   1 }, { 128, 129 }, {  64,  65 }, { 130, 131 }, {  32,  33 }, { 132, 133 }, {  66,  67 }, { 134, 135 }, {  16,  17 }, { 136, 137 }, {  68,  69 }, { 138, 139 }, {  34,  35 }, { 140, 141 }, {  70,  71 }, { 142, 143 }, {   8,   9 }, { 144, 145 }, {  72,  73 }, { 146, 147 }, {  36,  37 }, { 148, 149 }, {  74,  75 }, { 150, 151 }, {  18,  19 }, { 152, 153 }, {  76,  77 }, { 154, 155 }, {  38,  39 }, { 156, 157 }, {  78,  79 }, { 158, 159 } },
	{ {   2,   3 }, { 160, 161 }, {  80,  81 }, { 162, 163 }, {  40,  41 }, { 164, 165 }, {  82,  83 }, { 166, 167 }, {  20,  21 }, { 168, 169 }, {  84,  85 }, { 170, 171 }, {  42,  43 }, { 172, 173 }, {  86,  87 }, { 174, 175 }, {  10,  11 }, { 176, 177 }, {  88,  89 }, { 178, 179 }, {  44,  45 }, { 180, 181 }, {  90,  91 }, { 182, 183 }, {  22,  23 }, { 184, 185 }, {  92,  93 }, { 186, 187 }, {  46,  47 }, { 188, 189 }, {  94,  95 }, { 190, 191 } },
	{ {   4,   5 }, { 192, 193 }, {  96,  97 }, { 194, 195 }, {  48,  49 }, { 196, 197 }, {  98,  99 }, { 198, 199 }, {  24,  25 }, { 200, 201 }, { 100, 101 }, { 202, 203 }, {  50,  51 }, { 204, 205 }, { 102, 103 }, { 206, 207 }, {  12,  13 }, { 208, 209 }, { 104, 105 }, { 210, 211 }, {  52,  53 }, { 212, 213 }, { 106, 107 }, { 214, 215 }, {  26,  27 }, { 216, 217 }, { 108, 109 }, { 218, 219 }, {  54,  55 }, { 220, 221 }, { 110, 111 }, { 222, 223 } },
	{ {   6,   7 }, { 224, 225 }, { 112, 113 }, { 226, 227 }, {  56,  57 }, { 228, 229 }, { 114, 115 }, { 230, 231 }, {  28,  29 }, { 232, 233 }, { 116, 117 }, { 234, 235 }, {  58,  59 }, { 236, 237 }, { 118, 119 }, { 238, 239 }, {  14,  15 }, { 240, 241 }, { 120, 121 }, { 242, 243 }, {  60,  61 }, { 244, 245 }, { 122, 123 }, { 246, 247 }, {  30,  31 }, { 248, 249 }, { 124, 125 }, { 250, 251 }, {  62,  63 }, { 252, 253 }, { 126, 127 }, { 254, 255 } }
};


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

// Returns the total number of indexes in a patch's index buffer
int _NumPatchIBElements()
{
	int		nPrimitives	= 0;

	for ( int i = 0; i < Patch::NUMBER_OF_LODS; i++ )
	{
		nPrimitives += s_aPatchLodParameters[ i ].m_PrimitiveCount;
	}

	// We are using triangle lists so the number of vertexes is 3 * the number of primitives

	return nPrimitives * 3;
}

// Returns the total number of indexes in a skirt's index buffer
int _NumSkirtIBElements()
{
	int		nVerts	= 0;

	for ( int i = 0; i < Patch::NUMBER_OF_LODS; i++ )
	{
		nVerts += s_aSkirtLodParameters[ i ].m_PrimitiveCount + 2;
	}

	return nVerts;
}

// Returns the maximum LOD for a vertex at [i, j]
int _ComputeMaxLod( int i, int j )
{
	int		lod		= 0;

	if ( ( i | j ) == 0 )
	{
		lod = Patch::NUMBER_OF_LODS - 1;
	}
	else
	{
		while ( ( ( i | j ) & ( 1 << lod ) ) == 0 )
		{
			++lod;
		}
	}

	return lod;
}

} // anonymous namespace


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

Patch::DebugSettings					Patch::m_DebugSettings			= { false, false, false };
float const								Patch::VIEW_SPACE_TOLERANCE		= 0.02f;
CComPtr<IDirect3DIndexBuffer9>			Patch::m_qPatchIB;
CComPtr<IDirect3DIndexBuffer9>			Patch::m_qSkirtIB;
CComPtr<IDirect3DVertexDeclaration9>	Patch::m_qVertexDeclaration;

/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

Patch::Patch( IDirect3DDevice9 * pD3dDevice,
			  int i0, int j0,
			  float x0, float y0,
			  HeightField const & hf,
			  float xyScale,
			  Dxx::Camera const & camera )
	: m_pD3dDevice( pD3dDevice ),
	m_I0( i0 ),
	m_J0( j0 )
{
	m_pD3dDevice->AddRef();

	assert( SIZE == 33 );

	HRESULT		hr;

	// Initialize the LOD error tolerances and LOD distances

	InitializeLodControl( camera, hf );

	// Initialize the patch and skirt vertex buffers

	m_pPatchVB = PatchVBFactory::Create( m_pD3dDevice, hf, j0, i0, xyScale, x0, y0,
										 (__int16 const *)s_aPatchIJToIndexMap, SIZE ).release();
	m_pSkirtVB = SkirtVBFactory::Create( m_pD3dDevice, hf, j0, i0, xyScale, x0, y0,
										 (__int16 const *)s_aSkirtIJToIndexMap, SIZE, m_MaxError ).release();

	// If the index buffers have not been created yet then create them now

	if ( m_qPatchIB == 0 )
	{
		m_qPatchIB = PatchIBFactory::Create( m_pD3dDevice, _NumPatchIBElements(), NUMBER_OF_LODS, SIZE,
											 (__int16 const *)s_aPatchIJToIndexMap );
	}

	if ( m_qSkirtIB == 0 )
	{
		m_qSkirtIB = SkirtIBFactory::Create( m_pD3dDevice, _NumSkirtIBElements(), NUMBER_OF_LODS, SIZE,
											 (__int16 const *)s_aSkirtIJToIndexMap );
	}

	// If the vertex declaration hasn't been created yet, create it now

	if ( m_qVertexDeclaration == 0 )
	{
		hr = m_pD3dDevice->CreateVertexDeclaration( PatchVB::m_declaration, &m_qVertexDeclaration );
		assert_succeeded( hr );
	}
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

Patch::~Patch()
{
	delete m_pPatchVB;
	delete m_pSkirtVB;
	Wx::SafeRelease( m_pD3dDevice );
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Patch::InitializeLodControl( Dxx::Camera const & camera, HeightField const & hf )
{
	m_MaxError = 0.0f;
	m_aMinDistances[ 0 ] = 0.0f;

	for ( int lod = 1; lod < NUMBER_OF_LODS; lod++ )
	{
		int const	step	= 1 << lod;
		float		maxe	= 0.0f;

		for ( int i = m_I0; i < m_I0 + SIZE-1; i += step )
		{
			for ( int j = m_J0; j < m_J0 + SIZE-1; j += step )
			{
				for ( int m = 0; m < step; m++ )
				{
					for ( int n = 0; n < step; n++ )
					{
						float const		e	=   hf.GetInterpolatedZ( float(j + n), float(i + m), step )
											  - hf.GetZ( j + n, i + m );

						maxe = max( maxe, fabsf( e ) );
					}
				}
			}
			
			// Last column of vertexes

			{
				for ( int m = 0; m < step; m++ )
				{
					float const		e	=   hf.GetInterpolatedZ( float(m_J0 + SIZE-1), float(i + m), step )
										  - hf.GetZ( m_J0 + SIZE-1, i + m );

					maxe = max( maxe, fabsf( e ) );
				}
			}
		}

		// Last row of vertexes

		{
			for ( int j = m_J0; j < m_J0 + SIZE-1; j += step )
			{
				for ( int n = 0; n < step; n++ )
				{
					float const		e	=   hf.GetInterpolatedZ( float(j + n), float(m_I0 + SIZE-1), step )
										  - hf.GetZ( j + n, m_I0 + SIZE-1 );

					maxe = max( maxe, fabsf( e ) );
				}
			}
		}

		m_aMinDistances[ lod ] = 0.5f * maxe / tanf( camera.GetAngleOfView() * 0.5f ) / VIEW_SPACE_TOLERANCE;
		m_aMinDistances[ lod ] *= m_aMinDistances[ lod ];	// Store the squared distance
		m_MaxError = max( m_MaxError, maxe );
	}
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Patch::Draw( int lod )
{
	m_PerformanceInfo.m_TriangleCount = 0;

	// Draw the terrain

	if ( !m_DebugSettings.bHideTerrain )
	{
		assert_array_index_valid( s_aPatchLodParameters, lod );

		HRESULT		hr;

		LodParameters const &			lp	= s_aPatchLodParameters[ lod ];

		hr = m_pD3dDevice->SetStreamSource( 0, m_pPatchVB->m_pVB, 0, PatchVB::STRIDE );
		assert_succeeded( hr );

		hr = m_pD3dDevice->SetIndices( m_qPatchIB );
		assert_succeeded( hr );

		// Draw the patch

		hr = m_pD3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
												 0,
												 0,
												 lp.m_PatchSize * lp.m_PatchSize,
												 lp.m_IndexBase,
												 lp.m_PrimitiveCount );
		assert_succeeded( hr );

		m_PerformanceInfo.m_TriangleCount += lp.m_PrimitiveCount;
	}

	// Draw the skirt

	if ( !m_DebugSettings.bHideSkirt )
	{
		assert_array_index_valid( s_aSkirtLodParameters, lod );

		HRESULT		hr;

		LodParameters const &			lp	= s_aSkirtLodParameters[ lod ];

		hr = m_pD3dDevice->SetStreamSource( 0, m_pSkirtVB->m_pVB, 0, SkirtVB::STRIDE );
		assert_succeeded( hr );

		hr = m_pD3dDevice->SetIndices( m_qSkirtIB );
		assert_succeeded( hr );

		// Draw the patch

		hr = m_pD3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP,
												 0,
												 0,
												 (lp.m_PatchSize-1 ) * 4 * 2,
												 lp.m_IndexBase,
												 lp.m_PrimitiveCount );
		assert_succeeded( hr );
	
		m_PerformanceInfo.m_TriangleCount += lp.m_PrimitiveCount;
	}

	// Draw the bounding box

	if ( m_DebugSettings.bDrawBoundingBox )
	{
	}
}


/********************************************************************************************************************/
/*																													*/
/********************************************************************************************************************/

void Patch::AddConsoleVariables()
{
	Console::Instance()->AddVariable( "noskirt", &m_DebugSettings.bHideSkirt );
	Console::Instance()->AddVariable( "noterrain", &m_DebugSettings.bHideTerrain );
	Console::Instance()->AddVariable( "boundingbox", &m_DebugSettings.bDrawBoundingBox );
}
