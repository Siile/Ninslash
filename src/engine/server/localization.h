#ifndef ENGINE_SERVER_LOCALIZATION_H
#define ENGINE_SERVER_LOCALIZATION_H

#include <map>
#include <string>
#include <engine/storage.h>
#include <engine/external/json-parser/json.h>
#include <engine/localization.h>

struct SLanguageFile
{
    bool m_HasLocalized = false;
    char m_aLanguageName[64];
    std::map<std::string, std::string> m_aLocalizedTexts;
};

class CLocalization : public ILocalization
{
private:
    IStorage *m_pStorage;
    std::map<std::string, SLanguageFile> m_aLocalize;
public:
    CLocalization(IStorage *pStorage);

    virtual const char *GetLanguageCode(int Country);
    virtual bool Init();
    virtual const char *Localize(const char *pLanguage, const char *pText);
    
    bool LoadIndexFile();
    bool LoadLanguage(char aFile[64]);

    void AddNewLocalize(char aLang[64], const char *pKey, const char *pValue);
};

#endif