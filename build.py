
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
  
def execute(command, environment):
  print(str(command))
  result = None
  try:
    if (environment == None):
      result = subprocess.check_output(command)
    else:
      result = subprocess.check_output(command, env=environment)
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
  return execute(command, None)
  
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
          #print(path)
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
  
def make(build, api, system, parameters, binding, variant, environment):
  cwd = os.getcwd()
  if not (os.path.isdir(build)):
    os.makedirs(build)
  command = []
  if ("android" in system):
    """
    if (platform.system().lower() == "windows"):
      command.append("gradlew.bat")
    else:
      command.append("./gradlew")
    command.append("clean")
    os.chdir(build)
    if not (execute(command, environment)):
      return -1
    os.chdir(cwd)
    """
    command = []
    if (platform.system().lower() == "windows"):
      command.append("gradlew.bat")
    else:
      command.append("./gradlew")
    command.append("godot:bundle"+variant+"Aar")
    command.append("-Dgodot_api_json="+api)
    command.append("-Dhost="+platform.system().lower())
    command.append("-Dtarget=android")
    command.append("-Dandroid_architecture="+system.replace("android_", ""))
    if not (len(binding) == 0):
      #command.append("-Dcmake_path="+os.path.join(build, "..", "godot-cpp-cmake", "CMakeLists.txt").replace("\\", "/"))
      pass
  else:
    command.append("cmake")
    command.append("-G")
    if ("windows" in system):
      command.append("Visual Studio 15 2017 Win64")
    else:
      command.append("Unix Makefiles")
    command.append("../..")
    #command.append("--debug-trycompile")
    command.append("-DGODOT_API_JSON="+api)
    command.append("-DGDNATIVECPP_HOST="+platform.system().lower())
    command.append("-DGDNATIVECPP_TARGET="+system)
    command.append("-DCMAKE_BUILD_TYPE="+variant)
    for parameter in parameters:
      command.append(parameter)
  os.chdir(build)
  if not (execute(command, environment)):
    return -2
  os.chdir(cwd)
  if not ("android" in system):
    command = []
    command.append("cmake")
    command.append("--build")
    command.append(build)
    os.chdir(build)
    if not (execute(command, environment)):
      return -3
    os.chdir(cwd)
  return 0
  
def handle(root, target, api, system, parameters, bindings, variant, environment):
  name = target
  for binding in bindings:
    name = name.replace(binding, "")
  name = clean(name)
  for binding in bindings:
    if ((target == "all") or (((name in system) or (system in name)) and (binding in target))):
      if ("android" in system):
        build = os.path.join(root, "android").replace("\\", "/")
      else:
        build = os.path.join(root, "godot-cpp-cmake", "build", system).replace("\\", "/")
      result = make(build, api, system, parameters, binding, variant, environment)
      if not (result == 0):
        print(str(result))
        return -1
  build = os.path.join(root, "godot-cpp-cmake", "lib").replace("\\", "/")
  if (os.path.isdir(build)):
    for base, folders, files in os.walk(build):
      if not (base == build):
        for temp in files:
          path = os.path.join(build, temp)
          if not (os.path.isfile(path)):
            shutil.copy(os.path.join(base, temp), path)
  else:
    return -2
  if ((target == "all") or (((name in system) or (system in name)) and not (inclusion(bindings, target)))):
    if ("android" in system):
      build = os.path.join(root, "android").replace("\\", "/")
      if not (os.path.isdir(os.path.join(root, "bin", system).replace("\\", "/"))):
        return -3
    else:
      build = os.path.join(root, "build", system).replace("\\", "/")
    result = make(build, api, system, parameters, "", variant, environment)
    if not (result == 0):
      print(str(result))
      return -4
  return 0
  
def run(root, target, variant):
  #print(target)
  system = platform.system().lower()
  json = "godot_api.json"
  bindings = []
  bindings.append("bindings")
  if not ("GODOT_HOME" in os.environ):
    return -1
  if ((target == "all") or ("android" in target)):
    if not ("ANDROID_NDK_ROOT" in os.environ):
      return -2
    if not ("ANDROID_SDK" in os.environ):
      return -2
  godot = os.path.join(os.environ["GODOT_HOME"], "Godot").replace("\\", "/")
  ndk = ""
  sdk = ""
  if ((target == "all") or ("android" in target)):
    ndk = os.environ["ANDROID_NDK_ROOT"].replace("\\", "/")
    sdk = os.environ["ANDROID_SDK"].replace("\\", "/")
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
    if not (execute(command, None)):
      return -5
  #print(root)
  api = os.path.join(root, json).replace("\\", "/")
  #print(api)
  parameters = []
  if ((target == "all") or (system in target)):
    result = handle(root, target, api, system, parameters, bindings, variant, None)
    if not (result == 0):
      print(str(result))
      return -6
  androids = []
  androids.append("armeabi-v7a")
  androids.append("arm64-v8a")
  androids.append("x86")
  androids.append("x86_64")
  if ((target == "all") or ("android" in target)):
    local = ""
    local += "ndk.dir="+ndk+"\n"
    local += "sdk.dir="+sdk+"\n"
    if ("windows" in system):
      local = local.replace("/", "\\\\")
    local = local.replace(":", "\\:")
    descriptor = open(os.path.join(root, "android", "local.properties").replace("\\", "/"), "wt")
    descriptor.write(local)
    descriptor.close()
    for android in androids:
      result = handle(root, target, api, "android_"+android, parameters, bindings, variant, None)
      result = 0
      if not (result == 0):
        print(str(result))
        return -11
  libraries = []
  if ((target == "all") or (system in target)):
    walk = os.path.join(root, "build", system).replace("\\", "/")
    if (os.path.isdir(walk)):
      for base, folders, files in os.walk(walk):
        for name in files:
          if ((name.endswith(".dll")) or (name.endswith(".so")) or (name.endswith(".dylib"))):
            path = os.path.join(base, name).replace("\\", "/")
            libraries.append([path, system])
            break
    else:
      return -12
  if (((target == "all") or ("android" in target)) and not (inclusion(bindings, target))):
    for android in androids:
      walk = os.path.join(root, "android", "godot", "build", "intermediates", "cmake", variant.lower(), "obj", android).replace("\\", "/")
      if (os.path.isdir(walk)):
        for base, folders, files in os.walk(walk):
          for name in files:
            if ((name.endswith(".dll")) or (name.endswith(".so")) or (name.endswith(".dylib"))):
              path = os.path.join(base, name).replace("\\", "/")
              libraries.append([path, "android_"+android])
              break
      else:
        return -13
  for library in libraries:
    name = os.path.basename(library[0])
    if not (name.startswith("lib")):
      name = "lib"+name
    path = os.path.join(root, "bin", library[1]).replace("\\", "/")
    if not (os.path.isdir(path)):
      os.makedirs(path)
    shutil.copy(library[0], os.path.join(path, name).replace("\\", "/"))
  return 0
  
def launch(arguments):
  if (len(arguments) < 3):
    return False
  variants = []
  variants.append("Debug")
  variants.append("Release")
  script = os.path.basename(arguments[0])
  variant = None
  target = arguments[2]
  root = wd()
  for i in range(len(variants)):
    if (variants[i].lower() == arguments[1])):
      variant = variants[i]
      break
  if (variant == None):
    return False
  if not (sync(root)):
    return False
  if not (fix(root, script)):
    return False
  result = run(root, target, variant)
  print(str(result))
  if not (unfix(root, script)):
    return False
  if not (result == 0):
    return False
  return True
  
if (__name__ == "__main__"):
  print(str(launch(sys.argv)))
