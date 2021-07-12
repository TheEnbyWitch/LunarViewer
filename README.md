# LunarViewer

[![Windows Build](https://github.com/LunaRyuko/LunarViewer/actions/workflows/msbuild.yml/badge.svg?branch=main)](https://github.com/LunaRyuko/LunarViewer/actions/workflows/msbuild.yml) [![Linux Build](https://github.com/LunaRyuko/LunarViewer/actions/workflows/linux.yml/badge.svg?branch=main)](https://github.com/LunaRyuko/LunarViewer/actions/workflows/linux.yml)

A model viewer for Quake 1 and Hexen 2 with a focus on accurate representation.

![](docs/viewer_pic.png)

Powered by raylib and dear imgui!

## Features

 - Support for Quake 1 (IDPO) and Hexen 2 (RAPO) model formats
 - Hardware-accelerated with OpenGL, while still maintaining the software renderer look
 - Vertices are transformed and animated all on the GPU via a vertex shader
 - Supports previewing animations with interpolation

## Known issues

 - Sometimes when switching the render mode, the model's texture will get unloaded. Switching the render mode again a few times will fix it.
 - Models with animated textures (groupskins) are not supported yet
 - Models with groupframes are not supported yet
 - You can't change the skin that's being previewed
 - No config of any kind is saved (apart from imgui)
 - There's a slight offset on the UVs that I need to look into
 - The code is an absolute mess that needs to be cleaned up

## Building
### On Windows
Run `premake-2019.bat` to create a Visual Studio 2019 solution!
### On Linux/Mac
Run `premake-linux.sh` to create the Makefiles and then run `make` to build! (You can do `make config=release_x64` to make a release build)

## Helpful resources used in the making of this project

Loading Quake 1 MDL files - http://tfc.duke.free.fr/coding/mdl-specs-en.html

## Credits

 - Quake Mapping Discord (specifically Spoike and [Paril](https://www.planetminecraft.com/member/paril)) - help with the RAPO model format and flags
 - [MissLav](https://www.artstation.com/misslavender) - testing on Linux with Wine
 - [Joshua Barrett](https://github.com/jjbarr) - help with getting the application working on Linux

## Used open source projects and libraries

### raylib

https://github.com/raysan5/raylib/

Used as the rendering backend. The app also uses normal OpenGL code to do stuff that raylib itself doesn't expose.

The code was modified to fix the normal matrix (it was in view space and not world space) and to allow for the use of the stencil buffer.

### ImGui

https://github.com/ocornut/imgui

Used for the main UI

### raylib backend for ImGui

https://github.com/oskaritimperi/imgui-impl-raylib

### PhysicsFS

https://github.com/icculus/physfs

Not used for anything yet, but the goal is to use it to read files from PAKs and other archive formats it supports

### glslang

https://github.com/KhronosGroup/glslang

Used for preprocessing the shader files

### Premake

https://premake.github.io/

'nuff said
