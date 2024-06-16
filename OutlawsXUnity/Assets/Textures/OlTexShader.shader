﻿Shader "Unlit/OlTexShader"
{
    Properties
    {
        _MainTex ("Texture", 2D) = "white" {}
        _MulOffs ("Multiplier and Offset", Vector) = (1.0, 1.0, 0.0, 0.0)
        _TexRect ("Texture Rect", Vector) = (0.0, 0.0, 1.0, 1.0)
        _TexSize ("Texture Size", Vector) = (64.0, 64.0, 0.0, 0.0)
    }
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        LOD 100

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            // make fog work
            #pragma multi_compile_fog
            
            #include "UnityCG.cginc"

            struct TextureUvs {
                float4 texRect; ///< Top left corner of the sub-texture, and bottom right corner of the sub-texture. (0.0, 0.0) is top left of the texture atlas, (1.0, 1.0) is bottom right of the texture atlas.
                int2 texSize;   ///< Sub-texture size in pixels.
            };

            StructuredBuffer<TextureUvs> texInfos;

            struct appdata
            {
                float4 vertex : POSITION;
                float4 uvtf : TEXCOORD0; // U, V, TextureId, WallFlag1
            };

            struct v2f
            {
                float3 uvf : TEXCOORD0;     // U, V, WallFlag1
                uint f : TEXCOORD3;         // WallFlag1
                float4 texRect : TEXCOORD1;
                int2 texSize : TEXCOORD2;
                UNITY_FOG_COORDS(1)
                float4 vertex : SV_POSITION;
            };

            sampler2D _MainTex;
            float4 _MainTex_ST;
            
            v2f vert (appdata v)
            {
                v2f o;
                o.vertex = UnityObjectToClipPos(v.vertex);
                //o.uv = TRANSFORM_TEX(v.uv, _MainTex);

                int textureId = (int)v.uvtf.z;
                //int textureId = 54;

                int2 texSize = texInfos[textureId].texSize; ///< Sub-texture size in pixels.
                o.uvf.xy = v.uvtf.xy / texSize.xy; // 0.0 - 1.0 range now corresponds to full texture size.
                o.uvf.xy = o.uvf.xy * float2(8.0f, 8.0f);
                o.uvf.z = v.uvtf.w;
                o.f = asuint(v.uvtf.w);
                o.texSize = texSize;

                float4 texRect = texInfos[textureId].texRect;
                //o.texRect = texRect;
                o.texRect = float4(texRect.x, 1.0f - texRect.y, texRect.z, 1.0f - texRect.w);

                const uint HACK_SIGN_FLAG = (1u << 31);
                if (o.f & HACK_SIGN_FLAG) {
                    //o.vertex.z += 0.0001;
                }

                UNITY_TRANSFER_FOG(o,o.vertex);
                return o;
            }

            struct PixelOut 
            {
                fixed4 Color : COLOR;
                //float Depth : DEPTH;
            };
            
            float4 mymod(float2 a, float2 b) {  // xy = division, zw = reminder
                float2 d = -floor(a / b);
                float2 r = a + b * d; // Using '-' here causes some shader import bug. So we need to use '+'...
                return float4(-d.x, -d.y, r.x, r.y);
            }

            PixelOut frag (v2f i) : SV_Target
            {
                //float2 atlasSize(8192.0f, 8192.0f); 
                // Here i.uvf.xy is in "Outlaws texture coordinates". i.uvf.z is wallFlag1.
                uint wallFlag1 = i.f; //asuint(i.uvf.z);

                //float2 axc = float2(0.0f, 0.0f); //i.uvf.xy * i.texSize;
                //float2 rxc = axc + float2(-axc.x, -axc.y);
                //float4 texPixUV = float4(d.x, d.y, r.x, r.y);
                float4 texPixUV = mymod(i.uvf.xy * i.texSize, i.texSize);
                //int2 texPixUVi = int2(i.uvf.xy * i.texSize) % i.texSize;
                //float4 texPixUV = float4(0.0f, 0.0f, texPixUVi.x, texPixUVi.y);
                texPixUV.zw = floor(texPixUV.zw);

                float2 uv = (texPixUV.zw + float2(0.5f, 0.5f)) / float2(i.texSize);  

                const uint FLIP_TEXTURE_HORIZONTALLY = (1u << 2);
                if (wallFlag1 & FLIP_TEXTURE_HORIZONTALLY) {
                    uv.x = 1.0f - uv.x;
                }
                uv.y = 1.0f - uv.y;
                uv = lerp(i.texRect.xy, i.texRect.zw, uv); // Transform to coordinates inside the sprite sheet.
                fixed4 col = tex2Dgrad(_MainTex, uv, float2(0.0f, 0.0f), float2(0.0f, 0.0f)); // Gradients help avoid sampling texture beyond the texture extents. But breaks mipmapping.
                                                                                              // Fixing mip-mapping when using a texture atlas is not an easy task...
                //col.a = trans;

                if ((col.r == 0) && (col.g == 111.0f/255.0f) && (col.b == 127.0f/255.0f)) // transparent color
                    discard;

                const uint HACK_SIGN_FLAG = (1u << 31);
                float trans = 1.0f;
                float2 t = texPixUV.xy;  ///< This should be texPixUV without mod / texSize. Signs should not be repeating.
                //float2 t = float2(0.0f, 0.0f);  ///< This should be texPixUV without mod / texSize. Signs should not be repeating.
                if (wallFlag1 & HACK_SIGN_FLAG) {
                    if (t.x < -0.0) {
                        col = fixed4(1.0, 0.0, 0.0, 0.5);
                        discard;
                    }
                    if (t.x > -0.0) {
                        col = fixed4(0.0, 1.0, 0.0, 0.5);
                        discard;
                    }
                    if (t.y < 0.0) {
                        col = fixed4(1.0, 1.0, 0.0, 0.5);
                        discard;
                    }
                    if (t.y > 0.0) {
                        col = fixed4(0.0, 0.0, 1.0, 0.5);
                        discard;
                    }
                }

                // apply fog
                UNITY_APPLY_FOG(i.fogCoord, col);

                PixelOut po;
                po.Color = col;
                //po.depth = 

                return po;
            }
            ENDCG
        }
    }
}
