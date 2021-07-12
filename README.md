# LunarViewer
A model viewer for Quake 1 and Hexen 2 with a focus on accurate representation.

Powered by raylib and dear imgui!

## Features

 - Support for Quake 1 (IDPO) and Hexen 2 (RAPO) model formats
 - Hardware-accelerated with OpenGL, while still maintaining the software renderer look
 - Vertices are transformed and animated all on the GPU via a vertex shader
 - Supports previewing animations with interpolation

## Known issues

 - The code doesn't compile on anything that isn't Windows. Currently the only things from windows that I use are the MessageBox (for errors) and the Open File Dialog (Wine seems to be working alright so far). I'm unable to dedicate time to create Linux and Mac versions by myself, so any help would be appreciated!
 - Sometimes when switching the render mode, the model's texture will get unloaded. Switching the render mode again a few times will fix it.
 - Models with animated textures (groupskins) are not supported yet
 - Models with groupframes are not supported yet
 - You can't change the skin that's being previewed
 - No config of any kind is saved (apart from imgui)
 - There's a slight offset on the UVs that I need to look into
 - The code is an absolute mess that needs to be cleaned up

## Building
### On Windows
Run `premake5-2019.bat` to create a Visual Studio 2019 solution!
### On Linux/Mac
You're on your own for now. The project uses (premake5)[https://premake.github.io/], so it should be easy enough to set up.

## Helpful resources used in the making of this project

Loading Quake 1 MDL files - http://tfc.duke.free.fr/coding/mdl-specs-en.html

## Credits

 - Quake Mapping Discord (specifically Spoike and [Paril](https://www.planetminecraft.com/member/paril)) - help with the RAPO model format and flags
 - [MissLav](https://www.artstation.com/misslavender) - testing on Linux with Wine

## Used open source projects and libraries

### raylib

https://github.com/raysan5/raylib/

Used as the rendering backend. The app also uses normal OpenGL code to do stuff that raylib itself doesn't expose.

The code was modified to fix the normal matrix (it was in view space and not world space) and to allow for the use of the stencil buffer.

### ImGui

https://github.com/ocornut/imgui

Used for the main UI

### PhysicsFS

https://github.com/icculus/physfs

Not used for anything yet, but the goal is to use it to read files from PAKs and other archive formats it supports

### glslang

https://github.com/KhronosGroup/glslang

Used for preprocessing the shader files
