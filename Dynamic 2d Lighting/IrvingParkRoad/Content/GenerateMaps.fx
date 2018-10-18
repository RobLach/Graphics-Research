float4x4 World;
float4x4 View;
float4x4 Projection;

Texture heightMap;
Texture colorMap;

struct VertexShaderInput
{
    float4 Position : POSITION0;
	float2 TexCoord : TEXCOORD0;
};

sampler heightSampler = sampler_state
{
	texture = <heightMap>;
	MAGFILTER = POINT;
    MINFILTER = POINT;
    MIPFILTER = POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

sampler colorSampler = sampler_state
{
	texture = <colorMap>;
	MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    MIPFILTER = LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

struct VertexShaderOutput
{
    float4 Position : POSITION0;
    float2 TexCoord : TEXCOORD0;
};

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
    VertexShaderOutput output;

    float4 worldPosition = mul(input.Position, World);
    float4 viewPosition = mul(worldPosition, View);
    output.Position = mul(viewPosition, Projection);

    output.TexCoord = input.TexCoord;

    return output;
}

struct PixelShaderOutput
{
	float4 Color : COLOR0;
	float4 Height : COLOR1;
};

PixelShaderOutput PixelShaderFunction(VertexShaderOutput input)
{
	PixelShaderOutput output;
	output.Color = tex2D(colorSampler, input.TexCoord);
	output.Height = tex2D(heightSampler, input.TexCoord);
    return output;
}

technique Default
{
    pass Pass1
    {   
    	AlphaBlendEnable = true;
    	BlendOp = Add;
		SrcBlend = SrcAlpha;
		DestBlend = InvSrcAlpha;
        VertexShader = compile vs_2_0 VertexShaderFunction();
        PixelShader = compile ps_2_0 PixelShaderFunction();
    }
}