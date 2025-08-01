#ifndef ENGINE_LOCALIZATION_H
#define ENGINE_LOCALIZATION_H

#include "kernel.h"
#include "storage.h"

class ILocalization : public IInterface
{
    MACRO_INTERFACE("localization", 0)
public:

    virtual void Init() = 0;
    virtual const char *GetLanguageCode(int Country) = 0;
    virtual const char *Localize(const char *pLanguage, const char *pText) = 0;
};

extern ILocalization *CreateLocalization(IStorage *pStorage);

#endif