#include "platform.h"

#ifdef __linux__

#include <GL/glx.h>
#include <GL/glxext.h>
#include <string.h>

static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = NULL;
// the GLX_EXT_swap_control extension does not have
// a GetSwapIntervalEXT func, use glXQueryDrawable instead

bool platform_gl_extension_supported(const char* extension)
{
    //Display* display = XOpenDisplay(":0");
    Display* display = glXGetCurrentDisplay();
    const char* extension_list = glXQueryExtensionsString(display, 0);

    // if extension not found in list
    if(strstr(extension_list, extension) == NULL) return false;

    return true;
}

bool platform_init_vsync(void)
{
    if(!platform_gl_extension_supported("GLX_EXT_swap_control")) return false;

    glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress((const GLubyte*)"glXSwapIntervalEXT");

    return true;
}

void platform_toggle_vsync(bool on)
{
    Display* display = glXGetCurrentDisplay();
    GLXDrawable drawable = glXGetCurrentDrawable();

    glXSwapIntervalEXT(display, drawable, (int)on);
}

#endif