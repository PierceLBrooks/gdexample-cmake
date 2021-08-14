
Requirements:

 * Windows 10
 * Android Studio (SDK & NDK)
 * Visual Studio 2017
 * CMake 3.10
 * Python 3
 * Godot 3.2

Usage:

 * Export the directory path containing the installed Android NDK as an environment variabled named "ANDROID_NDK_ROOT"
 * Export the directory path containing the install Android SDK as an environment variabled named "ANDROID_SDK"
 * Export the directory path containing the installed Godot executable as an environment variable named "GODOT_HOME"
 * Call the "build.bat" script from the terminal
 * Open the repository clone as a project in Godot and run the main scene

Examples:

 * python build.py [debug|release] all
 * python build.py [debug|release] windows
 * python build.py [debug|release] android
 * python build.py [debug|release] windows-bindings
 * python build.py [debug|release] android-bindings
