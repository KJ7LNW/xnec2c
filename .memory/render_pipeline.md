# Render pipeline and backends

Apply each line when changing prerender, render-layer dispatch, or render backends, OpenGL framebuffer handling included.

## User-given

prerender -> render -> {cairo,opengl,...}

prerender contains child rendered static and municable values that are calculated in parallel, partitioned by frequency. These are passed to the parent, to the rendering abstraction, which makes choices about what should be rendered and passes them to the and rendering back ends. The rendering backend calling conventions are uniform and basically do what they're told. There shouldn't be any domain-specific branching inside the back-end. Those choices all have to happen at the render layer.

- Backends are leaf functions; zero content-flag evaluation, zero mode branching; Dispatch resolves all content flags into data pointers (colors, field sets, params) before calling backend vtable entries. Engine specific configurations are allowed to access engine specific configuration flags directly. 
- Never blit into, resolve into, or assume the sample count, format, or completeness of the GTK-provided default_fbo. Resolve multisample color into a self-allocated single-sample FBO, then write it into default_fbo with a shader copy; a direct multisample blit into default_fbo is rejected under Wayland/EGL.
