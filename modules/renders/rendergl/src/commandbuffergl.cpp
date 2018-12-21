#include "commandbuffergl.h"

#include "agl.h"
#include "analytics/profiler.h"

#include <resources/amaterialgl.h>
#include <resources/ameshgl.h>
#include <resources/atexturegl.h>

#include "resources/arendertexturegl.h"

#define TRANSFORM_BIND  0

#define COLOR_BIND  2
#define TIMER_BIND  3
#define CLIP_BIND   4

#define VERTEX_ATRIB    0
#define NORMAL_ATRIB    1
#define TANGENT_ATRIB   2
#define COLOR_ATRIB     3
#define UV0_ATRIB       4

#define INSTANCE_ATRIB  5

#define MODEL_UNIFORM   0
#define VIEW_UNIFORM    1
#define PROJ_UNIFORM    2

CommandBufferGL::CommandBufferGL() {
    PROFILER_MARKER;

    m_StaticVertex.clear();

    glGenBuffers(1, &m_InstanceBuffer);

#ifdef GL_ES_VERSION_2_0
    glGenProgramPipelinesEXT(1, &m_Pipeline);
#else
    glGenProgramPipelines(1, &m_Pipeline);
#endif
}

CommandBufferGL::~CommandBufferGL() {
#ifdef GL_ES_VERSION_2_0
    glDeleteProgramPipelinesEXT(1, &m_Pipeline);
#else
    glDeleteProgramPipelines(1, &m_Pipeline);
#endif
    glDeleteBuffers(1, &m_InstanceBuffer);
}

void CommandBufferGL::clearRenderTarget(bool clearColor, const Vector4 &color, bool clearDepth, float depth) {
    PROFILER_MARKER;

    uint32_t flags  = 0;
    if(clearColor) {
        flags   |= GL_COLOR_BUFFER_BIT;
        glClearColor(color.x, color.y, color.z, color.w);
    }
    if(clearDepth) {
        flags   |= GL_DEPTH_BUFFER_BIT;
        glClearDepthf(depth);
    }
    glClear(flags);
}

void CommandBufferGL::putUniforms(uint32_t program, MaterialInstance *instance) {
    glProgramUniform1fEXT   (program, TIMER_BIND, Timer::time());
    glProgramUniform1fEXT   (program, CLIP_BIND,  0.99f);
    glProgramUniform4fvEXT  (program, COLOR_BIND, 1, m_Color.v);

    int32_t location;
    // Push uniform values to shader
    for(const auto &it : m_Uniforms) {
        location = glGetUniformLocation(program, it.first.c_str());
        if(location > -1) {
            const Variant &data = it.second;
            switch(data.type()) {
                case MetaType::VECTOR2: glProgramUniform2fvEXT      (program, location, 1, data.toVector2().v); break;
                case MetaType::VECTOR3: glProgramUniform3fvEXT      (program, location, 1, data.toVector3().v); break;
                case MetaType::VECTOR4: glProgramUniform4fvEXT      (program, location, 1, data.toVector4().v); break;
                case MetaType::MATRIX4: glProgramUniformMatrix4fvEXT(program, location, 1, GL_FALSE, data.toMatrix4().mat); break;
                default:                glProgramUniform1fEXT       (program, location, data.toFloat()); break;
            }
        }
    }

    for(const auto &it : instance->params()) {
        location    = glGetUniformLocation(program, it.first.c_str());
        if(location > -1) {
            const MaterialInstance::Info &data  = it.second;
            switch(data.type) {
                case 0: break;
                case MetaType::INTEGER: glProgramUniform1ivEXT      (program, location, data.count, static_cast<const int32_t *>(data.ptr)); break;
                case MetaType::VECTOR2: glProgramUniform2fvEXT      (program, location, data.count, static_cast<const float *>(data.ptr)); break;
                case MetaType::VECTOR3: glProgramUniform3fvEXT      (program, location, data.count, static_cast<const float *>(data.ptr)); break;
                case MetaType::VECTOR4: glProgramUniform4fvEXT      (program, location, data.count, static_cast<const float *>(data.ptr)); break;
                case MetaType::MATRIX4: glProgramUniformMatrix4fvEXT(program, location, data.count, GL_FALSE, static_cast<const float *>(data.ptr)); break;
                default:                glProgramUniform1fvEXT      (program, location, data.count, static_cast<const float *>(data.ptr)); break;
            }
        }
    }

    AMaterialGL *mat    = static_cast<AMaterialGL *>(instance->material());

    glEnable(GL_TEXTURE_2D);
    uint8_t i   = 0;
    for(auto it : mat->textures()) {
        const Texture *tex  = static_cast<const ATextureGL *>(it.second);
        const Texture *tmp  = instance->texture(it.first.c_str());
        if(tmp) {
            tex = tmp;
        } else {
            tmp = texture(it.first.c_str());
            if(tmp) {
                tex = tmp;
            }
        }

        if(tex) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture((tex->isCubemap()) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, (uint32_t)(size_t)tex->nativeHandle());
        }
        i++;
    }
}

