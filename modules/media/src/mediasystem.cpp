#include "mediasystem.h"

#include <AL/al.h>

#include <log.h>

#include <analytics/profiler.h>
#include <components/camera.h>
#include <components/actor.h>
#include <components/transform.h>

#include "components/audiosource.h"
#include "resources/audioclip.h"

MediaSystem::MediaSystem(Engine *engine) :
        ISystem(engine),
        m_pDevice(nullptr),
        m_pContext(nullptr) {
    PROFILER_MARKER;

    ObjectSystem system;

    AudioSource::registerClassFactory(&system);

    AudioClip::registerClassFactory(&system);
}

MediaSystem::~MediaSystem() {
    PROFILER_MARKER;

    alcDestroyContext(m_pContext);
    alcCloseDevice(m_pDevice);
}

bool MediaSystem::init() {
    PROFILER_MARKER;

    m_pDevice   = alcOpenDevice(nullptr);
    if(m_pDevice) {
        m_pContext  = alcCreateContext(m_pDevice, nullptr);
        if(alcGetError(m_pDevice) == AL_NO_ERROR) {
            alcMakeContextCurrent(m_pContext);

            return true;
        }
    }
    return false;
}

const char *MediaSystem::name() const {
    return "Media";
}

void MediaSystem::update(Scene &, uint32_t) {
    PROFILER_MARKER;

    Camera *camera  = Camera::current();
    if(camera) {
        Actor &a    = camera->actor();

        Transform *t    = a.transform();

        alListenerfv(AL_POSITION,    t->worldPosition().v);

        Quaternion rot  = t->worldRotation();

        Vector3 dir = rot * Vector3(0.0f, 0.0f,-1.0f);
        Vector3 up  = rot * Vector3(0.0f, 1.0f, 0.0f);
        float orientation[] = { dir.x, dir.y, dir.z, up.x, up.y, up.z };

        alListenerfv(AL_ORIENTATION, orientation);
    }
}
