Shader "Unlit/OlTexShader"
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
                float4 texRect;
                int2 texSize;
            };

            StructuredBuffer<TextureUvs> texInfos;

            struct appdata
            {
                float4 vertex : POSITION;
                float4 uvtf : TEXCOORD0;
            };

            struct v2f
            {
                float3 uvf : TEXCOORD0;
                uint f : TEXCOORD3;
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

                int2 texSize = texInfos[textureId].texSize;
                //int2 texSize = int2(64, 64);
                o.uvf.xy = v.uvtf.xy / texSize.xy; // 0.0 - 1.0 range now corresponds to full texture size.
                o.uvf.xy = o.uvf.xy * float2(8.0f, 8.0f);
                o.uvf.z = v.uvtf.w;
                o.f = asuint(v.uvtf.w);
                o.texSize = texSize;

                float4 texRect = texInfos[textureId].texRect;
                //o.texRect = texRect;
                o.texRect = float4(texRect.x, 1.0f - texRect.y, texRect.z, 1.0f - texRect.w);
                //o.texRect = float4(0.203125000, 0.975708485, 0.218750000, 0.967611313);
                //o.texRect = float4(0.203125, 0.0242915, 0.21875, 0.03238866);

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
            
            PixelOut frag (v2f i) : SV_Target
            {
                // Here i.uvf.xy is in "Outlaws texture coordinates". i.uvf.z is wallFlag1.
                uint wallFlag1 = i.f; //asuint(i.uvf.z);
                float2 uv = i.uvf.xy; // + float2(0.5f, 0.5f) / i.texSize;


                float2 t = uv;
                uv = frac(uv);   // This is wrap-around for texture values.
                t -= uv;

                const uint FLIP_TEXTURE_HORIZONTALLY = (1u << 2);
                if (wallFlag1 & FLIP_TEXTURE_HORIZONTALLY) {
                    uv.x = 1.0f - uv.x;
                }
                uv.y = 1.0f - uv.y;
                uv = lerp(i.texRect.xy, i.texRect.zw, uv); // Transform to coordinates inside the sprite sheet.
                fixed4 col = tex2D(_MainTex, uv);
                //col.a = trans;

                if ((col.r == 0) && (col.g == 111.0f/255.0f) && (col.b == 127.0f/255.0f)) // transparent color
                    discard;

                const uint HACK_SIGN_FLAG = (1u << 31);
                float trans = 1.0f;
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
