#version 330

layout(location = 0) out vec4 out_color;

uniform sampler2D texture_color;
uniform sampler2D texture_depth;

uniform int screen_width, screen_height;

uniform int c;//control variable
in vec2 texcoord;

vec3 blur(){
	float dx = 1.0f/screen_width;
	float dy = 1.0f/screen_height;
	vec3 sum = vec3(0,0,0);
	for(int i=-5;i<5;i++) 
	for(int j=-5;j<5;j++) 
		sum += texture(texture_color, texcoord + vec2(i * dx, j  * dy)).xyz;
	return sum/50;
}

vec3 depth(){
	float t2 = pow(texture(texture_depth, texcoord).x , 256);	
	return vec3(t2,t2,t2);
}

vec3 color(){
	return texture(texture_color, texcoord).xyz;
}

void main(){

	out_color = vec4(blur(),1);

	if(c==0) out_color = vec4(blur(), 1);
	else if(c==1) out_color = vec4(depth(), 1);
	else if(c==2) out_color = vec4(color(),1);
}