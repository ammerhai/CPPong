struct VertexShaderInput {
	float4 pos : VERTEX_POSITION;
	float4 color : VERTEX_COLOR;
	uint vid : SV_VertexID;
};

struct VertexShaderOutput {
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};



VertexShaderOutput main(VertexShaderInput input) {
	VertexShaderOutput output;
	output.pos = input.pos;

#if 0
	output.color = input.color;
#else
	if (input.vid == 0)
		output.color = float4(1, 0, 1, 1);
	else if (input.vid == 1)
		output.color = float4(0, 1, 1, 1);
	else if (input.vid == 2)
		output.color = float4(1, 1, 0, 1);
	else
		output.color = float4(1, 0, 1, 1);
#endif



	return output;
}