void CommandBufferGL::drawMesh(const Matrix4 &model, Mesh *mesh, uint32_t surface, uint8_t layer, MaterialInstance *material) {
    PROFILER_MARKER;

    if(mesh && material) {
        AMeshGL *m      = static_cast<AMeshGL *>(mesh);
        uint32_t lod    = 0;

        AMaterialGL *mat    = static_cast<AMaterialGL *>(material->material());
        uint32_t vertex     = mat->getProgram(AMaterialGL::Static);
        uint32_t fragment   = mat->bind(layer);
        if(vertex && fragment) {
            glProgramUniformMatrix4fvEXT(vertex, MODEL_UNIFORM, 1, GL_FALSE, model.mat);
            glProgramUniformMatrix4fvEXT(vertex, VIEW_UNIFORM, 1, GL_FALSE, m_View.mat);
            glProgramUniformMatrix4fvEXT(vertex, PROJ_UNIFORM, 1, GL_FALSE, m_Projection.mat);
#ifdef GL_ES_VERSION_2_0
            glUseProgramStagesEXT(m_Pipeline, GL_VERTEX_SHADER_BIT_EXT, vertex);
            glUseProgramStagesEXT(m_Pipeline, GL_FRAGMENT_SHADER_BIT_EXT, fragment);
            glBindProgramPipelineEXT(m_Pipeline);
#else
            glUseProgramStages(m_Pipeline, GL_VERTEX_SHADER_BIT, vertex);
            glUseProgramStages(m_Pipeline, GL_FRAGMENT_SHADER_BIT, fragment);
            glBindProgramPipeline(m_Pipeline);
#endif
            putUniforms(vertex, material);
            putUniforms(fragment, material);

            bindVao(m, surface, lod);

            Mesh::Modes mode    = mesh->mode(surface);
            if(mode > Mesh::MODE_LINES) {
                uint32_t vert   = mesh->vertexCount(surface, lod);
                glDrawArrays((mode == Mesh::MODE_TRIANGLE_STRIP) ? GL_TRIANGLE_STRIP : GL_LINE_STRIP, 0, vert);

                PROFILER_STAT(POLYGONS, vert - 2);
            } else {
                uint32_t index  = mesh->indexCount(surface, lod);
                glDrawElements((mode == Mesh::MODE_TRIANGLES) ? GL_TRIANGLES : GL_LINES,
                               index, GL_UNSIGNED_INT, nullptr);

                PROFILER_STAT(POLYGONS, index / 3);
            }
            PROFILER_STAT(DRAWCALLS, 1);

            glBindVertexArray(0);

            mat->unbind(layer);
#ifdef GL_ES_VERSION_2_0
            glBindProgramPipelineEXT(0);
#else
            glBindProgramPipeline(0);
#endif
        }
    }
}

