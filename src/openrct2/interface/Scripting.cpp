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
#include "../core/Memory.hpp"
#include "../core/String.hpp"
#include "../OpenRCT2.h"
#include "Scripting.h"

extern "C"
{
    #include "../localisation/localisation.h"
    #include "../ride/ride.h"
    #include "console.h"
}

static int bind_get_ride(duk_context * ctx)
{
    sint32 numArgs = duk_get_top(ctx);
    if (numArgs == 0)
    {
        return DUK_RET_TYPE_ERROR;
    }

    sint32 index = duk_to_number(ctx, 0);

    rct_ride * ride = nullptr;
    if (index >= 0 && index < MAX_RIDES)
    {
        ride = get_ride(index);
        if (ride->type == RIDE_TYPE_NULL)
        {
            ride = nullptr;
        }
    }

    if (ride == nullptr)
    {
        duk_push_null(ctx);
    }
    else
    {
        utf8 rideName[128];
        format_string(rideName, sizeof(rideName), ride->name, &ride->name_arguments);

        duk_idx_t objidx = duk_push_object(ctx);
        duk_push_string(ctx, rideName);
        duk_put_prop_string(ctx, objidx, "name");
        duk_push_int(ctx, ride->excitement);
        duk_put_prop_string(ctx, objidx, "excitement");
        duk_push_int(ctx, ride->intensity);
        duk_put_prop_string(ctx, objidx, "intensity");
        duk_push_int(ctx, ride->nausea);
        duk_put_prop_string(ctx, objidx, "nausea");
    }
    return 1;
}

class ScriptEngine final : public IScriptEngine
{
private:
    IPlatformEnvironment * _env = nullptr;
    duk_context * _context = nullptr;

public:
    ScriptEngine(IPlatformEnvironment * env) :
        _env(env)
    {
        _context = duk_create_heap_default();
        if (_context == nullptr)
        {
            throw Exception("Unable to initialise duktape context.");
        }

        RegisterFunction("GetRide", bind_get_ride);
    }

    ~ScriptEngine() override
    {
        duk_destroy_heap(_context);
    }

    void Update() override
    {

    }

    void ConsoleEval(const std::string &s) override
    {
        if (duk_peval_string(_context, s.c_str()) != 0)
        {
            std::string result = std::string(duk_safe_to_string(_context, -1));
            ConsoleWriteLineError(result);
        }
        else
        {
            auto type = duk_get_type(_context, -1);
            if (type == DUK_TYPE_OBJECT)
            {
                std::string result = duk_json_encode(_context, -1);
                ConsoleWriteLine(result);
            }
            else
            {
                std::string result = duk_to_string(_context, -1);
                ConsoleWriteLine(result);
            }
        }
        duk_pop(_context);
    }

private:
    void RegisterFunction(const std::string s, duk_c_function function)
    {
        duk_push_c_function(_context, function, DUK_VARARGS);
        duk_put_global_string(_context, s.c_str());
    }

    void ConsoleWriteLine(std::string s)
    {
        utf8 * text = String::Duplicate(s.c_str());
        utf8_remove_format_codes(text, false);
        console_writeline(text);
        Memory::Free(text);
    }

    void ConsoleWriteLineError(std::string s)
    {
        console_writeline_error(s.c_str());
    }
};

IScriptEngine * CreateScriptEngine(IPlatformEnvironment * env)
{
    return new ScriptEngine(env);
}

extern "C"
{
    IScriptEngine * script_engine_get()
    {
        return OpenRCT2::GetScriptEngine();
    }

    void scripting_console_execute(const utf8 * s)
    {
        auto scriptEngine = script_engine_get();
        scriptEngine->ConsoleEval(s);
    }

    void script_engine_update()
    {
        auto scriptEngine = script_engine_get();
        scriptEngine->Update();
    }
}
