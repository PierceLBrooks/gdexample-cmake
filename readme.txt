
Requirements:

 * Windows 10
 * Android Studio (SDK & NDK)
 * Visual Studio 2017
 * CMake 3.10
 * Python 3
 * Godot 3.2

Usage:

 * Export the directory path containing the install Android NDK as an environment variabled named "ANDROID_NDK_ROOT"
 * Export the directory path containing the installed Godot executable as an environment variable named "GODOT_HOME"
 * Call the "build.bat" script from the terminal
 * Open the repo clone as a project in Godot and run the main scene

Examples:

 * python build.py all
 * python build.py windows
 * python build.py android
 * python build.py windows-bindings
 * python build.py android-bindings
