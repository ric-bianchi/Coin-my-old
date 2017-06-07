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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifdef HAVE_NODEKITS

/*! \file SoForeignFileKit.h */
#include <ForeignFiles/SoForeignFileKit.h>

#include <assert.h>
#include <stdlib.h>

#include <Inventor/errors/SoDebugError.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/SoType.h>
#include <Inventor/SbName.h>
#include <Inventor/SoInput.h>
#include <Inventor/C/tidbits.h>

#include <ForeignFiles/SoSTLFileKit.h>

#include "nodekits/SoSubKitP.h"
#include "misc/SbHash.h"
#include "tidbitsp.h"

/*!
  \page foreignfilesupport Foreign File Format Support

  This is a page describing foreign file format support in Coin.

  Formats supported so far:
  - STL / Stereolithography files (.stl)

  The SoSTLFileKit server as a first example on how foreign file
  format support can be implemented through the SoForeignFileKit
  interface nodekit.  For STL, the file format is so trivial that it
  maps directly into a few Open Inventor nodes. For other file
  formats, the direct mapping might not be possible or desirable
  (maybe important information will get lost in such a conversion),
  and the implementor might choose to implement a set of internal
  custom nodes for doing the rendering (and applying the other
  actions), and choose to first do pure Open Inventor organization
  when the node kit is asked to write its contents as a scene graph.

  FIXME: Document autoloading when implemented and tested

  \sa SoForeignFileKit, SoSTLFileKit
*/

/*!
  \class SoForeignFileKit ForeignFiles/SoForeignFileKit.h
  \brief Abstract base class for foreign file format support in Coin.

  Abstract base class for foreign file format support in Coin.

  \relates foreignfilesupport
  \COIN_CLASS_EXTENSION
  \since Coin 3.0
*/

/*
  TODO / UNRESOLVED ISSUES:

  - file format sub-formats (e.g. stl: stl ascii, stl binary, stl binary colored)
  - specify/implement format specification/syntax for writeScene()
  - improve file type handler registering
  - What if multiple kits support the same filetype?

*/

class SoForeignFileKitP {
public:
  SoForeignFileKitP(SoForeignFileKit * COIN_UNUSED_ARG(api)) { }

  static SbHash<const char *, SoType> * fileexts;

};

SbHash<const char *, SoType> * SoForeignFileKitP::fileexts = NULL;

SO_KIT_ABSTRACT_SOURCE(SoForeignFileKit);

static void
foreignfilekit_cleanup(void)
{
  delete SoForeignFileKitP::fileexts;
  SoForeignFileKitP::fileexts = NULL;
}

void
SoForeignFileKit::initClass(void)
{
  assert(SoForeignFileKit::classTypeId == SoType::badType());
  // Make sure parent class has been initialized.
  assert(inherited::getClassTypeId() != SoType::badType());

  SO_KIT_INIT_ABSTRACT_CLASS(SoForeignFileKit, SoBaseKit, SoBaseKit);
  // set up support dictionary
  SoForeignFileKitP::fileexts = new SbHash<const char *, SoType>(11);
  coin_atexit((coin_atexit_f*)foreignfilekit_cleanup, CC_ATEXIT_NORMAL);

  SoForeignFileKit::initClasses();
}

void
SoForeignFileKit::initClasses(void)
{
  SoSTLFileKit::initClass();
}


#define PRIVATE(obj) ((obj)->pimpl)


SoForeignFileKit::SoForeignFileKit(void)
{
  PRIVATE(this) = NULL;
  // PRIVATE(this) = new SoForeignFileKitP(this);

  SO_KIT_INTERNAL_CONSTRUCTOR(SoForeignFileKit);

  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, TRUE, this, \x0, FALSE);

  SO_KIT_INIT_INSTANCE();
}

SoForeignFileKit::~SoForeignFileKit(void)
{
  // delete PRIVATE(this);
}

/*!
  Registers a concrete SoForeignFileKit subtype to be a handler for files with the given extension.
  One class can be a handler for multiple filename extensions.
  
  FIXME: \e identify is not implemented
 */
SbBool
SoForeignFileKit::registerFileExtension(SoType handler, SbName extension, SoForeignFileIdentifyFunc * COIN_UNUSED_ARG(identify))
{
  assert(SoForeignFileKitP::fileexts != NULL);
  assert(handler.canCreateInstance());

  if (extension.getString()[0] == '.') {
#if COIN_DEBUG
    SoDebugError::post("SoForeignFileKit::registerFileExtension",
                       "Do not include the extension separator '.' "
                       "when registering file extension.");
#endif // COIN_DEBUG
  }

  if (SoForeignFileKitP::fileexts->put(extension.getString(), handler)) {
    return TRUE;
  }
  return FALSE;
}

