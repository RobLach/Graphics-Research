//float2 ScreenDimensions;

//float2 LightPos;
//float LightRadius;

struct VertexShaderInput
{
    float3 Position : POSITION0;
};

struct VertexShaderOutput
{
    float4 Position : POSITION0;
};

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
    VertexShaderOutput output = (VertexShaderOutput)0;
    output.Position = float4(input.Position,1);
    return output;
}

struct PixelShaderOutput
{
    float4 Color : COLOR0;
    float4 Height : COLOR1;
    float4 Light : COLOR2;
    float4 Final : COLOR3;
};


PixelShaderOutput PixelShaderFunction(VertexShaderOutput input)
{
	PixelShaderOutput output = (PixelShaderOutput)0;
	
	output.Color = float4(0, 0, 0, 0);
	output.Height = float4(0, 0, 0, 0);
	output.Light = float4(0, 0, 0, 0);
	output.Final = float4(0,0,0,1);
	
	return output;
}

technique Technique1
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VertexShaderFunction();
        PixelShader = compile ps_2_0 PixelShaderFunction();
    }
}
