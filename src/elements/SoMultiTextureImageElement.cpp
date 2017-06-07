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
  \class SoMultiTextureImageElement Inventor/elements/SoMultiTextureImageElement.h
  \brief The SoMultiTextureImageElement class is yet to be documented.
  \ingroup elements

  FIXME: write doc.

  \COIN_CLASS_EXTENSION

  \since Coin 2.2
*/

#include "coindefs.h"
#include "SbBasicP.h"

#include <Inventor/elements/SoMultiTextureImageElement.h>
#include <Inventor/elements/SoGLMultiTextureImageElement.h>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/misc/SoGLImage.h>
#include <Inventor/SbImage.h>
#include <Inventor/lists/SbList.h>
#include <cassert>

class SoMultiTextureImageElementP {
public:
  void ensureCapacity(int unit) const {
    while (unit >= this->unitdata.getLength()) {
      this->unitdata.append(SoMultiTextureImageElement::UnitData());
    }
  }
  mutable SbList<SoMultiTextureImageElement::UnitData> unitdata;
};

SO_ELEMENT_CUSTOM_CONSTRUCTOR_SOURCE(SoMultiTextureImageElement);

#define PRIVATE(obj) obj->pimpl

SoMultiTextureImageElement::SoMultiTextureImageElement(void)
{
  PRIVATE(this) = new SoMultiTextureImageElementP;

  this->setTypeId(SoMultiTextureImageElement::classTypeId);
  this->setStackIndex(SoMultiTextureImageElement::classStackIndex);
}

/*!
  This static method initializes static data for the
  SoMultiTextureImageElement class.
*/

void
SoMultiTextureImageElement::initClass(void)
{
  SO_ELEMENT_INIT_CLASS(SoMultiTextureImageElement, inherited);
}

/*!
  The destructor.
*/

SoMultiTextureImageElement::~SoMultiTextureImageElement()
{
  delete PRIVATE(this);
}

//! FIXME: write doc.

void
SoMultiTextureImageElement::init(SoState * state)
{
  inherited::init(state);
}

/*!
  Resets this element to its original values.
*/
void
SoMultiTextureImageElement::setDefault(SoState * const state, SoNode * const COIN_UNUSED_ARG(node), const int unit)
{
  SoMultiTextureImageElement * elem =
    coin_safe_cast<SoMultiTextureImageElement *>
    (state->getElement(classStackIndex));
  PRIVATE(elem)->ensureCapacity(unit);
  PRIVATE(elem)->unitdata[unit] = UnitData();  
}

//! FIXME: write doc.
void
SoMultiTextureImageElement::set(SoState * const state, SoNode * const node,
                                const int unit,
                                const SbVec2s & size, const int numComponents,
                                const unsigned char * bytes,
                                const Wrap wrapS,
                                const Wrap wrapT,
                                const Model model,
                                const SbColor & blendColor)
{
  SoMultiTextureImageElement * elem =
    coin_safe_cast<SoMultiTextureImageElement *>
    (state->getElement(classStackIndex));

  PRIVATE(elem)->ensureCapacity(unit);
  elem->setElt(unit, node->getNodeId(), size, numComponents, bytes, wrapS, wrapT,
               model, blendColor);
}

/*!
  FIXME: write doc.

  \COIN_FUNCTION_EXTENSION
*/
void
SoMultiTextureImageElement::set(SoState * const state, SoNode * const node,
                                const int unit,
                                const SbVec3s & size, const int numComponents,
                                const unsigned char * bytes,
                                const Wrap wrapS,
                                const Wrap wrapT,
                                const Wrap wrapR,
                                const Model model,
                                const SbColor & blendColor)
{
  SoMultiTextureImageElement * elem = coin_safe_cast<SoMultiTextureImageElement *>
    (state->getElement(classStackIndex));
  
  PRIVATE(elem)->ensureCapacity(unit);
  elem->setElt(unit, node->getNodeId(), size, numComponents, bytes, wrapS, wrapT, wrapR,
               model, blendColor);
}

//! FIXME: write doc.

const unsigned char *
SoMultiTextureImageElement::get(SoState * const state,
                                const int unit,
                                SbVec2s & size,
                                int & numComponents,
                                Wrap & wrapS,
                                Wrap & wrapT,
                                Model & model,
                                SbColor &blendColor)
{
  const SoMultiTextureImageElement * elem =
    coin_assert_cast<const SoMultiTextureImageElement *>
    (getConstElement(state, classStackIndex));

  PRIVATE(elem)->ensureCapacity(unit);
  const UnitData & ud = PRIVATE(elem)->unitdata[unit];
  
  wrapS = ud.wrapS;
  wrapT = ud.wrapT;
  model = ud.model;
  blendColor = ud.blendColor;
  
  return getImage(state, unit, size, numComponents);
}