/*!
  Creates an instance of a suitable SoForeignFileKit subtype.
  Returns NULL on failure or a kit with refcount of 1 on success.
*/
static SoForeignFileKit *create_foreignfilekit(const char *filename, SbBool exhaust)
{
  assert(SoForeignFileKitP::fileexts != NULL);

  const char * extptr = strrchr(filename, '.');
  if (extptr) {
    extptr++;
    SbName ext(SbString(extptr).lower());
    SoType handler = SoType::badType();
    if (SoForeignFileKitP::fileexts->get(ext.getString(), handler)) {
      SoForeignFileKit * foreignfile = (SoForeignFileKit *)handler.createInstance();
      foreignfile->ref();
      if (foreignfile->canReadFile(filename)) {
        return foreignfile;
      }
      else {
        foreignfile->unref();
      }
    }
    else {
      // We try to synthesize a classname from the extension (e.g. SoFBXFileKit),
      // and load it using the SoType autoloader feature.
      SbString filekitname;
      filekitname.sprintf("So%sFileKit", SbString(ext.getString()).upper().getString());
      SoType filekittype = SoType::fromName(SbName(filekitname));
      if (!filekittype.isBad()) return create_foreignfilekit(filename, exhaust);

      // FIXME: Some filekits supports more than one file format/extension (e.g. FBX).
      // We need a way of mapping extensions to library, or a way of loading
      // each external kit and testing for support.
      // FIXME: Temporary hack: Load SoFBXFileKit
      filekitname = "SoFBXFileKit";
      filekittype = SoType::fromName(SbName(filekitname));
      if (!filekittype.isBad()) return create_foreignfilekit(filename, exhaust);
    }
  }
  if (exhaust) {
    // FIXME: Implement
    // SoForeignFileKitP::fileexts->apply()
  }
  return NULL;
}

/*!
  Checks if the filename can be read by a registered SoForeignFileKit handler.
  
  FIXME: \e exhaust is not implemented.
 */
SbBool
SoForeignFileKit::isFileSupported(const char * filename, SbBool exhaust)
{
  SoForeignFileKit * foreignfile = create_foreignfilekit(filename, exhaust);
  SbBool success = (foreignfile != NULL);
  if (foreignfile) foreignfile->unref();
  return success;
}

/*!
  Convenience method. Will extract the filename from \e in and call
  the other ieFileSupported() method.

  Will return false if file is not supported or /e in is not representing a normal file.
 */
SbBool
SoForeignFileKit::isFileSupported(SoInput * in, SbBool exhaust)
{
  assert(in);
  if (in->getCurFileName() == NULL || in->getNumBytesRead() > 0) {
    // can only read proper files, from the beginning
    return FALSE;
  }
  return SoForeignFileKit::isFileSupported(in->getCurFileName(), exhaust);
}

/*!
  Creates an instance of a suitable SoForeignFileKit subtype from the given file 
  and reads its content.
  Returns NULL on failure or a kit with refcount of 0 on success.

  FIXME: \e exhaust is not implemented.
 */
SoForeignFileKit *
SoForeignFileKit::createForeignFileKit(const char * filename, SbBool exhaust)
{
  SoForeignFileKit * foreignfile = create_foreignfilekit(filename, exhaust);
  if (foreignfile) {
    if (foreignfile->readFile(filename)) {
      foreignfile->unrefNoDelete();
    } else {
      foreignfile->unref();
      foreignfile = NULL;
    }
  }
  return foreignfile;
}

/*!
  Convenience method. Will extract the filename from \e in and call
  the other ieFileSupported() method.
*/
SoForeignFileKit *
SoForeignFileKit::createForeignFileKit(SoInput * in, SbBool exhaust)
{
  assert(in);
  if (in->getCurFileName() == NULL || in->getNumBytesRead() > 0) {
    // can only read proper files, from the beginning
    return FALSE;
  }
  SoForeignFileKit * kit =
    SoForeignFileKit::createForeignFileKit(in->getCurFileName(), exhaust);
  if (kit) {
    if (!in->eof()) {
      // Places the stream at the end of the current file on the
      // stack. This is a "hack-ish", but its done this way
      // instead of loosening the protection of SoInput::popFile().
      char dummy;
      while (!in->eof() && in->get(dummy));
      assert(in->eof());
    }
  }
  return kit;
}

/*!
  Checks if this concrete class can read the given file.
*/
SbBool
SoForeignFileKit::canReadFile(const char * COIN_UNUSED_ARG(filename)) const
{
  return FALSE;
}

/*!
  Reads the given file into the internal representation.
  If successful, Coin should now be able to render the scene.
  If you need a pure Coin scenegraph, call convert().
*/
SbBool
SoForeignFileKit::readFile(const char * COIN_UNUSED_ARG(filename))
{
  return FALSE;
}

/*!
  Checks if this concrete class can write to the given file.
*/
SbBool
SoForeignFileKit::canWriteFile(const char * COIN_UNUSED_ARG(filename)) const
{
  return FALSE;
}

/*!
  Writes the current contents to the given file.

  \sa canWriteFile
*/
SbBool
SoForeignFileKit::writeFile(const char * COIN_UNUSED_ARG(filename))
{
  return FALSE;
}

#undef PRIVATE
#endif // HAVE_NODEKITS
