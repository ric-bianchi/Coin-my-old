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

#include <Inventor/scxml/ScXMLInitialElt.h>

/*!
  \class ScXMLInitialElt ScXMLInitialElt.h Inventor/scxml/ScXMLInitialElt.h
  \brief implements the &lt;initial&gt; SCXML element.

  An &lt;initial&gt; elements has no attributes, and should contain one
  conditionless &lt;transition&gt; element with a target attribute that
  identifies a descendant state element of the parent state element.

  \since Coin 3.0
  \ingroup scxml
*/

#include <cassert>

#include <boost/scoped_ptr.hpp>

#include <Inventor/errors/SoDebugError.h>
#include <Inventor/C/XML/element.h>
#include <Inventor/scxml/ScXML.h>
#include <Inventor/scxml/ScXMLTransitionElt.h>

#include "scxml/ScXMLCommonP.h"

// *************************************************************************

class ScXMLInitialEltReader : public ScXMLEltReader {
public:
  ScXMLInitialEltReader(void);
  virtual ScXMLElt * read(ScXMLElt * container, cc_xml_elt * xmlelt, ScXMLDocument * doc, ScXMLStateMachine * sm);
};

ScXMLInitialEltReader::ScXMLInitialEltReader(void)
: ScXMLEltReader("initial")
{
}

ScXMLElt *
ScXMLInitialEltReader::read(ScXMLElt * container, cc_xml_elt * xmlelt, ScXMLDocument * doc, ScXMLStateMachine * sm)
{
  assert(container && xmlelt);

  ScXMLInitialElt * initial = new ScXMLInitialElt;
  initial->setContainer(container);
  this->setXMLAttributes(initial, xmlelt);

  // handle XML attributes
  if (unlikely(!initial->handleXMLAttributes())) {
    delete initial;
    return NULL;
  }

  const int numchildren = cc_xml_elt_get_num_children(xmlelt);
  for (int c = 0; c < numchildren; ++c) {
    cc_xml_elt * element = cc_xml_elt_get_child(xmlelt, c);
    const char * elementtype = cc_xml_elt_get_type(element);

    if (strcmp(elementtype, COIN_XML_CDATA_TYPE) == 0) {
      // ignore CDATA
      continue;
    }

    if (strcmp(elementtype, "transition") == 0) {
      // single, conditionless <transition>
      if (unlikely(initial->getTransition())) {
        SoDebugError::post("ScXMLInitialEltReader::read",
                           "<initial> can only contain one <transition> element");
        delete initial;
        return NULL;
      }
      ScXMLEltReader * transitionreader = ScXMLTransitionElt::getElementReader();
      assert(transitionreader);
      ScXMLElt * transitionobj = transitionreader->read(initial, element, doc, sm);
      if (unlikely(!transitionobj)) {
        delete initial;
        return NULL;
      }

      assert(transitionobj->isOfType(ScXMLTransitionElt::getClassTypeId()));
      ScXMLTransitionElt * transitionelt = static_cast<ScXMLTransitionElt *>(transitionobj);
      if (unlikely(!transitionelt->isConditionLess())) {
        SoDebugError::post("ScXMLInitialEltReader::read",
                           "<initial> must contain a conditionless <transition> element");
        delete transitionelt;
        delete initial;
        return NULL;
      }
      initial->setTransition(transitionelt);
    }

    else {
      SoDebugError::post("ScXMLInitialEltReader::read",
                         "unexpected XML element '<%s>' found in <initial>",
                         elementtype);
      delete initial;
      return NULL;
    }
  }

  return initial;
} // read

// *************************************************************************

class ScXMLInitialElt::PImpl {
public:
  PImpl(void) : transitionptr(NULL) { }

  boost::scoped_ptr<ScXMLTransitionElt> transitionptr;
};

#define PRIVATE(obj) ((obj)->pimpl)

SCXML_ELEMENT_SOURCE(ScXMLInitialElt);

void
ScXMLInitialElt::initClass(void)
{
  SCXML_OBJECT_INIT_CLASS(ScXMLInitialElt, ScXMLElt, "ScXMLElt");
  SCXML_ELEMENT_REGISTER_READER(ScXMLInitialElt, "initial", ScXMLInitialEltReader);
}

void
ScXMLInitialElt::cleanClass(void)
{
  SCXML_ELEMENT_UNREGISTER_READER(ScXMLInitialElt);
  ScXMLInitialElt::classTypeId = SoType::badType();
}

ScXMLInitialElt::ScXMLInitialElt(void)
{
}

ScXMLInitialElt::~ScXMLInitialElt(void)
{
}

void
ScXMLInitialElt::copyContents(const ScXMLElt * rhs)
{
  inherited::copyContents(rhs);
  const ScXMLInitialElt * orig = coin_assert_cast<const ScXMLInitialElt *>(rhs);
  if (orig->getTransition()) {
    ScXMLTransitionElt * transition =
      coin_assert_cast<ScXMLTransitionElt *>(orig->getTransition()->clone());
    this->setTransition(transition);
  }
}

const ScXMLElt *
ScXMLInitialElt::search(const char * attrname, const char * attrvalue) const
{
  const ScXMLElt * hit = inherited::search(attrname, attrvalue);
  if (hit) {
    return hit;
  }
  if (PRIVATE(this)->transitionptr.get()) {
    hit = PRIVATE(this)->transitionptr->search(attrname, attrvalue);
    if (hit) {
      return hit;
    }
  }
  return NULL;
}

SCXML_SINGLE_OBJECT_API_IMPL(ScXMLInitialElt, ScXMLTransitionElt, PRIVATE(this)->transitionptr, Transition);

#undef PRIVATE
