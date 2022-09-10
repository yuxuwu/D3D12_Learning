// Globals
cbuffer cbPerObject {
    float4x4 gWorldViewProj;
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

    output.position = mul(float4(input.position.xyz, 1.0f), gWorldViewProj);
    //output.position = float4(input.position, 1.0f);

    output.color = input.color;

    return output;
}