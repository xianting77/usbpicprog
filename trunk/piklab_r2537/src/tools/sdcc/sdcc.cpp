/***************************************************************************
 *   Copyright (C) 2006 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "sdcc.h"

#include "devices/list/device_list.h"
#include "sdcc_compile.h"
#include "sdcc_config.h"
#include "devices/pic/pic/pic_memory.h"
#include "common/global/pfile.h"
#include "common/global/process.h"
#include "sdcc_generator.h"

//----------------------------------------------------------------------------
bool SDCC::Base::checkExecutableResult(bool, QStringList &lines) const
{
  return ( lines.count()>0 && lines[0].startsWith("SDCC") );
}

//----------------------------------------------------------------------------
QString SDCC::Group::informationText() const
{
  return i18n("The <a href=\"%1\">Small Devices C Compiler</a> is an open-source C compiler for several families of microcontrollers.").arg("http://sdcc.sourceforge.net");
}

const ::Tool::Base *SDCC::Group::base(Tool::Category category) const
{
  if ( category==Tool::Category::Assembler ) return Tool::lister().group("gputils")->base(category);
  return Tool::Group::base(category);
}

Tool::Base *SDCC::Group::baseFactory(Tool::Category category) const
{
  if ( category==Tool::Category::Compiler || category==Tool::Category::Linker) return new ::SDCC::Base;
  return 0;
}

SDCC::Group::Group()
  : _checkDevicesTmp(_sview, ".c")
{
  // used to obtain device list
  if ( _checkDevicesTmp.openForWrite() ) _checkDevicesTmp.appendText("void main(void) {}\n");
  _checkDevicesTmp.close();
}

QStringList SDCC::Group::checkDevicesOptions(uint i) const
{
  QStringList options;
  options += QString("-m") + SDCC::FAMILY_DATA[i].name;
  options += "-phelp";
  options += _checkDevicesTmp.url().filepath();
  return options;
}

PURL::Directory SDCC::Group::autodetectDirectory(Compile::DirectoryType type, const PURL::Directory &dir, bool withWine) const
{
  return Tool::lister().group("gputils")->autodetectDirectory(type, dir, withWine);
}

bool SDCC::Group::needs(Tool::ActionType type, Tool::Category category) const
{
  if ( type==Tool::Project && category==Tool::Category::Linker ) return true;
  return false;
}

Compile::Process *SDCC::Group::processFactory(const Compile::Data &data) const
{
  switch (data.category.type()) {
    case Tool::Category::Assembler:
      return Tool::lister().group("gputils")->processFactory(data);
    case Tool::Category::Compiler:
      if (data.project) return new SDCC::CompileProjectFile;
      return new SDCC::CompileStandaloneFile;
    case Tool::Category::Linker:
      return new SDCC::LinkProjectFile;
    default: break;
  }
  return 0;
}

Compile::Config *SDCC::Group::configFactory(::Project *project) const
{
  return new Config(project);
}

PURL::FileType SDCC::Group::implementationType(PURL::ToolType type) const
{
  if ( type==PURL::ToolType::Assembler ) return PURL::AsmGPAsm;
  if ( type==PURL::ToolType::Compiler ) return PURL::CSource;
  return PURL::Nb_FileTypes;
}

Tool::SourceGenerator *SDCC::Group::sourceGeneratorFactory() const
{
  return new SourceGenerator;
}

bool SDCC::Group::generateDebugInformation(const QString &device) const
{
  switch (family(device)) {
    case P14: return false;
    case P16: return true;
    case Nb_Families: break;
  }
  Q_ASSERT(false);
  return false;
}