#pragma once
#include "GL\glew.h"
#include "GL\wglew.h"

class WGLPbufferRenderer;

class WGLRenderer
{
public:
    WGLRenderer(HWND hWND, bool useCoreOGL = false);
    virtual ~WGLRenderer();

    virtual bool init();
    virtual bool cleanup();
    virtual bool run();

protected:
    HWND hWnd;
    HDC hDC;
    HGLRC hGLRC;
    bool bUseCoreOGL;

    int iWidth;
    int iHeight;

    GLuint texture;

private:
    WGLPbufferRenderer* pbufferRenderer;
};

class WGLPbufferRenderer : public WGLRenderer
{
    friend class WGLRenderer;

public:
    WGLPbufferRenderer(HWND hWND);
    virtual ~WGLPbufferRenderer();

    virtual bool init();
    virtual bool cleanup();
    virtual bool run();

private:
    HPBUFFERARB hPbufferHandle;
    HDC hPbufferDC;
    HGLRC hPbufferRC;
};
