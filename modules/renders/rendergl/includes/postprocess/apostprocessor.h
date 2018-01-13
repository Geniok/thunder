#ifndef APOSTPROCESSOR_H
#define APOSTPROCESSOR_H

#include <stdint.h>

#include "resources/amaterialgl.h"
#include "resources/atexturegl.h"

#include "apipeline.h"

class APipeline;

class APostProcessor {
public:
    virtual ~APostProcessor     () {

    }

    virtual ATextureGL         *draw                (ATextureGL &source, APipeline &pipeline) {
        if(m_pMaterial) {
            pipeline.makeOrtho();
            m_pMaterial->bind(pipeline, IDrawObjectGL::UI, AMaterialGL::Static);
            pipeline.drawScreen(source, m_ResultTexture);
            return &m_ResultTexture;
        }
        return &source;
    }

    virtual void                resize              (uint32_t width, uint32_t height) {
        m_ResultTexture.resize(width, height);
    }

    virtual void                reset               (const string &path) {
        m_pMaterial = Engine::loadResource<AMaterialGL>(path);

        m_ResultTexture.create(GL_TEXTURE_2D, GL_R11F_G11F_B10F, GL_RGB, GL_FLOAT);
    }

protected:
    ATextureGL                  m_ResultTexture;

    AMaterialGL                *m_pMaterial;
};

#endif // APOSTPROCESS_H
