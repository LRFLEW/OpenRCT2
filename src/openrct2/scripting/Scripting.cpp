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

#ifdef _WIN32
    #include <../duktape/duktape.h>
#else
    #include <duktape.h>
#endif

#include "../Context.h"
#include "../core/Console.hpp"
#include "../core/Exception.hpp"
#include "../core/File.h"
#include "../core/FileScanner.h"
#include "../core/Memory.hpp"
#include "../core/Path.hpp"
#include "../core/String.hpp"
#include "../OpenRCT2.h"
#include "../PlatformEnvironment.h"
#include "Scripting.h"

extern "C"
{
    #include "../interface/console.h"
    #include "../localisation/localisation.h"
    #include "../management/finance.h"
    #include "../ride/ride.h"
}

using namespace OpenRCT2;
using namespace OpenRCT2::Scripting;

static int bind_console_log(duk_context * ctx)
{
    IScriptEngine * engine = GetContext()->GetScriptEngine();

    sint32 numArgs = duk_get_top(ctx);
    if (numArgs == 0)
    {
        return DUK_RET_TYPE_ERROR;
    }

    std::string s = duk_safe_to_string(ctx, 0);
    Console::WriteLine("script: %s", s.c_str());
    engine->ConsoleWriteLine(s);
    return 1;
}

static int bind_park_money_get(duk_context * ctx)
{
    money32 cash = finance_get_current_cash();
    duk_push_int(ctx, cash);
    return 1;
}

static int bind_park_money_set(duk_context * ctx)
{
    sint32 numArgs = duk_get_top(ctx);
    if (numArgs == 0)
    {
        return DUK_RET_TYPE_ERROR;
    }

    money32 cash = duk_to_int(ctx, 0);
    finance_set_current_cash(cash);
    return 1;
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

        duk_push_pointer(ctx, ride);
        duk_put_prop_string(ctx, objidx, "@ride");

        duk_push_string(ctx, "totalCustomers");
        duk_push_c_function(ctx, [](duk_context * ctx2) -> int
        {
            duk_push_this(ctx2);
            duk_get_prop_string(ctx2, -1, "@ride");
            auto ride2 = (rct_ride *)duk_get_pointer(ctx2, -1);
            if (ride2 != nullptr)
            {
                sint32 result = ride2->total_customers;
                duk_push_int(ctx2, result);
            }
            else
            {
                duk_push_int(ctx2, 0);
            }
            return 1;
        }, 0);
        duk_push_c_function(ctx, [](duk_context * ctx2) -> int
        {
            duk_push_this(ctx2);
            duk_get_prop_string(ctx2, -1, "@ride");
            auto ride2 = (rct_ride *)duk_get_pointer(ctx2, -1);
            if (ride2 != nullptr)
            {
                sint32 numArgs2 = duk_get_top(ctx2);
                if (numArgs2 == 0)
                {
                    return DUK_RET_TYPE_ERROR;
                }

                sint32 value = duk_to_int(ctx2, 0);
                ride2->total_customers = value;
            }
            else
            {
                duk_push_int(ctx2, 0);
            }
            return 1;
        }, 1);
        duk_def_prop(ctx, objidx, DUK_DEFPROP_HAVE_GETTER |
                                  DUK_DEFPROP_HAVE_SETTER |
                                  DUK_DEFPROP_SET_ENUMERABLE);
    }
    return 1;
}

class ScriptEngine final : public IScriptEngine
{
private:
    IPlatformEnvironment * _env = nullptr;
    duk_context * _context = nullptr;
    bool _initialised = false;

public:
    ScriptEngine(IPlatformEnvironment * env) :
        _env(env)
    {
        _context = duk_create_heap_default();
        if (_context == nullptr)
        {
            throw Exception("Unable to initialise duktape context.");
        }
    }

    ~ScriptEngine() override
    {
        duk_destroy_heap(_context);
    }

