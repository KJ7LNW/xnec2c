#version 150

uniform float u_alpha;
uniform float u_color_dim;
uniform sampler2D u_peel_depth;
uniform int u_peel_pass;
uniform sampler2D u_layer_depth;
uniform int u_coplanar_pass;

#if defined(FLOW_MARK_LIC)
uniform sampler2D noise_tex;

/* Texture translation carrying the convolution mark along the flow */
uniform float u_scroll_phase;
#endif

/* Round-trip quantization step for GL_DEPTH_COMPONENT24: 1/2^24.
 * Covers write→24-bit→read error without manual bias.
 * Shared with ground-plane-fragment.glsl. */
const float DEPTH_QUANT_STEP = 6e-8;

centroid in vec4 vertexColor;
centroid in vec3 viewNormal;
centroid in vec3 viewPos;
centroid in vec2 vUV;
centroid in vec2 vFlowDir;
centroid in float vFlowMag;
out vec4 fragColor;

/* Lighting constants — normalize(vec3(-0.3, 0.5, 1.0)) precomputed */
const vec3 LIGHT_DIR = vec3(-0.2592106738356498, 0.4320177897260830, 0.8640355794521660);
const float SPECULAR_POWER = 32.0;
const float SPECULAR_INTENSITY = 0.7;
const float RIM_POWER = 2.0;
const float RIM_INTENSITY = 0.5;
const float SHADE_MIN = 0.5;
const float SHADE_RANGE = 0.5;
const float SATURATION_BOOST = 1.6;
const float BRIGHTNESS_BOOST = 1.15;
const vec3 LUMA_WEIGHTS = vec3(0.299, 0.587, 0.114);

/* Chevron parameters */
const float CHEVRON_FREQ = 6.0;
const float CHEVRON_MIN_CONTRAST = 0.12;
const float CHEVRON_MAX_CONTRAST = 0.55;

/* LIC parameters: screen-space integration with 1-pixel steps.
 * 41 samples (±20 steps) produce streaks ~40 pixels long.
 * DLIC_SPEED scales phase offset for flow animation.
 * NOISE_TEX_SIZE must match NOISE_SIZE in opengl_view_scene.c. */
const float NOISE_TEX_SIZE = 256.0;
const int LIC_STEPS = 20;
const float DLIC_SPEED = 0.5;

#if defined(FLOW_MARK_DIRECTIONAL) || \
    defined(FLOW_MARK_BIDIRECTIONAL) || defined(FLOW_MARK_LIC)
bool float_greater_eps(float a, float b, float epsilon)
{
  return a > b + epsilon;
}

/* Apply the compiled flow mark to one lit patch fragment. */
vec3 apply_flow_mark(vec3 color)
{
  if (!float_greater_eps(vFlowMag, -1e-6, 1e-6))
    return color;

#if defined(FLOW_MARK_LIC)
  /* Convert the tangent-plane direction through the UV-to-screen Jacobian. */
  vec2 duv_dx = dFdx(vUV);
  vec2 duv_dy = dFdy(vUV);
  float det = duv_dx.x * duv_dy.y - duv_dx.y * duv_dy.x;
  vec2 flow_screen;

  if (float_greater_eps(abs(det), 0.0, 1e-8))
  {
    flow_screen = vec2(
        duv_dy.y * vFlowDir.x - duv_dx.y * vFlowDir.y,
       -duv_dy.x * vFlowDir.x + duv_dx.x * vFlowDir.y) / det;
    flow_screen = normalize(flow_screen);
  }
  else
  {
    flow_screen = vFlowDir;
  }

  /* Integrate screen-space noise while translating it along the flow. */
  vec2 noise_uv = gl_FragCoord.xy / NOISE_TEX_SIZE;
  vec2 step = flow_screen / NOISE_TEX_SIZE;
  vec2 phase_offset =
      flow_screen * u_scroll_phase * DLIC_SPEED / NOISE_TEX_SIZE;
  float acc = 0.0;
  vec2 pos = noise_uv + phase_offset - step * float(LIC_STEPS);

  for (int k = 0; k < 2 * LIC_STEPS + 1; k++)
  {
    acc += texture(noise_tex, pos).r;
    pos += step;
  }

  acc /= float(2 * LIC_STEPS + 1);
  acc = clamp((acc - 0.5) * 3.0 + 0.5, 0.0, 1.0);

  /* Modulate the lit color by convolution contrast and flow magnitude. */
  float lic_strength = mix(0.3, 0.9, vFlowMag);
  color *= mix(1.0 - lic_strength, 1.0 + lic_strength, acc);
#else
  /* Rotate UV coordinates with the resolved unit direction directly. */
  vec2 centered = vUV - vec2(0.5);
  vec2 rotated = vec2(
      centered.x * vFlowDir.x + centered.y * vFlowDir.y,
     -centered.x * vFlowDir.y + centered.y * vFlowDir.x);
  float chevron;

#if defined(FLOW_MARK_BIDIRECTIONAL)
  float row_fwd = fract( rotated.x * CHEVRON_FREQ);
  float row_rev = fract(-rotated.x * CHEVRON_FREQ);
  float chev_fwd = abs(rotated.y) * 2.0 + row_fwd;
  float chev_rev = abs(rotated.y) * 2.0 + row_rev;
  chev_fwd = 1.0 - smoothstep(0.45, 0.55, fract(chev_fwd));
  chev_rev = 1.0 - smoothstep(0.45, 0.55, fract(chev_rev));
  chevron = max(chev_fwd, chev_rev);
#else
  float row = fract(rotated.x * CHEVRON_FREQ);
  chevron = abs(rotated.y) * 2.0 + row;
  chevron = 1.0 - smoothstep(0.45, 0.55, fract(chevron));
#endif

  /* Darken chevron marks in proportion to the resolved magnitude. */
  float contrast = mix(CHEVRON_MIN_CONTRAST,
                       CHEVRON_MAX_CONTRAST, vFlowMag);
  color = mix(color, color * 0.4, chevron * contrast);
#endif

  return color;
}
#endif

