
# Author: Pierce Brooks

import os
import sys
import shutil
import inspect
import logging
import platform
import traceback
import subprocess

def wd():
  filename = inspect.getframeinfo(inspect.currentframe()).filename
  path = os.path.dirname(os.path.abspath(filename))
  return path
  
def execute(command):
  print(str(command))
  result = None
  try:
    result = subprocess.check_output(command)
  except subprocess.CalledProcessError as error:
    result = None
    print(error.output.decode())
  except Exception as exception:
    result = None
    logging.error(traceback.format_exc())
  except:
    result = None
  if not (result == None):
    print(result.decode())
    return True
  return False
  
def sync(root):
  command = []
  command.append("git")
  command.append("submodule")
  command.append("update")
  command.append("--init")
  command.append("--recursive")
  return execute(command)
  
def replace(root, script, pattern, replacement):
  for base, folders, files in os.walk(root):
    for name in files:
      if (name == script):
        continue
      if not (name.endswith(".hpp")):
        continue
      path = os.path.join(base, name)
      descriptor = open(path, "rt")
      lines = descriptor.readlines()
      descriptor.close()
      for line in lines:
        if (pattern in line):
          print(path)
          descriptor = open(path, "rt")
          content = descriptor.read()
          descriptor.close()
          content = content.replace(pattern, replacement)
          descriptor = open(path, "wt")
          descriptor.write(content)
          descriptor.close()
          return True
  return True
  
def fix(root, script):
  if not (replace(root, script, "void register_signal(String name, Dictionary args = Dictionary()) {", "void register_signal_fix(String name, Dictionary args = Dictionary()) {")):
    return False
  if not (replace(root, script, "register_signal<T>(name, Dictionary::make(varargs...));", "register_signal_fix<T>(name, Dictionary::make(varargs...));")):
    return False
  return True
  
def unfix(root, script):
  if not (replace(root, script, "void register_signal_fix(String name, Dictionary args = Dictionary()) {", "void register_signal(String name, Dictionary args = Dictionary()) {")):
    return False
  if not (replace(root, script, "register_signal_fix<T>(name, Dictionary::make(varargs...));", "register_signal<T>(name, Dictionary::make(varargs...));")):
    return False
  return True
  
def clean(string):
  result = ""
  for i in range(len(string)):
    character = string[i:(i+1)]
    if (character.isalpha()):
      result += character
  return result
  
def inclusion(includes, include):
  for i in range(len(includes)):
    if (includes[i] in include):
      return True
  return False
  
def make(build, api, system, parameters, binding, toolchain):
  cwd = os.getcwd()
  if not (os.path.isdir(build)):
    os.makedirs(build)
  command = []
  command.append("cmake")
  command.append("-G")
  if ("android" in system):
    command.append("Ninja")
  else:
    if ("windows" in system):
      command.append("Visual Studio 15 2017 Win64")
    else:
      command.append("Unix Makefiles")
  command.append("../..")
  command.append("--debug-trycompile")
  command.append("-DGODOT_API_JSON="+api)
  command.append("-DGDNATIVECPP_HOST="+platform.system().lower())
  if ("android" in system):
    command.append("-DGDNATIVECPP_TARGET=android")
  else:
    command.append("-DGDNATIVECPP_TARGET="+system)
  for parameter in parameters:
    command.append(parameter)
  os.chdir(build)
  if not (execute(command)):
    return -1
  os.chdir(cwd)
  command = []
  command.append("cmake")
  command.append("--build")
  command.append(build)
  if ("android" in system):
    os.chdir(toolchain)
  else:
    os.chdir(build)
  if not (execute(command)):
    return -2
  os.chdir(cwd)
  return 0
  
def handle(root, target, api, system, parameters, bindings, toolchain):
  name = target
  for binding in bindings:
    name = name.replace(binding, "")
  name = clean(name)
  for binding in bindings:
    build = os.path.join(root, "godot-cpp-cmake", "build", system).replace("\\", "/")
    if ((target == "all") or (((name in system) or (system in name)) and (binding in target))):
      result = make(build, api, system, parameters, binding, toolchain)
      if not (result == 0):
        print(str(result))
        return -1
  build = os.path.join(root, "godot-cpp-cmake", "lib")
  for base, folders, files in os.walk(build):
    if not (base == build):
      for name in files:
        path = os.path.join(build, name)
        if not (os.path.isfile(path)):
          shutil.copy(os.path.join(base, name), path)
  build = os.path.join(root, "build", system).replace("\\", "/")
  if ((target == "all") or (((name in system) or (system in name)) and not (inclusion(bindings, target)))):
    result = make(build, api, system, parameters, "", toolchain)
    if not (result == 0):
      print(str(result))
      return -2
  return 0
  
