#include "localization.h"
#include <engine/localization.h>

CLocalization::CLocalization(IStorage *pStorage)
{
    m_pStorage = pStorage;
}

bool CLocalization::Init()
{
    if(LoadIndexFile())
        return true;
    return false;
}

bool CLocalization::LoadIndexFile()
{
    const char *pIndex = "./data/server/languages/index.json";
    IOHANDLE File = m_pStorage->OpenFile(pIndex, IOFLAG_READ, IStorage::TYPE_ALL);
	if(!File)
	{
		dbg_msg("Localization", "can't open ./server_lang/index.json");
		return false;
	}

    const int FileSize = (int)io_length(File);
	char* pFileData = (char*)malloc(FileSize);
	io_read(File, pFileData, FileSize);
	io_close(File);

    // parse json data
    json_settings JsonSettings;
    mem_zero(&JsonSettings, sizeof(JsonSettings));
    char aError[256];
    json_value *pJsonData = json_parse_ex(&JsonSettings, pFileData, FileSize, aError);
	free(pFileData);
    if(pJsonData == nullptr)
	{
		dbg_msg("Localization", "Can't load the localization file %s : %s", pIndex, aError);
		return false;
	}

    const json_value &rStart = (*pJsonData)["language indices"];

    if (rStart.type == json_array)
    {
		// Set i = 1, Skip English
        for (unsigned i = 1; i < rStart.u.array.length; ++i)
        {
			if(!LoadLanguage(rStart[i]["file"]))
				return false;
        }
    }

    // clean up
    json_value_free(pJsonData);

    return true;
}

bool CLocalization::LoadLanguage(const char *pFile)
{
    char aFilePath[64];
    str_format(aFilePath, sizeof(aFilePath), "./data/server/languages/%s.json", pFile);
    IOHANDLE File = m_pStorage->OpenFile(aFilePath, IOFLAG_READ, IStorage::TYPE_ALL);
    int FileSize = (int)io_length(File);
    char *pFileData = new char[FileSize + 1];
    io_read(File, pFileData, FileSize);
    pFileData[FileSize] = 0;
    io_close(File);

    // parse json data
    json_settings JsonSettings;
    mem_zero(&JsonSettings, sizeof(JsonSettings));
    char aError[256];
    json_value *pJsonData = json_parse_ex(&JsonSettings, pFileData, FileSize, aError);
    if(pJsonData == 0)
	{
		dbg_msg("Localization", "Can't load the localization file %s : %s", aFilePath, aError);
		delete[] pFileData;
		return false;
	}

    const json_value &rStart = (*pJsonData)["translation"];

    if (rStart.type == json_array)
    {
		str_copy(m_aLocalize[pFile].m_aLanguageName, pFile, sizeof(m_aLocalize[pFile].m_aLanguageName));
        for (unsigned i = 0; i < rStart.u.array.length; ++i)
			AddNewLocalize(pFile, rStart[i]["key"], rStart[i]["value"]);
	}

    // clean up
    json_value_free(pJsonData);
    delete[] pFileData;

    return true;
}

void CLocalization::AddNewLocalize(const char *pName, const char *pKey, const char *pValue)
{
	m_aLocalize[pName].m_aLocalizedTexts[pKey] = pValue;
}

