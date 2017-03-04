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

#include <../duktape/duktape.h>
#include "../core/Exception.hpp"

extern "C"
{
    #include "console.h"
}

class ScriptingContext
{
private:
    duk_context * _context = nullptr;

public:
    ScriptingContext()
    {
        _context = duk_create_heap_default();
        if (_context == nullptr)
        {
            throw Exception("Unable to initialise duktape context.");
        }
    }

    ~ScriptingContext()
    {
        duk_destroy_heap(_context);
    }

    void ConsoleEval(std::string s)
    {
        if (duk_peval_string(_context, s.c_str()) != 0)
        {
            std::string result = std::string(duk_safe_to_string(_context, -1));
            ConsoleWriteLineError(result);
        }
        else
        {
            std::string result = duk_to_string(_context, -1);
            ConsoleWriteLine(result);
        }
        duk_pop(_context);
    }

private:
    void ConsoleWriteLine(std::string s)
    {
        console_writeline(s.c_str());
    }

    void ConsoleWriteLineError(std::string s)
    {
        console_writeline_error(s.c_str());
    }
};

static ScriptingContext * _scriptingContext;

extern "C"
{
    void scripting_initialise()
    {
        _scriptingContext = new ScriptingContext();
    }

    void scripting_console_execute(const utf8 * s)
    {
        _scriptingContext->ConsoleEval(s);
    }
}