def run(root, target):
  #print(target)
  system = platform.system().lower()
  json = "godot_api.json"
  sdk = "30"
  bindings = []
  bindings.append("bindings")
  if not ("GODOT_HOME" in os.environ):
    return -1
  if not ("ANDROID_NDK_ROOT" in os.environ):
    return -2
  godot = os.path.join(os.environ["GODOT_HOME"], "Godot").replace("\\", "/")
  ndk = os.environ["ANDROID_NDK_ROOT"].replace("\\", "/")
  if not ((os.path.isfile(godot)) or (os.path.isfile(godot+".exe"))):
    return -3
  if not (os.path.isdir(ndk)):
    return -4
  #print(godot)
  if ((target == "all") or (inclusion(bindings, target))):
    command = []
    command.append(godot)
    command.append("--gdnative-generate-json-api")
    command.append(json)
    if not (execute(command)):
      return -5
  #print(root)
  api = os.path.join(root, json).replace("\\", "/")
  #print(api)
  parameters = []
  if ((target == "all") or (system in target)):
    result = handle(root, target, api, system, parameters, bindings, "")
    if not (result == 0):
      print(str(result))
      return -6
  androids = []
  #androids.append("armeabi-v7a")
  androids.append("arm64-v8a")
  #androids.append("x86")
  #androids.append("x86_64")
  if ((target == "all") or ("android" in target)):
    toolchain = os.path.join(ndk, "toolchains", "llvm", "prebuilt", system+"-x86_64")
    if not (os.path.isdir(toolchain)):
      return -7
    targets = {}
    marches = {}
    targets["armeabi-v7a"] = "arm7a-linux-androideabi"
    targets["arm64-v8a"] = "aarch64-linux-android"
    targets["x86"] = "i686-linux-android"
    targets["x86_64"] = "x86_64-linux-android"
    marches["armeabi-v7a"] = "armv7-a"
    marches["arm64-v8a"] = "armv8-a"
    marches["x86"] = "i686"
    marches["x86_64"] = "x86-64"
    for android in androids:
      link = os.path.join(toolchain, "sysroot", "usr", "lib", targets[android], sdk)
      if not (os.path.isdir(link)):
        return -8
      parameters = []
      parameters.append("-DCMAKE_SYSTEM_NAME=Android")
      parameters.append("-DANDROID_ABI="+android)
      parameters.append("-DANDROID_PLATFORM="+sdk)
      parameters.append("-DANDROID_NDK_ROOT="+ndk)
      parameters.append("-DGDNATIVECPP_ANDROID_TOOLCHAIN="+toolchain.replace("\\", "/"))
      if (android in targets):
        parameters.append("-DGDNATIVECPP_ANDROID_LINK="+link.replace("\\", "/"))
        parameters.append("-DGDNATIVECPP_ANDROID_TARGET="+targets[android])
      else:
        return -9
      if (android in marches):
        parameters.append("-DGDNATIVECPP_ANDROID_MARCH="+marches[android])
      else:
        return -10
      result = handle(root, target, api, "android_"+android, parameters, bindings, link)
      if not (result == 0):
        print(str(result))
        return -11
  libraries = []
  if ((target == "all") or (system in target)):
    walk = os.path.join(root, "build", system)
    if (os.path.isdir(walk)):
      for base, folders, files in os.walk(walk):
        for name in files:
          if ((name.endswith(".dll")) or (name.endswith(".so")) or (name.endswith(".dylib"))):
            path = os.path.join(base, name)
            libraries.append([path, system])
            break
    else:
      return -12
  if (((target == "all") or ("android" in target)) and not (inclusion(bindings, target))):
    for android in androids:
      walk = os.path.join(root, "build", "android_"+android)
      if (os.path.isdir(walk)):
        for base, folders, files in os.walk(walk):
          for name in files:
            if ((name.endswith(".dll")) or (name.endswith(".so")) or (name.endswith(".dylib"))):
              path = os.path.join(base, name)
              libraries.append([path, "android_"+android])
              break
      else:
        return -13
  for library in libraries:
    name = os.path.basename(library[0])
    if not (name.startswith("lib")):
      name = "lib"+name
    path = os.path.join(root, "bin", library[1])
    if not (os.path.isdir(path)):
      os.makedirs(path)
    shutil.copy(library[0], os.path.join(path, name))
  return 0
  
def launch(arguments):
  if (len(arguments) < 2):
    return False
  script = os.path.basename(arguments[0])
  target = arguments[1]
  root = wd()
  if not (sync(root)):
    return False
  if not (fix(root, script)):
    return False
  result = run(root, target)
  print(str(result))
  if not (unfix(root, script)):
    return False
  if not (result == 0):
    return False
  return True
  
if (__name__ == "__main__"):
  print(str(launch(sys.argv)))
