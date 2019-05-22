#version 330 core

layout(location = 0) out vec4 vFragColor;	//fragment shader output

smooth in vec3 vUV;				//3D texture coordinates form vertex shader 
								//interpolated by rasterizer

//uniform
uniform bool tfs;
uniform sampler3D volume;			//volume dataset
uniform sampler1D color_tex;		//volume dataset


//color
smooth in vec3 final_color;

void main()
{
	//Here we sample the volume dataset using the 3D texture coordinates from the vertex shader.
	//Note that since at the time of texture creation, we gave the internal format as GL_RED
	//we can get the sample value from the texture using the red channel. Here, we set all 4
	//components as the sample value in the texture which gives us a shader of grey.
	
	if(tfs)
	{
		vFragColor = texture(color_tex, texture(volume, vUV).r);
	}
	
	else
	{
		vFragColor = vec4(final_color, 1.0f) * 1.0f;
	}
	
}








