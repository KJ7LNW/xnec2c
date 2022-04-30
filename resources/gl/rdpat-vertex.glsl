#version 120

// for 130 to 120 transformation, see
// https://github.com/projectM-visualizer/projectm/pull/399/commits/093164d583e758794e93b8d965722095d0a04484

attribute vec3 position;
attribute vec3 color;

uniform mat4 mvp;

varying vec4 vertexColor;

void main() {
  gl_Position = mvp * vec4(position, 1.0);
  vertexColor = vec4(color, 1.0);
}
