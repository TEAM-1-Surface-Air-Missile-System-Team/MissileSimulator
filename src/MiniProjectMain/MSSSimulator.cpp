#pragma once
#include <nFramework/nIntegrator/nIntegrator.h>
#include <nFramework/util/util.h>
#include <nFramework/util/IniHandler.h>
#include <nFramework/nLineStream/NLineStreamMain.h>
#include <filesystem>

using namespace std::filesystem;

int main() {
    NIntegrator* nIntegrator = &(NIntegrator::getInstance());
    nlinelog::INLineLog* lgr = &(nlinelog::NLineLog::getInstance());
    nlinestream::NLineTstream ntcout{ nlinestream::Level::COUT };
    nlinestream::NLineTstream ntcerr{ nlinestream::Level::CERR };

    lgr->enableLevelTag();

    IniHandler iniHandler;


    ntcout << _T("MiniProject ") << _T("[") << _T(__FUNCTION__) << _T("] ") << std::endl;

    iniHandler.readIni(_T("MSSSimulator.ini"));

    if (iniHandler.readString(_T("SchemaRegistryData"), _T("USE")) == _T("true")) {
        tstring srPath = iniHandler.readString(_T("SchemaRegistryData"), _T("PATH"));
        tstring srXml = iniHandler.readString(_T("SchemaRegistryData"), _T("XML"));
        srPath = srPath == _T("") ? _T("") : srPath + _T("\\");
        srXml = srPath + srXml;
        nIntegrator->setSchemaRegistryData(srXml);
    }
    else {
        // else
    }

    unsigned int numOfComponents = iniHandler.readUInteger(_T("Components"), _T("Count"));

    const tstring origPath = current_path();

    /************************************************************************
        plug in components
    ************************************************************************/
    for (unsigned int cnt = 1; cnt <= numOfComponents; cnt++) {

        tstringstream dllPrefix;
        dllPrefix << _T("Component_") << cnt;

        tstring dllIndex;
        dllPrefix >> dllIndex;

        tstring power = iniHandler.readString(dllIndex, _T("POWER"));
        tstring dllPath = iniHandler.readString(dllIndex, _T("PATH"));
        tstring dllName = iniHandler.readString(dllIndex, _T("DLL"));
        tstring nomName = iniHandler.readString(dllIndex, _T("XML"));

        dllPath = dllPath == _T("") ? _T("") : dllPath + _T("\\");
        dllPath = origPath + _T("\\") + dllPath;

#ifdef _DEBUG
        dllName = dllName + _T("d.dll");
#else
        dllName = dllName + _T(".dll");
#endif // _DEBUG

        if (power == _T("on")) {
            current_path(dllPath);

            nIntegrator->plugInComponent(dllName, nomName);

            current_path(origPath);
        }
        else if (power == _T("off")) {
            tcout << dllName << _T(" Plug-in module is off. Do nothing.") << std::endl;
        }
        else {
            tcout << _T("Wrong input. Do nothing.") << std::endl;
        }
    }

    nIntegrator->validateComponentsPluggedIn();

    for (unsigned int cnt = 1; cnt <= numOfComponents; cnt++) {
        tstringstream dllPrefix;
        dllPrefix << _T("Component_") << cnt;

        tstring dllIndex;
        dllPrefix >> dllIndex;

        tstring power = iniHandler.readString(dllIndex, _T("POWER"));
        tstring dllName = iniHandler.readString(dllIndex, _T("DLL"));

        if (power == _T("on")) {
            (nIntegrator->getUserManager(dllName))->start();
        }

    }

    tcin.get();

    /************************************************************************
        plug out components
    ************************************************************************/
    for (unsigned int cnt = 1; cnt <= numOfComponents; cnt++) {
        tstringstream dllPrefix;
        dllPrefix << _T("Component_") << cnt;

        tstring dllIndex;
        dllPrefix >> dllIndex;

        tstring power = iniHandler.readString(dllIndex, _T("POWER"));
        tstring dllName = iniHandler.readString(dllIndex, _T("DLL"));

        if (power == _T("on")) {
            (nIntegrator->getUserManager(dllName))->stop();
        }

    }

    for (unsigned int cnt = 1; cnt <= numOfComponents; cnt++) {
        tstringstream dllPrefix;
        dllPrefix << _T("Component_") << cnt;

        tstring dllIndex;
        dllPrefix >> dllIndex;

        tstring power = iniHandler.readString(dllIndex, _T("POWER"));
        tstring dllName = iniHandler.readString(dllIndex, _T("DLL"));

        if (power == _T("on")) {
            nIntegrator->plugOutComponent(dllName);
            tcout << _T("Plug out ") << dllName << _T(" module.") << std::endl;
        }
    }

    ntcout << _T("The End") << std::endl;



    return 0;
}
