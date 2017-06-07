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

#ifdef _MSC_VER
#pragma warning(disable:4786) // symbol truncated
#endif // _MSC_VER

#include <Inventor/scxml/ScXMLObject.h>

/*!
  \class ScXMLObject ScXMLObject.h Inventor/scxml/ScXMLObject.h
  \brief Base class for all SCXML objects.

  \since Coin 3.0
  \ingroup scxml
*/

/*!
  \fn ScXMLObject::getTypeId() const

  \brief \copybrief SoBase::getTypeId() const

  \sa SoBase::getTypeId() const
*/

/*!
  \fn ScXMLObject::getClassTypeId()

  \brief \copybrief SoBase::getClassTypeId()

  \sa SoBase::getClassTypeId
*/

#include <cstring>
#include <cassert>
#include <map>

#include <Inventor/SbName.h>
#include <Inventor/scxml/ScXML.h>

#include "scxml/ScXMLP.h"

// *************************************************************************

class ScXMLObject::PImpl {
public:
  typedef std::map<const char *, char *> AttributeMap;
  typedef std::pair<const char *, char *> AttributeEntry;
  AttributeMap attributemap;

  ~PImpl(void)
  {
    AttributeMap::iterator it = this->attributemap.begin();
    while (it != this->attributemap.end()) {
      delete [] it->second;
      ++it;
    }
    this->attributemap.clear();
  }
};

#define PRIVATE(obj) ((obj)->pimpl)

SCXML_OBJECT_ABSTRACT_SOURCE(ScXMLObject);


/*!
  \brief \copybrief SoBase::initClass()

  \sa SoBase::initClass()
*/
void
ScXMLObject::initClass(void)
{
  ScXMLObject::classTypeId =
    SoType::createType(SoType::badType(), SbName("ScXMLObject"));
  // not registered with ScXML
}

/*!
  \brief Does the opposite of initClass, deallocates any class
  specific resources and resets the class to an uninitialized state
  with respect to the Coin typesystem
 */
void
ScXMLObject::cleanClass(void)
{
  ScXMLObject::classTypeId = SoType::badType();
}

ScXMLObject::ScXMLObject(void)
: containerptr(NULL)
{
}

ScXMLObject::~ScXMLObject(void)
{
  this->containerptr = NULL;
}

/*!
  \brief \copybrief SoBase::isOfType(SoType type) const

  \sa SoBase::isOfType(SoType type) const
*/
SbBool
ScXMLObject::isOfType(SoType type) const
{
  return this->getTypeId().isDerivedFrom(type);
}

// *************************************************************************

/*!
  Internal method.
  Friendly forwarding for ScXMLObject subclasses.
*/
void
ScXMLObject::registerClassType(const char * xmlns, const char * classname, SoType type)
{
  ScXMLP::registerClassType(xmlns, classname, type);
}

/*!
  Internal method.
  Friendly forwarding for ScXMLObject subclasses.
*/
void
ScXMLObject::registerInvokeClassType(const char * xmlns, const char * targettype, const char * source, SoType type)
{
  ScXMLP::registerInvokeClassType(xmlns, targettype, source, type);
}

#undef PRIVATE
