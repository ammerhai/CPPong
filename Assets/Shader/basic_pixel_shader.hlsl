struct PixelShaderInput {
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

struct PixelShaderOutput {
	float4 color : SV_TARGET;
};


PixelShaderOutput main(PixelShaderInput input) {
	PixelShaderOutput output;
#if 1
	output.color = input.color;
#else
	output.color = float4(1, 0, 1, 1);
#endif
	return output;
}