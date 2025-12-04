struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
    float3 normal : NORMAL0;
};

float4 main(PS_IN input) : SV_TARGET
{
    float3 N = normalize(input.normal);
    float3 L = normalize(float3(0.0f, 1.0f, 0.0f)); // ã•ûŒü‚©‚çŒõ

    float diff = saturate(dot(N, L));
    float3 c = input.color.rgb * (0.2 + 0.8 * diff);

    return float4(c, input.color.a);
}