/*!
  FIXME: write doc.

  \COIN_FUNCTION_EXTENSION

*/
const unsigned char *
SoMultiTextureImageElement::get(SoState * const state,
                                const int unit,
                                SbVec3s & size,
                                int & numComponents,
                                Wrap & wrapS,
                                Wrap & wrapT,
                                Wrap & wrapR,
                                Model & model,
                                SbColor &blendColor)
{
  const SoMultiTextureImageElement * elem =
    coin_assert_cast<const SoMultiTextureImageElement *>
    (getConstElement(state, classStackIndex));
  
  PRIVATE(elem)->ensureCapacity(unit);
  const UnitData & ud = PRIVATE(elem)->unitdata[unit];
  
  wrapS = ud.wrapS;
  wrapT = ud.wrapT;
  wrapR = ud.wrapR;
  model = ud.model;
  blendColor = ud.blendColor;

  return getImage(state, unit, size, numComponents);
}

/*!
  FIXME: write doc
*/
const unsigned char *
SoMultiTextureImageElement::getImage(SoState * const state,
                                     const int unit,
                                     SbVec2s &size,
                                     int & numComponents)
{
  const SoMultiTextureImageElement * elem =
    coin_assert_cast<const SoMultiTextureImageElement *>
    (getConstElement(state, classStackIndex));

  PRIVATE(elem)->ensureCapacity(unit);
  const UnitData & ud = PRIVATE(elem)->unitdata[unit];
  
  size.setValue(ud.size[0], ud.size[1]);
  numComponents = ud.numComponents;
  return ud.bytes;
}

/*!
  FIXME: write doc
*/
const unsigned char *
SoMultiTextureImageElement::getImage(SoState * const state,
                                     const int unit,
                                     SbVec3s & size,
                                     int & numComponents)
{
  const SoMultiTextureImageElement * elem =
    coin_assert_cast<const SoMultiTextureImageElement *>
    (getConstElement(state, classStackIndex));

  PRIVATE(elem)->ensureCapacity(unit);
  const UnitData & ud = PRIVATE(elem)->unitdata[unit];
  
  size = ud.size;
  numComponents = ud.numComponents;
  return ud.bytes;
}


//! FIXME: write doc.

SbBool
SoMultiTextureImageElement::containsTransparency(SoState * const state)
{
  const SoMultiTextureImageElement * elem =
    coin_assert_cast<const SoMultiTextureImageElement *>
    (getConstElement(state, classStackIndex));
  
  for (int i = 0; i < PRIVATE(elem)->unitdata.getLength(); i++) {
    if (elem->hasTransparency(i)) return TRUE;
  }
  return FALSE;
}

/*!
  Called by containsTransparency(). Returns \e TRUE if image data has
  transparency. Default method does a very poor job of detecting this,
  since it returns \e TRUE when the number of components are 2 or
  4. Override whenever it is important to know this
  (SoGLTextureImageElement overrides it to avoid transparency handling
  where possible).
*/
SbBool
SoMultiTextureImageElement::hasTransparency(const int unit) const
{
  if (unit < PRIVATE(this)->unitdata.getLength()) {
    const UnitData & ud = PRIVATE(this)->unitdata[unit];
    return (ud.numComponents==2 || ud.numComponents==4);
  }
  return FALSE;
}

const SoMultiTextureImageElement::UnitData &
SoMultiTextureImageElement::getUnitData(const int unit) const
{
  assert(unit < PRIVATE(this)->unitdata.getLength());
  return PRIVATE(this)->unitdata[unit];
}

/*!
  Get the number of units with image data set.
*/
int 
SoMultiTextureImageElement::getNumUnits() const
{
  return PRIVATE(this)->unitdata.getLength();
}

//! FIXME: write doc.

const unsigned char *
SoMultiTextureImageElement::getDefault(SbVec2s & size, int & numComponents)
{
  size.setValue(0,0);
  numComponents = 0;
  return NULL;
}

/*!
  FIXME: write doc.

*/
const unsigned char *
SoMultiTextureImageElement::getDefault(SbVec3s & size, int & numComponents)
{
  size.setValue(0,0,0);
  numComponents = 0;
  return NULL;
}

void
SoMultiTextureImageElement::push(SoState * COIN_UNUSED_ARG(state))
{
  const SoMultiTextureImageElement * prev =
    coin_assert_cast<SoMultiTextureImageElement *>
    (this->getNextInStack());

  PRIVATE(this)->unitdata = PRIVATE(prev)->unitdata;
}

SbBool
SoMultiTextureImageElement::matches(const SoElement * elem) const
{
  const SoMultiTextureImageElement * e =
    coin_assert_cast<const SoMultiTextureImageElement *>
    (elem);
  const int n = PRIVATE(e)->unitdata.getLength();
  if (n != PRIVATE(this)->unitdata.getLength()) return FALSE;

  for (int i = 0; i < n; i++) {
    if (PRIVATE(e)->unitdata[i].nodeid != PRIVATE(this)->unitdata[i].nodeid) {
      return FALSE;
    }
  }
  return TRUE;
}

SoElement *
SoMultiTextureImageElement::copyMatchInfo(void) const
{
  SoMultiTextureImageElement * elem = static_cast<SoMultiTextureImageElement *>
    (getTypeId().createInstance());
  PRIVATE(elem)->unitdata = PRIVATE(this)->unitdata;
  return elem;
}


