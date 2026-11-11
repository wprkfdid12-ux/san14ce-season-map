// SAN14 season map v26k27: the game's own shader d3e1f71e809bac74, translated instruction by instruction (unchanged).
// Used only by the test build's speed test (ways 1 and 2: drawn as the game draws it).

SamplerState s3 : register(s3);
SamplerState s4 : register(s4);
SamplerState s5 : register(s5);
SamplerState s6 : register(s6);
SamplerState s7 : register(s7);
SamplerState s8 : register(s8);
SamplerState s9 : register(s9);
SamplerState s10 : register(s10);
Texture2D<float4> t3 : register(t3);
Texture2D<float4> t4 : register(t4);
Texture2D<float4> t5 : register(t5);
Texture2D<float4> t6 : register(t6);
Texture2DArray<float4> t7 : register(t7);
Texture2DArray<float4> t8 : register(t8);
Texture2DArray<float4> t9 : register(t9);
Texture2D<float4> t10 : register(t10);
cbuffer cb0 : register(b0) { float4 cb0[1027]; }


void main(float4 v0 : SV_Position,
          float4 v1 : TEXCOORD0,
          float4 v2 : TEXCOORD1,
          float4 v3 : TEXCOORD2,
          float4 v4 : TEXCOORD3,
          float3 v5 : TEXCOORD4,
          float3 v6 : TEXCOORD5,
          float2 v7 : TEXCOORD6,
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
    uint4 r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0, r11 = 0, r12 = 0, r13 = 0, r14 = 0, r15 = 0, r16 = 0, r17 = 0, r18 = 0, r19 = 0, r20 = 0, r21 = 0, r22 = 0, r23 = 0, r24 = 0, r25 = 0, r26 = 0, r27 = 0, r28 = 0, r29 = 0, r30 = 0, r31 = 0;
    r0.x = asuint((t3.Sample(s3, (v7.xy))).x);
    r0.x = asuint((asfloat(r0.x)) * (255.0));
    r0.x = asuint((int)(asfloat(r0.x)));
    r0.x = (uint)((1.0) >= (cb0[517 + asint((asfloat(r0.x)))].x)) * 0xffffffffu;
    if (((r0.x) != 0)) discard;
    r0.x = asuint(max((abs(v6.z)), (abs(v6.y))));
    r0.x = asuint(max((asfloat(r0.x)), (abs(v6.x))));
    r0.y = asuint((cb0[6].x) + (9.999999747378752e-05));
    r0.y = (uint)((asfloat(r0.y)) >= (asfloat(r0.x))) * 0xffffffffu;
    r0.xz = (uint2)((asfloat(r0.xx)) == (abs(v6.xy))) * 0xffffffffu;
    r1.y = ((r0.z) != 0) ? (asuint(v4.z)) : (asuint(v4.y));
    r1.x = (asuint(v4.x));
    r0.xz = ((r0.xx) != 0) ? (asuint(v4.yz)) : (r1.xy);
    r1.xy = asuint((v1.zz) * (v3.xz));
    r1.xy = asuint(frac((asfloat(r1.xy))));
    r1.zw = asuint((-asfloat(r1.yx)) + (float2(1.0,1.0)));
    r0.w = asuint((v1.w) + (-1.0));
    r0.w = asuint((1.0) / (asfloat(r0.w)));
    r2.xy = asuint((asfloat(r0.ww)) * (asfloat(r1.xy)));
    r2.zw = asuint((-asfloat(r2.xy)));
    r2.xy = asuint((asfloat(r0.ww)) * (asfloat(r1.wz)));
    r3.xyzw = asuint((asfloat(r2.zwxw)) + (v7.xyxy));
    r4.xyz = asuint((t3.Sample(s3, (asfloat(r3.xy)))).xyz);
    r3.xyz = asuint((t3.Sample(s3, (asfloat(r3.zw)))).xyz);
    r2.xyzw = asuint((asfloat(r2.zyxy)) + (v7.xyxy));
    r5.xyz = asuint((t3.Sample(s3, (asfloat(r2.xy)))).xyz);
    r2.xyz = asuint((t3.Sample(s3, (asfloat(r2.zw)))).xyz);
    r6.xyz = asuint(ddx((asfloat(r0.xzx))));
    r7.xyz = asuint(ddy((asfloat(r0.xzx))));
    if (((r0.y) != 0)) {
        r8.xy = asuint((asfloat(r1.zw)) * (asfloat(r1.xy)));
        r0.yw = asuint((asfloat(r1.zy)) * (asfloat(r1.wx)));
        r2.w = asuint((asfloat(r3.z)) * (asfloat(r8.x)));
        r2.w = asuint((asfloat(r0.y)) * (asfloat(r4.z)) + (asfloat(r2.w)));
        r2.w = asuint((asfloat(r8.y)) * (asfloat(r5.z)) + (asfloat(r2.w)));
        r2.w = asuint((asfloat(r0.w)) * (asfloat(r2.z)) + (asfloat(r2.w)));
        r2.w = asuint((asfloat(r2.w)) + (-0.5));
        r8.zw = asuint((asfloat(r4.xy)) * (float2(255.0,255.0)));
        r8.zw = asuint((int2)(asfloat(r8.zw)));
        r9.z = asuint((float)(asint(cb0[7 + asint((asfloat(r8.z)))].x)));
        r10.x = (asuint(cb0[517 + asint((asfloat(r8.z)))].x));
        r10.y = (asuint(cb0[772 + asint((asfloat(r8.z)))].x));
        r11.xyz = asuint((asfloat(r6.zyz)) / (asfloat(r10.xyx)));
        r12.xyz = asuint((asfloat(r7.zyz)) / (asfloat(r10.xyx)));
        r9.xy = asuint((asfloat(r0.xz)) / (asfloat(r10.xy)));
        r3.w = (uint)((asint(r8.w)) >= (255)) * 0xffffffffu;
        if (((r3.w) != 0)) {
            r10.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r9.xyz)), (asfloat(r11.zy)), (asfloat(r12.zy)))).xyzw);
            r13.xyz = asuint((t8.SampleGrad(s8, (asfloat(r9.xyz)), (asfloat(r11.zy)), (asfloat(r12.zy)))).xyz);
            r14.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r9.xyz)), (asfloat(r11.zy)), (asfloat(r12.zy)))).xyzw);
        } else {
            r15.z = asuint((float)(asint(cb0[7 + asint((asfloat(r8.w)))].x)));
            r16.x = (asuint(cb0[517 + asint((asfloat(r8.w)))].x));
            r16.y = (asuint(cb0[772 + asint((asfloat(r8.w)))].x));
            r17.xyz = asuint((asfloat(r6.zyz)) / (asfloat(r16.xyx)));
            r18.xyz = asuint((asfloat(r7.zyz)) / (asfloat(r16.xyx)));
            r15.xy = asuint((asfloat(r0.xz)) / (asfloat(r16.xy)));
            r3.w = (uint)((0.9900000095367432) < (asfloat(r4.z))) * 0xffffffffu;
            if (((r3.w) != 0)) {
                r10.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r15.xyz)), (asfloat(r17.zy)), (asfloat(r18.zy)))).xyzw);
                r13.xyz = asuint((t8.SampleGrad(s8, (asfloat(r15.xyz)), (asfloat(r17.zy)), (asfloat(r18.zy)))).xyz);
                r14.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r15.xyz)), (asfloat(r17.zy)), (asfloat(r18.zy)))).xyzw);
            } else {
                r16.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r9.xyz)), (asfloat(r11.zy)), (asfloat(r12.zy)))).xyzw);
                r19.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r15.xyz)), (asfloat(r17.zy)), (asfloat(r18.zy)))).xyzw);
                r20.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r9.xyz)), (asfloat(r11.zy)), (asfloat(r12.zy)))).xyzw);
                r21.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r15.xyz)), (asfloat(r17.zy)), (asfloat(r18.zy)))).xyzw);
                r9.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r9.xyz)), (asfloat(r11.xy)), (asfloat(r12.xy)))).xyzw);
                r11.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r15.xyz)), (asfloat(r17.xy)), (asfloat(r18.xy)))).xyzw);
                r3.w = asuint((asfloat(r2.w)) * (2.0) + (asfloat(r21.w)));
                r4.w = asuint(max((asfloat(r3.w)), (asfloat(r20.w))));
                r5.w = asuint((1.0) + (-cb0[262 + asint((asfloat(r8.w)))].x));
                r4.w = asuint((asfloat(r4.w)) + (-asfloat(r5.w)));
                r5.w = asuint((-asfloat(r4.w)) + (asfloat(r20.w)));
                r5.w = asuint(max((asfloat(r5.w)), (0.0)));
                r3.w = asuint((asfloat(r3.w)) + (-asfloat(r4.w)));
                r3.w = asuint(max((asfloat(r3.w)), (0.0)));
                r4.w = asuint((asfloat(r3.w)) + (asfloat(r5.w)));
                r12.xyzw = asuint((asfloat(r3.wwww)) * (asfloat(r19.xyzw)));
                r12.xyzw = asuint((asfloat(r16.xyzw)) * (asfloat(r5.wwww)) + (asfloat(r12.xyzw)));
                r10.xyzw = asuint((asfloat(r12.xyzw)) / (asfloat(r4.wwww)));
                r11.xyzw = asuint((asfloat(r3.wwww)) * (asfloat(r11.xyzw)));
                r9.xyzw = asuint((asfloat(r9.xyzw)) * (asfloat(r5.wwww)) + (asfloat(r11.xyzw)));
                r14.xyzw = asuint((asfloat(r9.xyzw)) / (asfloat(r4.wwww)));
                r9.xyz = asuint((asfloat(r3.www)) * (asfloat(r21.xyz)));
                r9.xyz = asuint((asfloat(r20.xyz)) * (asfloat(r5.www)) + (asfloat(r9.xyz)));
                r13.xyz = asuint((asfloat(r9.xyz)) / (asfloat(r4.www)));
            }
        }
        r9.xyz = (uint3)((asfloat(r3.xyz)) == (asfloat(r4.xyz))) * 0xffffffffu;
        r3.w = (r9.y) & (r9.x);
        r3.w = (r9.z) & (r3.w);
        if (((r3.w) != 0)) {
            r9.xyzw = (r10.xyzw);
            r11.xyzw = (r14.xyzw);
            r12.xyz = (r13.xyz);
        } else {
            r8.zw = asuint((asfloat(r3.xy)) * (float2(255.0,255.0)));
            r8.zw = asuint((int2)(asfloat(r8.zw)));
            r15.z = asuint((float)(asint(cb0[7 + asint((asfloat(r8.z)))].x)));
            r16.x = (asuint(cb0[517 + asint((asfloat(r8.z)))].x));
            r16.y = (asuint(cb0[772 + asint((asfloat(r8.z)))].x));
            r17.xyz = asuint((asfloat(r6.zyz)) / (asfloat(r16.xyx)));
            r18.xyz = asuint((asfloat(r7.zyz)) / (asfloat(r16.xyx)));
            r15.xy = asuint((asfloat(r0.xz)) / (asfloat(r16.xy)));
            r3.w = (uint)((asint(r8.w)) >= (255)) * 0xffffffffu;
            if (((r3.w) != 0)) {
                r9.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r15.xyz)), (asfloat(r17.zy)), (asfloat(r18.zy)))).xyzw);
                r12.xyz = asuint((t8.SampleGrad(s8, (asfloat(r15.xyz)), (asfloat(r17.zy)), (asfloat(r18.zy)))).xyz);
                r11.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r15.xyz)), (asfloat(r17.zy)), (asfloat(r18.zy)))).xyzw);
            } else {
                r16.z = asuint((float)(asint(cb0[7 + asint((asfloat(r8.w)))].x)));
                r19.x = (asuint(cb0[517 + asint((asfloat(r8.w)))].x));
                r19.y = (asuint(cb0[772 + asint((asfloat(r8.w)))].x));
                r20.xyz = asuint((asfloat(r6.zyz)) / (asfloat(r19.xyx)));
                r21.xyz = asuint((asfloat(r7.zyz)) / (asfloat(r19.xyx)));
                r16.xy = asuint((asfloat(r0.xz)) / (asfloat(r19.xy)));
                r3.w = (uint)((0.9900000095367432) < (asfloat(r3.z))) * 0xffffffffu;
                if (((r3.w) != 0)) {
                    r9.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r16.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                    r12.xyz = asuint((t8.SampleGrad(s8, (asfloat(r16.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyz);
                    r11.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r16.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                } else {
                    r19.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r15.xyz)), (asfloat(r17.zy)), (asfloat(r18.zy)))).xyzw);
                    r22.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r16.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                    r23.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r15.xyz)), (asfloat(r17.zy)), (asfloat(r18.zy)))).xyzw);
                    r24.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r16.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                    r15.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r15.xyz)), (asfloat(r17.xy)), (asfloat(r18.xy)))).xyzw);
                    r16.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r16.xyz)), (asfloat(r20.xy)), (asfloat(r21.xy)))).xyzw);
                    r3.w = asuint((asfloat(r2.w)) * (2.0) + (asfloat(r24.w)));
                    r4.w = asuint(max((asfloat(r3.w)), (asfloat(r23.w))));
                    r5.w = asuint((1.0) + (-cb0[262 + asint((asfloat(r8.w)))].x));
                    r4.w = asuint((asfloat(r4.w)) + (-asfloat(r5.w)));
                    r5.w = asuint((-asfloat(r4.w)) + (asfloat(r23.w)));
                    r5.w = asuint(max((asfloat(r5.w)), (0.0)));
                    r3.w = asuint((asfloat(r3.w)) + (-asfloat(r4.w)));
                    r3.w = asuint(max((asfloat(r3.w)), (0.0)));
                    r4.w = asuint((asfloat(r3.w)) + (asfloat(r5.w)));
                    r17.xyzw = asuint((asfloat(r3.wwww)) * (asfloat(r22.xyzw)));
                    r17.xyzw = asuint((asfloat(r19.xyzw)) * (asfloat(r5.wwww)) + (asfloat(r17.xyzw)));
                    r9.xyzw = asuint((asfloat(r17.xyzw)) / (asfloat(r4.wwww)));
                    r16.xyzw = asuint((asfloat(r3.wwww)) * (asfloat(r16.xyzw)));
                    r15.xyzw = asuint((asfloat(r15.xyzw)) * (asfloat(r5.wwww)) + (asfloat(r16.xyzw)));
                    r11.xyzw = asuint((asfloat(r15.xyzw)) / (asfloat(r4.wwww)));
                    r15.xyz = asuint((asfloat(r3.www)) * (asfloat(r24.xyz)));
                    r15.xyz = asuint((asfloat(r23.xyz)) * (asfloat(r5.www)) + (asfloat(r15.xyz)));
                    r12.xyz = asuint((asfloat(r15.xyz)) / (asfloat(r4.www)));
                }
            }
        }
        r15.xyz = (uint3)((asfloat(r4.xyz)) == (asfloat(r5.xyz))) * 0xffffffffu;
        r3.w = (r15.y) & (r15.x);
        r3.w = (r15.z) & (r3.w);
        if (((r3.w) != 0)) {
            r15.xyzw = (r10.xyzw);
            r16.xyzw = (r14.xyzw);
            r17.xyz = (r13.xyz);
        } else {
            r18.xyz = (uint3)((asfloat(r3.xyz)) == (asfloat(r5.xyz))) * 0xffffffffu;
            r3.w = (r18.y) & (r18.x);
            r3.w = (r18.z) & (r3.w);
            if (((r3.w) != 0)) {
                r15.xyzw = (r9.xyzw);
                r16.xyzw = (r11.xyzw);
                r17.xyz = (r12.xyz);
            } else {
                r8.zw = asuint((asfloat(r5.xy)) * (float2(255.0,255.0)));
                r8.zw = asuint((int2)(asfloat(r8.zw)));
                r18.z = asuint((float)(asint(cb0[7 + asint((asfloat(r8.z)))].x)));
                r19.x = (asuint(cb0[517 + asint((asfloat(r8.z)))].x));
                r19.y = (asuint(cb0[772 + asint((asfloat(r8.z)))].x));
                r20.xyz = asuint((asfloat(r6.zyz)) / (asfloat(r19.xyx)));
                r21.xyz = asuint((asfloat(r7.zyz)) / (asfloat(r19.xyx)));
                r18.xy = asuint((asfloat(r0.xz)) / (asfloat(r19.xy)));
                r3.w = (uint)((asint(r8.w)) >= (255)) * 0xffffffffu;
                if (((r3.w) != 0)) {
                    r15.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                    r17.xyz = asuint((t8.SampleGrad(s8, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyz);
                    r16.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                } else {
                    r19.z = asuint((float)(asint(cb0[7 + asint((asfloat(r8.w)))].x)));
                    r22.x = (asuint(cb0[517 + asint((asfloat(r8.w)))].x));
                    r22.y = (asuint(cb0[772 + asint((asfloat(r8.w)))].x));
                    r23.xyz = asuint((asfloat(r6.zyz)) / (asfloat(r22.xyx)));
                    r24.xyz = asuint((asfloat(r7.zyz)) / (asfloat(r22.xyx)));
                    r19.xy = asuint((asfloat(r0.xz)) / (asfloat(r22.xy)));
                    r3.w = (uint)((0.9900000095367432) < (asfloat(r5.z))) * 0xffffffffu;
                    if (((r3.w) != 0)) {
                        r15.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r19.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                        r17.xyz = asuint((t8.SampleGrad(s8, (asfloat(r19.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyz);
                        r16.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r19.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                    } else {
                        r22.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                        r25.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r19.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                        r26.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                        r27.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r19.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                        r18.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r18.xyz)), (asfloat(r20.xy)), (asfloat(r21.xy)))).xyzw);
                        r19.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r19.xyz)), (asfloat(r23.xy)), (asfloat(r24.xy)))).xyzw);
                        r3.w = asuint((asfloat(r2.w)) * (2.0) + (asfloat(r27.w)));
                        r4.w = asuint(max((asfloat(r3.w)), (asfloat(r26.w))));
                        r5.w = asuint((1.0) + (-cb0[262 + asint((asfloat(r8.w)))].x));
                        r4.w = asuint((asfloat(r4.w)) + (-asfloat(r5.w)));
                        r5.w = asuint((-asfloat(r4.w)) + (asfloat(r26.w)));
                        r5.w = asuint(max((asfloat(r5.w)), (0.0)));
                        r3.w = asuint((asfloat(r3.w)) + (-asfloat(r4.w)));
                        r3.w = asuint(max((asfloat(r3.w)), (0.0)));
                        r4.w = asuint((asfloat(r3.w)) + (asfloat(r5.w)));
                        r20.xyzw = asuint((asfloat(r3.wwww)) * (asfloat(r25.xyzw)));
                        r20.xyzw = asuint((asfloat(r22.xyzw)) * (asfloat(r5.wwww)) + (asfloat(r20.xyzw)));
                        r15.xyzw = asuint((asfloat(r20.xyzw)) / (asfloat(r4.wwww)));
                        r19.xyzw = asuint((asfloat(r3.wwww)) * (asfloat(r19.xyzw)));
                        r18.xyzw = asuint((asfloat(r18.xyzw)) * (asfloat(r5.wwww)) + (asfloat(r19.xyzw)));
                        r16.xyzw = asuint((asfloat(r18.xyzw)) / (asfloat(r4.wwww)));
                        r18.xyz = asuint((asfloat(r3.www)) * (asfloat(r27.xyz)));
                        r18.xyz = asuint((asfloat(r26.xyz)) * (asfloat(r5.www)) + (asfloat(r18.xyz)));
                        r17.xyz = asuint((asfloat(r18.xyz)) / (asfloat(r4.www)));
                    }
                }
            }
        }
        r18.xyz = (uint3)((asfloat(r2.xyz)) == (asfloat(r4.xyz))) * 0xffffffffu;
        r3.w = (r18.y) & (r18.x);
        r3.w = (r18.z) & (r3.w);
        if (((r3.w) != 0)) {
            r18.xyzw = (r10.xyzw);
            r19.xyzw = (r14.xyzw);
            r20.xyz = (r13.xyz);
        } else {
            r21.xyz = (uint3)((asfloat(r2.xyz)) == (asfloat(r3.xyz))) * 0xffffffffu;
            r3.w = (r21.y) & (r21.x);
            r3.w = (r21.z) & (r3.w);
            if (((r3.w) != 0)) {
                r18.xyzw = (r9.xyzw);
                r19.xyzw = (r11.xyzw);
                r20.xyz = (r12.xyz);
            } else {
                r21.xyz = (uint3)((asfloat(r2.xyz)) == (asfloat(r5.xyz))) * 0xffffffffu;
                r3.w = (r21.y) & (r21.x);
                r3.w = (r21.z) & (r3.w);
                if (((r3.w) != 0)) {
                    r18.xyzw = (r15.xyzw);
                    r19.xyzw = (r16.xyzw);
                    r20.xyz = (r17.xyz);
                } else {
                    r8.zw = asuint((asfloat(r2.xy)) * (float2(255.0,255.0)));
                    r8.zw = asuint((int2)(asfloat(r8.zw)));
                    r21.z = asuint((float)(asint(cb0[7 + asint((asfloat(r8.z)))].x)));
                    r22.x = (asuint(cb0[517 + asint((asfloat(r8.z)))].x));
                    r22.y = (asuint(cb0[772 + asint((asfloat(r8.z)))].x));
                    r23.xyz = asuint((asfloat(r6.zyz)) / (asfloat(r22.xyx)));
                    r24.xyz = asuint((asfloat(r7.zyz)) / (asfloat(r22.xyx)));
                    r21.xy = asuint((asfloat(r0.xz)) / (asfloat(r22.xy)));
                    r3.w = (uint)((asint(r8.w)) >= (255)) * 0xffffffffu;
                    if (((r3.w) != 0)) {
                        r18.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r21.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                        r20.xyz = asuint((t8.SampleGrad(s8, (asfloat(r21.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyz);
                        r19.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r21.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                    } else {
                        r22.z = asuint((float)(asint(cb0[7 + asint((asfloat(r8.w)))].x)));
                        r25.x = (asuint(cb0[517 + asint((asfloat(r8.w)))].x));
                        r25.y = (asuint(cb0[772 + asint((asfloat(r8.w)))].x));
                        r6.xyz = asuint((asfloat(r6.xyz)) / (asfloat(r25.xyx)));
                        r7.xyz = asuint((asfloat(r7.xyz)) / (asfloat(r25.xyx)));
                        r22.xy = asuint((asfloat(r0.xz)) / (asfloat(r25.xy)));
                        r0.x = (uint)((0.9900000095367432) < (asfloat(r2.z))) * 0xffffffffu;
                        if (((r0.x) != 0)) {
                            r18.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r22.xyz)), (asfloat(r6.zy)), (asfloat(r7.zy)))).xyzw);
                            r20.xyz = asuint((t8.SampleGrad(s8, (asfloat(r22.xyz)), (asfloat(r6.zy)), (asfloat(r7.zy)))).xyz);
                            r19.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r22.xyz)), (asfloat(r6.zy)), (asfloat(r7.zy)))).xyzw);
                        } else {
                            r25.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r21.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                            r26.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r22.xyz)), (asfloat(r6.zy)), (asfloat(r7.zy)))).xyzw);
                            r27.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r21.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                            r28.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r22.xyz)), (asfloat(r6.zy)), (asfloat(r7.zy)))).xyzw);
                            r21.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r21.xyz)), (asfloat(r23.xy)), (asfloat(r24.xy)))).xyzw);
                            r6.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r22.xyz)), (asfloat(r6.xy)), (asfloat(r7.xy)))).xyzw);
                            r0.x = asuint((asfloat(r2.w)) * (2.0) + (asfloat(r28.w)));
                            r0.z = asuint(max((asfloat(r0.x)), (asfloat(r27.w))));
                            r2.w = asuint((1.0) + (-cb0[262 + asint((asfloat(r8.w)))].x));
                            r0.z = asuint((asfloat(r0.z)) + (-asfloat(r2.w)));
                            r2.w = asuint((-asfloat(r0.z)) + (asfloat(r27.w)));
                            r2.w = asuint(max((asfloat(r2.w)), (0.0)));
                            r0.x = asuint((-asfloat(r0.z)) + (asfloat(r0.x)));
                            r0.x = asuint(max((asfloat(r0.x)), (0.0)));
                            r0.z = asuint((asfloat(r0.x)) + (asfloat(r2.w)));
                            r7.xyzw = asuint((asfloat(r0.xxxx)) * (asfloat(r26.xyzw)));
                            r7.xyzw = asuint((asfloat(r25.xyzw)) * (asfloat(r2.wwww)) + (asfloat(r7.xyzw)));
                            r18.xyzw = asuint((asfloat(r7.xyzw)) / (asfloat(r0.zzzz)));
                            r6.xyzw = asuint((asfloat(r0.xxxx)) * (asfloat(r6.xyzw)));
                            r6.xyzw = asuint((asfloat(r21.xyzw)) * (asfloat(r2.wwww)) + (asfloat(r6.xyzw)));
                            r19.xyzw = asuint((asfloat(r6.xyzw)) / (asfloat(r0.zzzz)));
                            r6.xyz = asuint((asfloat(r0.xxx)) * (asfloat(r28.xyz)));
                            r6.xyz = asuint((asfloat(r27.xyz)) * (asfloat(r2.www)) + (asfloat(r6.xyz)));
                            r20.xyz = asuint((asfloat(r6.xyz)) / (asfloat(r0.zzz)));
                        }
                    }
                }
            }
        }
        r6.xyzw = asuint((asfloat(r8.xxxx)) * (asfloat(r9.xyzw)));
        r6.xyzw = asuint((asfloat(r0.yyyy)) * (asfloat(r10.xyzw)) + (asfloat(r6.xyzw)));
        r6.xyzw = asuint((asfloat(r15.xyzw)) * (asfloat(r8.yyyy)) + (asfloat(r6.xyzw)));
        r6.xyzw = asuint((asfloat(r18.xyzw)) * (asfloat(r0.wwww)) + (asfloat(r6.xyzw)));
        r7.xyz = asuint((asfloat(r8.xxx)) * (asfloat(r12.xyz)));
        r7.xyz = asuint((asfloat(r0.yyy)) * (asfloat(r13.xyz)) + (asfloat(r7.xyz)));
        r7.xyz = asuint((asfloat(r17.xyz)) * (asfloat(r8.yyy)) + (asfloat(r7.xyz)));
        r7.xyz = asuint((asfloat(r20.xyz)) * (asfloat(r0.www)) + (asfloat(r7.xyz)));
        r9.xyzw = asuint((asfloat(r8.xxxx)) * (asfloat(r11.xyzw)));
        r9.xyzw = asuint((asfloat(r0.yyyy)) * (asfloat(r14.xyzw)) + (asfloat(r9.xyzw)));
        r8.xyzw = asuint((asfloat(r16.xyzw)) * (asfloat(r8.yyyy)) + (asfloat(r9.xyzw)));
        r0.xyzw = asuint((asfloat(r19.xyzw)) * (asfloat(r0.wwww)) + (asfloat(r8.xyzw)));
    } else {
        r8.xyz = asuint((abs(v6.xyz)) + (-cb0[6].xxx));
        r8.xyz = asuint(max((asfloat(r8.xyz)), (float3(0.0,0.0,0.0))));
        r2.w = asuint(dot((asfloat(r8.xyz)), (asfloat(r8.xyz))));
        r2.w = asuint(rsqrt((asfloat(r2.w))));
        r9.xyz = asuint((asfloat(r2.www)) * (asfloat(r8.xyz)));
        r3.w = asuint((asfloat(r9.y)) + (asfloat(r9.x)));
        r2.w = asuint((asfloat(r8.z)) * (asfloat(r2.w)) + (asfloat(r3.w)));
        r9.xyz = asuint((asfloat(r9.xyz)) / (asfloat(r2.www)));
        r8.xyz = (uint3)((float3(0.0,0.0,0.0)) < (asfloat(r8.xyz))) * 0xffffffffu;
        if (((r8.x) != 0)) {
            r2.w = asuint((asfloat(r1.z)) * (asfloat(r1.w)));
            r8.xw = asuint((asfloat(r1.zw)) * (asfloat(r1.xy)));
            r3.w = asuint((asfloat(r1.y)) * (asfloat(r1.x)));
            r4.w = asuint((asfloat(r3.z)) * (asfloat(r8.x)));
            r4.w = asuint((asfloat(r2.w)) * (asfloat(r4.z)) + (asfloat(r4.w)));
            r4.w = asuint((asfloat(r8.w)) * (asfloat(r5.z)) + (asfloat(r4.w)));
            r4.w = asuint((asfloat(r3.w)) * (asfloat(r2.z)) + (asfloat(r4.w)));
            r4.w = asuint((asfloat(r4.w)) + (-0.5));
            r10.xyz = asuint(ddx((v4.yzy)));
            r11.xyz = asuint(ddy((v4.yzy)));
            r12.xy = asuint((asfloat(r4.xy)) * (float2(255.0,255.0)));
            r12.xy = asuint((int2)(asfloat(r12.xy)));
            r13.z = asuint((float)(asint(cb0[7 + asint((asfloat(r12.x)))].x)));
            r14.x = (asuint(cb0[517 + asint((asfloat(r12.x)))].x));
            r14.y = (asuint(cb0[772 + asint((asfloat(r12.x)))].x));
            r12.xzw = asuint((asfloat(r10.zyz)) / (asfloat(r14.xyx)));
            r15.xyz = asuint((asfloat(r11.zyz)) / (asfloat(r14.xyx)));
            r13.xy = asuint((v4.yz) / (asfloat(r14.xy)));
            r5.w = (uint)((asint(r12.y)) >= (255)) * 0xffffffffu;
            if (((r5.w) != 0)) {
                r14.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r13.xyz)), (asfloat(r12.wz)), (asfloat(r15.zy)))).xyzw);
                r16.xyz = asuint((t8.SampleGrad(s8, (asfloat(r13.xyz)), (asfloat(r12.wz)), (asfloat(r15.zy)))).xyz);
                r17.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r13.xyz)), (asfloat(r12.wz)), (asfloat(r15.zy)))).xyzw);
            } else {
                r18.z = asuint((float)(asint(cb0[7 + asint((asfloat(r12.y)))].x)));
                r19.x = (asuint(cb0[517 + asint((asfloat(r12.y)))].x));
                r19.y = (asuint(cb0[772 + asint((asfloat(r12.y)))].x));
                r20.xyz = asuint((asfloat(r10.zyz)) / (asfloat(r19.xyx)));
                r21.xyz = asuint((asfloat(r11.zyz)) / (asfloat(r19.xyx)));
                r18.xy = asuint((v4.yz) / (asfloat(r19.xy)));
                r5.w = (uint)((0.9900000095367432) < (asfloat(r4.z))) * 0xffffffffu;
                if (((r5.w) != 0)) {
                    r14.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                    r16.xyz = asuint((t8.SampleGrad(s8, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyz);
                    r17.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                } else {
                    r19.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r13.xyz)), (asfloat(r12.wz)), (asfloat(r15.zy)))).xyzw);
                    r22.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                    r23.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r13.xyz)), (asfloat(r12.wz)), (asfloat(r15.zy)))).xyzw);
                    r24.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                    r13.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r13.xyz)), (asfloat(r12.xz)), (asfloat(r15.xy)))).xyzw);
                    r15.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r18.xyz)), (asfloat(r20.xy)), (asfloat(r21.xy)))).xyzw);
                    r5.w = asuint((asfloat(r4.w)) * (2.0) + (asfloat(r24.w)));
                    r7.w = asuint(max((asfloat(r5.w)), (asfloat(r23.w))));
                    r9.w = asuint((1.0) + (-cb0[262 + asint((asfloat(r12.y)))].x));
                    r7.w = asuint((asfloat(r7.w)) + (-asfloat(r9.w)));
                    r9.w = asuint((-asfloat(r7.w)) + (asfloat(r23.w)));
                    r9.w = asuint(max((asfloat(r9.w)), (0.0)));
                    r5.w = asuint((asfloat(r5.w)) + (-asfloat(r7.w)));
                    r5.w = asuint(max((asfloat(r5.w)), (0.0)));
                    r7.w = asuint((asfloat(r5.w)) + (asfloat(r9.w)));
                    r12.xyzw = asuint((asfloat(r5.wwww)) * (asfloat(r22.xyzw)));
                    r12.xyzw = asuint((asfloat(r19.xyzw)) * (asfloat(r9.wwww)) + (asfloat(r12.xyzw)));
                    r14.xyzw = asuint((asfloat(r12.xyzw)) / (asfloat(r7.wwww)));
                    r12.xyzw = asuint((asfloat(r5.wwww)) * (asfloat(r15.xyzw)));
                    r12.xyzw = asuint((asfloat(r13.xyzw)) * (asfloat(r9.wwww)) + (asfloat(r12.xyzw)));
                    r17.xyzw = asuint((asfloat(r12.xyzw)) / (asfloat(r7.wwww)));
                    r12.xyz = asuint((asfloat(r5.www)) * (asfloat(r24.xyz)));
                    r12.xyz = asuint((asfloat(r23.xyz)) * (asfloat(r9.www)) + (asfloat(r12.xyz)));
                    r16.xyz = asuint((asfloat(r12.xyz)) / (asfloat(r7.www)));
                }
            }
            r12.xyz = (uint3)((asfloat(r3.xyz)) == (asfloat(r4.xyz))) * 0xffffffffu;
            r5.w = (r12.y) & (r12.x);
            r5.w = (r12.z) & (r5.w);
            if (((r5.w) != 0)) {
                r12.xyzw = (r14.xyzw);
                r13.xyzw = (r17.xyzw);
                r15.xyz = (r16.xyz);
            } else {
                r18.xy = asuint((asfloat(r3.xy)) * (float2(255.0,255.0)));
                r18.xy = asuint((int2)(asfloat(r18.xy)));
                r19.z = asuint((float)(asint(cb0[7 + asint((asfloat(r18.x)))].x)));
                r20.x = (asuint(cb0[517 + asint((asfloat(r18.x)))].x));
                r20.y = (asuint(cb0[772 + asint((asfloat(r18.x)))].x));
                r18.xzw = asuint((asfloat(r10.zyz)) / (asfloat(r20.xyx)));
                r21.xyz = asuint((asfloat(r11.zyz)) / (asfloat(r20.xyx)));
                r19.xy = asuint((v4.yz) / (asfloat(r20.xy)));
                r5.w = (uint)((asint(r18.y)) >= (255)) * 0xffffffffu;
                if (((r5.w) != 0)) {
                    r12.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r19.xyz)), (asfloat(r18.wz)), (asfloat(r21.zy)))).xyzw);
                    r15.xyz = asuint((t8.SampleGrad(s8, (asfloat(r19.xyz)), (asfloat(r18.wz)), (asfloat(r21.zy)))).xyz);
                    r13.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r19.xyz)), (asfloat(r18.wz)), (asfloat(r21.zy)))).xyzw);
                } else {
                    r20.z = asuint((float)(asint(cb0[7 + asint((asfloat(r18.y)))].x)));
                    r22.x = (asuint(cb0[517 + asint((asfloat(r18.y)))].x));
                    r22.y = (asuint(cb0[772 + asint((asfloat(r18.y)))].x));
                    r23.xyz = asuint((asfloat(r10.zyz)) / (asfloat(r22.xyx)));
                    r24.xyz = asuint((asfloat(r11.zyz)) / (asfloat(r22.xyx)));
                    r20.xy = asuint((v4.yz) / (asfloat(r22.xy)));
                    r5.w = (uint)((0.9900000095367432) < (asfloat(r3.z))) * 0xffffffffu;
                    if (((r5.w) != 0)) {
                        r12.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r20.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                        r15.xyz = asuint((t8.SampleGrad(s8, (asfloat(r20.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyz);
                        r13.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r20.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                    } else {
                        r22.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r19.xyz)), (asfloat(r18.wz)), (asfloat(r21.zy)))).xyzw);
                        r25.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r20.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                        r26.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r19.xyz)), (asfloat(r18.wz)), (asfloat(r21.zy)))).xyzw);
                        r27.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r20.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                        r19.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r19.xyz)), (asfloat(r18.xz)), (asfloat(r21.xy)))).xyzw);
                        r20.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r20.xyz)), (asfloat(r23.xy)), (asfloat(r24.xy)))).xyzw);
                        r5.w = asuint((asfloat(r4.w)) * (2.0) + (asfloat(r27.w)));
                        r7.w = asuint(max((asfloat(r5.w)), (asfloat(r26.w))));
                        r9.w = asuint((1.0) + (-cb0[262 + asint((asfloat(r18.y)))].x));
                        r7.w = asuint((asfloat(r7.w)) + (-asfloat(r9.w)));
                        r9.w = asuint((-asfloat(r7.w)) + (asfloat(r26.w)));
                        r9.w = asuint(max((asfloat(r9.w)), (0.0)));
                        r5.w = asuint((asfloat(r5.w)) + (-asfloat(r7.w)));
                        r5.w = asuint(max((asfloat(r5.w)), (0.0)));
                        r7.w = asuint((asfloat(r5.w)) + (asfloat(r9.w)));
                        r18.xyzw = asuint((asfloat(r5.wwww)) * (asfloat(r25.xyzw)));
                        r18.xyzw = asuint((asfloat(r22.xyzw)) * (asfloat(r9.wwww)) + (asfloat(r18.xyzw)));
                        r12.xyzw = asuint((asfloat(r18.xyzw)) / (asfloat(r7.wwww)));
                        r18.xyzw = asuint((asfloat(r5.wwww)) * (asfloat(r20.xyzw)));
                        r18.xyzw = asuint((asfloat(r19.xyzw)) * (asfloat(r9.wwww)) + (asfloat(r18.xyzw)));
                        r13.xyzw = asuint((asfloat(r18.xyzw)) / (asfloat(r7.wwww)));
                        r18.xyz = asuint((asfloat(r5.www)) * (asfloat(r27.xyz)));
                        r18.xyz = asuint((asfloat(r26.xyz)) * (asfloat(r9.www)) + (asfloat(r18.xyz)));
                        r15.xyz = asuint((asfloat(r18.xyz)) / (asfloat(r7.www)));
                    }
                }
            }
            r18.xyz = (uint3)((asfloat(r4.xyz)) == (asfloat(r5.xyz))) * 0xffffffffu;
            r5.w = (r18.y) & (r18.x);
            r5.w = (r18.z) & (r5.w);
            if (((r5.w) != 0)) {
                r18.xyzw = (r14.xyzw);
                r19.xyzw = (r17.xyzw);
                r20.xyz = (r16.xyz);
            } else {
                r21.xyz = (uint3)((asfloat(r3.xyz)) == (asfloat(r5.xyz))) * 0xffffffffu;
                r5.w = (r21.y) & (r21.x);
                r5.w = (r21.z) & (r5.w);
                if (((r5.w) != 0)) {
                    r18.xyzw = (r12.xyzw);
                    r19.xyzw = (r13.xyzw);
                    r20.xyz = (r15.xyz);
                } else {
                    r21.xy = asuint((asfloat(r5.xy)) * (float2(255.0,255.0)));
                    r21.xy = asuint((int2)(asfloat(r21.xy)));
                    r22.z = asuint((float)(asint(cb0[7 + asint((asfloat(r21.x)))].x)));
                    r23.x = (asuint(cb0[517 + asint((asfloat(r21.x)))].x));
                    r23.y = (asuint(cb0[772 + asint((asfloat(r21.x)))].x));
                    r21.xzw = asuint((asfloat(r10.zyz)) / (asfloat(r23.xyx)));
                    r24.xyz = asuint((asfloat(r11.zyz)) / (asfloat(r23.xyx)));
                    r22.xy = asuint((v4.yz) / (asfloat(r23.xy)));
                    r5.w = (uint)((asint(r21.y)) >= (255)) * 0xffffffffu;
                    if (((r5.w) != 0)) {
                        r18.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r22.xyz)), (asfloat(r21.wz)), (asfloat(r24.zy)))).xyzw);
                        r20.xyz = asuint((t8.SampleGrad(s8, (asfloat(r22.xyz)), (asfloat(r21.wz)), (asfloat(r24.zy)))).xyz);
                        r19.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r22.xyz)), (asfloat(r21.wz)), (asfloat(r24.zy)))).xyzw);
                    } else {
                        r23.z = asuint((float)(asint(cb0[7 + asint((asfloat(r21.y)))].x)));
                        r25.x = (asuint(cb0[517 + asint((asfloat(r21.y)))].x));
                        r25.y = (asuint(cb0[772 + asint((asfloat(r21.y)))].x));
                        r26.xyz = asuint((asfloat(r10.zyz)) / (asfloat(r25.xyx)));
                        r27.xyz = asuint((asfloat(r11.zyz)) / (asfloat(r25.xyx)));
                        r23.xy = asuint((v4.yz) / (asfloat(r25.xy)));
                        r5.w = (uint)((0.9900000095367432) < (asfloat(r5.z))) * 0xffffffffu;
                        if (((r5.w) != 0)) {
                            r18.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r23.xyz)), (asfloat(r26.zy)), (asfloat(r27.zy)))).xyzw);
                            r20.xyz = asuint((t8.SampleGrad(s8, (asfloat(r23.xyz)), (asfloat(r26.zy)), (asfloat(r27.zy)))).xyz);
                            r19.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r23.xyz)), (asfloat(r26.zy)), (asfloat(r27.zy)))).xyzw);
                        } else {
                            r25.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r22.xyz)), (asfloat(r21.wz)), (asfloat(r24.zy)))).xyzw);
                            r28.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r23.xyz)), (asfloat(r26.zy)), (asfloat(r27.zy)))).xyzw);
                            r29.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r22.xyz)), (asfloat(r21.wz)), (asfloat(r24.zy)))).xyzw);
                            r30.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r23.xyz)), (asfloat(r26.zy)), (asfloat(r27.zy)))).xyzw);
                            r22.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r22.xyz)), (asfloat(r21.xz)), (asfloat(r24.xy)))).xyzw);
                            r23.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r23.xyz)), (asfloat(r26.xy)), (asfloat(r27.xy)))).xyzw);
                            r5.w = asuint((asfloat(r4.w)) * (2.0) + (asfloat(r30.w)));
                            r7.w = asuint(max((asfloat(r5.w)), (asfloat(r29.w))));
                            r9.w = asuint((1.0) + (-cb0[262 + asint((asfloat(r21.y)))].x));
                            r7.w = asuint((asfloat(r7.w)) + (-asfloat(r9.w)));
                            r9.w = asuint((-asfloat(r7.w)) + (asfloat(r29.w)));
                            r9.w = asuint(max((asfloat(r9.w)), (0.0)));
                            r5.w = asuint((asfloat(r5.w)) + (-asfloat(r7.w)));
                            r5.w = asuint(max((asfloat(r5.w)), (0.0)));
                            r7.w = asuint((asfloat(r5.w)) + (asfloat(r9.w)));
                            r21.xyzw = asuint((asfloat(r5.wwww)) * (asfloat(r28.xyzw)));
                            r21.xyzw = asuint((asfloat(r25.xyzw)) * (asfloat(r9.wwww)) + (asfloat(r21.xyzw)));
                            r18.xyzw = asuint((asfloat(r21.xyzw)) / (asfloat(r7.wwww)));
                            r21.xyzw = asuint((asfloat(r5.wwww)) * (asfloat(r23.xyzw)));
                            r21.xyzw = asuint((asfloat(r22.xyzw)) * (asfloat(r9.wwww)) + (asfloat(r21.xyzw)));
                            r19.xyzw = asuint((asfloat(r21.xyzw)) / (asfloat(r7.wwww)));
                            r21.xyz = asuint((asfloat(r5.www)) * (asfloat(r30.xyz)));
                            r21.xyz = asuint((asfloat(r29.xyz)) * (asfloat(r9.www)) + (asfloat(r21.xyz)));
                            r20.xyz = asuint((asfloat(r21.xyz)) / (asfloat(r7.www)));
                        }
                    }
                }
            }
            r21.xyz = (uint3)((asfloat(r2.xyz)) == (asfloat(r4.xyz))) * 0xffffffffu;
            r5.w = (r21.y) & (r21.x);
            r5.w = (r21.z) & (r5.w);
            if (((r5.w) != 0)) {
                r21.xyzw = (r14.xyzw);
                r22.xyzw = (r17.xyzw);
                r23.xyz = (r16.xyz);
            } else {
                r24.xyz = (uint3)((asfloat(r2.xyz)) == (asfloat(r3.xyz))) * 0xffffffffu;
                r5.w = (r24.y) & (r24.x);
                r5.w = (r24.z) & (r5.w);
                if (((r5.w) != 0)) {
                    r21.xyzw = (r12.xyzw);
                    r22.xyzw = (r13.xyzw);
                    r23.xyz = (r15.xyz);
                } else {
                    r24.xyz = (uint3)((asfloat(r2.xyz)) == (asfloat(r5.xyz))) * 0xffffffffu;
                    r5.w = (r24.y) & (r24.x);
                    r5.w = (r24.z) & (r5.w);
                    if (((r5.w) != 0)) {
                        r21.xyzw = (r18.xyzw);
                        r22.xyzw = (r19.xyzw);
                        r23.xyz = (r20.xyz);
                    } else {
                        r24.xy = asuint((asfloat(r2.xy)) * (float2(255.0,255.0)));
                        r24.xy = asuint((int2)(asfloat(r24.xy)));
                        r25.z = asuint((float)(asint(cb0[7 + asint((asfloat(r24.x)))].x)));
                        r26.x = (asuint(cb0[517 + asint((asfloat(r24.x)))].x));
                        r26.y = (asuint(cb0[772 + asint((asfloat(r24.x)))].x));
                        r24.xzw = asuint((asfloat(r10.zyz)) / (asfloat(r26.xyx)));
                        r27.xyz = asuint((asfloat(r11.zyz)) / (asfloat(r26.xyx)));
                        r25.xy = asuint((v4.yz) / (asfloat(r26.xy)));
                        r5.w = (uint)((asint(r24.y)) >= (255)) * 0xffffffffu;
                        if (((r5.w) != 0)) {
                            r21.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r25.xyz)), (asfloat(r24.wz)), (asfloat(r27.zy)))).xyzw);
                            r23.xyz = asuint((t8.SampleGrad(s8, (asfloat(r25.xyz)), (asfloat(r24.wz)), (asfloat(r27.zy)))).xyz);
                            r22.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r25.xyz)), (asfloat(r24.wz)), (asfloat(r27.zy)))).xyzw);
                        } else {
                            r26.z = asuint((float)(asint(cb0[7 + asint((asfloat(r24.y)))].x)));
                            r28.x = (asuint(cb0[517 + asint((asfloat(r24.y)))].x));
                            r28.y = (asuint(cb0[772 + asint((asfloat(r24.y)))].x));
                            r10.xyz = asuint((asfloat(r10.xyz)) / (asfloat(r28.xyx)));
                            r11.xyz = asuint((asfloat(r11.xyz)) / (asfloat(r28.xyx)));
                            r26.xy = asuint((v4.yz) / (asfloat(r28.xy)));
                            r5.w = (uint)((0.9900000095367432) < (asfloat(r2.z))) * 0xffffffffu;
                            if (((r5.w) != 0)) {
                                r21.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r26.xyz)), (asfloat(r10.zy)), (asfloat(r11.zy)))).xyzw);
                                r23.xyz = asuint((t8.SampleGrad(s8, (asfloat(r26.xyz)), (asfloat(r10.zy)), (asfloat(r11.zy)))).xyz);
                                r22.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r26.xyz)), (asfloat(r10.zy)), (asfloat(r11.zy)))).xyzw);
                            } else {
                                r28.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r25.xyz)), (asfloat(r24.wz)), (asfloat(r27.zy)))).xyzw);
                                r29.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r26.xyz)), (asfloat(r10.zy)), (asfloat(r11.zy)))).xyzw);
                                r30.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r25.xyz)), (asfloat(r24.wz)), (asfloat(r27.zy)))).xyzw);
                                r31.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r26.xyz)), (asfloat(r10.zy)), (asfloat(r11.zy)))).xyzw);
                                r25.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r25.xyz)), (asfloat(r24.xz)), (asfloat(r27.xy)))).xyzw);
                                r10.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r26.xyz)), (asfloat(r10.xy)), (asfloat(r11.xy)))).xyzw);
                                r4.w = asuint((asfloat(r4.w)) * (2.0) + (asfloat(r31.w)));
                                r5.w = asuint(max((asfloat(r4.w)), (asfloat(r30.w))));
                                r7.w = asuint((1.0) + (-cb0[262 + asint((asfloat(r24.y)))].x));
                                r5.w = asuint((asfloat(r5.w)) + (-asfloat(r7.w)));
                                r7.w = asuint((-asfloat(r5.w)) + (asfloat(r30.w)));
                                r7.w = asuint(max((asfloat(r7.w)), (0.0)));
                                r4.w = asuint((asfloat(r4.w)) + (-asfloat(r5.w)));
                                r4.w = asuint(max((asfloat(r4.w)), (0.0)));
                                r5.w = asuint((asfloat(r4.w)) + (asfloat(r7.w)));
                                r11.xyzw = asuint((asfloat(r4.wwww)) * (asfloat(r29.xyzw)));
                                r11.xyzw = asuint((asfloat(r28.xyzw)) * (asfloat(r7.wwww)) + (asfloat(r11.xyzw)));
                                r21.xyzw = asuint((asfloat(r11.xyzw)) / (asfloat(r5.wwww)));
                                r10.xyzw = asuint((asfloat(r4.wwww)) * (asfloat(r10.xyzw)));
                                r10.xyzw = asuint((asfloat(r25.xyzw)) * (asfloat(r7.wwww)) + (asfloat(r10.xyzw)));
                                r22.xyzw = asuint((asfloat(r10.xyzw)) / (asfloat(r5.wwww)));
                                r10.xyz = asuint((asfloat(r4.www)) * (asfloat(r31.xyz)));
                                r10.xyz = asuint((asfloat(r30.xyz)) * (asfloat(r7.www)) + (asfloat(r10.xyz)));
                                r23.xyz = asuint((asfloat(r10.xyz)) / (asfloat(r5.www)));
                            }
                        }
                    }
                }
            }
            r10.xyzw = asuint((asfloat(r8.xxxx)) * (asfloat(r12.xyzw)));
            r10.xyzw = asuint((asfloat(r2.wwww)) * (asfloat(r14.xyzw)) + (asfloat(r10.xyzw)));
            r10.xyzw = asuint((asfloat(r18.xyzw)) * (asfloat(r8.wwww)) + (asfloat(r10.xyzw)));
            r10.xyzw = asuint((asfloat(r21.xyzw)) * (asfloat(r3.wwww)) + (asfloat(r10.xyzw)));
            r11.xyz = asuint((asfloat(r8.xxx)) * (asfloat(r15.xyz)));
            r11.xyz = asuint((asfloat(r2.www)) * (asfloat(r16.xyz)) + (asfloat(r11.xyz)));
            r11.xyz = asuint((asfloat(r20.xyz)) * (asfloat(r8.www)) + (asfloat(r11.xyz)));
            r11.xyz = asuint((asfloat(r23.xyz)) * (asfloat(r3.www)) + (asfloat(r11.xyz)));
            r12.xyzw = asuint((asfloat(r8.xxxx)) * (asfloat(r13.xyzw)));
            r12.xyzw = asuint((asfloat(r2.wwww)) * (asfloat(r17.xyzw)) + (asfloat(r12.xyzw)));
            r12.xyzw = asuint((asfloat(r19.xyzw)) * (asfloat(r8.wwww)) + (asfloat(r12.xyzw)));
            r12.xyzw = asuint((asfloat(r22.xyzw)) * (asfloat(r3.wwww)) + (asfloat(r12.xyzw)));
            r6.xyzw = asuint((asfloat(r9.xxxx)) * (asfloat(r10.xyzw)));
            r7.xyz = asuint((asfloat(r9.xxx)) * (asfloat(r11.xyz)));
            r0.xyzw = asuint((asfloat(r9.xxxx)) * (asfloat(r12.xyzw)));
        } else {
            r0.xyzw = (uint4(0u,0u,0u,0u));
            r7.xyz = (uint3(0u,0u,0u));
            r6.xyzw = (uint4(0u,0u,0u,0u));
        }
        if (((r8.y) != 0)) {
            r2.w = asuint((asfloat(r1.z)) * (asfloat(r1.w)));
            r8.xy = asuint((asfloat(r1.zw)) * (asfloat(r1.xy)));
            r3.w = asuint((asfloat(r1.y)) * (asfloat(r1.x)));
            r4.w = asuint((asfloat(r3.z)) * (asfloat(r8.x)));
            r4.w = asuint((asfloat(r2.w)) * (asfloat(r4.z)) + (asfloat(r4.w)));
            r4.w = asuint((asfloat(r8.y)) * (asfloat(r5.z)) + (asfloat(r4.w)));
            r4.w = asuint((asfloat(r3.w)) * (asfloat(r2.z)) + (asfloat(r4.w)));
            r4.w = asuint((asfloat(r4.w)) + (-0.5));
            r10.xyz = asuint(ddx((v4.xzx)));
            r11.xyz = asuint(ddy((v4.xzx)));
            r9.xw = asuint((asfloat(r4.xy)) * (float2(255.0,255.0)));
            r9.xw = asuint((int2)(asfloat(r9.xw)));
            r12.z = asuint((float)(asint(cb0[7 + asint((asfloat(r9.x)))].x)));
            r13.x = (asuint(cb0[517 + asint((asfloat(r9.x)))].x));
            r13.y = (asuint(cb0[772 + asint((asfloat(r9.x)))].x));
            r14.xyz = asuint((asfloat(r10.zyz)) / (asfloat(r13.xyx)));
            r15.xyz = asuint((asfloat(r11.zyz)) / (asfloat(r13.xyx)));
            r12.xy = asuint((v4.xz) / (asfloat(r13.xy)));
            r5.w = (uint)((asint(r9.w)) >= (255)) * 0xffffffffu;
            if (((r5.w) != 0)) {
                r13.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r12.xyz)), (asfloat(r14.zy)), (asfloat(r15.zy)))).xyzw);
                r16.xyz = asuint((t8.SampleGrad(s8, (asfloat(r12.xyz)), (asfloat(r14.zy)), (asfloat(r15.zy)))).xyz);
                r17.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r12.xyz)), (asfloat(r14.zy)), (asfloat(r15.zy)))).xyzw);
            } else {
                r18.z = asuint((float)(asint(cb0[7 + asint((asfloat(r9.w)))].x)));
                r19.x = (asuint(cb0[517 + asint((asfloat(r9.w)))].x));
                r19.y = (asuint(cb0[772 + asint((asfloat(r9.w)))].x));
                r20.xyz = asuint((asfloat(r10.zyz)) / (asfloat(r19.xyx)));
                r21.xyz = asuint((asfloat(r11.zyz)) / (asfloat(r19.xyx)));
                r18.xy = asuint((v4.xz) / (asfloat(r19.xy)));
                r5.w = (uint)((0.9900000095367432) < (asfloat(r4.z))) * 0xffffffffu;
                if (((r5.w) != 0)) {
                    r13.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                    r16.xyz = asuint((t8.SampleGrad(s8, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyz);
                    r17.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                } else {
                    r19.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r12.xyz)), (asfloat(r14.zy)), (asfloat(r15.zy)))).xyzw);
                    r22.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                    r23.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r12.xyz)), (asfloat(r14.zy)), (asfloat(r15.zy)))).xyzw);
                    r24.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                    r12.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r12.xyz)), (asfloat(r14.xy)), (asfloat(r15.xy)))).xyzw);
                    r14.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r18.xyz)), (asfloat(r20.xy)), (asfloat(r21.xy)))).xyzw);
                    r5.w = asuint((asfloat(r4.w)) * (2.0) + (asfloat(r24.w)));
                    r7.w = asuint(max((asfloat(r5.w)), (asfloat(r23.w))));
                    r8.w = asuint((1.0) + (-cb0[262 + asint((asfloat(r9.w)))].x));
                    r7.w = asuint((asfloat(r7.w)) + (-asfloat(r8.w)));
                    r8.w = asuint((-asfloat(r7.w)) + (asfloat(r23.w)));
                    r8.w = asuint(max((asfloat(r8.w)), (0.0)));
                    r5.w = asuint((asfloat(r5.w)) + (-asfloat(r7.w)));
                    r5.w = asuint(max((asfloat(r5.w)), (0.0)));
                    r7.w = asuint((asfloat(r5.w)) + (asfloat(r8.w)));
                    r15.xyzw = asuint((asfloat(r5.wwww)) * (asfloat(r22.xyzw)));
                    r15.xyzw = asuint((asfloat(r19.xyzw)) * (asfloat(r8.wwww)) + (asfloat(r15.xyzw)));
                    r13.xyzw = asuint((asfloat(r15.xyzw)) / (asfloat(r7.wwww)));
                    r14.xyzw = asuint((asfloat(r5.wwww)) * (asfloat(r14.xyzw)));
                    r12.xyzw = asuint((asfloat(r12.xyzw)) * (asfloat(r8.wwww)) + (asfloat(r14.xyzw)));
                    r17.xyzw = asuint((asfloat(r12.xyzw)) / (asfloat(r7.wwww)));
                    r12.xyz = asuint((asfloat(r5.www)) * (asfloat(r24.xyz)));
                    r12.xyz = asuint((asfloat(r23.xyz)) * (asfloat(r8.www)) + (asfloat(r12.xyz)));
                    r16.xyz = asuint((asfloat(r12.xyz)) / (asfloat(r7.www)));
                }
            }
            r12.xyz = (uint3)((asfloat(r3.xyz)) == (asfloat(r4.xyz))) * 0xffffffffu;
            r5.w = (r12.y) & (r12.x);
            r5.w = (r12.z) & (r5.w);
            if (((r5.w) != 0)) {
                r12.xyzw = (r13.xyzw);
                r14.xyzw = (r17.xyzw);
                r15.xyz = (r16.xyz);
            } else {
                r9.xw = asuint((asfloat(r3.xy)) * (float2(255.0,255.0)));
                r9.xw = asuint((int2)(asfloat(r9.xw)));
                r18.z = asuint((float)(asint(cb0[7 + asint((asfloat(r9.x)))].x)));
                r19.x = (asuint(cb0[517 + asint((asfloat(r9.x)))].x));
                r19.y = (asuint(cb0[772 + asint((asfloat(r9.x)))].x));
                r20.xyz = asuint((asfloat(r10.zyz)) / (asfloat(r19.xyx)));
                r21.xyz = asuint((asfloat(r11.zyz)) / (asfloat(r19.xyx)));
                r18.xy = asuint((v4.xz) / (asfloat(r19.xy)));
                r5.w = (uint)((asint(r9.w)) >= (255)) * 0xffffffffu;
                if (((r5.w) != 0)) {
                    r12.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                    r15.xyz = asuint((t8.SampleGrad(s8, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyz);
                    r14.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                } else {
                    r19.z = asuint((float)(asint(cb0[7 + asint((asfloat(r9.w)))].x)));
                    r22.x = (asuint(cb0[517 + asint((asfloat(r9.w)))].x));
                    r22.y = (asuint(cb0[772 + asint((asfloat(r9.w)))].x));
                    r23.xyz = asuint((asfloat(r10.zyz)) / (asfloat(r22.xyx)));
                    r24.xyz = asuint((asfloat(r11.zyz)) / (asfloat(r22.xyx)));
                    r19.xy = asuint((v4.xz) / (asfloat(r22.xy)));
                    r5.w = (uint)((0.9900000095367432) < (asfloat(r3.z))) * 0xffffffffu;
                    if (((r5.w) != 0)) {
                        r12.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r19.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                        r15.xyz = asuint((t8.SampleGrad(s8, (asfloat(r19.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyz);
                        r14.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r19.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                    } else {
                        r22.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                        r25.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r19.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                        r26.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r18.xyz)), (asfloat(r20.zy)), (asfloat(r21.zy)))).xyzw);
                        r27.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r19.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                        r18.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r18.xyz)), (asfloat(r20.xy)), (asfloat(r21.xy)))).xyzw);
                        r19.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r19.xyz)), (asfloat(r23.xy)), (asfloat(r24.xy)))).xyzw);
                        r5.w = asuint((asfloat(r4.w)) * (2.0) + (asfloat(r27.w)));
                        r7.w = asuint(max((asfloat(r5.w)), (asfloat(r26.w))));
                        r8.w = asuint((1.0) + (-cb0[262 + asint((asfloat(r9.w)))].x));
                        r7.w = asuint((asfloat(r7.w)) + (-asfloat(r8.w)));
                        r8.w = asuint((-asfloat(r7.w)) + (asfloat(r26.w)));
                        r8.w = asuint(max((asfloat(r8.w)), (0.0)));
                        r5.w = asuint((asfloat(r5.w)) + (-asfloat(r7.w)));
                        r5.w = asuint(max((asfloat(r5.w)), (0.0)));
                        r7.w = asuint((asfloat(r5.w)) + (asfloat(r8.w)));
                        r20.xyzw = asuint((asfloat(r5.wwww)) * (asfloat(r25.xyzw)));
                        r20.xyzw = asuint((asfloat(r22.xyzw)) * (asfloat(r8.wwww)) + (asfloat(r20.xyzw)));
                        r12.xyzw = asuint((asfloat(r20.xyzw)) / (asfloat(r7.wwww)));
                        r19.xyzw = asuint((asfloat(r5.wwww)) * (asfloat(r19.xyzw)));
                        r18.xyzw = asuint((asfloat(r18.xyzw)) * (asfloat(r8.wwww)) + (asfloat(r19.xyzw)));
                        r14.xyzw = asuint((asfloat(r18.xyzw)) / (asfloat(r7.wwww)));
                        r18.xyz = asuint((asfloat(r5.www)) * (asfloat(r27.xyz)));
                        r18.xyz = asuint((asfloat(r26.xyz)) * (asfloat(r8.www)) + (asfloat(r18.xyz)));
                        r15.xyz = asuint((asfloat(r18.xyz)) / (asfloat(r7.www)));
                    }
                }
            }
            r18.xyz = (uint3)((asfloat(r4.xyz)) == (asfloat(r5.xyz))) * 0xffffffffu;
            r5.w = (r18.y) & (r18.x);
            r5.w = (r18.z) & (r5.w);
            if (((r5.w) != 0)) {
                r18.xyzw = (r13.xyzw);
                r19.xyzw = (r17.xyzw);
                r20.xyz = (r16.xyz);
            } else {
                r21.xyz = (uint3)((asfloat(r3.xyz)) == (asfloat(r5.xyz))) * 0xffffffffu;
                r5.w = (r21.y) & (r21.x);
                r5.w = (r21.z) & (r5.w);
                if (((r5.w) != 0)) {
                    r18.xyzw = (r12.xyzw);
                    r19.xyzw = (r14.xyzw);
                    r20.xyz = (r15.xyz);
                } else {
                    r9.xw = asuint((asfloat(r5.xy)) * (float2(255.0,255.0)));
                    r9.xw = asuint((int2)(asfloat(r9.xw)));
                    r21.z = asuint((float)(asint(cb0[7 + asint((asfloat(r9.x)))].x)));
                    r22.x = (asuint(cb0[517 + asint((asfloat(r9.x)))].x));
                    r22.y = (asuint(cb0[772 + asint((asfloat(r9.x)))].x));
                    r23.xyz = asuint((asfloat(r10.zyz)) / (asfloat(r22.xyx)));
                    r24.xyz = asuint((asfloat(r11.zyz)) / (asfloat(r22.xyx)));
                    r21.xy = asuint((v4.xz) / (asfloat(r22.xy)));
                    r5.w = (uint)((asint(r9.w)) >= (255)) * 0xffffffffu;
                    if (((r5.w) != 0)) {
                        r18.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r21.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                        r20.xyz = asuint((t8.SampleGrad(s8, (asfloat(r21.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyz);
                        r19.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r21.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                    } else {
                        r22.z = asuint((float)(asint(cb0[7 + asint((asfloat(r9.w)))].x)));
                        r25.x = (asuint(cb0[517 + asint((asfloat(r9.w)))].x));
                        r25.y = (asuint(cb0[772 + asint((asfloat(r9.w)))].x));
                        r26.xyz = asuint((asfloat(r10.zyz)) / (asfloat(r25.xyx)));
                        r27.xyz = asuint((asfloat(r11.zyz)) / (asfloat(r25.xyx)));
                        r22.xy = asuint((v4.xz) / (asfloat(r25.xy)));
                        r5.w = (uint)((0.9900000095367432) < (asfloat(r5.z))) * 0xffffffffu;
                        if (((r5.w) != 0)) {
                            r18.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r22.xyz)), (asfloat(r26.zy)), (asfloat(r27.zy)))).xyzw);
                            r20.xyz = asuint((t8.SampleGrad(s8, (asfloat(r22.xyz)), (asfloat(r26.zy)), (asfloat(r27.zy)))).xyz);
                            r19.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r22.xyz)), (asfloat(r26.zy)), (asfloat(r27.zy)))).xyzw);
                        } else {
                            r25.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r21.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                            r28.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r22.xyz)), (asfloat(r26.zy)), (asfloat(r27.zy)))).xyzw);
                            r29.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r21.xyz)), (asfloat(r23.zy)), (asfloat(r24.zy)))).xyzw);
                            r30.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r22.xyz)), (asfloat(r26.zy)), (asfloat(r27.zy)))).xyzw);
                            r21.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r21.xyz)), (asfloat(r23.xy)), (asfloat(r24.xy)))).xyzw);
                            r22.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r22.xyz)), (asfloat(r26.xy)), (asfloat(r27.xy)))).xyzw);
                            r5.w = asuint((asfloat(r4.w)) * (2.0) + (asfloat(r30.w)));
                            r7.w = asuint(max((asfloat(r5.w)), (asfloat(r29.w))));
                            r8.w = asuint((1.0) + (-cb0[262 + asint((asfloat(r9.w)))].x));
                            r7.w = asuint((asfloat(r7.w)) + (-asfloat(r8.w)));
                            r8.w = asuint((-asfloat(r7.w)) + (asfloat(r29.w)));
                            r8.w = asuint(max((asfloat(r8.w)), (0.0)));
                            r5.w = asuint((asfloat(r5.w)) + (-asfloat(r7.w)));
                            r5.w = asuint(max((asfloat(r5.w)), (0.0)));
                            r7.w = asuint((asfloat(r5.w)) + (asfloat(r8.w)));
                            r23.xyzw = asuint((asfloat(r5.wwww)) * (asfloat(r28.xyzw)));
                            r23.xyzw = asuint((asfloat(r25.xyzw)) * (asfloat(r8.wwww)) + (asfloat(r23.xyzw)));
                            r18.xyzw = asuint((asfloat(r23.xyzw)) / (asfloat(r7.wwww)));
                            r22.xyzw = asuint((asfloat(r5.wwww)) * (asfloat(r22.xyzw)));
                            r21.xyzw = asuint((asfloat(r21.xyzw)) * (asfloat(r8.wwww)) + (asfloat(r22.xyzw)));
                            r19.xyzw = asuint((asfloat(r21.xyzw)) / (asfloat(r7.wwww)));
                            r21.xyz = asuint((asfloat(r5.www)) * (asfloat(r30.xyz)));
                            r21.xyz = asuint((asfloat(r29.xyz)) * (asfloat(r8.www)) + (asfloat(r21.xyz)));
                            r20.xyz = asuint((asfloat(r21.xyz)) / (asfloat(r7.www)));
                        }
                    }
                }
            }
            r21.xyz = (uint3)((asfloat(r2.xyz)) == (asfloat(r4.xyz))) * 0xffffffffu;
            r5.w = (r21.y) & (r21.x);
            r5.w = (r21.z) & (r5.w);
            if (((r5.w) != 0)) {
                r21.xyzw = (r13.xyzw);
                r22.xyzw = (r17.xyzw);
                r23.xyz = (r16.xyz);
            } else {
                r24.xyz = (uint3)((asfloat(r2.xyz)) == (asfloat(r3.xyz))) * 0xffffffffu;
                r5.w = (r24.y) & (r24.x);
                r5.w = (r24.z) & (r5.w);
                if (((r5.w) != 0)) {
                    r21.xyzw = (r12.xyzw);
                    r22.xyzw = (r14.xyzw);
                    r23.xyz = (r15.xyz);
                } else {
                    r24.xyz = (uint3)((asfloat(r2.xyz)) == (asfloat(r5.xyz))) * 0xffffffffu;
                    r5.w = (r24.y) & (r24.x);
                    r5.w = (r24.z) & (r5.w);
                    if (((r5.w) != 0)) {
                        r21.xyzw = (r18.xyzw);
                        r22.xyzw = (r19.xyzw);
                        r23.xyz = (r20.xyz);
                    } else {
                        r9.xw = asuint((asfloat(r2.xy)) * (float2(255.0,255.0)));
                        r9.xw = asuint((int2)(asfloat(r9.xw)));
                        r24.z = asuint((float)(asint(cb0[7 + asint((asfloat(r9.x)))].x)));
                        r25.x = (asuint(cb0[517 + asint((asfloat(r9.x)))].x));
                        r25.y = (asuint(cb0[772 + asint((asfloat(r9.x)))].x));
                        r26.xyz = asuint((asfloat(r10.zyz)) / (asfloat(r25.xyx)));
                        r27.xyz = asuint((asfloat(r11.zyz)) / (asfloat(r25.xyx)));
                        r24.xy = asuint((v4.xz) / (asfloat(r25.xy)));
                        r5.w = (uint)((asint(r9.w)) >= (255)) * 0xffffffffu;
                        if (((r5.w) != 0)) {
                            r21.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r24.xyz)), (asfloat(r26.zy)), (asfloat(r27.zy)))).xyzw);
                            r23.xyz = asuint((t8.SampleGrad(s8, (asfloat(r24.xyz)), (asfloat(r26.zy)), (asfloat(r27.zy)))).xyz);
                            r22.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r24.xyz)), (asfloat(r26.zy)), (asfloat(r27.zy)))).xyzw);
                        } else {
                            r25.z = asuint((float)(asint(cb0[7 + asint((asfloat(r9.w)))].x)));
                            r28.x = (asuint(cb0[517 + asint((asfloat(r9.w)))].x));
                            r28.y = (asuint(cb0[772 + asint((asfloat(r9.w)))].x));
                            r10.xyz = asuint((asfloat(r10.xyz)) / (asfloat(r28.xyx)));
                            r11.xyz = asuint((asfloat(r11.xyz)) / (asfloat(r28.xyx)));
                            r25.xy = asuint((v4.xz) / (asfloat(r28.xy)));
                            r5.w = (uint)((0.9900000095367432) < (asfloat(r2.z))) * 0xffffffffu;
                            if (((r5.w) != 0)) {
                                r21.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r25.xyz)), (asfloat(r10.zy)), (asfloat(r11.zy)))).xyzw);
                                r23.xyz = asuint((t8.SampleGrad(s8, (asfloat(r25.xyz)), (asfloat(r10.zy)), (asfloat(r11.zy)))).xyz);
                                r22.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r25.xyz)), (asfloat(r10.zy)), (asfloat(r11.zy)))).xyzw);
                            } else {
                                r28.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r24.xyz)), (asfloat(r26.zy)), (asfloat(r27.zy)))).xyzw);
                                r29.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r25.xyz)), (asfloat(r10.zy)), (asfloat(r11.zy)))).xyzw);
                                r30.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r24.xyz)), (asfloat(r26.zy)), (asfloat(r27.zy)))).xyzw);
                                r31.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r25.xyz)), (asfloat(r10.zy)), (asfloat(r11.zy)))).xyzw);
                                r24.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r24.xyz)), (asfloat(r26.xy)), (asfloat(r27.xy)))).xyzw);
                                r10.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r25.xyz)), (asfloat(r10.xy)), (asfloat(r11.xy)))).xyzw);
                                r4.w = asuint((asfloat(r4.w)) * (2.0) + (asfloat(r31.w)));
                                r5.w = asuint(max((asfloat(r4.w)), (asfloat(r30.w))));
                                r7.w = asuint((1.0) + (-cb0[262 + asint((asfloat(r9.w)))].x));
                                r5.w = asuint((asfloat(r5.w)) + (-asfloat(r7.w)));
                                r7.w = asuint((-asfloat(r5.w)) + (asfloat(r30.w)));
                                r7.w = asuint(max((asfloat(r7.w)), (0.0)));
                                r4.w = asuint((asfloat(r4.w)) + (-asfloat(r5.w)));
                                r4.w = asuint(max((asfloat(r4.w)), (0.0)));
                                r5.w = asuint((asfloat(r4.w)) + (asfloat(r7.w)));
                                r11.xyzw = asuint((asfloat(r4.wwww)) * (asfloat(r29.xyzw)));
                                r11.xyzw = asuint((asfloat(r28.xyzw)) * (asfloat(r7.wwww)) + (asfloat(r11.xyzw)));
                                r21.xyzw = asuint((asfloat(r11.xyzw)) / (asfloat(r5.wwww)));
                                r10.xyzw = asuint((asfloat(r4.wwww)) * (asfloat(r10.xyzw)));
                                r10.xyzw = asuint((asfloat(r24.xyzw)) * (asfloat(r7.wwww)) + (asfloat(r10.xyzw)));
                                r22.xyzw = asuint((asfloat(r10.xyzw)) / (asfloat(r5.wwww)));
                                r10.xyz = asuint((asfloat(r4.www)) * (asfloat(r31.xyz)));
                                r10.xyz = asuint((asfloat(r30.xyz)) * (asfloat(r7.www)) + (asfloat(r10.xyz)));
                                r23.xyz = asuint((asfloat(r10.xyz)) / (asfloat(r5.www)));
                            }
                        }
                    }
                }
            }
            r10.xyzw = asuint((asfloat(r8.xxxx)) * (asfloat(r12.xyzw)));
            r10.xyzw = asuint((asfloat(r2.wwww)) * (asfloat(r13.xyzw)) + (asfloat(r10.xyzw)));
            r10.xyzw = asuint((asfloat(r18.xyzw)) * (asfloat(r8.yyyy)) + (asfloat(r10.xyzw)));
            r10.xyzw = asuint((asfloat(r21.xyzw)) * (asfloat(r3.wwww)) + (asfloat(r10.xyzw)));
            r11.xyz = asuint((asfloat(r8.xxx)) * (asfloat(r15.xyz)));
            r11.xyz = asuint((asfloat(r2.www)) * (asfloat(r16.xyz)) + (asfloat(r11.xyz)));
            r11.xyz = asuint((asfloat(r20.xyz)) * (asfloat(r8.yyy)) + (asfloat(r11.xyz)));
            r11.xyz = asuint((asfloat(r23.xyz)) * (asfloat(r3.www)) + (asfloat(r11.xyz)));
            r12.xyzw = asuint((asfloat(r8.xxxx)) * (asfloat(r14.xyzw)));
            r12.xyzw = asuint((asfloat(r2.wwww)) * (asfloat(r17.xyzw)) + (asfloat(r12.xyzw)));
            r12.xyzw = asuint((asfloat(r19.xyzw)) * (asfloat(r8.yyyy)) + (asfloat(r12.xyzw)));
            r12.xyzw = asuint((asfloat(r22.xyzw)) * (asfloat(r3.wwww)) + (asfloat(r12.xyzw)));
            r6.xyzw = asuint((asfloat(r10.xyzw)) * (asfloat(r9.yyyy)) + (asfloat(r6.xyzw)));
            r7.xyz = asuint((asfloat(r11.xyz)) * (asfloat(r9.yyy)) + (asfloat(r7.xyz)));
            r0.xyzw = asuint((asfloat(r12.xyzw)) * (asfloat(r9.yyyy)) + (asfloat(r0.xyzw)));
        }
        if (((r8.z) != 0)) {
            r2.w = asuint((asfloat(r1.z)) * (asfloat(r1.w)));
            r1.zw = asuint((asfloat(r1.zw)) * (asfloat(r1.xy)));
            r1.x = asuint((asfloat(r1.y)) * (asfloat(r1.x)));
            r1.y = asuint((asfloat(r3.z)) * (asfloat(r1.z)));
            r1.y = asuint((asfloat(r2.w)) * (asfloat(r4.z)) + (asfloat(r1.y)));
            r1.y = asuint((asfloat(r1.w)) * (asfloat(r5.z)) + (asfloat(r1.y)));
            r1.y = asuint((asfloat(r1.x)) * (asfloat(r2.z)) + (asfloat(r1.y)));
            r1.y = asuint((asfloat(r1.y)) + (-0.5));
            r8.xyz = asuint(ddx((v4.xyx)));
            r9.xyw = asuint(ddy((v4.xyx)));
            r10.xy = asuint((asfloat(r4.xy)) * (float2(255.0,255.0)));
            r10.xy = asuint((int2)(asfloat(r10.xy)));
            r11.z = asuint((float)(asint(cb0[7 + asint((asfloat(r10.x)))].x)));
            r12.x = (asuint(cb0[517 + asint((asfloat(r10.x)))].x));
            r12.y = (asuint(cb0[772 + asint((asfloat(r10.x)))].x));
            r10.xzw = asuint((asfloat(r8.zyz)) / (asfloat(r12.xyx)));
            r13.xyz = asuint((asfloat(r9.wyw)) / (asfloat(r12.xyx)));
            r11.xy = asuint((v4.xy) / (asfloat(r12.xy)));
            r3.w = (uint)((asint(r10.y)) >= (255)) * 0xffffffffu;
            if (((r3.w) != 0)) {
                r12.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r11.xyz)), (asfloat(r10.wz)), (asfloat(r13.zy)))).xyzw);
                r14.xyz = asuint((t8.SampleGrad(s8, (asfloat(r11.xyz)), (asfloat(r10.wz)), (asfloat(r13.zy)))).xyz);
                r15.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r11.xyz)), (asfloat(r10.wz)), (asfloat(r13.zy)))).xyzw);
            } else {
                r16.z = asuint((float)(asint(cb0[7 + asint((asfloat(r10.y)))].x)));
                r17.x = (asuint(cb0[517 + asint((asfloat(r10.y)))].x));
                r17.y = (asuint(cb0[772 + asint((asfloat(r10.y)))].x));
                r18.xyz = asuint((asfloat(r8.zyz)) / (asfloat(r17.xyx)));
                r19.xyz = asuint((asfloat(r9.wyw)) / (asfloat(r17.xyx)));
                r16.xy = asuint((v4.xy) / (asfloat(r17.xy)));
                r3.w = (uint)((0.9900000095367432) < (asfloat(r4.z))) * 0xffffffffu;
                if (((r3.w) != 0)) {
                    r12.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r16.xyz)), (asfloat(r18.zy)), (asfloat(r19.zy)))).xyzw);
                    r14.xyz = asuint((t8.SampleGrad(s8, (asfloat(r16.xyz)), (asfloat(r18.zy)), (asfloat(r19.zy)))).xyz);
                    r15.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r16.xyz)), (asfloat(r18.zy)), (asfloat(r19.zy)))).xyzw);
                } else {
                    r17.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r11.xyz)), (asfloat(r10.wz)), (asfloat(r13.zy)))).xyzw);
                    r20.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r16.xyz)), (asfloat(r18.zy)), (asfloat(r19.zy)))).xyzw);
                    r21.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r11.xyz)), (asfloat(r10.wz)), (asfloat(r13.zy)))).xyzw);
                    r22.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r16.xyz)), (asfloat(r18.zy)), (asfloat(r19.zy)))).xyzw);
                    r11.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r11.xyz)), (asfloat(r10.xz)), (asfloat(r13.xy)))).xyzw);
                    r13.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r16.xyz)), (asfloat(r18.xy)), (asfloat(r19.xy)))).xyzw);
                    r3.w = asuint((asfloat(r1.y)) * (2.0) + (asfloat(r22.w)));
                    r4.w = asuint(max((asfloat(r3.w)), (asfloat(r21.w))));
                    r5.w = asuint((1.0) + (-cb0[262 + asint((asfloat(r10.y)))].x));
                    r4.w = asuint((asfloat(r4.w)) + (-asfloat(r5.w)));
                    r5.w = asuint((-asfloat(r4.w)) + (asfloat(r21.w)));
                    r5.w = asuint(max((asfloat(r5.w)), (0.0)));
                    r3.w = asuint((asfloat(r3.w)) + (-asfloat(r4.w)));
                    r3.w = asuint(max((asfloat(r3.w)), (0.0)));
                    r4.w = asuint((asfloat(r3.w)) + (asfloat(r5.w)));
                    r10.xyzw = asuint((asfloat(r3.wwww)) * (asfloat(r20.xyzw)));
                    r10.xyzw = asuint((asfloat(r17.xyzw)) * (asfloat(r5.wwww)) + (asfloat(r10.xyzw)));
                    r12.xyzw = asuint((asfloat(r10.xyzw)) / (asfloat(r4.wwww)));
                    r10.xyzw = asuint((asfloat(r3.wwww)) * (asfloat(r13.xyzw)));
                    r10.xyzw = asuint((asfloat(r11.xyzw)) * (asfloat(r5.wwww)) + (asfloat(r10.xyzw)));
                    r15.xyzw = asuint((asfloat(r10.xyzw)) / (asfloat(r4.wwww)));
                    r10.xyz = asuint((asfloat(r3.www)) * (asfloat(r22.xyz)));
                    r10.xyz = asuint((asfloat(r21.xyz)) * (asfloat(r5.www)) + (asfloat(r10.xyz)));
                    r14.xyz = asuint((asfloat(r10.xyz)) / (asfloat(r4.www)));
                }
            }
            r10.xyz = (uint3)((asfloat(r3.xyz)) == (asfloat(r4.xyz))) * 0xffffffffu;
            r3.w = (r10.y) & (r10.x);
            r3.w = (r10.z) & (r3.w);
            if (((r3.w) != 0)) {
                r10.xyzw = (r12.xyzw);
                r11.xyzw = (r15.xyzw);
                r13.xyz = (r14.xyz);
            } else {
                r16.xy = asuint((asfloat(r3.xy)) * (float2(255.0,255.0)));
                r16.xy = asuint((int2)(asfloat(r16.xy)));
                r17.z = asuint((float)(asint(cb0[7 + asint((asfloat(r16.x)))].x)));
                r18.x = (asuint(cb0[517 + asint((asfloat(r16.x)))].x));
                r18.y = (asuint(cb0[772 + asint((asfloat(r16.x)))].x));
                r16.xzw = asuint((asfloat(r8.zyz)) / (asfloat(r18.xyx)));
                r19.xyz = asuint((asfloat(r9.wyw)) / (asfloat(r18.xyx)));
                r17.xy = asuint((v4.xy) / (asfloat(r18.xy)));
                r3.w = (uint)((asint(r16.y)) >= (255)) * 0xffffffffu;
                if (((r3.w) != 0)) {
                    r10.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r17.xyz)), (asfloat(r16.wz)), (asfloat(r19.zy)))).xyzw);
                    r13.xyz = asuint((t8.SampleGrad(s8, (asfloat(r17.xyz)), (asfloat(r16.wz)), (asfloat(r19.zy)))).xyz);
                    r11.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r17.xyz)), (asfloat(r16.wz)), (asfloat(r19.zy)))).xyzw);
                } else {
                    r18.z = asuint((float)(asint(cb0[7 + asint((asfloat(r16.y)))].x)));
                    r20.x = (asuint(cb0[517 + asint((asfloat(r16.y)))].x));
                    r20.y = (asuint(cb0[772 + asint((asfloat(r16.y)))].x));
                    r21.xyz = asuint((asfloat(r8.zyz)) / (asfloat(r20.xyx)));
                    r22.xyz = asuint((asfloat(r9.wyw)) / (asfloat(r20.xyx)));
                    r18.xy = asuint((v4.xy) / (asfloat(r20.xy)));
                    r3.w = (uint)((0.9900000095367432) < (asfloat(r3.z))) * 0xffffffffu;
                    if (((r3.w) != 0)) {
                        r10.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r18.xyz)), (asfloat(r21.zy)), (asfloat(r22.zy)))).xyzw);
                        r13.xyz = asuint((t8.SampleGrad(s8, (asfloat(r18.xyz)), (asfloat(r21.zy)), (asfloat(r22.zy)))).xyz);
                        r11.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r18.xyz)), (asfloat(r21.zy)), (asfloat(r22.zy)))).xyzw);
                    } else {
                        r20.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r17.xyz)), (asfloat(r16.wz)), (asfloat(r19.zy)))).xyzw);
                        r23.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r18.xyz)), (asfloat(r21.zy)), (asfloat(r22.zy)))).xyzw);
                        r24.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r17.xyz)), (asfloat(r16.wz)), (asfloat(r19.zy)))).xyzw);
                        r25.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r18.xyz)), (asfloat(r21.zy)), (asfloat(r22.zy)))).xyzw);
                        r17.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r17.xyz)), (asfloat(r16.xz)), (asfloat(r19.xy)))).xyzw);
                        r18.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r18.xyz)), (asfloat(r21.xy)), (asfloat(r22.xy)))).xyzw);
                        r3.w = asuint((asfloat(r1.y)) * (2.0) + (asfloat(r25.w)));
                        r4.w = asuint(max((asfloat(r3.w)), (asfloat(r24.w))));
                        r5.w = asuint((1.0) + (-cb0[262 + asint((asfloat(r16.y)))].x));
                        r4.w = asuint((asfloat(r4.w)) + (-asfloat(r5.w)));
                        r5.w = asuint((-asfloat(r4.w)) + (asfloat(r24.w)));
                        r5.w = asuint(max((asfloat(r5.w)), (0.0)));
                        r3.w = asuint((asfloat(r3.w)) + (-asfloat(r4.w)));
                        r3.w = asuint(max((asfloat(r3.w)), (0.0)));
                        r4.w = asuint((asfloat(r3.w)) + (asfloat(r5.w)));
                        r16.xyzw = asuint((asfloat(r3.wwww)) * (asfloat(r23.xyzw)));
                        r16.xyzw = asuint((asfloat(r20.xyzw)) * (asfloat(r5.wwww)) + (asfloat(r16.xyzw)));
                        r10.xyzw = asuint((asfloat(r16.xyzw)) / (asfloat(r4.wwww)));
                        r16.xyzw = asuint((asfloat(r3.wwww)) * (asfloat(r18.xyzw)));
                        r16.xyzw = asuint((asfloat(r17.xyzw)) * (asfloat(r5.wwww)) + (asfloat(r16.xyzw)));
                        r11.xyzw = asuint((asfloat(r16.xyzw)) / (asfloat(r4.wwww)));
                        r16.xyz = asuint((asfloat(r3.www)) * (asfloat(r25.xyz)));
                        r16.xyz = asuint((asfloat(r24.xyz)) * (asfloat(r5.www)) + (asfloat(r16.xyz)));
                        r13.xyz = asuint((asfloat(r16.xyz)) / (asfloat(r4.www)));
                    }
                }
            }
            r16.xyz = (uint3)((asfloat(r4.xyz)) == (asfloat(r5.xyz))) * 0xffffffffu;
            r3.w = (r16.y) & (r16.x);
            r3.w = (r16.z) & (r3.w);
            if (((r3.w) != 0)) {
                r16.xyzw = (r12.xyzw);
                r17.xyzw = (r15.xyzw);
                r18.xyz = (r14.xyz);
            } else {
                r19.xyz = (uint3)((asfloat(r3.xyz)) == (asfloat(r5.xyz))) * 0xffffffffu;
                r3.w = (r19.y) & (r19.x);
                r3.w = (r19.z) & (r3.w);
                if (((r3.w) != 0)) {
                    r16.xyzw = (r10.xyzw);
                    r17.xyzw = (r11.xyzw);
                    r18.xyz = (r13.xyz);
                } else {
                    r19.xy = asuint((asfloat(r5.xy)) * (float2(255.0,255.0)));
                    r19.xy = asuint((int2)(asfloat(r19.xy)));
                    r20.z = asuint((float)(asint(cb0[7 + asint((asfloat(r19.x)))].x)));
                    r21.x = (asuint(cb0[517 + asint((asfloat(r19.x)))].x));
                    r21.y = (asuint(cb0[772 + asint((asfloat(r19.x)))].x));
                    r19.xzw = asuint((asfloat(r8.zyz)) / (asfloat(r21.xyx)));
                    r22.xyz = asuint((asfloat(r9.wyw)) / (asfloat(r21.xyx)));
                    r20.xy = asuint((v4.xy) / (asfloat(r21.xy)));
                    r3.w = (uint)((asint(r19.y)) >= (255)) * 0xffffffffu;
                    if (((r3.w) != 0)) {
                        r16.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r20.xyz)), (asfloat(r19.wz)), (asfloat(r22.zy)))).xyzw);
                        r18.xyz = asuint((t8.SampleGrad(s8, (asfloat(r20.xyz)), (asfloat(r19.wz)), (asfloat(r22.zy)))).xyz);
                        r17.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r20.xyz)), (asfloat(r19.wz)), (asfloat(r22.zy)))).xyzw);
                    } else {
                        r21.z = asuint((float)(asint(cb0[7 + asint((asfloat(r19.y)))].x)));
                        r23.x = (asuint(cb0[517 + asint((asfloat(r19.y)))].x));
                        r23.y = (asuint(cb0[772 + asint((asfloat(r19.y)))].x));
                        r24.xyz = asuint((asfloat(r8.zyz)) / (asfloat(r23.xyx)));
                        r25.xyz = asuint((asfloat(r9.wyw)) / (asfloat(r23.xyx)));
                        r21.xy = asuint((v4.xy) / (asfloat(r23.xy)));
                        r3.w = (uint)((0.9900000095367432) < (asfloat(r5.z))) * 0xffffffffu;
                        if (((r3.w) != 0)) {
                            r16.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r21.xyz)), (asfloat(r24.zy)), (asfloat(r25.zy)))).xyzw);
                            r18.xyz = asuint((t8.SampleGrad(s8, (asfloat(r21.xyz)), (asfloat(r24.zy)), (asfloat(r25.zy)))).xyz);
                            r17.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r21.xyz)), (asfloat(r24.zy)), (asfloat(r25.zy)))).xyzw);
                        } else {
                            r23.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r20.xyz)), (asfloat(r19.wz)), (asfloat(r22.zy)))).xyzw);
                            r26.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r21.xyz)), (asfloat(r24.zy)), (asfloat(r25.zy)))).xyzw);
                            r27.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r20.xyz)), (asfloat(r19.wz)), (asfloat(r22.zy)))).xyzw);
                            r28.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r21.xyz)), (asfloat(r24.zy)), (asfloat(r25.zy)))).xyzw);
                            r20.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r20.xyz)), (asfloat(r19.xz)), (asfloat(r22.xy)))).xyzw);
                            r21.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r21.xyz)), (asfloat(r24.xy)), (asfloat(r25.xy)))).xyzw);
                            r3.w = asuint((asfloat(r1.y)) * (2.0) + (asfloat(r28.w)));
                            r4.w = asuint(max((asfloat(r3.w)), (asfloat(r27.w))));
                            r5.w = asuint((1.0) + (-cb0[262 + asint((asfloat(r19.y)))].x));
                            r4.w = asuint((asfloat(r4.w)) + (-asfloat(r5.w)));
                            r5.w = asuint((-asfloat(r4.w)) + (asfloat(r27.w)));
                            r5.w = asuint(max((asfloat(r5.w)), (0.0)));
                            r3.w = asuint((asfloat(r3.w)) + (-asfloat(r4.w)));
                            r3.w = asuint(max((asfloat(r3.w)), (0.0)));
                            r4.w = asuint((asfloat(r3.w)) + (asfloat(r5.w)));
                            r19.xyzw = asuint((asfloat(r3.wwww)) * (asfloat(r26.xyzw)));
                            r19.xyzw = asuint((asfloat(r23.xyzw)) * (asfloat(r5.wwww)) + (asfloat(r19.xyzw)));
                            r16.xyzw = asuint((asfloat(r19.xyzw)) / (asfloat(r4.wwww)));
                            r19.xyzw = asuint((asfloat(r3.wwww)) * (asfloat(r21.xyzw)));
                            r19.xyzw = asuint((asfloat(r20.xyzw)) * (asfloat(r5.wwww)) + (asfloat(r19.xyzw)));
                            r17.xyzw = asuint((asfloat(r19.xyzw)) / (asfloat(r4.wwww)));
                            r19.xyz = asuint((asfloat(r3.www)) * (asfloat(r28.xyz)));
                            r19.xyz = asuint((asfloat(r27.xyz)) * (asfloat(r5.www)) + (asfloat(r19.xyz)));
                            r18.xyz = asuint((asfloat(r19.xyz)) / (asfloat(r4.www)));
                        }
                    }
                }
            }
            r4.xyz = (uint3)((asfloat(r2.xyz)) == (asfloat(r4.xyz))) * 0xffffffffu;
            r3.w = (r4.y) & (r4.x);
            r3.w = (r4.z) & (r3.w);
            if (((r3.w) != 0)) {
                r4.xyzw = (r12.xyzw);
                r19.xyzw = (r15.xyzw);
                r20.xyz = (r14.xyz);
            } else {
                r3.xyz = (uint3)((asfloat(r2.xyz)) == (asfloat(r3.xyz))) * 0xffffffffu;
                r3.x = (r3.y) & (r3.x);
                r3.x = (r3.z) & (r3.x);
                if (((r3.x) != 0)) {
                    r4.xyzw = (r10.xyzw);
                    r19.xyzw = (r11.xyzw);
                    r20.xyz = (r13.xyz);
                } else {
                    r3.xyz = (uint3)((asfloat(r2.xyz)) == (asfloat(r5.xyz))) * 0xffffffffu;
                    r3.x = (r3.y) & (r3.x);
                    r3.x = (r3.z) & (r3.x);
                    if (((r3.x) != 0)) {
                        r4.xyzw = (r16.xyzw);
                        r19.xyzw = (r17.xyzw);
                        r20.xyz = (r18.xyz);
                    } else {
                        r2.xy = asuint((asfloat(r2.xy)) * (float2(255.0,255.0)));
                        r2.xy = asuint((int2)(asfloat(r2.xy)));
                        r3.z = asuint((float)(asint(cb0[7 + asint((asfloat(r2.x)))].x)));
                        r5.x = (asuint(cb0[517 + asint((asfloat(r2.x)))].x));
                        r5.y = (asuint(cb0[772 + asint((asfloat(r2.x)))].x));
                        r21.xyz = asuint((asfloat(r8.zyz)) / (asfloat(r5.xyx)));
                        r22.xyz = asuint((asfloat(r9.wyw)) / (asfloat(r5.xyx)));
                        r3.xy = asuint((v4.xy) / (asfloat(r5.xy)));
                        r2.x = (uint)((asint(r2.y)) >= (255)) * 0xffffffffu;
                        if (((r2.x) != 0)) {
                            r4.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r3.xyz)), (asfloat(r21.zy)), (asfloat(r22.zy)))).xyzw);
                            r20.xyz = asuint((t8.SampleGrad(s8, (asfloat(r3.xyz)), (asfloat(r21.zy)), (asfloat(r22.zy)))).xyz);
                            r19.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r3.xyz)), (asfloat(r21.zy)), (asfloat(r22.zy)))).xyzw);
                        } else {
                            r5.z = asuint((float)(asint(cb0[7 + asint((asfloat(r2.y)))].x)));
                            r23.x = (asuint(cb0[517 + asint((asfloat(r2.y)))].x));
                            r23.y = (asuint(cb0[772 + asint((asfloat(r2.y)))].x));
                            r8.xyz = asuint((asfloat(r8.xyz)) / (asfloat(r23.xyx)));
                            r9.xyw = asuint((asfloat(r9.xyw)) / (asfloat(r23.xyx)));
                            r5.xy = asuint((v4.xy) / (asfloat(r23.xy)));
                            r2.x = (uint)((0.9900000095367432) < (asfloat(r2.z))) * 0xffffffffu;
                            if (((r2.x) != 0)) {
                                r4.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r5.xyz)), (asfloat(r8.zy)), (asfloat(r9.wy)))).xyzw);
                                r20.xyz = asuint((t8.SampleGrad(s8, (asfloat(r5.xyz)), (asfloat(r8.zy)), (asfloat(r9.wy)))).xyz);
                                r19.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r5.xyz)), (asfloat(r8.zy)), (asfloat(r9.wy)))).xyzw);
                            } else {
                                r23.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r3.xyz)), (asfloat(r21.zy)), (asfloat(r22.zy)))).xyzw);
                                r24.xyzw = asuint((t7.SampleGrad(s7, (asfloat(r5.xyz)), (asfloat(r8.zy)), (asfloat(r9.wy)))).xyzw);
                                r25.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r3.xyz)), (asfloat(r21.zy)), (asfloat(r22.zy)))).xyzw);
                                r26.xyzw = asuint((t8.SampleGrad(s8, (asfloat(r5.xyz)), (asfloat(r8.zy)), (asfloat(r9.wy)))).xyzw);
                                r3.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r3.xyz)), (asfloat(r21.xy)), (asfloat(r22.xy)))).xyzw);
                                r5.xyzw = asuint((t9.SampleGrad(s9, (asfloat(r5.xyz)), (asfloat(r8.xy)), (asfloat(r9.xy)))).xyzw);
                                r1.y = asuint((asfloat(r1.y)) * (2.0) + (asfloat(r26.w)));
                                r2.x = asuint(max((asfloat(r1.y)), (asfloat(r25.w))));
                                r2.y = asuint((1.0) + (-cb0[262 + asint((asfloat(r2.y)))].x));
                                r2.x = asuint((-asfloat(r2.y)) + (asfloat(r2.x)));
                                r2.y = asuint((-asfloat(r2.x)) + (asfloat(r25.w)));
                                r2.y = asuint(max((asfloat(r2.y)), (0.0)));
                                r1.y = asuint((asfloat(r1.y)) + (-asfloat(r2.x)));
                                r1.y = asuint(max((asfloat(r1.y)), (0.0)));
                                r2.x = asuint((asfloat(r1.y)) + (asfloat(r2.y)));
                                r8.xyzw = asuint((asfloat(r1.yyyy)) * (asfloat(r24.xyzw)));
                                r8.xyzw = asuint((asfloat(r23.xyzw)) * (asfloat(r2.yyyy)) + (asfloat(r8.xyzw)));
                                r4.xyzw = asuint((asfloat(r8.xyzw)) / (asfloat(r2.xxxx)));
                                r5.xyzw = asuint((asfloat(r1.yyyy)) * (asfloat(r5.xyzw)));
                                r3.xyzw = asuint((asfloat(r3.xyzw)) * (asfloat(r2.yyyy)) + (asfloat(r5.xyzw)));
                                r19.xyzw = asuint((asfloat(r3.xyzw)) / (asfloat(r2.xxxx)));
                                r3.xyz = asuint((asfloat(r1.yyy)) * (asfloat(r26.xyz)));
                                r3.xyz = asuint((asfloat(r25.xyz)) * (asfloat(r2.yyy)) + (asfloat(r3.xyz)));
                                r20.xyz = asuint((asfloat(r3.xyz)) / (asfloat(r2.xxx)));
                            }
                        }
                    }
                }
            }
            r3.xyzw = asuint((asfloat(r1.zzzz)) * (asfloat(r10.xyzw)));
            r3.xyzw = asuint((asfloat(r2.wwww)) * (asfloat(r12.xyzw)) + (asfloat(r3.xyzw)));
            r3.xyzw = asuint((asfloat(r16.xyzw)) * (asfloat(r1.wwww)) + (asfloat(r3.xyzw)));
            r3.xyzw = asuint((asfloat(r4.xyzw)) * (asfloat(r1.xxxx)) + (asfloat(r3.xyzw)));
            r2.xyz = asuint((asfloat(r1.zzz)) * (asfloat(r13.xyz)));
            r2.xyz = asuint((asfloat(r2.www)) * (asfloat(r14.xyz)) + (asfloat(r2.xyz)));
            r2.xyz = asuint((asfloat(r18.xyz)) * (asfloat(r1.www)) + (asfloat(r2.xyz)));
            r2.xyz = asuint((asfloat(r20.xyz)) * (asfloat(r1.xxx)) + (asfloat(r2.xyz)));
            r4.xyzw = asuint((asfloat(r1.zzzz)) * (asfloat(r11.xyzw)));
            r4.xyzw = asuint((asfloat(r2.wwww)) * (asfloat(r15.xyzw)) + (asfloat(r4.xyzw)));
            r4.xyzw = asuint((asfloat(r17.xyzw)) * (asfloat(r1.wwww)) + (asfloat(r4.xyzw)));
            r1.xyzw = asuint((asfloat(r19.xyzw)) * (asfloat(r1.xxxx)) + (asfloat(r4.xyzw)));
            r6.xyzw = asuint((asfloat(r3.xyzw)) * (asfloat(r9.zzzz)) + (asfloat(r6.xyzw)));
            r7.xyz = asuint((asfloat(r2.xyz)) * (asfloat(r9.zzz)) + (asfloat(r7.xyz)));
            r0.xyzw = asuint((asfloat(r1.xyzw)) * (asfloat(r9.zzzz)) + (asfloat(r0.xyzw)));
        }
    }
    r1.xyz = asuint((-v4.xyz) + (cb0[0].xyz));
    r1.x = asuint(dot((asfloat(r1.xyz)), (asfloat(r1.xyz))));
    r1.x = asuint(sqrt((asfloat(r1.x))));
    r1.y = (uint)((asfloat(r1.x)) >= (cb0[6].z)) * 0xffffffffu;
    if (((r1.y) != 0)) {
        r2.xyzw = asuint((t4.Sample(s4, (v7.xy))).xyzw);
        r1.yzw = asuint((t5.Sample(s5, (v7.xy))).xyz);
        r3.xyzw = asuint((t6.Sample(s6, (v7.xy))).xyzw);
        r1.x = asuint((asfloat(r1.x)) + (-cb0[6].z));
        r4.x = asuint((-cb0[6].z) + (cb0[6].w));
        r1.x = asuint(saturate((asfloat(r1.x)) / (asfloat(r4.x))));
        r2.xyzw = asuint((-asfloat(r6.xyzw)) + (asfloat(r2.xyzw)));
        r6.xyzw = asuint((asfloat(r1.xxxx)) * (asfloat(r2.xyzw)) + (asfloat(r6.xyzw)));
        r1.yzw = asuint((-asfloat(r7.xyz)) + (asfloat(r1.yzw)));
        r7.xyz = asuint((asfloat(r1.xxx)) * (asfloat(r1.yzw)) + (asfloat(r7.xyz)));
        r2.xyzw = asuint((-asfloat(r0.xyzw)) + (asfloat(r3.xyzw)));
        r0.xyzw = asuint((asfloat(r1.xxxx)) * (asfloat(r2.xyzw)) + (asfloat(r0.xyzw)));
        o0.w = asfloat((r6.w));
    } else {
        o0.w = asfloat((r6.w));
    }
    r1.xyz = asuint((t10.Sample(s10, (v7.xy))).xyz);
    r1.xyz = asuint((asfloat(r1.xyz)) * (asfloat(r6.xyz)));
    r2.xy = (asuint(cb0[1026].yy)) & (uint2(32u,64u));
    r3.xyz = asuint((abs(v6.xyz)) + (-cb0[6].xxx));
    r3.xyz = asuint(max((asfloat(r3.xyz)), (float3(0.0,0.0,0.0))));
    r3.xyz = (uint3)((float3(0.0,0.0,0.0)) < (asfloat(r3.xyz))) * 0xffffffffu;
    r1.w = (r3.x) & (1u);
    r1.w = asuint((-asint(r3.y)) + (asint(r1.w)));
    r1.w = asuint((-asint(r3.z)) + (asint(r1.w)));
    r3.xyz = (uint3)((asint(r1.www)) == (int3(1,2,3))) * 0xffffffffu;
    r4.xyz = ((r3.xxx) != 0) ? (uint3(0u,1065353216u,0u)) : (r1.xyz);
    r3.xyw = ((r3.yyy) != 0) ? (uint3(1065353216u,1065353216u,0u)) : (r4.xyz);
    r3.xyz = ((r3.zzz) != 0) ? (uint3(1065353216u,0u,0u)) : (r3.xyw);
    r1.xyz = ((r2.xxx) != 0) ? (r3.xyz) : (r1.xyz);
    r1.w = asuint((cb0[5].z) * (cb0[5].y));
    r3.xyzw = asuint((asfloat(r1.wwww)) * (float4(0.03125,0.0625,0.125,0.25)) + (float4(10.0,10.0,10.0,10.0)));
    r3.xyzw = (uint4)((asfloat(r3.xyzw)) >= (v2.xxxx)) * 0xffffffffu;
    r1.w = asuint((asfloat(r1.w)) * (0.5) + (10.0));
    r1.w = (uint)((asfloat(r1.w)) >= (v2.x)) * 0xffffffffu;
    r2.xzw = ((r1.www) != 0) ? (uint3(0u,1061997773u,1065353216u)) : (uint3(0u,0u,1065353216u));
    r2.xzw = ((r3.www) != 0) ? (uint3(0u,1065353216u,0u)) : (r2.xzw);
    r2.xzw = ((r3.zzz) != 0) ? (uint3(1065353216u,1065353216u,0u)) : (r2.xzw);
    r2.xzw = ((r3.yyy) != 0) ? (uint3(1065353216u,1053609165u,0u)) : (r2.xzw);
    r2.xzw = ((r3.xxx) != 0) ? (uint3(1065353216u,0u,0u)) : (r2.xzw);
    o0.xyz = asfloat(((r2.yyy) != 0) ? (r2.xzw) : (r1.xyz));
    r1.xyz = asuint((asfloat(r7.xyz)) * (float3(2.0,2.0,2.0)) + (float3(-1.0,-1.0,-1.0)));
    r1.w = asuint(dot((asfloat(r1.xyz)), (asfloat(r1.xyz))));
    r1.w = asuint(rsqrt((asfloat(r1.w))));
    r1.xyz = asuint((asfloat(r1.www)) * (asfloat(r1.xyz)));
    r1.w = asuint(dot((v6.xyz), (v6.xyz)));
    r1.w = asuint(rsqrt((asfloat(r1.w))));
    r2.xyz = asuint((asfloat(r1.www)) * (v6.zxy));
    r1.w = asuint(dot((v5.xyz), (v5.xyz)));
    r1.w = asuint(rsqrt((asfloat(r1.w))));
    r3.xyz = asuint((asfloat(r1.www)) * (v5.zxy));
    r4.xyz = asuint((asfloat(r2.zxy)) * (asfloat(r3.xyz)));
    r4.xyz = asuint((asfloat(r3.zxy)) * (asfloat(r2.xyz)) + (-asfloat(r4.xyz)));
    r4.xyz = asuint((asfloat(r4.xyz)) * (v2.www));
    r0.xyzw = asuint((asfloat(r0.xyzw)) * (cb0[1].xyzw));
    r5.xyz = asuint((asfloat(r1.yyy)) * (asfloat(r3.yzx)));
    r1.xyw = asuint((asfloat(r4.xyz)) * (asfloat(r1.xxx)) + (asfloat(r5.xyz)));
    r1.xyz = asuint((asfloat(r2.yzx)) * (asfloat(r1.zzz)) + (asfloat(r1.xyw)));
    r1.w = asuint(dot((asfloat(r3.yzx)), (asfloat(r1.xyz))));
    r3.xyz = asuint((-asfloat(r1.www)) * (asfloat(r1.zxy)) + (asfloat(r3.xyz)));
    r1.w = asuint(dot((asfloat(r3.xyz)), (asfloat(r3.xyz))));
    r1.w = asuint(rsqrt((asfloat(r1.w))));
    r3.xyz = asuint((asfloat(r1.www)) * (asfloat(r3.xyz)));
    r1.w = asuint((float)(asint(cb0[5].x)));
    r4.y = asuint(dot((cb0[2].xyz), (asfloat(r1.xyz))));
    r4.z = asuint(dot((cb0[3].xyz), (asfloat(r1.xyz))));
    r4.x = asuint(dot((cb0[4].xyz), (asfloat(r1.xyz))));
    r1.x = asuint((abs(asfloat(r4.z))) + (abs(asfloat(r4.y))));
    r1.x = asuint((abs(asfloat(r4.x))) + (asfloat(r1.x)));
    r1.xyz = asuint((asfloat(r4.xyz)) / (asfloat(r1.xxx)));
    r4.xy = asuint((-abs(asfloat(r1.zy))) + (float2(1.0,1.0)));
    r5.xyz = (uint3)((asfloat(r1.xyz)) >= (float3(0.0,0.0,0.0))) * 0xffffffffu;
    r4.zw = ((r5.yz) != 0) ? (uint2(1065353216u,1065353216u)) : (uint2(3212836864u,3212836864u));
    r4.xy = asuint((asfloat(r4.zw)) * (asfloat(r4.xy)));
    r1.xy = ((r5.xx) != 0) ? (r1.yz) : (r4.xy);
    o1.xy = (asfloat(r1.xy)) * (float2(0.5,0.5)) + (float2(0.5,0.5));
    r1.x = asuint((abs(asfloat(r2.z))) + (abs(asfloat(r2.y))));
    r1.x = asuint((abs(asfloat(r2.x))) + (asfloat(r1.x)));
    r1.xyz = asuint((asfloat(r2.xyz)) / (asfloat(r1.xxx)));
    r2.xy = asuint((-abs(asfloat(r1.zy))) + (float2(1.0,1.0)));
    r4.xyz = (uint3)((asfloat(r1.xyz)) >= (float3(0.0,0.0,0.0))) * 0xffffffffu;
    r2.zw = ((r4.yz) != 0) ? (uint2(1065353216u,1065353216u)) : (uint2(3212836864u,3212836864u));
    r2.xy = asuint((asfloat(r2.zw)) * (asfloat(r2.xy)));
    r1.xy = ((r4.xx) != 0) ? (r1.yz) : (r2.xy);
    o4.xy = (asfloat(r1.xy)) * (float2(0.5,0.5)) + (float2(0.5,0.5));
    r1.x = asuint((abs(asfloat(r3.z))) + (abs(asfloat(r3.y))));
    r1.x = asuint((abs(asfloat(r3.x))) + (asfloat(r1.x)));
    r1.xyz = asuint((asfloat(r3.xyz)) / (asfloat(r1.xxx)));
    r2.xy = asuint((-abs(asfloat(r1.zy))) + (float2(1.0,1.0)));
    r3.xyz = (uint3)((asfloat(r1.xyz)) >= (float3(0.0,0.0,0.0))) * 0xffffffffu;
    r2.zw = ((r3.yz) != 0) ? (uint2(1065353216u,1065353216u)) : (uint2(3212836864u,3212836864u));
    r2.xy = asuint((asfloat(r2.zw)) * (asfloat(r2.xy)));
    r1.xy = ((r3.xx) != 0) ? (r1.yz) : (r2.xy);
    o4.zw = (asfloat(r1.xy)) * (float2(0.5,0.5)) + (float2(0.5,0.5));
    r1.x = (uint)(asfloat(r1.w));
    r1.x = asuint((asint(r1.x)) << ((8u) & 31));
    r1.x = asuint((float)(r1.x));
    o1.w = (asfloat(r1.x)) * (1.5259021893143654e-05);
    o1.z = asfloat((0u));
    o2.xyzw = (asfloat(r0.xyzw)) * (float4(1.0,1.0,1.0,0.5));
    o3.xyzw = asfloat((uint4(0u,0u,0u,0u)));
    return;
}
