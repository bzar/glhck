#define _init_c_
#include "internal.h"
#include "render/render.h"

/* tracing channel for this file */
#define GLHCK_CHANNEL GLHCK_CHANNEL_GLHCK

/* \brief initialize */
GLHCKAPI int glhckInit(int argc, char **argv)
{
   CALL("%d, %p", argc, argv);

   if (_glhckInitialized)
      goto success;

   /* reset global state */
   memset(&_GLHCKlibrary,                 0, sizeof(__GLHCKlibrary));
   memset(&_GLHCKlibrary.render,          0, sizeof(__GLHCKrender));
   memset(&_GLHCKlibrary.render.api,      0, sizeof(__GLHCKrenderAPI));
   memset(&_GLHCKlibrary.texture,         0, sizeof(__GLHCKtexture));

   /* init trace system */
   _glhckTraceInit(argc, argv);
   atexit(glhckTerminate);
   _glhckInitialized = 1;

success:
   RET("%d", RETURN_OK);
   return RETURN_OK;
}

/* \brief creates virtual display and inits renderer */
GLHCKAPI int glhckCreateDisplay(int width, int height, glhckRenderType renderType)
{
   CALL("%d, %d, %d", width, height, renderType);

   if (!_glhckInitialized)
      goto fail;

   /* autodetect */
   if (renderType == GLHCK_RENDER_NONE)
      renderType = GLHCK_RENDER_OPENGL;

   /* close display if created already */
   if (_GLHCKlibrary.render.type == renderType) goto success;
   else glhckCloseDisplay();

   /* init renderer */
   if (renderType == GLHCK_RENDER_OPENGL)
      _glhckRenderOpenGL();

   /* check that initialization was success */
   if (!_GLHCKlibrary.render.name)
      goto fail;

   _GLHCKlibrary.render.type = renderType;

success:
   RET("%d", RETURN_OK);
   return RETURN_OK;

fail:
   RET("%d", RETURN_FAIL);
   return RETURN_FAIL;
}

/* \brief close the virutal display */
GLHCKAPI void glhckCloseDisplay(void)
{
   TRACE();

   /* nothing to terminate */
   if (!_glhckInitialized || !_GLHCKlibrary.render.name)
      return;

   _GLHCKlibrary.render.api.terminate();
   _GLHCKlibrary.render.type = GLHCK_RENDER_NONE;
}

/* \brief render scene using renderer */
GLHCKAPI void glhckRender(void)
{
   TRACE();

   /* can't render */
   if (!_glhckInitialized || !_GLHCKlibrary.render.name)
      return;

   _GLHCKlibrary.render.api.render();
}

/* \brief frees virtual display and deinits glue framework */
GLHCKAPI void glhckTerminate(void)
{
   TRACE();
   if (!_glhckInitialized) return;
   glhckCloseDisplay();
   _glhckTextureCacheRelease();
#ifndef NDEBUG
   _glhckTrackTerminate();
#endif
}
