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

#include <Inventor/scxml/SoScXMLStateMachine.h>

/*!
  \class SoScXMLStateMachine SoScXMLStateMachine.h Inventor/scxml/SoScXMLStateMachine.h
  \brief Integration level for using Coin events with SCXML-based state machines.

  This class is part of integrating the Coin types with the SCXML
  subsystem.  SoScXMLStateMachines adds convenience for using SCXML
  state machines together with Coin events.

  \since Coin 3.0
  \ingroup soscxml
*/

#include <boost/scoped_ptr.hpp>
#include <boost/intrusive_ptr.hpp>

#include <Inventor/SbString.h>
#include <Inventor/SbViewportRegion.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/events/SoEvents.h>
#include <Inventor/scxml/ScXML.h>
#include <Inventor/scxml/SoScXMLEvent.h>
#include "base/coinString.h"
#include "scxml/SbStringConvert.h"
#include "SbBasicP.h"

// *************************************************************************

class SoScXMLStateMachine::PImpl {
public:
  PImpl(void)
    : scenegraphroot(NULL), activecamera(NULL), viewport(100, 100)
  { }
  ~PImpl(void) { }

  // hold a couple of custom non-SoEvent-based events
  boost::scoped_ptr<ScXMLEvent> preGLRenderEvent;
  boost::scoped_ptr<ScXMLEvent> postGLRenderEvent;

  boost::intrusive_ptr<SoNode> scenegraphroot;
  boost::intrusive_ptr<SoCamera> activecamera;
  SbViewportRegion viewport;

  mutable SbString varstring;
};

#define PRIVATE(obj) ((obj)->pimpl)

SCXML_OBJECT_SOURCE(SoScXMLStateMachine);

void
SoScXMLStateMachine::initClass(void)
{
  SCXML_OBJECT_INIT_CLASS(SoScXMLStateMachine, ScXMLStateMachine, "ScXMLStateMachine");
}

void
SoScXMLStateMachine::cleanClass(void)
{
  SoScXMLStateMachine::classTypeId = SoType::badType();
}

SoScXMLStateMachine::SoScXMLStateMachine(void)
{
  // initialize custom events
  PRIVATE(this)->preGLRenderEvent.reset(new ScXMLEvent);
  PRIVATE(this)->preGLRenderEvent->setEventName("sim.coin3d.coin.GLRender.PRE_RENDER");
  PRIVATE(this)->postGLRenderEvent.reset(new ScXMLEvent);
  PRIVATE(this)->postGLRenderEvent->setEventName("sim.coin3d.coin.GLRender.POST_RENDER");
}

SoScXMLStateMachine::~SoScXMLStateMachine(void)
{
}

void
SoScXMLStateMachine::setSceneGraphRoot(SoNode * root)
{
  PRIVATE(this)->scenegraphroot = root;
}

SoNode *
SoScXMLStateMachine::getSceneGraphRoot(void) const
{
  return PRIVATE(this)->scenegraphroot.get();
}

void
SoScXMLStateMachine::setActiveCamera(SoCamera * camera)
{
  PRIVATE(this)->activecamera = camera;
}

SoCamera *
SoScXMLStateMachine::getActiveCamera(void) const
{
  return PRIVATE(this)->activecamera.get();
}

void
SoScXMLStateMachine::setViewportRegion(const SbViewportRegion & vp)
{
  PRIVATE(this)->viewport = vp;
}

const SbViewportRegion &
SoScXMLStateMachine::getViewportRegion(void) const
{
  return PRIVATE(this)->viewport;
}

void
SoScXMLStateMachine::preGLRender(void)
{
  this->queueEvent(PRIVATE(this)->preGLRenderEvent.get());
  this->processEventQueue();
}

void
SoScXMLStateMachine::postGLRender(void)
{
  this->queueEvent(PRIVATE(this)->postGLRenderEvent.get());
  this->processEventQueue();
}

/*!
  This function makes the state machine process an SoEvent.
*/
SbBool
SoScXMLStateMachine::processSoEvent(const SoEvent * event)
{
  // FIXME: Not sure if this check should be here and not somewhere else,
  // but removing this again makes us crash on NULL scenegraphs. kintel 20080729.
  if (PRIVATE(this)->scenegraphroot.get()) {
    boost::scoped_ptr<SoScXMLEvent> wrapperevent;
    wrapperevent.reset(new SoScXMLEvent);
    wrapperevent->setSoEvent(event);
    wrapperevent->setUpIdentifier();
    this->queueEvent(wrapperevent.get());
    return this->processEventQueue();
  }
  else {
    return false;
  }
}

