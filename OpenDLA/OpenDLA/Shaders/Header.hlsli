struct VS_INPUT
{
	float3 vertexPosition : POSITION;	// Raw position with no transformations
	float3 vertexColor : COLOR0;
};

struct VS_OUTPUT
{
	float4 vPosViewportSpace : SV_POSITION;
	float3 vertexColor : COLOR0;
};

struct PS_INPUT
{
	float4 vPosViewportSpace : SV_POSITION;
	float3 vertexColor : COLOR0;
};

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix orthoOriginZProjection;  // projection matrix
};