void CommandBufferGL::drawMeshInstanced(const Matrix4 *models, uint32_t count, Mesh *mesh, uint32_t surface, uint8_t layer, MaterialInstance *material, bool particle) {
    PROFILER_MARKER;

    if(mesh && material) {
        AMeshGL *m      = static_cast<AMeshGL *>(mesh);
        uint32_t lod    = 0;

        AMaterialGL *mat    = static_cast<AMaterialGL *>(material->material());
        uint32_t vertex     = mat->getProgram((particle) ? AMaterialGL::Particle : AMaterialGL::Instanced);
        uint32_t fragment   = mat->bind(layer);

        if(vertex && fragment) {
            glProgramUniformMatrix4fvEXT(vertex, MODEL_UNIFORM, 1, GL_FALSE, Matrix4().mat);
            glProgramUniformMatrix4fvEXT(vertex, VIEW_UNIFORM, 1, GL_FALSE, m_View.mat);
            glProgramUniformMatrix4fvEXT(vertex, PROJ_UNIFORM, 1, GL_FALSE, m_Projection.mat);

            glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer);
            glBufferData(GL_ARRAY_BUFFER, count * sizeof(Matrix4), models, GL_DYNAMIC_DRAW);
#ifdef GL_ES_VERSION_2_0
            glUseProgramStagesEXT(m_Pipeline, GL_VERTEX_SHADER_BIT_EXT, vertex);
            glUseProgramStagesEXT(m_Pipeline, GL_FRAGMENT_SHADER_BIT_EXT, fragment);
            glBindProgramPipelineEXT(m_Pipeline);
#else
            glUseProgramStages(m_Pipeline, GL_VERTEX_SHADER_BIT, vertex);
            glUseProgramStages(m_Pipeline, GL_FRAGMENT_SHADER_BIT, fragment);
            glBindProgramPipeline(m_Pipeline);
#endif
            putUniforms(vertex, material);
            putUniforms(fragment, material);

            bindVao(m, surface, lod);

            Mesh::Modes mode    = mesh->mode(surface);
            if(mode > Mesh::MODE_LINES) {
                uint32_t vert   = mesh->vertexCount(surface, lod);
                glDrawArraysInstanced((mode == Mesh::MODE_TRIANGLE_STRIP) ? GL_TRIANGLE_STRIP : GL_LINE_STRIP, 0, vert, count);

                PROFILER_STAT(POLYGONS, index - 2 * count);
            } else {
                uint32_t index  = mesh->indexCount(surface, lod);
                glDrawElementsInstanced((mode == Mesh::MODE_TRIANGLES) ? GL_TRIANGLES : GL_LINES,
                               index, GL_UNSIGNED_INT, nullptr, count);

                PROFILER_STAT(POLYGONS, (index / 3) * count);
            }
            PROFILER_STAT(DRAWCALLS, 1);

            glBindVertexArray(0);

            mat->unbind(layer);
#ifdef GL_ES_VERSION_2_0
            glBindProgramPipelineEXT(0);
#else
            glBindProgramPipeline(0);
#endif
        }
    }
}

void CommandBufferGL::setRenderTarget(const TargetBuffer &target, const RenderTexture *depth) {
    PROFILER_MARKER;

    uint32_t colors[8];

    uint32_t buffer = 0;
    if(!target.empty()) {
        for(uint32_t i = 0; i < target.size(); i++) {
            const ARenderTextureGL *c = static_cast<const ARenderTextureGL *>(target[i]);
            if(i == 0) {
                buffer  = c->buffer();
                glBindFramebuffer(GL_FRAMEBUFFER, buffer);
            }
            colors[i]   = GL_COLOR_ATTACHMENT0 + i;
            glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, (uint32_t)(size_t)c->nativeHandle(), 0 );
        }
    }

    if(depth) {
        const ARenderTextureGL *t = static_cast<const ARenderTextureGL *>(depth);
        if(!buffer) {
            glBindFramebuffer(GL_FRAMEBUFFER, t->buffer());
        }
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, (uint32_t)(size_t)t->nativeHandle(), 0 );
    }

    if(target.size() > 1) {
        glDrawBuffers( target.size(), colors );
    }
}

