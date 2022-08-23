#ifndef BUFFER_H
#define BUFFER_H

#include <glad/glad.h>

class FrameBuffer
{
public:
    unsigned int ID;

    FrameBuffer()
    {
        glGenFramebuffers(1, &ID);
    }

    void BindTexture(unsigned int textureID, unsigned int attachment = GL_COLOR_ATTACHMENT0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, textureID, 0);
    }

    void BindTextureMultisample(unsigned int textureID)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureID, 0);
    }

    void BindRenderBufferObject(unsigned int rboID, unsigned int attachment = GL_DEPTH_STENCIL_ATTACHMENT)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rboID);
    }

    void BindFrameBuffer()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
    }

    void UnbindFrameBuffer()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

class FrameBufferEXT
{
public:
    unsigned int ID;

    FrameBufferEXT()
    {
        glGenFramebuffersEXT(1, &ID);
    }

    void BindTexture(unsigned int textureID, unsigned int attachment = GL_COLOR_ATTACHMENT0)
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_2D, textureID, 0);
    }

    void BindTextureMultisample(unsigned int textureID, unsigned int attachment = GL_COLOR_ATTACHMENT0)
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_2D_MULTISAMPLE, textureID, 0);
    }

    void BindRenderBufferObject(unsigned int rboID, unsigned int attachment = GL_DEPTH_STENCIL_ATTACHMENT)
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ID);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, attachment, GL_RENDERBUFFER_EXT, rboID);
    }

    void BindFrameBuffer()
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ID);
    }

    void UnbindFrameBuffer()
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    }
};

class RenderBuffer
{
public:
    unsigned int ID;

    RenderBuffer(int width, int height)
    {
        glGenRenderbuffers(1, &ID);
        glBindRenderbuffer(GL_RENDERBUFFER, ID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    }
};

class RenderBufferMultisample
{
public:
    unsigned int ID;

    RenderBufferMultisample(){}

    RenderBufferMultisample(int width, int height, int samples)
    {
        glGenRenderbuffers(1, &ID);
        glBindRenderbuffer(GL_RENDERBUFFER, ID);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
    }
};

class RenderBufferMultisampleCoverageNV
{
public:
    unsigned int ID;

    RenderBufferMultisampleCoverageNV(int width, int height, int mode)
    {
        int csaaSamples[4][2] = {{4,8}, {8,8}, {4,16}, {8,16}};
        glGenRenderbuffersEXT(1,&ID);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, ID);
        glRenderbufferStorageMultisampleCoverageNV( GL_RENDERBUFFER_EXT, csaaSamples[0][mode], csaaSamples[1][mode], GL_DEPTH_COMPONENT, width, height);
    }
};

class UniformBuffer
{
public:
    unsigned int ID;

    UniformBuffer()
    {
        glGenBuffers(1, &ID);
    }

    void BindData(void *data, int size, unsigned int usage)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, ID);
        glBufferData(GL_UNIFORM_BUFFER, size, data, usage);
    }

    void BindUniformBuffer()
    {
        glBindBuffer(GL_UNIFORM_BUFFER, ID);
    }

    void UnbindUniformBuffer()
    {
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

};

#endif //BUFFER_H