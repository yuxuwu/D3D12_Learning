// Typedefs
struct PixelInputType {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 PS(PixelInputType input) : SV_TARGET {
    //return float4(0.0f, 0.0f, 0.0f, 1.0f);
    return input.color;
}