    void Update() override
    {
        if (!_initialised)
        {
            Initialise();
        }

        if (duk_get_global_string(_context, "context"))
        {
            if (duk_get_prop_string(_context, -1, "onTick") && duk_is_callable(_context, -1))
            {
                if (duk_pcall(_context, 0) != DUK_EXEC_SUCCESS)
                {
                    std::string result = std::string(duk_safe_to_string(_context, -1));
                    ConsoleWriteLineError(result);
                }
            }
            duk_pop(_context);
        }
        duk_pop(_context);
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
            if (type == DUK_TYPE_OBJECT && !duk_is_function(_context, -1))
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

    void ConsoleWriteLine(const std::string &s) override
    {
        utf8 * text = String::Duplicate(s.c_str());
        utf8_remove_format_codes(text, false);
        console_writeline(text);
        Memory::Free(text);

        Console::WriteLine("%s", s.c_str());
    }

private:
    void Initialise()
    {
        InitialiseRuntime();
        LoadScripts();
        _initialised = true;
    }

    void InitialiseRuntime()
    {
        duk_idx_t objIdx;

        objIdx = duk_push_object(_context);
        duk_push_string(_context, "money");
        duk_push_c_function(_context, bind_park_money_get, 0);
        duk_push_c_function(_context, bind_park_money_set, 1);
        duk_def_prop(_context, objIdx, DUK_DEFPROP_HAVE_GETTER |
                                       DUK_DEFPROP_HAVE_SETTER |
                                       DUK_DEFPROP_SET_ENUMERABLE);
        duk_put_global_string(_context, "park");

        objIdx = duk_push_object(_context);
        duk_push_null(_context);
        duk_put_prop_string(_context, objIdx, "onTick");
        duk_put_global_string(_context, "context");

        objIdx = duk_push_object(_context);
        duk_push_c_function(_context, bind_console_log, DUK_VARARGS);
        duk_put_prop_string(_context, objIdx, "log");
        duk_put_global_string(_context, "console");

        objIdx = duk_push_object(_context);
        duk_push_c_function(_context, bind_get_ride, DUK_VARARGS);
        duk_put_prop_string(_context, objIdx, "getRide");
        duk_put_global_string(_context, "map");
    }

    void LoadScripts()
    {
        std::string scriptsDirectory = _env->GetDirectoryPath(DIRBASE::USER, DIRID::SCRIPTS);
        std::string pattern = Path::Combine(scriptsDirectory, "*.js");
        auto * fileScanner = Path::ScanDirectory(pattern, true);
        while (fileScanner->Next())
        {
            std::string scriptPath = fileScanner->GetPath();
            LoadScript(scriptPath);
        }
        delete fileScanner;
    }

    void LoadScript(const std::string &path)
    {
        try
        {
            size_t fileSize;
            void * fileData = File::ReadAllBytes(path, &fileSize);
            duk_uint_t flags = DUK_COMPILE_EVAL | DUK_COMPILE_SAFE | DUK_COMPILE_NORESULT | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NOFILENAME;
            duk_int_t result = duk_eval_raw(_context, (const char *)fileData, fileSize, flags);
            if (result != DUK_ERR_NONE)
            {
                ConsoleEval("Error loading script: '" + path + "'");
            }
        }
        catch (const Exception &)
        {
        }
    }

    void RegisterFunction(const std::string s, duk_c_function function)
    {
        duk_push_c_function(_context, function, DUK_VARARGS);
        duk_put_global_string(_context, s.c_str());
    }

    void ConsoleWriteLineError(std::string s)
    {
        Console::Error::WriteLine("%s", s.c_str());
        console_writeline_error(s.c_str());
    }
};

IScriptEngine * OpenRCT2::Scripting::CreateScriptEngine(IPlatformEnvironment * env)
{
    return new ScriptEngine(env);
}

extern "C"
{
    IScriptEngine * script_engine_get()
    {
        return GetContext()->GetScriptEngine();
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
