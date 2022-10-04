// Globals
cbuffer cbPerObject {
    float4x4 gWorldViewProj;
    float4 offset;
};

// Typedefs
struct VertexInputType {
    float3 position : POSITION;
    float4 color : COLOR;
};
struct PixelInputType {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PixelInputType VS(VertexInputType input) {
    PixelInputType output;
    float3 positionWithOffset = input.position + offset.xyz;
    output.position = mul(float4(positionWithOffset.xyz, 1.0f), gWorldViewProj);

    output.color = input.color;

    return output;
}