/*!
  The string returned from this function is only valid until the next variable is
  requested.
*/
const char *
SoScXMLStateMachine::getVariable(const char * key) const
{
  if (strncmp(key, "_event.", 7) == 0) {
    // printf("scan for key '%s'\n", key);
    const char * subkey = key + 7;
    const ScXMLEvent * ev = this->getCurrentEvent();
    if (ev->isOfType(SoScXMLEvent::getClassTypeId())) {
      const SoScXMLEvent * soev = static_cast<const SoScXMLEvent *>(ev);
      const SoEvent * coinev = soev->getSoEvent();

      if (strcmp(subkey, "getTime()") == 0) {
        SbTime timeval = coinev->getTime();
        double doubletime = timeval.getValue();
        PRIVATE(this)->varstring = SbStringConvert::toString(doubletime);
        return PRIVATE(this)->varstring.getString();
      }

      else if (strcmp(subkey, "getPosition().x") == 0) {
        SbVec2s pos = coinev->getPosition();
        PRIVATE(this)->varstring = SbStringConvert::toString(static_cast<double>(pos[0]));
        return PRIVATE(this)->varstring.getString();
      }
      else if (strcmp(subkey, "getPosition().y") == 0) {
        SbVec2s pos = coinev->getPosition();
        PRIVATE(this)->varstring = SbStringConvert::toString(static_cast<double>(pos[1]));
        return PRIVATE(this)->varstring.getString();
      }
      else if (strcmp(subkey, "getPosition()") == 0) {
        SbVec2s pos = coinev->getPosition();
        PRIVATE(this)->varstring = SbStringConvert::toString(pos);
        return PRIVATE(this)->varstring.getString();
      }

      else if (strcmp(subkey, "getNormalizedPosition().x") == 0) {
        SbVec2f pos = coinev->getNormalizedPosition(this->getViewportRegion());
        PRIVATE(this)->varstring = SbStringConvert::toString(static_cast<double>(pos[0]));
        return PRIVATE(this)->varstring.getString();
      }
      else if (strcmp(subkey, "getNormalizedPosition().y") == 0) {
        SbVec2f pos = coinev->getNormalizedPosition(this->getViewportRegion());
        PRIVATE(this)->varstring = SbStringConvert::toString(static_cast<double>(pos[1]));
        return PRIVATE(this)->varstring.getString();
      }
      else if (strcmp(subkey, "getNormalizedPosition()") == 0) {
        SbVec2f pos = coinev->getNormalizedPosition(this->getViewportRegion());
        PRIVATE(this)->varstring = SbStringConvert::toString(pos);
        return PRIVATE(this)->varstring.getString();
      }

      else if (strcmp(subkey, "wasShiftDown()") == 0) {
        SbBool wasdown = coinev->wasShiftDown();
        PRIVATE(this)->varstring = SbStringConvert::toString<bool>(wasdown);
        return PRIVATE(this)->varstring.getString();
      }

      else if (strcmp(subkey, "wasCtrlDown()") == 0) {
        SbBool wasdown = coinev->wasCtrlDown();
        PRIVATE(this)->varstring = SbStringConvert::toString<bool>(wasdown);
        return PRIVATE(this)->varstring.getString();
      }

      else if (strcmp(subkey, "wasAltDown()") == 0) {
        SbBool wasdown = coinev->wasAltDown();
        PRIVATE(this)->varstring = SbStringConvert::toString<bool>(wasdown);
        return PRIVATE(this)->varstring.getString();
      }

      else if (strcmp(subkey, "getState()") == 0 && coinev->isOfType(SoButtonEvent::getClassTypeId())) {
        const SoButtonEvent * bevent = coin_assert_cast<const SoButtonEvent *>(coinev);
        SbString enumname;
        SoButtonEvent::enumToString(bevent->getState(), enumname);
        PRIVATE(this)->varstring.sprintf("'%s'", enumname.getString());
        return PRIVATE(this)->varstring.getString();
      }

      else if (strcmp(subkey, "getKey()") == 0 && coinev->isOfType(SoKeyboardEvent::getClassTypeId())) {
        const SoKeyboardEvent * kbevent = coin_assert_cast<const SoKeyboardEvent *>(coinev);
        SbString enumname;
        SoKeyboardEvent::enumToString(kbevent->getKey(), enumname);
        PRIVATE(this)->varstring.sprintf("'%s'", enumname.getString());
        return PRIVATE(this)->varstring.getString();
      }

      else if (strcmp(subkey, "getPrintableCharacter()") == 0 && coinev->isOfType(SoKeyboardEvent::getClassTypeId())) {
        const SoKeyboardEvent * kbevent = coin_assert_cast<const SoKeyboardEvent *>(coinev);
        char printable = kbevent->getPrintableCharacter();
        PRIVATE(this)->varstring.sprintf("'%c'", printable);
        return PRIVATE(this)->varstring.getString();
      }

      else if (strcmp(subkey, "getButton()") == 0 && coinev->isOfType(SoMouseButtonEvent::getClassTypeId())) {
        const SoMouseButtonEvent * mbevent = coin_assert_cast<const SoMouseButtonEvent *>(coinev);
        SbString enumname;
        SoMouseButtonEvent::enumToString(mbevent->getButton(), enumname);
        PRIVATE(this)->varstring.sprintf("'%s'", enumname.getString());
        return PRIVATE(this)->varstring.getString();
      }

      else if (strcmp(subkey, "getButton()") == 0 && coinev->isOfType(SoSpaceballButtonEvent::getClassTypeId())) {
        const SoSpaceballButtonEvent * mbevent = coin_assert_cast<const SoSpaceballButtonEvent *>(coinev);
        SbString enumname;
        SoSpaceballButtonEvent::enumToString(mbevent->getButton(), enumname);
        PRIVATE(this)->varstring.sprintf("'%s'", enumname.getString());
        return PRIVATE(this)->varstring.getString();
      }

      // FIXME: x., .y, .z
      else if (strcmp(subkey, "getTranslation()") == 0 && coinev->isOfType(SoMotion3Event::getClassTypeId())) {
        const SoMotion3Event * m3event = coin_assert_cast<const SoMotion3Event *>(coinev);
        SbVec3f translation = m3event->getTranslation();
        PRIVATE(this)->varstring = SbStringConvert::toString(translation);
        return PRIVATE(this)->varstring.getString();
      }

      // FIXME: .angle, .axis
      else if (strcmp(subkey, "getRotation()") == 0 && coinev->isOfType(SoMotion3Event::getClassTypeId())) {
        const SoMotion3Event * m3event = coin_assert_cast<const SoMotion3Event *>(coinev);
        SbRotation rotation = m3event->getRotation();
        PRIVATE(this)->varstring = SbStringConvert::toString(rotation);
        return PRIVATE(this)->varstring.getString();
      }

      // FIXME: make this into a evaluator-level RayPick(SbVec2f) function instead
      else if (strcmp(key + 7, "pickposition3") == 0) {
        SbVec2s location2 = coinev->getPosition();
        SoRayPickAction rpa(this->getViewportRegion());
        rpa.setPoint(location2);
        rpa.apply(this->getSceneGraphRoot());
        SoPickedPoint * pp = rpa.getPickedPoint();
        if (pp) {
          SbVec3f pickpos = pp->getPoint();
          PRIVATE(this)->varstring = SbStringConvert::toString(pickpos);
        } else {
          PRIVATE(this)->varstring.sprintf("FALSE"); // need a valid undefined-value
        }
        return PRIVATE(this)->varstring.getString();
      }
    }
  }

  else if (strncmp(key, "coin:", 5) == 0) {
    const char * subkey = key + 5;
    if (strncmp(subkey, "camera.", 7) == 0) {
      SoCamera * camera = this->getActiveCamera();
      if (!camera) {
        SoDebugError::post("SoScXMLStateMachine::getVariable",
                           "queried for camera, but no camera is set.");
        return NULL;
      }
      const char * detail = subkey + 7;
      if (strcmp(detail, "getTypeId()") == 0) {
        PRIVATE(this)->varstring.sprintf("'%s'", camera->getTypeId().getName().getString());
        return PRIVATE(this)->varstring.getString();
      }
    }

    // get generic field access working and intercept for more So-specific stuff
    // coin:viewport
    // coin:camera
    // coin:scene
  }

  //else {
  //}

  // couldn't resolve the symbol - try parent class to get '_data' and other '_event'
  // locations resolved
  return inherited::getVariable(key);
}

#undef PRIVATE