const char *CLocalization::GetLanguageCode(int Country)
{
	// Constants from 'data/countryflags/index.txt'
	switch(Country)
	{
		/* ar - Arabic ************************************/
		case 12: //Algeria
		case 48: //Bahrain
		case 262: //Djibouti
		case 818: //Egypt
		case 368: //Iraq
		case 400: //Jordan
		case 414: //Kuwait
		case 422: //Lebanon
		case 434: //Libya
		case 478: //Mauritania
		case 504: //Morocco
		case 512: //Oman
		case 275: //Palestine
		case 634: //Qatar
		case 682: //Saudi Arabia
		case 706: //Somalia
		case 729: //Sudan
		case 760: //Syria
		case 788: //Tunisia
		case 784: //United Arab Emirates
		case 887: //Yemen
			return "ar";
		/* bg - Bosnian *************************************/
		case 100: //Bulgaria
			return "bg";
		/* bs - Bosnian *************************************/
		case 70: //Bosnia and Hercegovina
			return "bs";
		/* cs - Czech *************************************/
		case 203: //Czechia
			return "cs";
		/* de - German ************************************/
		case 40: //Austria
		case 276: //Germany
		case 438: //Liechtenstein
		case 756: //Switzerland
			return "de";
		/* el - Greek ***********************************/
		case 300: //Greece
		case 196: //Cyprus
			return "el";
		/* es - Spanish ***********************************/
		case 32: //Argentina
		case 68: //Bolivia
		case 152: //Chile
		case 170: //Colombia
		case 188: //Costa Rica
		case 192: //Cuba
		case 214: //Dominican Republic
		case 218: //Ecuador
		case 222: //El Salvador
		case 226: //Equatorial Guinea
		case 320: //Guatemala
		case 340: //Honduras
		case 484: //Mexico
		case 558: //Nicaragua
		case 591: //Panama
		case 600: //Paraguay
		case 604: //Peru
		case 630: //Puerto Rico
		case 724: //Spain
		case 858: //Uruguay
		case 862: //Venezuela
			return "es";
		/* fa - Farsi ************************************/
		case 364: //Islamic Republic of Iran
		case 4: //Afghanistan
			return "fa";
		/* fr - French ************************************/
		case 204: //Benin
		case 854: //Burkina Faso
		case 178: //Republic of the Congo
		case 384: //Cote d’Ivoire
		case 266: //Gabon
		case 324: //Ginea
		case 466: //Mali
		case 562: //Niger
		case 686: //Senegal
		case 768: //Togo
		case 250: //France
		case 492: //Monaco
			return "fr";
		/* hr - Croatian **********************************/
		case 191: //Croatia
			return "hr";
		/* hu - Hungarian *********************************/
		case 348: //Hungary
			return "hu";
		/* it - Italian ***********************************/
		case 380: //Italy
			return "it";
		/* ja - Japanese **********************************/
		case 392: //Japan
			return "ja";
		/* la - Latin *************************************/
		case 336: //Vatican
			return "la";
		/* nl - Dutch *************************************/
		case 533: //Aruba
		case 531: //Curaçao
		case 534: //Sint Maarten
		case 528: //Netherland
		case 740: //Suriname
		case 56: //Belgique
			return "nl";
		/* pl - Polish *************************************/
		case 616: //Poland
			return "pl";
		/* pt - Portuguese ********************************/
		case 24: //Angola
		case 76: //Brazil
		case 132: //Cape Verde
		//case 226: //Equatorial Guinea: official language, but not national language
		//case 446: //Macao: official language, but spoken by less than 1% of the population
		case 508: //Mozambique
		case 626: //Timor-Leste
		case 678: //São Tomé and Príncipe
			return "pt";
		/* ru - Russian ***********************************/
		case 112: //Belarus
		case 643: //Russia
		case 398: //Kazakhstan
			return "ru";
		/* sk - Slovak ************************************/
		case 703: //Slovakia
			return "sk";
		/* sr - Serbian ************************************/
		case 688: //Serbia
			return "sr";
		/* tl - Tagalog ************************************/
		case 608: //Philippines
			return "tl";
		/* tr - Turkish ************************************/
		case 31: //Azerbaijan
		case 792: //Turkey
			return "tr";
		/* uk - Ukrainian **********************************/
		case 804: //Ukraine
			return "uk";
		/* zh-cn - Chinese (Simplified) **********************************/
		case 156: //People’s Republic of China
		case 344: //Hong Kong
		case 446: //Macau
			return "zh-cn";
		case 826: // United Kingdom of Great Britain and Northern Ireland
		case 840: // United States of America
			return "en";
		default:
			return "en";
	}
}

const char *CLocalization::Localize(const char *pLanguage, const char *pText)
{
    if(m_aLocalize[pLanguage].m_aLocalizedTexts[pText].length())
        return m_aLocalize[pLanguage].m_aLocalizedTexts[pText].c_str();
    return pText;
}

ILocalization *CreateLocalization(IStorage *pStorage) { return new CLocalization(pStorage); }