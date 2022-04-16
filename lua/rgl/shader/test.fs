#version 330 core
in  vec2  TexCoords;
out vec4  color;
  
uniform sampler2D scene;
uniform bool funny;

void main()
{
    color = vec4(0.0f);
    vec3 sample[9];

	if (funny) {
        color =  texture(scene, TexCoords) + vec4(1.0, 1.0, 1.0, 0.0);
	} else  {
        color =  texture(scene, TexCoords);
    }
}