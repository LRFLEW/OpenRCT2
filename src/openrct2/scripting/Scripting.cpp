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

#include <memory>
#include <duktape.h>

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
#include "Bindings.hpp"
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

class ScriptContext final
{
private:
    duk_context * const _context;
    std::string const   _path;

public:
    ScriptContext(duk_context * context, const std::string &path)
        : _context(context),
          _path(path)
    {
    }

    void LoadScript()
    {
        std::string projectedVariables = "context,map,park,ui";
        std::string code;
        {
            size_t fileSize;
            auto fileData = std::unique_ptr<char>((char *)File::ReadAllBytes(_path, &fileSize));
            code = std::string(fileData.get(), fileSize);
        }

        // Wrap the script in a function and pass the global objects as variables
        // so that if the script modifies them, they are not modified for other scripts.
        code = "(function(" + projectedVariables + "){" + code + "})(" + projectedVariables + ");";

        auto flags = DUK_COMPILE_EVAL | DUK_COMPILE_SAFE | DUK_COMPILE_NORESULT | DUK_COMPILE_NOSOURCE | DUK_COMPILE_NOFILENAME;
        auto result = duk_eval_raw(_context, code.c_str(), code.size(), flags);
        if (result != DUK_ERR_NONE)
        {
            throw std::runtime_error("Failed to load script.");
        }
    }
};

class ScriptEngine final : public IScriptEngine
{
private:
    IPlatformEnvironment *  _env            = nullptr;
    duk_context *           _context        = nullptr;
    bool                    _initialised    = false;

    std::vector<ScriptContext>  _scripts;

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
        if (gScreenFlags & SCREEN_FLAGS_TITLE_DEMO)
        {
            return;
        }

        if (!_initialised)
        {
            Initialise();
        }

        CallHook("tick");
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
            std::string result = Stringify(_context, -1);
            ConsoleWriteLine(result);
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
        duk_push_pointer(_context, this);
        duk_put_prop_string(_context, objIdx, PROP_NATIVE_REF);
        RegisterFunction(_context, objIdx, "subscribe", Subscribe);
        duk_put_global_string(_context, "context");

        objIdx = duk_push_object(_context);
        duk_push_c_function(_context, ConsoleLog, DUK_VARARGS);
        duk_put_prop_string(_context, objIdx, "log");
        duk_put_global_string(_context, "console");

        Bindings::CreatePark(_context);
        PutGlobal("park");
        Bindings::CreateMap(_context);
        PutGlobal("map");
        Bindings::CreateUi(_context);
        PutGlobal("ui");
    }

    static int Subscribe(duk_context * ctx)
    {
        //  [0: hook name] [1: callback]
        sint32 numArgs = duk_get_top(ctx);
        if (numArgs != 2 ||
            !duk_is_string(ctx, 0) ||
            !duk_is_callable(ctx, 1))
        {
            return DUK_RET_TYPE_ERROR;
        }

        auto hookName = "hook:" + std::string(duk_get_string(ctx, 0));

        duk_push_global_stash(ctx);
        duk_get_prop_string(ctx, 2, hookName.c_str());
        //  [0: hook name] [1: callback] [2: global] [3: global[hookName] or undefined]
        if (!duk_is_array(ctx, 3))
        {
            duk_pop(ctx);
            duk_push_array(ctx);
            duk_dup(ctx, 3);
            //  [0: hook name] [1: callback] [2: global] [3: new array] [4: new array]
            duk_put_prop_string(ctx, 2, hookName.c_str());
        }

        duk_dup(ctx, 1);
        //  [0: hook name] [1: callback] [2: global] [3: array] [4: callback]
        auto arrayLength = duk_get_length(ctx, 3);
        duk_put_prop_index(ctx, 3, (duk_uarridx_t)arrayLength);
        duk_pop_2(ctx);
        //  [0: hook name] [1: callback]
        return 0;
    }

    void CallHook(const std::string &name)
    {
        auto hookName = "hook:" + name;
        auto ctx = _context;
        duk_push_global_stash(ctx);
        duk_get_prop_string(ctx, -1, hookName.c_str());
        if (duk_is_array(ctx, -1))
        {
            auto arrayLength = duk_get_length(ctx, -1);
            for (size_t i = 0; i < arrayLength; i++)
            {
                duk_get_prop_index(ctx, -1, (duk_uarridx_t)i);
                if (duk_is_callable(ctx, -1))
                {
                    auto rc = duk_pcall(ctx, 0);
                    if (rc != DUK_EXEC_SUCCESS)
                    {
                        if (duk_is_error(ctx, -1))
                        {
                            // Accessing .stack might cause an error to be thrown, so wrap this
                            // access in a duk_safe_call() if it matters.
                            duk_get_prop_string(ctx, -1, "stack");
                            ConsoleWriteLine("error: " + std::string(duk_safe_to_string(ctx, -1)));
                            duk_pop(ctx);
                        }
                        else
                        {
                            // Non-Error value, coerce safely to string.
                            ConsoleWriteLine("error: " + std::string(duk_safe_to_string(ctx, -1)));
                        }
                    }
                }
                duk_pop(ctx);
            }
        }
        duk_pop_2(ctx);
    }

    void PutGlobal(const char * name)
    {
        duk_put_global_string(_context, name);
    }

    void LoadScripts()
    {
        auto scriptPaths = ScanScriptPaths();
        for (auto scriptPath : scriptPaths)
        {
            auto scriptContext = ScriptContext(_context, scriptPath);
            try
            {
                scriptContext.LoadScript();
                _scripts.push_back(std::move(scriptContext));
            }
            catch (const std::exception &ex)
            {
                ConsoleWriteLine("Error loading script: '" + scriptPath + "'");
                ConsoleWriteLine(ex.what());
            }
        }
    }

    std::vector<std::string> ScanScriptPaths()
    {
        auto scriptPaths = std::vector<std::string>();
        auto scriptsDirectory = _env->GetDirectoryPath(DIRBASE::USER, DIRID::SCRIPTS);
        auto pattern = Path::Combine(scriptsDirectory, "*.js");
        auto * fileScanner = Path::ScanDirectory(pattern, true);
        while (fileScanner->Next())
        {
            auto scriptPath = fileScanner->GetPath();
            scriptPaths.push_back(scriptPath);
        }
        delete fileScanner;
        return scriptPaths;
    }

    void ConsoleWriteLineError(std::string s)
    {
        Console::Error::WriteLine("%s", s.c_str());
        console_writeline_error(s.c_str());
    }

    static int ConsoleLog(duk_context * ctx)
    {
        IScriptEngine * engine = GetContext()->GetScriptEngine();

        sint32 numArgs = duk_get_top(ctx);
        if (numArgs == 0)
        {
            return DUK_RET_TYPE_ERROR;
        }

        std::string s = Stringify(ctx, 0);
        engine->ConsoleWriteLine(s);
        return 1;
    }

    static std::string Stringify(duk_context * ctx, duk_idx_t idx)
    {
        auto type = duk_get_type(ctx, idx);
        if (type == DUK_TYPE_OBJECT && !duk_is_function(ctx, -1))
        {
            return duk_json_encode(ctx, -1);
        }
        else
        {
            return duk_safe_to_string(ctx, -1);
        }
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
