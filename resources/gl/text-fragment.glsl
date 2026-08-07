#version 150
uniform sampler2D tex;
/* Tint multiplied into the sampled texel; white leaves the texture as-is */
uniform vec3 u_color;
centroid in vec2 v_texcoord;
out vec4 fragColor;

void main() {
  vec4 color = texture(tex, v_texcoord);

  /* Discard fully-transparent texels to prevent depth buffer writes;
   * threshold preserves anti-aliased glyph edges */
  if (color.a < 0.001) discard;

  fragColor = vec4(color.rgb * u_color, color.a);
}
