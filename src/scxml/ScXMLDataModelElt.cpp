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

#include <Inventor/scxml/ScXMLDataModelElt.h>

/*!
  \class ScXMLDataModelElt ScXMLDataModelElt.h Inventor/scxml/ScXMLDataModelElt.h
  \brief implements the &lt;datamodel&gt; SCXML element.

  \ingroup scxml
*/

#include <cassert>
#include <cstring>
#include <vector>

#include <Inventor/C/XML/element.h>
#include <Inventor/scxml/ScXMLDataElt.h>
#include <Inventor/errors/SoDebugError.h>

#include "scxml/ScXMLCommonP.h"
#include "SbBasicP.h"

#ifndef COIN_WORKAROUND_NO_USING_STD_FUNCS
using std::strcmp;
#endif // !COIN_WORKAROUND_NO_USING_STD_FUNCS

// *************************************************************************

class ScXMLDataModelEltReader : public ScXMLEltReader {
public:
  ScXMLDataModelEltReader(void);
  virtual ScXMLElt * read(ScXMLElt * container, cc_xml_elt * xmlelt, ScXMLDocument * doc, ScXMLStateMachine * sm);
};

ScXMLDataModelEltReader::ScXMLDataModelEltReader(void)
: ScXMLEltReader("datamodel")
{
}

ScXMLElt *
ScXMLDataModelEltReader::read(ScXMLElt * container, cc_xml_elt * xmlelt, ScXMLDocument * doc, ScXMLStateMachine * sm)
{
  assert(container && xmlelt);
  ScXMLDataModelElt * datamodel = new ScXMLDataModelElt;

  datamodel->setContainer(container);
  this->setXMLAttributes(datamodel, xmlelt);

  // handle XML attributes
  if (unlikely(!datamodel->handleXMLAttributes())) {
    delete datamodel;
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

    if (strcmp(elementtype, "data") == 0) {
      // <data> - zero or more times
      ScXMLEltReader * datareader = ScXMLDataElt::getElementReader();
      assert(datareader);
      ScXMLElt * dataobj = datareader->read(datamodel, element, doc, sm);
      if (unlikely(!dataobj)) {
        delete datamodel;
        return NULL;
      }
      assert(dataobj->isOfType(ScXMLDataElt::getClassTypeId()));
      datamodel->addData(static_cast<ScXMLDataElt *>(dataobj));
    }

    else {
      SoDebugError::post("ScXMLDataModelEltReader::read",
                         "unexpected XML element '<%s>' found in <datamodel>",
                         elementtype);
      delete datamodel;
      return NULL;
    }
  }

  return datamodel;
}

// *************************************************************************

class ScXMLDataModelElt::PImpl {
public:
  PImpl(void) { }
  ~PImpl(void)
  {
    SCXML__CLEAR_STD_VECTOR(this->datalist, ScXMLDataElt *);
  }

  std::vector<ScXMLDataElt *> datalist;

};

#define PRIVATE(obj) ((obj)->pimpl)

SCXML_ELEMENT_SOURCE(ScXMLDataModelElt);

void
ScXMLDataModelElt::initClass(void)
{
  SCXML_OBJECT_INIT_CLASS(ScXMLDataModelElt, ScXMLElt, "ScXMLElt");
  SCXML_ELEMENT_REGISTER_READER(ScXMLDataModelElt, "datamodel", ScXMLDataModelEltReader);
}

void
ScXMLDataModelElt::cleanClass(void)
{
  SCXML_ELEMENT_UNREGISTER_READER(ScXMLDataModelElt);
  ScXMLDataModelElt::classTypeId = SoType::badType();
}

ScXMLDataModelElt::ScXMLDataModelElt(void)
: schema(NULL)
{
}

ScXMLDataModelElt::~ScXMLDataModelElt(void)
{
  this->setSchemaAttribute(NULL);
}

void
ScXMLDataModelElt::setSchemaAttribute(const char * COIN_UNUSED_ARG(schema))
{
}

// const char * ScXMLDataModelElt::getSchemaAttribute(void) const

SbBool
ScXMLDataModelElt::handleXMLAttributes(void)
{
  if (!inherited::handleXMLAttributes()) { return FALSE; }

  this->setSchemaAttribute(this->getXMLAttribute("schema"));

  return TRUE;
}

void
ScXMLDataModelElt::copyContents(const ScXMLElt * rhs)
{
  inherited::copyContents(rhs);
  const ScXMLDataModelElt * orig = coin_assert_cast<const ScXMLDataModelElt *>(rhs);
  this->setSchemaAttribute(orig->getSchemaAttribute());

  for (int c = 0; c < orig->getNumData(); ++c) {
    ScXMLDataElt * data =
      coin_assert_cast<ScXMLDataElt *>(orig->getData(c)->clone());
    this->addData(data);
  }
}

const ScXMLElt *
ScXMLDataModelElt::search(const char * attrname, const char * attrvalue) const
{
  const ScXMLElt * hit = inherited::search(attrname, attrvalue);
  if (hit) {
    return hit;
  }
  if (strcmp(attrname, "schema") == 0) {
    if (this->schema && strcmp(attrvalue, this->schema) == 0) {
      return this;
    }
  }
  std::vector<ScXMLDataElt *>::const_iterator it = PRIVATE(this)->datalist.begin();
  while (it != PRIVATE(this)->datalist.end()) {
    hit = (*it)->search(attrname, attrvalue);
    if (hit) {
      return hit;
    }
    ++it;
  }
  return NULL;
}

SCXML_LIST_OBJECT_API_IMPL(ScXMLDataModelElt, ScXMLDataElt, PRIVATE(this)->datalist, Data, Data);

#undef PRIVATE
