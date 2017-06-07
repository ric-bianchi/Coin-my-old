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

#include <Inventor/navigation/SoScXMLPanTarget.h>

/*!
  \class SoScXMLPanTarget SoScXMLPanTarget.h Inventor/scxml/SoScXMLPanTarget.h
  \brief Navigation system event target for panning operations.

  \ingroup navigation
*/

#include <cassert>

#include <Inventor/SbVec2f.h>
#include <Inventor/SbPlane.h>
#include <Inventor/SbLine.h>
#include <Inventor/SbViewVolume.h>
#include <Inventor/scxml/ScXMLEvent.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoFrustumCamera.h>
#include <Inventor/scxml/SoScXMLStateMachine.h>
#include <Inventor/navigation/SoScXMLNavigation.h>
#include <Inventor/navigation/SoScXMLDollyTarget.h>
#include <Inventor/navigation/SoScXMLFlightControlTarget.h>
#include "scxml/SbStringConvert.h"
#include "coindefs.h"

namespace {

class PanData : public SoScXMLNavigationTarget::Data {
public:
  SbVec2f lastpos;
  SbPlane panplane;
};

static SoScXMLNavigationTarget::Data * NewPanData(void) { return new PanData; }

} // namespace

class SoScXMLPanTarget::PImpl {
public:
  static SbName BEGIN;
  static SbName UPDATE;
  static SbName END;
  static SbName SET_FOCAL_POINT;
  static SbName MOVE;
};

SbName SoScXMLPanTarget::PImpl::BEGIN;
SbName SoScXMLPanTarget::PImpl::UPDATE;
SbName SoScXMLPanTarget::PImpl::END;
SbName SoScXMLPanTarget::PImpl::SET_FOCAL_POINT;
SbName SoScXMLPanTarget::PImpl::MOVE;

// *************************************************************************

#define PRIVATE

SCXML_OBJECT_SOURCE(SoScXMLPanTarget);

void
SoScXMLPanTarget::initClass(void)
{
  SCXML_OBJECT_INIT_CLASS(SoScXMLPanTarget, SoScXMLNavigationTarget, "SoScXMLNavigationTarget");

#define EVENT_PREFIX COIN_NAVIGATION_PAN_TARGET_EVENT_PREFIX
  SoScXMLPanTarget::PImpl::BEGIN            = SbName(EVENT_PREFIX ".BEGIN");
  SoScXMLPanTarget::PImpl::UPDATE           = SbName(EVENT_PREFIX ".UPDATE");
  SoScXMLPanTarget::PImpl::END              = SbName(EVENT_PREFIX ".END");
  SoScXMLPanTarget::PImpl::SET_FOCAL_POINT  = SbName(EVENT_PREFIX ".SET_FOCAL_POINT");
  SoScXMLPanTarget::PImpl::MOVE             = SbName(EVENT_PREFIX ".MOVE");
#undef EVENT_PREFIX
}

void
SoScXMLPanTarget::cleanClass(void)
{
  SoScXMLPanTarget::classTypeId = SoType::badType();
}

SoScXMLPanTarget * SoScXMLPanTarget::theSingleton = NULL;

SoScXMLPanTarget *
SoScXMLPanTarget::constructSingleton(void)
{
  assert(SoScXMLPanTarget::theSingleton == NULL);
  SoScXMLPanTarget::theSingleton =
    static_cast<SoScXMLPanTarget *>(SoScXMLPanTarget::classTypeId.createInstance());
  return SoScXMLPanTarget::theSingleton;
}

void
SoScXMLPanTarget::destructSingleton(void)
{
  assert(SoScXMLPanTarget::theSingleton != NULL);
  delete SoScXMLPanTarget::theSingleton;
  SoScXMLPanTarget::theSingleton = NULL;
}

SoScXMLPanTarget *
SoScXMLPanTarget::singleton(void)
{
  assert(SoScXMLPanTarget::theSingleton != NULL);
  return SoScXMLPanTarget::theSingleton;
}

const SbName &
SoScXMLPanTarget::BEGIN(void)
{
  return SoScXMLPanTarget::PImpl::BEGIN;
}

const SbName &
SoScXMLPanTarget::UPDATE(void)
{
  return SoScXMLPanTarget::PImpl::UPDATE;
}

const SbName &
SoScXMLPanTarget::END(void)
{
  return SoScXMLPanTarget::PImpl::END;
}

