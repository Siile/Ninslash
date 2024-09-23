#ifndef ENGINE_SERVER_LOCALIZATION_H
#define ENGINE_SERVER_LOCALIZATION_H

#include <map>
#include <string>
#include <engine/storage.h>
#include <engine/external/json-parser/json.h>
#include <engine/localization.h>

struct SLanguageFile
{
    char m_aLanguageName[64];
    std::map<std::string, std::string> m_aLocalizedTexts;
};

class CLocalization : public ILocalization
{
private:
    IStorage *m_pStorage;
    std::map<std::string, SLanguageFile> m_aLocalize;

    static void LoadLocalizations(void *pUser);

    bool LoadLanguage(const char *aFile);
    void AddNewLocalize(const char *pName, const char *pKey, const char *pValue);

public:
    CLocalization(IStorage *pStorage);
    
    virtual void Init();
    virtual const char *GetLanguageCode(int Country);
    virtual const char *Localize(const char *pLanguage, const char *pText);
};

#endif