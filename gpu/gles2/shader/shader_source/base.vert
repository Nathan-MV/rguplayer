
uniform mat4 u_projectionMat;

uniform vec2 u_texSize;
uniform vec2 u_transOffset;

attribute vec2 a_position;
attribute vec2 a_texCoord;

varying vec2 v_texCoord;

void main() {
	gl_Position = u_projectionMat * vec4(a_position + u_transOffset, 0.0, 1.0);

	v_texCoord = a_texCoord * u_texSize;
}
