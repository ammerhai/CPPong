struct VertexShaderInput {
	uint vid : SV_VertexID;
};

struct VertexShaderOutput {
	float4 pos : SV_POSITION;
};

VertexShaderOutput main(VertexShaderInput input) {
	VertexShaderOutput output;

	output.pos = float4(0, 0, 0, 1);


	if (input.vid == 0) {
		output.pos.xy = float2(-0.002, 1);
	}
	if (input.vid == 1) {
		output.pos.xy = float2(0.002, -1);
	}
	if (input.vid == 2) {
		output.pos.xy = float2(-0.002, -1);
	}
	if (input.vid == 3) {
		output.pos.xy = float2(0.002, 1);
	}


	return output;
}