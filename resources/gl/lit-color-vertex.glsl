#version 150
in vec3 position;
in vec3 normal;
in vec4 color;
in vec2 uv;
in vec2 direction;
in float magnitude;
uniform mat4 mvp;
uniform mat4 u_mv;
centroid out vec4 vertexColor;
centroid out vec3 viewNormal;
centroid out vec3 viewPos;
centroid out vec2 vUV;
centroid out vec2 vFlowDir;
centroid out float vFlowMag;

void main() {
  /* Arrow marks arrive resolved into world space, so every vertex draws
   * at the position its producer wrote. */
  gl_Position = mvp * vec4(position, 1.0);

  viewPos = (u_mv * vec4(position, 1.0)).xyz;
  viewNormal = normalize(mat3(u_mv) * normal);
  vertexColor = color;
  vUV = uv;

  /* Tangent-plane direction and magnitude the render layer resolved */
  vFlowDir = direction;
  vFlowMag = magnitude;
}
