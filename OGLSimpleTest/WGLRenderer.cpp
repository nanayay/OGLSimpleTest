#include "stdafx.h"
#include "WGLRenderer.h"

WGLRenderer::WGLRenderer(HWND hWND, bool useCoreOGL)
:
hWnd(hWND),
hDC(NULL),
hGLRC(NULL),
bUseCoreOGL(useCoreOGL),
iWidth(0),
iHeight(0),
texture(0),
pbufferRenderer(NULL)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    iWidth = rc.right - rc.left;
    iHeight = rc.bottom - rc.top;

    hDC = GetDC(hWnd);
}

WGLRenderer::~WGLRenderer()
{
}

bool WGLRenderer::init()
{
    OutputDebugString(L"WGLRenderer init begin\n");

    // init ogl context
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,
        8,
        0,
        0,
        0,
        0, 0, 0
    };
    int pixelFormt = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, pixelFormt, &pfd);
    hGLRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hGLRC);

    // init glew
    glewExperimental = bUseCoreOGL ? true : false;

    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        OutputDebugString(L"Call glewInit failed\n");
        return false;
    }

    OutputDebugString(L"Call glewInit OK\n");

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iWidth, iHeight, 0, GL_RGBA, GL_UNSIGNED_INT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    pbufferRenderer = new WGLPbufferRenderer(hWnd);
    pbufferRenderer->init();

    OutputDebugString(L"WGLRenderer init end\n");
    return true;
}

bool WGLRenderer::cleanup()
{
    OutputDebugString(L"WGLRenderer cleanup begin\n");

    pbufferRenderer->cleanup();
    delete pbufferRenderer;

    glDeleteTextures(1, &texture);
    texture = 0;

    wglMakeCurrent(hDC, hGLRC);
    wglMakeCurrent(hDC, 0);
    wglDeleteContext(hGLRC);

    hGLRC = NULL;

    OutputDebugString(L"WGLRenderer cleanup end\n");
    return true;
}

bool WGLRenderer::run()
{
    OutputDebugString(L"WGLRenderer run begin\n");

    const char* vendor, *render, *version, *shaderversion;
    vendor = (const char*)glGetString(GL_VENDOR);
    render = (const char*)glGetString(GL_RENDERER);
    version = (const char*)glGetString(GL_VERSION);
    shaderversion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

#if 0
    wglMakeCurrent(hDC, hGLRC);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, iWidth, 0, iHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_CULL_FACE);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0, 0.0, 0.0); glVertex2i(iWidth / 2, iHeight / 2 + iHeight / 4);
    glColor3f(0.0, 1.0, 0.0); glVertex2i(iWidth / 2 - iWidth / 4, iHeight / 2 - iHeight / 4);
    glColor3f(0.0, 0.0, 1.0); glVertex2i(iWidth / 2 + iWidth / 4, iHeight / 2 - iHeight / 4);
    glEnd();
    SwapBuffers(hDC);
#else

    // call pbuffer's render context
    wglMakeCurrent(pbufferRenderer->hPbufferDC, pbufferRenderer->hPbufferRC);
    pbufferRenderer->run();

    // call window's render context
    wglMakeCurrent(hDC, hGLRC);
    glClearColor(1.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, iWidth, 0, iHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    wglBindTexImageARB(pbufferRenderer->hPbufferHandle, WGL_FRONT_LEFT_ARB);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_TRIANGLES);
#if 1
    glTexCoord2f(0.0, 0.0); glVertex2i(iWidth / 2, iHeight / 2 + iHeight / 4);
    glTexCoord2f(1.0, 0.0); glVertex2i(iWidth / 2 - iWidth / 4, iHeight / 2 - iHeight / 4);
    glTexCoord2f(1.0, 1.0); glVertex2i(iWidth / 2 + iWidth / 4, iHeight / 2 - iHeight / 4);
