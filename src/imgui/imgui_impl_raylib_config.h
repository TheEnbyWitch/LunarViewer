#ifndef H_IM_RAYLIB_CONFIG
#define H_IM_RAYLIB_CONFIG

/*	This file contains a few macros for you to play with when using the Raylib ImGui implementation.
	To use them, just uncomment the macro under its description, and it should be all set!
*/

//	COMPATIBILITY_MODE			Toggles the integrated rlgl code.
#define COMPATIBILITY_MODE

//	AUTO_FONTATLAS				REQUIRES COMPATIBILITY - Can be done manually, but is pretty useful...
//#define AUTO_FONTATLAS

/*	ENABLE_SCODETOUTF8			Toggled by default, since the code was passing
						raylib's scancodes as codepoints, and ImGui uses UTF-8
						on widgets like InputText.
*/
#define ENABLE_SCODETOUTF8

#endif
