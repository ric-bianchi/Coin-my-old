#ifndef COIN_GLUE_INTERNAL_WGL_H
#define COIN_GLUE_INTERNAL_WGL_H

/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) by Kongsberg Oil & Gas Technologies.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using Coin with software that can not be combined with the GNU
 *  GPL, and for taking advantage of the additional benefits of our
 *  support services, please contact Kongsberg Oil & Gas Technologies
 *  about acquiring a Coin Professional Edition License.
 *
 *  See http://www.coin3d.org/ for more information.
 *
 *  Kongsberg Oil & Gas Technologies, Bygdoy Alle 5, 0257 Oslo, NORWAY.
 *  http://www.sim.no/  sales@sim.no  coin-support@coin3d.org
 *
\**************************************************************************/

#ifndef COIN_INTERNAL
#error this is a private header file
#endif

#include <Inventor/SbBasic.h> // SbBool
#include <Inventor/C/glue/gl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void * coin_wgl_getprocaddress(const cc_glglue * glue, const char * fname);

void * wglglue_context_create_offscreen(unsigned int width, unsigned int height, SbBool texture = TRUE);
SbBool wglglue_context_make_current(void * ctx);
void wglglue_context_reinstate_previous(void * ctx);
void wglglue_context_destruct(void * ctx);

void wglglue_context_bind_pbuffer(void * ctx);
void wglglue_context_release_pbuffer(void * ctx);
SbBool wglglue_context_pbuffer_is_bound(void * ctx);
SbBool wglglue_context_can_render_to_texture(void * ctx);

SbBool wglglue_context_pbuffer_max(void * ctx, unsigned int * lims);

/* This abomination is needed to support SoOffscreenRenderer::getDC(). */
const void * wglglue_context_win32_HDC(void * ctx);
void wglglue_copy_to_bitmap_win32_HDC(void * ctx);

void wglglue_cleanup(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !COIN_GLUE_INTERNAL_WGL_H */
