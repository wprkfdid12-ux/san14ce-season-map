// SAN14 season map v26k27: the game's own shader 9addb6cb884c7a7c, translated instruction by instruction (unchanged).
// Used only by the test build's speed test (ways 1 and 2: drawn as the game draws it).

SamplerState s3 : register(s3);
SamplerState s4 : register(s4);
SamplerState s5 : register(s5);
SamplerState s6 : register(s6);
SamplerState s10 : register(s10);
Texture2D<float4> t3 : register(t3);
Texture2D<float4> t4 : register(t4);
Texture2D<float4> t5 : register(t5);
Texture2D<float4> t6 : register(t6);
Texture2D<float4> t10 : register(t10);
cbuffer cb0 : register(b0) { float4 cb0[261]; }


void main(float4 v0 : SV_Position,
          float4 v1 : TEXCOORD0,
          float3 v2 : TEXCOORD1,
          float3 v3 : TEXCOORD2,
          float2 v4 : TEXCOORD3,
          out float4 o0 : SV_Target0,
          out float4 o1 : SV_Target1,
          out float4 o2 : SV_Target2,
          out float4 o3 : SV_Target3,
          out float4 o4 : SV_Target4)
{
    o0 = 0;
    o1 = 0;
    o2 = 0;
    o3 = 0;
    o4 = 0;
    uint4 r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    r0.x = asuint((t3.Sample(s3, (v4.xy))).x);
    r0.x = asuint((asfloat(r0.x)) * (255.0));
    r0.x = asuint((int)(asfloat(r0.x)));
    r0.x = (uint)((1.0) >= (cb0[6 + asint((asfloat(r0.x)))].x)) * 0xffffffffu;
    if (((r0.x) != 0)) discard;
    r0.xyzw = asuint((t4.Sample(s4, (v4.xy))).xyzw);
    r1.xyz = asuint((t5.Sample(s5, (v4.xy))).xyz);
    r2.xyzw = asuint((t6.Sample(s6, (v4.xy))).xyzw);
    r3.xyz = asuint((t10.Sample(s10, (v4.xy))).xyz);
    r0.xyz = asuint((asfloat(r0.xyz)) * (asfloat(r3.xyz)));
    r3.xy = (asuint(cb0[260].yy)) & (uint2(32u,64u));
    r4.xyz = asuint((abs(v3.xyz)) + (-cb0[5].xxx));
    r4.xyz = asuint(max((asfloat(r4.xyz)), (float3(0.0,0.0,0.0))));
    r4.xyz = (uint3)((float3(0.0,0.0,0.0)) < (asfloat(r4.xyz))) * 0xffffffffu;
    r1.w = (r4.x) & (1u);
    r1.w = asuint((-asint(r4.y)) + (asint(r1.w)));
    r1.w = asuint((-asint(r4.z)) + (asint(r1.w)));
    r4.xyz = (uint3)((asint(r1.www)) == (int3(1,2,3))) * 0xffffffffu;
    r5.xyz = ((r4.xxx) != 0) ? (uint3(0u,1065353216u,0u)) : (r0.xyz);
    r4.xyw = ((r4.yyy) != 0) ? (uint3(1065353216u,1065353216u,0u)) : (r5.xyz);
    r4.xyz = ((r4.zzz) != 0) ? (uint3(1065353216u,0u,0u)) : (r4.xyw);
    r0.xyz = ((r3.xxx) != 0) ? (r4.xyz) : (r0.xyz);
    r1.w = asuint((cb0[4].z) * (cb0[4].y));
    r4.xyzw = asuint((asfloat(r1.wwww)) * (float4(0.03125,0.0625,0.125,0.25)) + (float4(10.0,10.0,10.0,10.0)));
    r4.xyzw = (uint4)((asfloat(r4.xyzw)) >= (v1.xxxx)) * 0xffffffffu;
    r1.w = asuint((asfloat(r1.w)) * (0.5) + (10.0));
    r1.w = (uint)((asfloat(r1.w)) >= (v1.x)) * 0xffffffffu;
    r3.xzw = ((r1.www) != 0) ? (uint3(0u,1061997773u,1065353216u)) : (uint3(0u,0u,1065353216u));
    r3.xzw = ((r4.www) != 0) ? (uint3(0u,1065353216u,0u)) : (r3.xzw);
    r3.xzw = ((r4.zzz) != 0) ? (uint3(1065353216u,1065353216u,0u)) : (r3.xzw);
    r3.xzw = ((r4.yyy) != 0) ? (uint3(1065353216u,1053609165u,0u)) : (r3.xzw);
    r3.xzw = ((r4.xxx) != 0) ? (uint3(1065353216u,0u,0u)) : (r3.xzw);
    o0.xyz = asfloat(((r3.yyy) != 0) ? (r3.xzw) : (r0.xyz));
    r0.xyz = asuint((asfloat(r1.xyz)) * (float3(2.0,2.0,2.0)) + (float3(-1.0,-1.0,-1.0)));
    r1.x = asuint(dot((asfloat(r0.xyz)), (asfloat(r0.xyz))));
    r1.x = asuint(rsqrt((asfloat(r1.x))));
    r0.xyz = asuint((asfloat(r0.xyz)) * (asfloat(r1.xxx)));
    r1.x = asuint(dot((v3.xyz), (v3.xyz)));
    r1.x = asuint(rsqrt((asfloat(r1.x))));
    r1.xyz = asuint((asfloat(r1.xxx)) * (v3.zxy));
    r1.w = asuint(dot((v2.xyz), (v2.xyz)));
    r1.w = asuint(rsqrt((asfloat(r1.w))));
    r3.xyz = asuint((asfloat(r1.www)) * (v2.zxy));
    r4.xyz = asuint((asfloat(r1.zxy)) * (asfloat(r3.xyz)));
    r4.xyz = asuint((asfloat(r3.zxy)) * (asfloat(r1.xyz)) + (-asfloat(r4.xyz)));
    r4.xyz = asuint((asfloat(r4.xyz)) * (v1.www));
    r2.xyzw = asuint((asfloat(r2.xyzw)) * (cb0[0].xyzw));
    r5.xyz = asuint((asfloat(r0.yyy)) * (asfloat(r3.yzx)));
    r4.xyz = asuint((asfloat(r4.xyz)) * (asfloat(r0.xxx)) + (asfloat(r5.xyz)));
    r0.xyz = asuint((asfloat(r1.yzx)) * (asfloat(r0.zzz)) + (asfloat(r4.xyz)));
    r1.w = asuint(dot((asfloat(r3.yzx)), (asfloat(r0.xyz))));
    r3.xyz = asuint((-asfloat(r1.www)) * (asfloat(r0.zxy)) + (asfloat(r3.xyz)));
    r1.w = asuint(dot((asfloat(r3.xyz)), (asfloat(r3.xyz))));
    r1.w = asuint(rsqrt((asfloat(r1.w))));
    r3.xyz = asuint((asfloat(r1.www)) * (asfloat(r3.xyz)));
    r1.w = asuint((float)(asint(cb0[4].x)));
    r4.y = asuint(dot((cb0[1].xyz), (asfloat(r0.xyz))));
    r4.z = asuint(dot((cb0[2].xyz), (asfloat(r0.xyz))));
    r4.x = asuint(dot((cb0[3].xyz), (asfloat(r0.xyz))));
    r0.x = asuint((abs(asfloat(r4.z))) + (abs(asfloat(r4.y))));
    r0.x = asuint((abs(asfloat(r4.x))) + (asfloat(r0.x)));
    r0.xyz = asuint((asfloat(r4.xyz)) / (asfloat(r0.xxx)));
    r4.xy = asuint((-abs(asfloat(r0.zy))) + (float2(1.0,1.0)));
    r5.xyz = (uint3)((asfloat(r0.xyz)) >= (float3(0.0,0.0,0.0))) * 0xffffffffu;
    r4.zw = ((r5.yz) != 0) ? (uint2(1065353216u,1065353216u)) : (uint2(3212836864u,3212836864u));
    r4.xy = asuint((asfloat(r4.zw)) * (asfloat(r4.xy)));
    r0.xy = ((r5.xx) != 0) ? (r0.yz) : (r4.xy);
    o1.xy = (asfloat(r0.xy)) * (float2(0.5,0.5)) + (float2(0.5,0.5));
    r0.x = asuint((abs(asfloat(r1.z))) + (abs(asfloat(r1.y))));
    r0.x = asuint((abs(asfloat(r1.x))) + (asfloat(r0.x)));
    r0.xyz = asuint((asfloat(r1.xyz)) / (asfloat(r0.xxx)));
    r1.xy = asuint((-abs(asfloat(r0.zy))) + (float2(1.0,1.0)));
    r4.xyz = (uint3)((asfloat(r0.xyz)) >= (float3(0.0,0.0,0.0))) * 0xffffffffu;
    r4.yz = ((r4.yz) != 0) ? (uint2(1065353216u,1065353216u)) : (uint2(3212836864u,3212836864u));
    r1.xy = asuint((asfloat(r1.xy)) * (asfloat(r4.yz)));
    r0.xy = ((r4.xx) != 0) ? (r0.yz) : (r1.xy);
    o4.xy = (asfloat(r0.xy)) * (float2(0.5,0.5)) + (float2(0.5,0.5));
    r0.x = asuint((abs(asfloat(r3.z))) + (abs(asfloat(r3.y))));
    r0.x = asuint((abs(asfloat(r3.x))) + (asfloat(r0.x)));
    r0.xyz = asuint((asfloat(r3.xyz)) / (asfloat(r0.xxx)));
    r1.xy = asuint((-abs(asfloat(r0.zy))) + (float2(1.0,1.0)));
    r3.xyz = (uint3)((asfloat(r0.xyz)) >= (float3(0.0,0.0,0.0))) * 0xffffffffu;
    r3.yz = ((r3.yz) != 0) ? (uint2(1065353216u,1065353216u)) : (uint2(3212836864u,3212836864u));
    r1.xy = asuint((asfloat(r1.xy)) * (asfloat(r3.yz)));
    r0.xy = ((r3.xx) != 0) ? (r0.yz) : (r1.xy);
    o4.zw = (asfloat(r0.xy)) * (float2(0.5,0.5)) + (float2(0.5,0.5));
    r0.x = asuint((asfloat(r1.w)) * (1.0));
    r0.x = (uint)(asfloat(r0.x));
    r0.x = asuint((asint(r0.x)) << ((8u) & 31));
    r0.x = asuint((float)(r0.x));
    o1.w = (asfloat(r0.x)) * (1.5259021893143654e-05);
    o1.z = asfloat((0u));
    o2.xyzw = (asfloat(r2.xyzw)) * (float4(1.0,1.0,1.0,0.5));
    o3.xyzw = asfloat((uint4(0u,0u,0u,0u)));
    o0.w = asfloat((r0.w));
    return;
}
