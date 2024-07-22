struct VSOut
{
    float4 Pos : SV_POSITION;
};

cbuffer CBuf
{
    float4 faceColor[6];
};

float4 main(VSOut pixelIn, uint TriangleID : SV_PrimitiveID) : SV_TARGET
{
    float4 outColor = faceColor[TriangleID / 2];
    return outColor;
}