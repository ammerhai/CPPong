struct PixelShaderInput {
	float4 pos : SV_POSITION;
};

struct PixelShaderOutput {
	float4 color : SV_TARGET;
};

PixelShaderOutput main(PixelShaderInput input) {
	PixelShaderOutput output;

	float sinus = sin(19 * ((input.pos.y / 1080.0f) * 6.283185f) + 0.5f * 3.141564f);

	output.color = float4(0, 0, 0, 0);
	if (sinus < 0) {
		output.color = float4(1, 1, 1, 1);
	}


	return output;
}