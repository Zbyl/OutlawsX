Shader "Unlit/OlTexShaderSimp"
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

            float4 _MulOffs;
            float4 _TexRect;
            float4 _TexSize;

            struct appdata
            {
                float4 vertex : POSITION;
                float2 uv : TEXCOORD0;
            };

            struct v2f
            {
                float2 uv : TEXCOORD0;
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
                o.uv = v.uv;
                UNITY_TRANSFER_FOG(o,o.vertex);
                return o;
            }
            
            fixed4 frag (v2f i) : SV_Target
            {
                // sample the texture
                //i.uv = i.uv * _MainTex_ST.xy + _MainTex_ST.zw
                i.uv = i.uv * _MulOffs.xy + _MulOffs.zw;    // Just a helper to test various positionings.

                // Here i.uv is in "Outlaws texture coordinates".
                i.uv = i.uv / _TexSize.xy; // 0.0 - 1.0 range now corresponds to full texture size.
                i.uv = frac(i.uv);   // This is wrap-around for texture values.
                i.uv = lerp(_TexRect.xy, _TexRect.zw, i.uv); // Transform to coordinates inside the sprite sheet.
                fixed4 col = tex2D(_MainTex, i.uv);
                // apply fog
                UNITY_APPLY_FOG(i.fogCoord, col);
                return col;
            }
            ENDCG
        }
    }
}
