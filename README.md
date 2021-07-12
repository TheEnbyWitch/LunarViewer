# LunarViewer
A model viewer for Quake 1 and Hexen 2 using raylib and imgui!

## Features
 - Support for Quake 1 (IDPO) and Hexen 2 (RAPO) model formats

## Known issues

 - The code doesn't compile on anything that isn't Windows. Currently the only things from windows that I use are the MessageBox (for errors) and the Open File Dialog (Wine seems to be working alright so far)
 - Sometimes when switching the render mode, the model's texture will get unloaded. Switching the render mode again a few times will fix it.
 - Models with animated textures (groupskins) are not supported yet
 - Models with groupframes are not supported yet
 - You can't change the skin that's being previewed
 - No config of any kind is saved (apart from imgui)

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