void CommandBufferGL::setRenderTarget(uint32_t target) {
    glBindFramebuffer(GL_FRAMEBUFFER, target);
}

const Texture *CommandBufferGL::texture(const char *name) const {
    auto it = m_Textures.find(name);
    if(it != m_Textures.end()) {
        return (*it).second;
    }
    return nullptr;
}

void CommandBufferGL::bindVao(AMeshGL *mesh, uint32_t surface, uint32_t lod) {
    uint32_t key    = mesh->m_triangles[surface][lod];
    auto it = m_Objects.find(key);
    if(it == m_Objects.end()) {
        uint32_t id;
        glGenVertexArrays(1, &id);
        glBindVertexArray(id);

        // indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_triangles[surface][lod]);
        // vertices
        glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vertices[surface][lod]);
        glEnableVertexAttribArray(VERTEX_ATRIB);
        glVertexAttribPointer(VERTEX_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, 0);

        uint8_t flags   = mesh->flags();
        if(flags & Mesh::ATTRIBUTE_NORMALS) {
            glBindBuffer(GL_ARRAY_BUFFER, mesh->m_normals[surface][lod]);
            glEnableVertexAttribArray(NORMAL_ATRIB);
            glVertexAttribPointer(NORMAL_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        }
        if(flags & Mesh::ATTRIBUTE_TANGENTS) {
            glBindBuffer(GL_ARRAY_BUFFER, mesh->m_tangents[surface][lod]);
            glEnableVertexAttribArray(TANGENT_ATRIB);
            glVertexAttribPointer(TANGENT_ATRIB, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        }
        if(flags & Mesh::ATTRIBUTE_UV0) {
            glBindBuffer(GL_ARRAY_BUFFER, mesh->m_uv0[surface][lod]);
            glEnableVertexAttribArray(UV0_ATRIB);
            glVertexAttribPointer(UV0_ATRIB, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        }
        if(flags & Mesh::ATTRIBUTE_COLOR) {
            glBindBuffer(GL_ARRAY_BUFFER, mesh->m_colors[surface][lod]);
            glEnableVertexAttribArray(COLOR_ATRIB);
            glVertexAttribPointer(COLOR_ATRIB, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
        }

        glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer);
        for(uint32_t i = 0; i < 4; i++) {
            glEnableVertexAttribArray(INSTANCE_ATRIB + i);
            glVertexAttribPointer(INSTANCE_ATRIB + i, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4), (void *)(i * sizeof(Vector4)));
            glVertexAttribDivisor(INSTANCE_ATRIB + i, 1);
        }
/*
        } else {
            //// uv1
            //glEnableVertexAttribArray(2);
            //// uv2
            //glEnableVertexAttribArray(3);
            //// uv3
            //glEnableVertexAttribArray(4);
            //// colors
            //glEnableVertexAttribArray(7);
            //// indices
            //glEnableVertexAttribArray(8);
            //// weights
            //glEnableVertexAttribArray(9);
        }
*/
        m_Objects[key]  = id;
    } else {
        glBindVertexArray(it->second);
    }
}

void CommandBufferGL::setColor(const Vector4 &color) {
    m_Color = color;
}

void CommandBufferGL::setViewProjection(const Matrix4 &view, const Matrix4 &projection) {
    m_View          = view;
    m_Projection    = projection;
}

void CommandBufferGL::setGlobalValue(const char *name, const Variant &value) {
    m_Uniforms[name]    = value;
}

void CommandBufferGL::setGlobalTexture(const char *name, const Texture *value) {
    m_Textures[name]    = value;
}

void CommandBufferGL::setViewport(int32_t x, int32_t y, int32_t width, int32_t height) {
    glViewport(x, y, width, height);
}
