//===-- PluginLoader.cpp - Implement -load command line option ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the -load <plugin> command line option handler.
//
//===----------------------------------------------------------------------===//

#define DONT_GET_PLUGIN_LOADER_OPTION
#include "llvm/Support/PluginLoader.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/ManagedStatic.h"
#ifndef __wasi__
#include "llvm/Support/Mutex.h"
#endif
#include "llvm/Support/raw_ostream.h"
#include <vector>
using namespace llvm;

static ManagedStatic<std::vector<std::string> > Plugins;
#ifndef __wasi__
static ManagedStatic<sys::SmartMutex<true> > PluginsLock;
#endif

void PluginLoader::operator=(const std::string &Filename) {
#ifndef __wasi__
  sys::SmartScopedLock<true> Lock(*PluginsLock);
#endif
  std::string Error;
  if (sys::DynamicLibrary::LoadLibraryPermanently(Filename.c_str(), &Error)) {
    errs() << "Error opening '" << Filename << "': " << Error
           << "\n  -load request ignored.\n";
  } else {
    Plugins->push_back(Filename);
  }
}

unsigned PluginLoader::getNumPlugins() {
#ifndef __wasi__
  sys::SmartScopedLock<true> Lock(*PluginsLock);
#endif
  return Plugins.isConstructed() ? Plugins->size() : 0;
}

std::string &PluginLoader::getPlugin(unsigned num) {
#ifndef __wasi__
  sys::SmartScopedLock<true> Lock(*PluginsLock);
#endif
  assert(Plugins.isConstructed() && num < Plugins->size() &&
         "Asking for an out of bounds plugin");
  return (*Plugins)[num];
}
