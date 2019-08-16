#include "Header.hlsli"

float4 main(PS_INPUT psIn) : SV_TARGET
{
	return float4(psIn.vertexColor, 1.0f);
}