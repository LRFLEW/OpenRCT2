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

#ifdef _WIN32
    #include <../duktape/duktape.h>
#else
    #include <duktape.h>
#endif

#include "Scripting.h"

typedef struct rct_ride rct_ride;

namespace OpenRCT2
{
    namespace Scripting
    {
        /** The name of the property in which the native reference is stored. */
        constexpr const char * PROP_NATIVE_REF = "@NATIVE";

        /**
        * Gets the native reference attached to a script object.
        */
        template<typename T>
        T * GetNativeReference(duk_context * ctx)
        {
            duk_push_this(ctx);
            duk_get_prop_string(ctx, -1, PROP_NATIVE_REF);
            return static_cast<T *>(duk_get_pointer(ctx, -1));
        }

        static void RegisterFunction(duk_context * ctx, duk_idx_t objIdx, const char * name, duk_c_function func)
        {
            duk_push_c_function(ctx, func, DUK_VARARGS);
            duk_put_prop_string(ctx, objIdx, name);
        }

        template<duk_int_t (*TGetter)(), void (*TSetter)(duk_int_t)>
        static void RegisterProperty(duk_context * ctx, duk_idx_t objIdx, const char * name)
        {
            duk_push_string(ctx, name);
            duk_push_c_function(ctx, [](duk_context * ctx2) -> int
            {
                duk_int_t value = TGetter();
                duk_push_int(ctx2, value);
                return 1;
            }, 0);
            duk_push_c_function(ctx, [](duk_context * ctx2) -> int
            {
                sint32 numArgs = duk_get_top(ctx2);
                if (numArgs == 0)
                {
                    return DUK_RET_TYPE_ERROR;
                }

                duk_int_t value = duk_to_int(ctx2, 0);
                TSetter(value);
                return 1;
            }, 1);
            duk_def_prop(ctx, objIdx, DUK_DEFPROP_HAVE_GETTER |
                                      DUK_DEFPROP_HAVE_SETTER |
                                      DUK_DEFPROP_SET_ENUMERABLE);
        }

        namespace Bindings
        {
            void CreateMap(duk_context * ctx);
            void CreateRide(duk_context * ctx, rct_ride * ride);
            void CreatePark(duk_context * ctx);
        }
    }
}
