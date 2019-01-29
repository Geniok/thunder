#ifndef MEDIASYSTEM_H
#define MEDIASYSTEM_H

#include <system.h>

#include <AL/alc.h>

class Camera;

class MediaSystem : public ISystem {
public:
    MediaSystem                 ();
    ~MediaSystem                ();

    bool                        init                        ();

    const char                 *name                        () const;

    void                        update                      (Scene *);

protected:
    ALCdevice                  *m_pDevice;
    ALCcontext                 *m_pContext;

};

#endif // MEDIASYSTEM_H
