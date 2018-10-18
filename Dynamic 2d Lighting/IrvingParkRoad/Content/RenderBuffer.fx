float4x4 World;
float4x4 View;
float4x4 Projection;

float2 screenWidthHeight;

texture colorMap;
texture heightMap;
texture lightMap;

sampler colorSampler = sampler_state
{
	texture = <colorMap>;
	MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    MIPFILTER = LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
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

sampler lightSampler = sampler_state
{
	texture = <lightMap>;
	MAGFILTER = POINT;
    MINFILTER = POINT;
    MIPFILTER = POINT;
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

VertexShaderOutput VertexShaderRender(VertexShaderInput input)
{
    VertexShaderOutput output;

    float4 worldPosition = mul(input.Position, World);
    float4 viewPosition = mul(worldPosition, View);
    output.Position = mul(viewPosition, Projection);
    output.Posi = output.Position;
    output.TexCoord = input.TexCoord;		//pass the texture coordinates further
    return output;
}

float4 PixelShaderRender(VertexShaderOutput input) : COLOR0
{

    float2 texPos = postProjToScreen(input.Posi) + halfPixel();
    float2 uvtoxy = UVToScreen(input.Posi);
    
    float4 lightVal = tex2D(lightSampler, texPos);
    float4 colorVal = tex2D(colorSampler, texPos);
    float4 heightVal = tex2D(heightSampler, texPos);

    if(colorVal.a == 0.0f)
    {
		colorVal = float4(1,1,1,1);
    }
    
  
    
    //Implement Lach Derivative
    
    int iterations = 50;
    float dampener = 1.0f;
    float2 lookUp = texPos;
    float2 shifter =  float2(0,0);
    shifter.x = ((lightVal.g - 0.5f) * 2.0f);
    shifter.y = ((lightVal.b - 0.5f) * 2.0f);
    shifter /= screenWidthHeight;
    shifter = normalize(shifter) * -halfPixel() * 2.5f;
    //lookUp = lookUp + (shifter * iterations );
 
    for( int i = 1; i <= iterations ;  i++)
    {
		lookUp += shifter;
		float4 sample = tex2D(heightSampler, lookUp );
		if( sample.r > heightVal.r)
		{
			dampener *= 0.5;
		}
    } 
    
    //return saturate(clamp(lightVal.r*1.5f, 0.0f, 1.0f) * dampener) ;
    return saturate(colorVal * dampener);
}

technique Default
{
    pass Pass1
    {
    	AlphaBlendEnable=false;
		SrcBlend=srcalpha;
		DestBlend=invsrcalpha;
        VertexShader = compile vs_3_0 VertexShaderRender();
        PixelShader = compile ps_3_0 PixelShaderRender();
    }
}