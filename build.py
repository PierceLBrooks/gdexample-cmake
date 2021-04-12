
# Author: Pierce Brooks

import os
import sys
import shutil
import inspect
import platform
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
  
def make(build, api, system, parameters):
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
  if not (execute(command)):
    return -2
  return 0
  
def handle(root, target, api, system, parameters):
  build = os.path.join(root, "godot-cpp-cmake", "build", system).replace("\\", "/")
  if ((target == "all") or (((system in target) or (target in system)) and ("bindings" in target))):
    if not (make(build, api, system, parameters) == 0):
      return -1
  build = os.path.join(root, "godot-cpp-cmake", "lib")
  for base, folders, files in os.walk(build):
    if not (base == build):
      for name in files:
        path = os.path.join(build, name)
        if not (os.path.isfile(path)):
          shutil.copy(os.path.join(base, name), path)
  build = os.path.join(root, "build", system).replace("\\", "/")
  if ((target == "all") or (((system in target) or (target in system)) and not ("bindings" in system))):
    if not (make(build, api, system, parameters) == 0):
      return -2
  return 0
  
def run(root, target):
  #print(target)
  system = platform.system().lower()
  json = "godot_api.json"
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
    if not (handle(root, target, api, system, parameters) == 0):
      return -6
  androids = []
  androids.append("arm64-v8a")
  if ((target == "all") or ("android" in target)):
    for android in androids:
      parameters = []
      parameters.append("-DCMAKE_SYSTEM_NAME=Android")
      parameters.append("-DANDROID_ABI="+android)
      parameters.append("-DANDROID_NDK_ROOT="+ndk)
      if not (handle(root, target, api, "android_"+android, parameters) == 0):
        return -7
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
      return -8
  if ((target == "all") or ("android" in target)):
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
        return -9
  for library in libraries:
    name = os.path.basename(library[0])
    if not (name.startswith("lib")):
      name = "lib"+name
    shutil.copy(library[0], os.path.join(root, "bin", library[1], name))
  return 0
  
def launch(arguments):
  if (len(arguments) < 2):
    return False
  script = arguments[0]
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
