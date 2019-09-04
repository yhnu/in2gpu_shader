#version 330
layout(location = 0) out vec4 out_color;

uniform vec3 eye_position;

uniform float material_kd;

in vec3 world_pos;
in vec3 world_normal;


vec3 color= vec3(1.0,0.4,0.00);

vec3 Kd = vec3(1.00,1.0,1.00);

void main(){



	vec3 light_position = vec3(0,0,50);
	vec3 L = normalize( light_position - world_pos);
	vec3 V = normalize( eye_position - world_pos);

	float difuza = max(0, dot(L,world_normal));
	Kd = Kd * material_kd * difuza ;

	color = color + Kd ;

	out_color = vec4(color,1);
}