#include "angel.h"

#include "angelsystem.h"

#ifdef BUILD_SHARED
    #include "converters/angelconverter.h"
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
#ifdef BUILD_SHARED
    result  |= CONVERTER;
#endif
    return result;
}

ISystem *Angel::system() {
    return new AngelSystem(m_pEngine);
}

IConverter *Angel::converter() {
#ifdef BUILD_SHARED
    return new AngelConverter();
#endif
    return nullptr;
}