#else
    glColor3f(1.0, 0.0, 0.0); glVertex2i(iWidth / 2, iHeight / 2 + iHeight / 4);
    glColor3f(0.0, 1.0, 0.0); glVertex2i(iWidth / 2 - iWidth / 4, iHeight / 2 - iHeight / 4);
    glColor3f(0.0, 0.0, 1.0); glVertex2i(iWidth / 2 + iWidth / 4, iHeight / 2 - iHeight / 4);
#endif
    glEnd();

    glBindTexture(GL_TEXTURE_2D, texture);
    wglReleaseTexImageARB(pbufferRenderer->hPbufferHandle, WGL_FRONT_LEFT_ARB);

    SwapBuffers(hDC);
#endif

    OutputDebugString(L"WGLRenderer run end\n");
    return true;
}

WGLPbufferRenderer::WGLPbufferRenderer(HWND hWND)
: WGLRenderer(hWND, false)
{

}

WGLPbufferRenderer::~WGLPbufferRenderer()
{

}

bool WGLPbufferRenderer::init()
{
    const int MAX_ATTRIBS = 256;
    const int MAX_PFORMATS = 256;

    int iAttributes[2 * MAX_ATTRIBS];
    float fAttributes[2 * MAX_ATTRIBS];
    memset(iAttributes, 0, sizeof(iAttributes));
    memset(fAttributes, 0, sizeof(fAttributes));

    unsigned int attriIndex = 0;
    iAttributes[attriIndex++] = WGL_DRAW_TO_PBUFFER_ARB;
    iAttributes[attriIndex++] = GL_TRUE;
    iAttributes[attriIndex++] = WGL_BIND_TO_TEXTURE_RGBA_ARB;
    iAttributes[attriIndex++] = GL_TRUE;

    unsigned int numFormats = 0;
    int arrayFormats[MAX_PFORMATS];
    wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, MAX_PFORMATS, arrayFormats, &numFormats);

    int format = arrayFormats[0];
    attriIndex = 0;
    memset(iAttributes, 0, sizeof(iAttributes));

    iAttributes[attriIndex++] = WGL_TEXTURE_FORMAT_ARB;
    iAttributes[attriIndex++] = WGL_TEXTURE_RGBA_ARB;
    iAttributes[attriIndex++] = WGL_TEXTURE_TARGET_ARB;
    iAttributes[attriIndex++] = WGL_TEXTURE_2D_ARB;
    iAttributes[attriIndex++] = WGL_MIPMAP_TEXTURE_ARB;
    iAttributes[attriIndex++] = TRUE;

    hPbufferHandle = wglCreatePbufferARB(hDC, format, iWidth, iHeight, iAttributes);

    if (hPbufferHandle)
    {
        hPbufferDC = wglGetPbufferDCARB(hPbufferHandle);
        hPbufferRC = wglCreateContext(hPbufferDC);

        int width, height;
        wglQueryPbufferARB(hPbufferHandle, WGL_PBUFFER_WIDTH_ARB, &width);
        wglQueryPbufferARB(hPbufferHandle, WGL_PBUFFER_HEIGHT_ARB, &height);
    }

    return true;
}

bool WGLPbufferRenderer::run()
{
    wglMakeCurrent(hPbufferDC, hPbufferRC);
    glClearColor(0.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, iWidth, 0, iHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_CULL_FACE);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0, 0.0, 0.0); glVertex2i(iWidth / 2, iHeight / 2 + iHeight / 4);
    glColor3f(0.0, 1.0, 0.0); glVertex2i(iWidth / 2 - iWidth / 4, iHeight / 2 - iHeight / 4);
    glColor3f(0.0, 0.0, 1.0); glVertex2i(iWidth / 2 + iWidth / 4, iHeight / 2 - iHeight / 4);
    glEnd();

    return true;
}

bool WGLPbufferRenderer::cleanup()
{
    if (hPbufferHandle)
    {
        wglDeleteContext(hPbufferRC);
        wglReleasePbufferDCARB(hPbufferHandle, hPbufferDC);
        wglDestroyPbufferARB(hPbufferHandle);
        hPbufferDC = NULL;
        hPbufferDC = NULL;
        hPbufferRC = NULL;
    }

    return true;
}