struct VertexShaderInput {
	// Per Vertex
	float4 pos : VERTEX_POSITION;
	uint vid : SV_VertexID;

	// Per Instance
	float4 pos_size : INSTANCE_POSITION_SIZE;
	float4 left_color : LEFT_COLOR;
	float4 right_color : RIGHT_COLOR;

};

struct VertexShaderOutput {
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};



VertexShaderOutput main(VertexShaderInput input) {
	VertexShaderOutput output;

	float2 rect_pos = input.pos_size.xy;
	float2 rect_size = input.pos_size.zw;

	float3x3 coord_sys = {
		2, 0, -1,
		0, -2, 1,
		0, 0, 1
	};

	input.pos_size.xy = mul(coord_sys, float3(input.pos_size.xy, 1)).xy;

	float3x3 pos = {
		1, 0, input.pos_size.x, 
		0, 1, input.pos_size.y,
		0, 0, 1
	};

	float3x3 size = {
		rect_size.x, 0, 0,
		0, rect_size.y, 0,
		0, 0, 1
	};

	output.pos.xy = mul(pos, mul(size, input.pos.xyz)).xy;
	//output.pos.xy = mul(pos, input.pos.xyz).xy;
	output.pos.zw = float2(0, 1);


	if (input.vid == 0)
		output.color = input.left_color;
	else if (input.vid == 1)
		output.color = input.right_color;
	else if (input.vid == 2)
		output.color = float4(0, 1, 1, 1);
	else
		output.color = float4(1, 1, 1, 1);


	return output;
}