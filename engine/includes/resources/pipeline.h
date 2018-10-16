#ifndef APIPELINE
#define APIPELINE

#include <cstdint>
#include <stack>

#include <amath.h>

#include "engine.h"

class ICommandBuffer;

class Component;
class DirectLight;
class Scene;
class Camera;

class Mesh;
class MaterialInstance;
class RenderTexture;
class PostProcessor;

class NEXT_LIBRARY_EXPORT Pipeline : public Object {
    A_REGISTER(Pipeline, Object, Resources);

public:
    Pipeline                    ();

    virtual ~Pipeline           ();

    virtual void                draw                (Scene &scene, Camera &camera, uint32_t resource);

    virtual void                resize              (uint32_t width, uint32_t height);

    void                        cameraReset         (Camera &camera);

    RenderTexture              *target              (const string &target) const;

    Mesh                       *plane               () const;

    MaterialInstance           *sprite              () const;

protected:
    void                        drawComponents      (uint32_t layer);

    void                        combineComponents   (Object &object);

    void                        updateShadows       (Camera &camera, Object &object);

    void                        directUpdate        (Camera &camera, DirectLight *light);

    RenderTexture              *postProcess         (RenderTexture &source);

protected:
    typedef map<string, RenderTexture *> TargetMap;

    ICommandBuffer             *m_Buffer;

    TargetMap                   m_Targets;

    Vector2                     m_Screen;

    list<Component *>           m_ComponentList;

    list<PostProcessor *>       m_PostEffects;

    Mesh                       *m_pPlane;
    MaterialInstance           *m_pSprite;
};

#endif // APIPELINE
