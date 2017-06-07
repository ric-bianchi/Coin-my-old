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

#include <Inventor/scxml/ScXMLDataElt.h>

/*!
  \class ScXMLDataElt ScXMLDataElt.h Inventor/scxml/ScXMLDataElt.h
  \brief the \c &lt;data&gt; SCXML element.

  The \c &lt;data&gt; SCXML element is for storing variables.  They need
  to be contained within a \c &lt;datamodel&gt; element.  \c &lt;data&gt;
  elements are only functional in state machines with a profile that has an
  implementation of the Data Module.  The \c "minimum" profile does not,
  but the \c "x-coin" profile does.

  The following attributes are accepted:
  \li \c ID
  \li \c src - not supported
  \li \c expr

  The \c ID attribute is required and specified the variable name.
  The variable name must be unique for the whole SCXML state machine
  specification.

  The \c src attribute is not supported.

  The \c expr attribute specifies the value of the variable.  The attribute
  is evaluated based on which profile the state machine is using.

  The value of the \c &lt;data&gt; variable can be changed through the use
  of \c &lt;assign&gt; elements, and their values can be passed on to other
  systems through the use of the \a &lt;send&gt; element in the External
  Communications Module.

  \since Coin 4.0
  \sa ScXMLDataModelElt, ScXMLAssignElt, ScXMLSendElt
  \ingroup scxml
*/

#include <cassert>
#include <cstring>
#include <vector>

#include <Inventor/errors/SoDebugError.h>
#include <Inventor/C/XML/element.h>

#include "scxml/ScXMLCommonP.h"
#include "SbBasicP.h"

#ifndef COIN_WORKAROUND_NO_USING_STD_FUNCS
using std::strcmp;
#endif // !COIN_WORKAROUND_NO_USING_STD_FUNCS

// *************************************************************************

class ScXMLDataEltReader : public ScXMLEltReader {
public:
  ScXMLDataEltReader(void);
  virtual ScXMLElt * read(ScXMLElt * container, cc_xml_elt * xmlelt, ScXMLDocument * doc, ScXMLStateMachine * sm);
};

ScXMLDataEltReader::ScXMLDataEltReader(void)
: ScXMLEltReader("data")
{
}

ScXMLElt *
ScXMLDataEltReader::read(ScXMLElt * container, cc_xml_elt * xmlelt, ScXMLDocument * COIN_UNUSED_ARG(doc), ScXMLStateMachine * COIN_UNUSED_ARG(sm))
{
  assert(container && xmlelt);
  ScXMLDataElt * data = new ScXMLDataElt;
  data->setContainer(container);
  this->setXMLAttributes(data, xmlelt);

  // handle XML attributes
  if (unlikely(!data->handleXMLAttributes())) {
    delete data;
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

    SoDebugError::post("ScXMLDataEltReader::read",
                       "unexpected XML element '<%s>' found in <data>",
                       elementtype);
    delete data;
    return NULL;
  }

  return data;
}

// *************************************************************************

class ScXMLDataElt::PImpl {
public:
};

#define PRIVATE(obj) ((obj)->pimpl)

SCXML_ELEMENT_SOURCE(ScXMLDataElt);

void
ScXMLDataElt::initClass(void)
{
  SCXML_OBJECT_INIT_CLASS(ScXMLDataElt, ScXMLElt, "ScXMLElt");
  SCXML_ELEMENT_REGISTER_READER(ScXMLDataElt, "data", ScXMLDataEltReader);
}

void
ScXMLDataElt::cleanClass(void)
{
  SCXML_ELEMENT_UNREGISTER_READER(ScXMLDataElt);
  ScXMLDataElt::classTypeId = SoType::badType();
}

ScXMLDataElt::ScXMLDataElt(void)
: ID(NULL),
  src(NULL),
  expr(NULL)
{
}

ScXMLDataElt::~ScXMLDataElt(void)
{
  this->setIDAttribute(NULL);
  this->setSrcAttribute(NULL);
  this->setExprAttribute(NULL);
}

void
ScXMLDataElt::setIDAttribute(const char * IDstr)
{
  SCXML__SET_ATTRIBUTE_VALUE(this->ID, "ID", IDstr);
}

// const char * getIDAttribute(void) const { return this->ID; }

void
ScXMLDataElt::setSrcAttribute(const char * srcstr)
{
  SCXML__SET_ATTRIBUTE_VALUE(this->src, "src", srcstr);
}

// const char * getSrcAttribute(void) const { return this->src; }

void
ScXMLDataElt::setExprAttribute(const char * exprstr)
{
  SCXML__SET_ATTRIBUTE_VALUE(this->expr, "expr", exprstr);
}

// const char * getExprAttribute(void) const { return this->expr; }

SbBool
ScXMLDataElt::handleXMLAttributes(void)
{
  if (!inherited::handleXMLAttributes()) {
    return FALSE;
  }

  this->setIDAttribute(this->getXMLAttribute("ID"));
  this->setSrcAttribute(this->getXMLAttribute("src"));
  this->setExprAttribute(this->getXMLAttribute("expr"));

  return TRUE;
}

void
ScXMLDataElt::copyContents(const ScXMLElt * rhs)
{
  inherited::copyContents(rhs);
  const ScXMLDataElt * orig = coin_assert_cast<const ScXMLDataElt *>(rhs);
  this->setIDAttribute(orig->getIDAttribute());
  this->setSrcAttribute(orig->getSrcAttribute());
  this->setExprAttribute(orig->getExprAttribute());
}

const ScXMLElt *
ScXMLDataElt::search(const char * attrname, const char * attrvalue) const
{
  const ScXMLElt * hit = inherited::search(attrname, attrvalue);
  if (hit) {
    return hit;
  }
  if (strcmp(attrname, "ID") == 0) {
    if (this->ID && strcmp(attrvalue, this->ID) == 0) {
      return this;
    }
  }
  else if (strcmp(attrname, "src") == 0) {
    if (this->src && strcmp(attrvalue, this->src) == 0) {
      return this;
    }
  }
  else if (strcmp(attrname, "expr") == 0) {
    if (this->expr && strcmp(attrvalue, this->expr) == 0) {
      return this;
    }
  }
  return NULL;
}

void
ScXMLDataElt::setValueObj(ScXMLDataObj * obj)
{
  this->value = obj;
}

#undef PRIVATE
