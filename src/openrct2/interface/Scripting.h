#pragma region Copyright (c) 2014-2016 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#pragma once

#ifdef __cplusplus

#include <string>

interface IPlatformEnvironment;

interface IScriptEngine
{
    virtual ~IScriptEngine() = default;
    virtual void Update() abstract;
    virtual void ConsoleEval(const std::string &s) abstract;
};

IScriptEngine * CreateScriptEngine(IPlatformEnvironment * env);

#endif

#ifdef __cplusplus
extern "C"
{
#endif
    void scripting_console_execute(const utf8 * s);
    void script_engine_update();
#ifdef __cplusplus
}
#endif
