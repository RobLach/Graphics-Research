float4x4 World;
float4x4 View;
float4x4 Projection;

float2 LightPos;
float LightRadius;

float2 screenWidthHeight;

Texture heightMap;

sampler heightSampler = sampler_state
{
	texture = <heightMap>;
	MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    MIPFILTER = LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

struct VertexShaderInput
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD0;
};

struct VertexShaderOutput
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Posi : TEXCOORD1;
};

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
    VertexShaderOutput output;

    float4 worldPosition = mul(input.Position, World);
    float4 viewPosition = mul(worldPosition, View);
    output.Position = mul(viewPosition, Projection);
    output.Posi = output.Position;
    output.TexCoord = input.TexCoord;		//pass the texture coordinates further
    return output;
}

float2 postProjToScreen(float4 posi)
{
	float2 screenPos = posi.xy / posi.w;
	return (0.5f * (float2(screenPos.x, -screenPos.y)+1));
}

float2 UVToScreen(float4 posi)
{
	float utox = (posi.x + 1) * screenWidthHeight.x/2;
    float vtoy = abs((posi.y - 1) * screenWidthHeight.y/2);
    return float2(utox, vtoy);
}

float2 halfPixel()
{
	return -(0.5f / screenWidthHeight);
}


float4 PixelShaderFunction(VertexShaderOutput input) : COLOR0
{
	float4 light = float4(0.0f,0.0f,0.0f,0.0f);
	float2 screenPos = UVToScreen(input.Posi);
	float2 texPos = postProjToScreen(input.Posi) - halfPixel();

		float dist = distance(LightPos, screenPos);
		if(dist <= LightRadius)
		{
			light.r = LightRadius - dist;
			light.r = light.r / LightRadius;
			light.gb = normalize(LightPos - screenPos);
			light.g = (light.g + 1) / 2;
			light.b = (light.b + 1) / 2;
			light.a = 1.0f;
		}
	
	return light;
}

technique Default
{
    pass Pass1
	{
	    AlphaBlendEnable=false;
	    BlendOp = Add;
		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;
        VertexShader = compile vs_2_0 VertexShaderFunction();
        PixelShader = compile ps_2_0 PixelShaderFunction();
    }
}
