/** @file *//********************************************************************************************************

													TexturedParticle.fx

											Copyright 2003, John J. Bolton
	--------------------------------------------------------------------------------------------------------------

	$Header: $

	$NoKeywords: $

 ********************************************************************************************************************/

texture		LightMapTexture;
texture		DetailTexture0;
texture		DetailTexture1;
texture		DetailTexture2;
texture		DetailTexture3;
texture		DetailSelectorTexture;
float4x4	WorldViewProjectionMatrix;
float1		Lod;

//float4		LightMapTextureOffset;
//float4		LightMapTextureScale;
//float4		DetailTexture0Offset;
//float4		DetailTexture0Scale;
//float4		DetailTexture1Offset;
//float4		DetailTexture1Scale;
//float4		DetailTexture2Offset;
//float4		DetailTexture2Scale;
//float4		DetailTexture3Offset;
//float4		DetailTexture3Scale;
//float4		DetailSelectorTextureOffset;
//float4		DetailSelectorTextureScale;

technique T0
{
	pass P0
	{
		// Vertex shader constants
		
		VertexShaderConstant4[0]	= < WorldViewProjectionMatrix >;
		VertexShaderConstant1[4]	= < Lod >;
		VertexShaderConstant1[5]	= { 0, 0, 0, 0 };
		VertexShaderConstant1[6]	= { 1, 1, 1, 1 };
//		VertexShaderConstant1[7]	= < LightMapTextureOffset >;
//		VertexShaderConstant1[8]	= < LightMapTextureScale >;
//		VertexShaderConstant1[9]	= < DetailTexture0Offset >;
//		VertexShaderConstant1[10]	= < DetailTexture0Scale >;
//		VertexShaderConstant1[11]	= < DetailTexture1Offset >;
//		VertexShaderConstant1[12]	= < DetailTexture1Scale >;
//		VertexShaderConstant1[13]	= < DetailTexture2Offset >;
//		VertexShaderConstant1[14]	= < DetailTexture2Scale >;
//		VertexShaderConstant1[15]	= < DetailTexture3Offset >;
//		VertexShaderConstant1[16]	= < DetailTexture3Scale >;
//		VertexShaderConstant1[17]	= < DetailSelectorTextureOffset >;
//		VertexShaderConstant1[18]	= < DetailSelectorTextureScale >;

		// Texture stage 0

		Texture[0]		= < LightMapTexture >;

		MinFilter[0]	= LINEAR;
		MagFilter[0]	= LINEAR;
		MipFilter[0]	= LINEAR;
		AddressU[0]		= CLAMP;
		AddressV[0]		= CLAMP;

		// Texture stage 1

		Texture[1]		= < DetailTexture0 >;

		MinFilter[1]	= LINEAR;
		MagFilter[1]	= LINEAR;
		MipFilter[1]	= LINEAR;
		AddressU[1]		= WRAP;
		AddressV[1]		= WRAP;

//		// Texture stage 2
//
//		Texture[2]		= < DetailTexture1 >;
//
//		MinFilter[2]	= LINEAR;
//		MagFilter[2]	= LINEAR;
//		MipFilter[2]	= LINEAR;
//		AddressU[2]		= WRAP;
//		AddressV[2]		= WRAP;
//
//		// Texture stage 3
//
//		Texture[3]		= < DetailTexture2 >;
//
//		MinFilter[3]	= LINEAR;
//		MagFilter[3]	= LINEAR;
//		MipFilter[3]	= LINEAR;
//		AddressU[3]		= WRAP;
//		AddressV[3]		= WRAP;
//
//		// Texture stage 4
//
//		Texture[4]		= < DetailTexture3 >;
//
//		MinFilter[4]	= LINEAR;
//		MagFilter[4]	= LINEAR;
//		MipFilter[4]	= LINEAR;
//		AddressU[4]		= WRAP;
//		AddressV[4]		= WRAP;
//
//		// Texture stage 5
//
//		Texture[5]		= < DetailSelectorTexture >;
//
//		MinFilter[5]	= POINT;
//		MagFilter[5]	= POINT;
//		MipFilter[5]	= NONE;
//		AddressU[5]		= CLAMP;
//		AddressV[5]		= CLAMP;

		VertexShader =
			asm
			{
				vs_1_1

				// Constants
				//	c0-c3	world-view-projection matrix (transposed)
				//	c4		lod
				//	c5		0,0,0,0
				//	c6		1,1,1,1

				// Inputs
				dcl_position0	v0			// position data
				dcl_position1	v1			// interpolated Z (in x)
				dcl_blendweight	v2			// max lod (in x)
				dcl_texcoord0	v3			// light map uv coords
				dcl_texcoord1	v4			// detail map 0 uv coords
				dcl_texcoord2	v5			// detail map 1 uv coords
				dcl_texcoord3	v6			// detail map 2 uv coords
				dcl_texcoord4	v7			// detail map 3 uv coords
				dcl_texcoord5	v8			// detail map selector uv coords

				// Outputs
				//	oPos	vertex coordinates
				//	oT0		light map uv
				//	oT1		detail map 0 uv
				//	oT2		detail map 1 uv
				//	oT3		detail map 2 uv
				//	oT4		detail map 3 uv
				//	oT5		detail map selector uv

				add			r0.x, c4.x, -v2.x		// Get lod difference (lod - max lod)
				max			r0.x, r0.x, c5.x		// limit to 0
				mov			r2, v0					// Get position
				add			r1.z, v1.x, -r2.z		// Get z difference (interpolatedz - z)
				mad			r2.z, r0.x, r1.z, r2.z	// lerp r2.z = z + r0.x * ( v1.x - z )
				m4x4		oPos, r2, c0			// transform vertices by world-view-projection matrix
				
				mov			oT0, v3					// pass uv
				mov			oT1, v4					// pass uv
				mov			oT2, v5					// pass uv
				mov			oT3, v6					// pass uv
				mov			oT4, v7					// pass uv
				mov			oT5, v8					// pass uv


//				mov			r0, c5
//				
//				add			r0.xy, c7, v0
//				mul			oT0, r0, c8
//
//				add			r0.xy, c9, v0
//				mul			oT1, r0, c10
//
//				add			r0.xy, c11, v0
//				mul			oT2, r0, c12
//
//				add			r0.xy, c13, v0
//				mul			oT3, r0, c14
//
//				add			r0.xy, c15, v0
//				mul			oT4, r0, c16
//
//				add			r0.xy, c17, v0
//				mul			oT5, r0, c18
			};
			
		PixelShader  =
			asm
			{
				ps_1_4
				
				// Inputs
				//	none
				
				// Texture Stages
				//	t0		light map
				//	t1		detail0 map
				//	t2		detail1 map
				//	t3		detail2 map
				//	t4		detail3 map
				//	t5		detail selector map

				texld		r0,t0
				texld		r1,t1
//				texld		r2,t2
//				texld		r3,t3
//				texld		r4,t4
//				texld		r5,t5
				
//				mul			r1, r1, r5.r
//				mad			r1, r2, r5.b, r1
//				mad			r1, r3, r5.g, r1
//				mad			r1, r4, r5.a, r1
				mul			r0, r0, r1
			};
	}
}