const SbName &
SoScXMLPanTarget::SET_FOCAL_POINT(void)
{
  return SoScXMLPanTarget::PImpl::SET_FOCAL_POINT;
}

const SbName &
SoScXMLPanTarget::MOVE(void)
{
  return SoScXMLPanTarget::PImpl::MOVE;
}

SoScXMLPanTarget::SoScXMLPanTarget(void)
{
  this->setEventTargetType(SOSCXML_NAVIGATION_TARGETTYPE);
  this->setEventTargetName("Pan");
}

SoScXMLPanTarget::~SoScXMLPanTarget(void)
{
}

SbBool
SoScXMLPanTarget::processOneEvent(const ScXMLEvent * event)
{
  assert(event);

  const SbName sessionid = inherited::getSessionId(event);
  if (sessionid == SbName::empty()) { return FALSE; }

  const SbName & eventname = event->getEventName();

  if (eventname == BEGIN()) {
    PanData * data = static_cast<PanData *>(inherited::getSessionData(sessionid, NewPanData));
    assert(data);

    SoScXMLStateMachine * statemachine = inherited::getSoStateMachine(event, sessionid);
    if unlikely (!statemachine) { return FALSE; }

    SoCamera * camera = inherited::getActiveCamera(event, sessionid);
    if unlikely (!camera) { return FALSE; }

    if (!inherited::getEventSbVec2f(event, "mouseposition", data->lastpos)) {
      return FALSE;
    }

    const SbViewportRegion & vp = statemachine->getViewportRegion();
    SbViewVolume vv = camera->getViewVolume(vp.getViewportAspectRatio());
    data->panplane = vv.getPlane(camera->focalDistance.getValue());

    return TRUE;
  }

  else if (eventname == UPDATE()) {
    // _sessionid
    // mouseposition {SbVec2f}
    PanData * data = static_cast<PanData *>(inherited::getSessionData(sessionid, NewPanData));
    assert(data);

    SoScXMLStateMachine * statemachine = inherited::getSoStateMachine(event, sessionid);
    if unlikely (!statemachine) { return FALSE; }

    SoCamera * camera = inherited::getActiveCamera(event, sessionid);
    if unlikely (!camera) { return FALSE; }

    SbVec2f position;
    if (!inherited::getEventSbVec2f(event, "mouseposition", position)) {
      return FALSE;
    }

    const SbViewportRegion & vp = statemachine->getViewportRegion();
    float vpaspect = vp.getViewportAspectRatio();

    SoScXMLPanTarget::panCamera(camera, vpaspect, data->panplane,
                                data->lastpos, position);

    data->lastpos = position;
    return TRUE;
  }

  else if (eventname == END()) {
    // _sessionid
    this->freeSessionData(sessionid);
    return TRUE;
  }

  else if (eventname == SET_FOCAL_POINT()) {
    // _sessionid
    // worldspace {SbVec3f} or screenspace {SbVec2f}
    // [focaldistance] {float}
    // [upvector] {SbVec3f}
    SoScXMLStateMachine * statemachine = inherited::getSoStateMachine(event, sessionid);
    if unlikely (!statemachine) { return FALSE; }

    SoCamera * camera = inherited::getActiveCamera(event, sessionid);
    if unlikely (!camera) { return FALSE; }

    SbBool isworldspace = FALSE;
    SbVec3f worldspace(0.0f, 0.0f, 0.0f);
    SbVec2f screenspace(0.0f, 0.0f);
    if (event->getAssociation("worldspace")) {
      // if we have a pick-based position and we had a miss, worldspace would contain 'FALSE',
      // not an SbVec3f
      SbString valuestr = event->getAssociation("worldspace");
      if (SbStringConvert::typeOf(valuestr) == SbStringConvert::SBVEC3F &&
          inherited::getEventSbVec3f(event, "worldspace", worldspace, FALSE)) {
        isworldspace = TRUE;
      } else {
        return FALSE;
      }
    }
    else if (inherited::getEventSbVec2f(event, "screenspace", screenspace, FALSE)) {
      isworldspace = FALSE;
    }
    else {
      SoDebugError::post("SoScXMLPanTarget::processOneEvent",
                         "while processing %s: a 'worldspace' or 'screenspace' coordinate must be specified.",
                         eventname.getString());
      return FALSE;
    }

    double focaldistance = 0.0;
    SbBool usefocaldistance = inherited::getEventDouble(event, "focaldistance", focaldistance, FALSE);

    SbVec3f upvector(0.0f, 0.0f, 0.0f);
    SbBool useupvector = inherited::getEventSbVec3f(event, "upvector", upvector, FALSE);

    if (isworldspace) {
      SoScXMLPanTarget::panSetFocalPoint(camera, worldspace);
      if (usefocaldistance) {
        SoScXMLDollyTarget::jump(camera, float(focaldistance));
      }
      if (useupvector) {
        SoScXMLFlightControlTarget::resetRoll(camera, upvector);
      }
    } else {
      const SbViewportRegion & vp = statemachine->getViewportRegion();
      SbViewVolume vv = camera->getViewVolume(vp.getViewportAspectRatio());
      SbPlane panplane = vv.getPlane(camera->focalDistance.getValue());

      SoScXMLPanTarget::panCamera(camera, vp.getViewportAspectRatio(), panplane,
                                  screenspace, SbVec2f(0.5f, 0.5f));
      if (usefocaldistance) {
        SoScXMLDollyTarget::jump(camera, float(focaldistance));
      }
      if (useupvector) {
        SoScXMLFlightControlTarget::resetRoll(camera, upvector);
      }
    }

    return TRUE;
  }

  else if (eventname == MOVE()) {
    // _sessionid
    // translation {SbVec3f}
    // factor {double=1.0}
    // [cameraspace] {boolean=FALSE}
    SoScXMLStateMachine * statemachine = inherited::getSoStateMachine(event, sessionid);
    if unlikely (!statemachine) { return FALSE; }

    SoCamera * camera = inherited::getActiveCamera(event, sessionid);
    if unlikely (!camera) { return FALSE; }

    SbVec3f translation(0.0f, 0.0f, 0.0f);
    if (!inherited::getEventSbVec3f(event, "translation", translation)) {
      return FALSE;
    }

    double factor = 1.0;
    inherited::getEventDouble(event, "factor", factor, FALSE);

    translation *= static_cast<float>(factor);

    SbBool iscameraspace = FALSE;
    inherited::getEventSbBool(event, "cameraspace", iscameraspace, FALSE);

    SoScXMLPanTarget::translateCamera(camera, translation, iscameraspace);
  }

  else {
    SoDebugError::post("SoScXMLPanTarget::processOneEvent",
                       "while processing %s: unknown event",
                       eventname.getString());
    return FALSE;
  }

  return TRUE;
}


