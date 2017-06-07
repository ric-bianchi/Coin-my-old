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

/*!
  \class SoTextureCoordinateObject SoTextureCoordinateObject.h Inventor/nodes/SoTextureCoordinateObject.h
  \brief The SoTextureCoordinateObject class generates texture coordinates by...
  \ingroup nodes

  FIXME: not implemented yet. pederb, 2005-04-20

  <b>FILE FORMAT/DEFAULTS:</b>
  \code
    TextureCoordinateObject {
        factorS 1 0 0 0
        factorT 0 1 0 0
        factorR 0 0 1 0
        factorQ 0 0 0 1
    }
  \endcode
*/

// *************************************************************************

#include <Inventor/nodes/SoTextureCoordinateObject.h>
#include "coindefs.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <Inventor/elements/SoGLMultiTextureCoordinateElement.h>
#include <Inventor/elements/SoTextureUnitElement.h>
#include <Inventor/system/gl.h>
#include <Inventor/C/glue/gl.h>

#include "nodes/SoSubNodeP.h"

/*!
  \var SoSFVec3f SoTextureCoordinateObject::factorS
  
  FIXME doc.
*/
/*!
  \var SoSFVec3f SoTextureCoordinateObject::factorT
  
  FIXME doc.
*/
/*!
  \var SoSFVec3f SoTextureCoordinateObject::factorR
  
  FIXME doc.
*/
/*!
  \var SoSFVec3f SoTextureCoordinateObject::factorQ
  
  FIXME doc.
*/

// *************************************************************************

SO_NODE_SOURCE(SoTextureCoordinateObject);

/*!
  Constructor.
*/
SoTextureCoordinateObject::SoTextureCoordinateObject()
{
  SO_NODE_INTERNAL_CONSTRUCTOR(SoTextureCoordinateObject);

  SO_NODE_ADD_FIELD(factorS, (1.0f, 0.0f, 0.0f, 0.0f));
  SO_NODE_ADD_FIELD(factorT, (0.0f, 1.0f, 0.0f, 0.0f));
  SO_NODE_ADD_FIELD(factorR, (0.0f, 0.0f, 1.0f, 0.0f));
  SO_NODE_ADD_FIELD(factorQ, (0.0f, 0.0f, 0.0f, 1.0f));
}

/*!
  Destructor.
*/
SoTextureCoordinateObject::~SoTextureCoordinateObject()
{
}

// doc from parent
void
SoTextureCoordinateObject::initClass(void)
{
  SO_NODE_INTERNAL_INIT_CLASS(SoTextureCoordinateObject, SO_FROM_INVENTOR_2_0);
}

// generates texture coordinates for callback and raypick action
const SbVec4f &
SoTextureCoordinateObject::generate(void * userdata,
                                    const SbVec3f & p,
                                    const SbVec3f & COIN_UNUSED_ARG(n))
{
  SoTextureCoordinateObject *thisp =
    (SoTextureCoordinateObject*) userdata;

  SbVec4f p4(p[0], p[1], p[2], 1.0f);
  thisp->dummy_object.setValue(thisp->factorS.getValue().dot(p4),
                               thisp->factorT.getValue().dot(p4),
                               thisp->factorR.getValue().dot(p4),
                               thisp->factorQ.getValue().dot(p4));
  return thisp->dummy_object;
}

// doc from parent
void
SoTextureCoordinateObject::doAction(SoAction * action)
{
  SoState * state = action->getState();
  int unit = SoTextureUnitElement::get(state);
  SoMultiTextureCoordinateElement::setFunction(action->getState(), this, unit,
                                               SoTextureCoordinateObject::generate,
                                               this);
}

// doc from parent
void
SoTextureCoordinateObject::GLRender(SoGLRenderAction * action)
{
  SoState * state = action->getState();
  int unit = SoTextureUnitElement::get(state);

  const cc_glglue * glue = cc_glglue_instance(SoGLCacheContextElement::get(state));
  int maxunits = cc_glglue_max_texture_units(glue);
  if (unit < maxunits) {        
    SoGLMultiTextureCoordinateElement::setTexGen(action->getState(),
                                                 this, unit,
                                                 SoTextureCoordinateObject::handleTexgen,
                                                 this,
                                                 SoTextureCoordinateObject::generate,
                                                 this);
  }
}

// doc from parent
void
SoTextureCoordinateObject::callback(SoCallbackAction * action)
{
  SoTextureCoordinateObject::doAction((SoAction *)action);
}

// doc from parent
void
SoTextureCoordinateObject::pick(SoPickAction * action)
{
  SoTextureCoordinateObject::doAction((SoAction *)action);
}

// texgen callback. Turns on plane texgen in OpenGL
void
SoTextureCoordinateObject::handleTexgen(void * data)
{
  SoTextureCoordinateObject *thisp = (SoTextureCoordinateObject*)data;
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

  const SbVec4f & s = thisp->factorS.getValue();
  glTexGenfv(GL_S, GL_OBJECT_PLANE, s.getValue());

  const SbVec4f & t = thisp->factorT.getValue();
  glTexGenfv(GL_T, GL_OBJECT_PLANE, t.getValue());
  
  const SbVec4f & r = thisp->factorR.getValue();
  glTexGenfv(GL_R, GL_OBJECT_PLANE, r.getValue());

  const SbVec4f & q = thisp->factorQ.getValue();
  glTexGenfv(GL_Q, GL_OBJECT_PLANE, q.getValue());
}