//! FIXME: write doc.

void
SoMultiTextureImageElement::setElt(const int unit,
                                   const uint32_t nodeid,
                                   const SbVec2s &size, const int numComponents,
                                   const unsigned char * bytes,
                                   const Wrap wrapS,
                                   const Wrap wrapT,
                                   const Model model,
                                   const SbColor &blendColor)
{
  assert(unit < PRIVATE(this)->unitdata.getLength());
  UnitData & ud = PRIVATE(this)->unitdata[unit];
  
  ud.nodeid = nodeid;
  ud.size.setValue(size[0],size[1],1);
  ud.numComponents = numComponents;
  ud.bytes = bytes;
  ud.wrapS = wrapS;
  ud.wrapT = wrapT;
  ud.wrapR = REPEAT;
  ud.model = model;
  ud.blendColor = blendColor;
}

/*!
  FIXME: write doc.
*/
void
SoMultiTextureImageElement::setElt(const int unit,
                                   const uint32_t nodeid,
                                   const SbVec3s & size, const int numComponents,
                                   const unsigned char * bytes,
                                   const Wrap wrapS,
                                   const Wrap wrapT,
                                   const Wrap wrapR,
                                   const Model model,
                                   const SbColor &blendColor)
{
  assert(unit < PRIVATE(this)->unitdata.getLength());
  UnitData & ud = PRIVATE(this)->unitdata[unit];

  ud.nodeid = nodeid;
  ud.size = size;
  ud.numComponents = numComponents;
  ud.bytes = bytes;
  ud.wrapS = wrapS;
  ud.wrapT = wrapT;
  ud.wrapR = wrapR;
  ud.model = model;
  ud.blendColor = blendColor;
}

/*!
  FIXME: write doc.
*/
const SbColor &
SoMultiTextureImageElement::getBlendColor(SoState * const state, const int unit)
{
  const SoMultiTextureImageElement * elem =
    coin_assert_cast<const SoMultiTextureImageElement *>
    (getConstElement(state, classStackIndex));

  PRIVATE(elem)->ensureCapacity(unit);
  return PRIVATE(elem)->unitdata[unit].blendColor;
}

/*!
  FIXME: write doc.
*/
SoMultiTextureImageElement::Wrap
SoMultiTextureImageElement::getWrapS(SoState * const state, const int unit)
{
  const SoMultiTextureImageElement * elem =
    coin_assert_cast<const SoMultiTextureImageElement *>
    (getConstElement(state, classStackIndex));

  PRIVATE(elem)->ensureCapacity(unit);
  return PRIVATE(elem)->unitdata[unit].wrapT;
}

/*!
  FIXME: write doc.
*/
SoMultiTextureImageElement::Wrap
SoMultiTextureImageElement::getWrapT(SoState * const state, const int unit)
{
  const SoMultiTextureImageElement * elem =
    coin_assert_cast<const SoMultiTextureImageElement *>
    (getConstElement(state, classStackIndex));

  PRIVATE(elem)->ensureCapacity(unit);
  return PRIVATE(elem)->unitdata[unit].wrapS;
}

/*!
  FIXME: write doc.
*/
SoMultiTextureImageElement::Wrap
SoMultiTextureImageElement::getWrapR(SoState * const state, const int unit)
{
  const SoMultiTextureImageElement * elem =
    coin_assert_cast<const SoMultiTextureImageElement *>
    (getConstElement(state, classStackIndex));

  PRIVATE(elem)->ensureCapacity(unit);
  return PRIVATE(elem)->unitdata[unit].wrapR;
}

/*!
  FIXME: write doc.
*/
SoMultiTextureImageElement::Model
SoMultiTextureImageElement::getModel(SoState * const state, const int unit)
{
  const SoMultiTextureImageElement * elem =
    coin_assert_cast<const SoMultiTextureImageElement *>
    (getConstElement(state, classStackIndex));

  PRIVATE(elem)->ensureCapacity(unit);
  return PRIVATE(elem)->unitdata[unit].model;
}

SoMultiTextureImageElement::UnitData::UnitData()
  : nodeid(0),
    size(0,0,0),
    numComponents(0),
    bytes(0),
    wrapS(SoMultiTextureImageElement::REPEAT), 
    wrapT(SoMultiTextureImageElement::REPEAT), 
    wrapR(SoMultiTextureImageElement::REPEAT),
    model(SoMultiTextureImageElement::MODULATE),
    blendColor(0.0f, 0.0f, 0.0f)
{
}

SoMultiTextureImageElement::UnitData::UnitData(const UnitData & org)
  : nodeid(org.nodeid),
    size(org.size),
    numComponents(org.numComponents),
    bytes(org.bytes),
    wrapS(org.wrapS),
    wrapT(org.wrapT), 
    wrapR(org.wrapR),
    model(org.model),
    blendColor(org.blendColor)
{
}

#undef PRIVATE