// Move camera parallel with the plane orthogonal to the camera
// direction vector.

void
SoScXMLPanTarget::panCamera(SoCamera * camera,
                            float vpaspect,
                            const SbPlane & panplane,
                            const SbVec2f & previous,
                            const SbVec2f & current)
{
  if (camera == NULL) return; // can happen for empty scenegraph
  if (current == previous) return; // useless invocation

  // Find projection points for the last and current mouse coordinates.
  SbViewVolume vv = camera->getViewVolume(vpaspect);
  SbLine line;
  vv.projectPointToLine(current, line);
  SbVec3f current_planept;
  panplane.intersect(line, current_planept);
  vv.projectPointToLine(previous, line);
  SbVec3f old_planept;
  panplane.intersect(line, old_planept);

  // Reposition camera according to the vector difference between the
  // projected points.
  camera->position =
    camera->position.getValue() - (current_planept - old_planept);
}

void
SoScXMLPanTarget::panSetFocalPoint(SoCamera * camera, const SbVec3f & focalpoint)
{
  assert(camera);

  float currentdistance = camera->focalDistance.getValue();
  SbVec3f direction;
  camera->orientation.getValue().multVec(SbVec3f(0.0f, 0.0f, -1.0f), direction);
  direction.normalize();

  camera->position.setValue(focalpoint - direction * currentdistance);
}

void
SoScXMLPanTarget::translateCamera(SoCamera * camera, const SbVec3f & translation, SbBool cameraspace)
{
  assert(camera);
  if (!cameraspace) {
    camera->position.setValue(camera->position.getValue() + translation);
  }
  else {
    SbVec3f transformed;
    camera->orientation.getValue().multVec(translation, transformed);
    camera->position.setValue(camera->position.getValue() + transformed);
  }
}

#undef PRIVATE
