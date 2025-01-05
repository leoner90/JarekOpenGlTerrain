// FRAGMENT SHADER

#version 330

in vec4 color;
out vec4 outColor;

//textures
uniform sampler2D texture0;
in vec2 texCoord0;
 


void main(void) 
{
  outColor = color;
  outColor *= texture(texture0, texCoord0);
}
