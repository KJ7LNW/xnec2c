/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  The official website and doumentation for xnec2c is available here:
 *    https://www.xnec2c.org/
 */

#include "opengl_renderer.h"
#include "../shared.h"

#ifdef HAVE_OPENGL

/*-----------------------------------------------------------------------*/

/** compile_shader() - Compiles a shader from GResource path or filesystem path
 * @type: shader type (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)
 * @path: GResource path or filesystem path to shader source
 */
  static char *
shader_source_with_prologue(const char *source, const char *prologue,
    const char *path)
{
  GRegex *version_re;
  GMatchInfo *match;
  char *composed;
  gint version_end;

  /* The prologue must follow the version directive, which GLSL requires to
   * precede every other statement. */
  version_re = g_regex_new("^[ \\t]*#version[^\\n]*\\n", G_REGEX_MULTILINE,
      0, NULL);
  match = NULL;
  version_end = -1;

  if( g_regex_match(version_re, source, 0, &match) )
    g_match_info_fetch_pos(match, 0, NULL, &version_end);

  g_match_info_free(match);
  g_regex_unref(version_re);

  if( version_end < 0 )
  {
    pr_err("Shader declares no version directive: %s\n", path);
    return( NULL );
  }

  composed = g_strdup_printf("%.*s%s\n%s", version_end, source, prologue,
      source + version_end);

  return( composed );

} /* shader_source_with_prologue() */

/*-----------------------------------------------------------------------*/

/** compile_shader() - Compile one shader stage from a resource or file
 * @type: shader stage to compile
 * @path: resource path or filesystem path holding the source
 * @prologue: variant definitions inserted after the version directive,
 *            or NULL to compile the source unmodified
 */
  static GLuint
compile_shader(GLenum type, const char *path, const char *prologue)
{
  char *composed;
  GBytes *bytes;
  const char *source;
  char *file_source;
  gsize file_size;
  GLuint shader;
  GLint status, len;
  char *log;
  GError *error;
  gboolean from_file;

  error = NULL;
  bytes = g_resources_lookup_data(path, 0, &error);
  from_file = FALSE;
  file_source = NULL;

  if( !bytes )
  {
    g_clear_error(&error);

    if( !g_file_get_contents(path, &file_source, &file_size, &error) )
    {
      pr_err("Failed to load shader: %s (%s)\n", path,
        error ? error->message : "unknown error");
      g_clear_error(&error);
      return( 0 );
    }

    source = file_source;
    from_file = TRUE;
  }
  else
  {
    source = g_bytes_get_data(bytes, NULL);
  }

  composed = NULL;

  if( prologue != NULL )
  {
    composed = shader_source_with_prologue(source, prologue, path);

    if( composed == NULL )
    {
      if( from_file )
        g_free(file_source);
      else
        g_bytes_unref(bytes);

      return( 0 );
    }

    source = composed;
  }

  shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  g_free(composed);

  if( from_file )
    g_free(file_source);
  else
    g_bytes_unref(bytes);

  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if( status == GL_FALSE )
  {
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    log = g_malloc(len + 1);
    glGetShaderInfoLog(shader, len, NULL, log);
    pr_err("Shader compile error: %s: %s\n", path, log);
    g_free(log);
    glDeleteShader(shader);
    return( 0 );
  }

  return( shader );

} /* compile_shader() */

/*-----------------------------------------------------------------------*/

/** gl_shader_load() - Loads and compiles vertex and fragment shaders
 * @shader: shader structure to populate
 * @spec: shader sources and the optional fragment variant prologue
 */
  gboolean
gl_shader_load(gl_shader_t *shader, const gl_shader_spec_t *spec)
{
  GLint status;

  shader->vertex = compile_shader(GL_VERTEX_SHADER, spec->vertex_path, NULL);
  if( !shader->vertex )
    return( FALSE );

  shader->fragment = compile_shader(GL_FRAGMENT_SHADER, spec->fragment_path,
      spec->fragment_prologue);
  if( !shader->fragment )
  {
    glDeleteShader(shader->vertex);
    return( FALSE );
  }

  shader->program = glCreateProgram();
  glAttachShader(shader->program, shader->vertex);
  glAttachShader(shader->program, shader->fragment);
  glLinkProgram(shader->program);

  glGetProgramiv(shader->program, GL_LINK_STATUS, &status);
  if( status == GL_FALSE )
  {
    pr_err("Shader link failed: %s + %s\n", spec->vertex_path,
        spec->fragment_path);
    glDeleteProgram(shader->program);
    glDeleteShader(shader->vertex);
    glDeleteShader(shader->fragment);
    return( FALSE );
  }

  return( TRUE );

} /* gl_shader_load() */

/*-----------------------------------------------------------------------*/

/** gl_shader_destroy() - Cleanup shader resources
 * @shader: shader structure to destroy
 */
  void
gl_shader_destroy(gl_shader_t *shader)
{
  if( shader->program )
    glDeleteProgram(shader->program);
  if( shader->vertex )
    glDeleteShader(shader->vertex);
  if( shader->fragment )
    glDeleteShader(shader->fragment);

  shader->program = shader->vertex = shader->fragment = 0;

} /* gl_shader_destroy() */

/*-----------------------------------------------------------------------*/

#endif /* HAVE_OPENGL */
