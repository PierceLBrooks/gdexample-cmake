
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
  
def sync():
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
      handle = open(path, "rt")
      lines = handle.readlines()
      handle.close()
      for line in lines:
        if (pattern in line):
          print(path)
          handle = open(path, "rt")
          content = handle.read()
          handle.close()
          content = content.replace(pattern, replacement)
          handle = open(path, "wt")
          handle.write(content)
          handle.close()
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
  
def make(build, api):
  cwd = os.getcwd()
  if not (os.path.isdir(build)):
    os.makedirs(build)
  command = []
  command.append("cmake")
  command.append("-G")
  command.append("Visual Studio 15 2017 Win64")
  command.append("../..")
  command.append("-DGODOT_API_JSON="+api)
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
  
def run(root, target):
  #print(target)
  json = "godot_api.json"
  if not ("GODOT_HOME" in os.environ):
    return -1
  home = os.environ["GODOT_HOME"]
  #print(home)
  godot = os.path.join(home, "Godot").replace("\\", "/")
  #print(godot)
  command = []
  command.append(godot)
  command.append("--gdnative-generate-json-api")
  command.append(json)
  if not (execute(command)):
    return -2
  #print(root)
  api = os.path.join(root, json).replace("\\", "/")
  build = os.path.join(root, "godot-cpp-cmake", "build", platform.system().lower()).replace("\\", "/")
  if (target == "all"):
    if not (make(build, api) == 0):
      return -3
  build = os.path.join(root, "godot-cpp-cmake", "lib")
  for base, folders, files in os.walk(build):
    if not (base == build):
      for name in files:
        path = os.path.join(build, name)
        if not (os.path.isfile(path)):
          shutil.copy(os.path.join(base, name), path)
  build = os.path.join(root, "build", platform.system().lower()).replace("\\", "/")
  if not (make(build, api) == 0):
    return -4
  libraries = []
  for base, folders, files in os.walk(os.path.join(root, "build")):
    for name in files:
      if ((name.endswith(".dll")) or (name.endswith(".so")) or (name.endswith(".dylib"))):
        path = os.path.join(base, name)
        libraries.append(path)
        break
  for library in libraries:
    name = os.path.basename(library)
    if not (name.startswith("lib")):
      name = "lib"+name
    shutil.copy(library, os.path.join(root, "bin", name))
  return 0
  
def launch(arguments):
  if (len(arguments) < 2):
    return False
  script = arguments[0]
  target = arguments[1]
  root = wd()
  if not (sync()):
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
