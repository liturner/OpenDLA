#include "Header.hlsli"

VS_OUTPUT main(VS_INPUT vsIn)
{
	VS_OUTPUT vsOut;

	vsOut.vertexColor = vsIn.vertexColor;

	vsOut.vPosViewportSpace = float4(vsIn.vertexPosition, 1.0);
	vsOut.vPosViewportSpace = mul(vsOut.vPosViewportSpace, orthoOriginZProjection);

	return vsOut;
}