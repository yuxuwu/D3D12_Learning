// Globals
cbuffer cbPerObject {
    row_major float4x4 gViewProj;
};

// Typedefs
struct VertexInputType {
    float3 position : POSITION;
    float4 color : COLOR;
    row_major float4x4 World : WORLD;
    uint InstanceId : SV_InstanceId;
};
struct PixelInputType {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PixelInputType VS(VertexInputType input) {
    PixelInputType output;

    float4x4 wvp = mul(input.World, gViewProj);
    output.position = mul(float4(input.position, 1.0f), wvp);

    output.color = input.color;

    return output;
}