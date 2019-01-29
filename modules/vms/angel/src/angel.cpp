#include "angel.h"

#include "angelsystem.h"

#ifdef NEXT_SHARED
    #include "converters/angelbuilder.h"
#endif

IModule *moduleCreate(Engine *engine) {
    return new Angel(engine);
}

Angel::Angel(Engine *engine) :
        m_pEngine(engine) {
}

Angel::~Angel() {

}

const char *Angel::description() const {
    return "AngelScript Module";
}

const char *Angel::version() const {
    return "1.0";
}

uint8_t Angel::types() const {
    uint8_t result  = SYSTEM;
#ifdef NEXT_SHARED
    result  |= CONVERTER;
#endif
    return result;
}

ISystem *Angel::system() {
    return new AngelSystem();
}

IConverter *Angel::converter() {
#ifdef NEXT_SHARED
    return new AngelBuilder();
#endif
    return nullptr;
}