void main() {
  /* Screen-space depth gradient: shared by peel discard (MSAA
   * resolve offset bound) and coplanar tolerance (layer depth
   * was resolved to single-sample; gl_FragCoord.z is per-sample). */
  float dz = max(abs(dFdx(gl_FragCoord.z)),
                 abs(dFdy(gl_FragCoord.z)));

  /* Depth-peel discard: for passes > 0, reject fragments at or
   * nearer than the previous layer's depth.  Pass 0 renders the
   * nearest transparent layer with no discard.
   *
   * Epsilon covers two round-trip error sources:
   *   1. 24-bit depth texture quantization (DEPTH_QUANT_STEP)
   *   2. MSAA resolve offset — bounded by taxicab sample-to-center
   *      distance (<=1.0) times dz.
   *
   * Wire/patch depth ordering uses glPolygonOffset (hardware
   * slope-scaled bias) instead of manual gl_FragDepth.  Polygon
   * offset factor=2.0 exceeds this epsilon's dz coefficient (1.0),
   * providing margin of dz+r at all zoom levels. */
  if (u_peel_pass > 0)
  {
    float prev_z = texelFetch(u_peel_depth,
        ivec2(gl_FragCoord.xy), 0).r;
    float eps = max(DEPTH_QUANT_STEP, dz);
    if (gl_FragCoord.z <= prev_z + eps) discard;
  }

  /* Coplanar accumulation sub-pass: accept only fragments within
   * tolerance of the discovered layer depth.  The tolerance must
   * match the peel epsilon (dz-based) because layer_depth_tex is
   * single-sample resolved while gl_FragCoord.z is per-sample —
   * the MSAA resolve offset between them is bounded by dz. */
  if (u_coplanar_pass > 0)
  {
    float layer_z = texelFetch(u_layer_depth,
        ivec2(gl_FragCoord.xy), 0).r;
    float coplanar_tol = max(DEPTH_QUANT_STEP, dz);
    if (abs(gl_FragCoord.z - layer_z) > coplanar_tol) discard;
  }

  vec3 lightDir = LIGHT_DIR;
  vec3 norm = normalize(viewNormal);
  vec3 viewDir = normalize(-viewPos);
  vec3 baseColor = vertexColor.rgb * u_color_dim;

  /* Two-sided lighting: flip normal for back-facing fragments so
   * thin surfaces (patches) illuminate identically from both sides.
   * For closed surfaces (cylinders), back faces are depth-occluded
   * by front faces, so the flip has no visual effect.
   * NdotV reused below for rim lighting (guaranteed >= 0 after flip). */
  float NdotV = dot(norm, viewDir);
  if (NdotV < 0.0)
  {
    norm = -norm;
    NdotV = -NdotV;
  }

  /* Diffuse lighting */
  float diff = max(dot(norm, lightDir), 0.0);

  /* Specular highlight — explicit squaring chain guarantees 5 multiplies
   * on all drivers (pow may fall back to exp2/log2 transcendentals) */
  vec3 halfDir = normalize(lightDir + viewDir);
  float NdotH = max(dot(norm, halfDir), 0.0);
  float spec = NdotH * NdotH;
  spec *= spec;
  spec *= spec;
  spec *= spec;
  spec *= spec;

  /* Rim lighting — NdotV from two-sided flip, already >= 0 */
  float rim = 1.0 - NdotV;
  rim = rim * rim * RIM_INTENSITY;

  /* Shading - never below SHADE_MIN to preserve vibrancy */
  float shade = SHADE_MIN + diff * SHADE_RANGE;
  vec3 color = baseColor * shade;

  /* Saturation boost */
  float luma = dot(color, LUMA_WEIGHTS);
  color = mix(vec3(luma), color, SATURATION_BOOST);

  /* Add specular and rim */
  color += vec3(spec * SPECULAR_INTENSITY);
  color += baseColor * rim;

  /* Final brightness boost */
  color *= BRIGHTNESS_BOOST;

  /* Flow marks apply after lighting so their contrast is independent of
   * viewing angle.  Programs compiled without a mark leave the lit color. */
#if defined(FLOW_MARK_DIRECTIONAL) || \
    defined(FLOW_MARK_BIDIRECTIONAL) || defined(FLOW_MARK_LIC)
  color = apply_flow_mark(color);
#endif

  fragColor = vec4(clamp(color, 0.0, 1.0),
                   vertexColor.a * u_alpha);
}
