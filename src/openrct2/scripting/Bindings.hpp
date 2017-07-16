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

#include <duktape.h>

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
            auto instance = static_cast<T *>(duk_get_pointer(ctx, -1));
            duk_pop(ctx);
            return instance;
        }

        inline void RegisterFunction(duk_context * ctx, duk_idx_t objIdx, const char * name, duk_c_function func)
        {
            duk_push_c_function(ctx, func, DUK_VARARGS);
            duk_put_prop_string(ctx, objIdx, name);
        }

        template<duk_int_t (*TGetter)(), void (*TSetter)(duk_int_t)>
        void RegisterProperty(duk_context * ctx, duk_idx_t objIdx, const char * name)
        {
            objIdx = duk_normalize_index(ctx, objIdx);
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

        template<duk_int_t(*TGetter)(duk_context *), void(*TSetter)(duk_context *, duk_int_t)>
        void RegisterProperty(duk_context * ctx, duk_idx_t objIdx, const char * name)
        {
            objIdx = duk_normalize_index(ctx, objIdx);
            duk_push_string(ctx, name);
            duk_push_c_function(ctx, [](duk_context * ctx2) -> int
            {
                duk_int_t value = TGetter(ctx2);
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
                TSetter(ctx2, value);
                return 1;
            }, 1);
            duk_def_prop(ctx, objIdx, DUK_DEFPROP_HAVE_GETTER |
                                      DUK_DEFPROP_HAVE_SETTER |
                                      DUK_DEFPROP_SET_ENUMERABLE);
        }

        inline std::string GetObjectPropString(duk_context * ctx, duk_idx_t objIdx, const char * key)
        {
            std::string result;
            if (duk_get_prop_string(ctx, objIdx, key))
            {
                auto cstr = duk_get_string(ctx, -1);
                if (cstr != nullptr)
                {
                    result = std::string(cstr);
                }
            }
            duk_pop(ctx);
            return result;
        }

        inline sint32 GetObjectPropInt32(duk_context * ctx, duk_idx_t objIdx, const char * key)
        {
            sint32 result = 0;
            if (duk_get_prop_string(ctx, objIdx, key) && duk_is_number(ctx, -1))
            {
                result = duk_get_int(ctx, -1);
            }
            duk_pop(ctx);
            return result;
        }

        inline bool ObjectPropExists(duk_context * ctx, duk_idx_t objIdx, const char * key)
        {
            return duk_has_prop_string(ctx, objIdx, key);
        }

        template<typename T>
        duk_idx_t PushBindingObject(duk_context * ctx, T * nativeInstance)
        {
            duk_push_object(ctx);
            duk_push_pointer(ctx, nativeInstance);
            duk_put_prop_string(ctx, -2, PROP_NATIVE_REF);
            return duk_get_top_index(ctx);
        }

        inline void StashObject(duk_context * ctx, duk_idx_t objIdx, const std::string &key)
        {
            objIdx = duk_normalize_index(ctx, objIdx);
            duk_push_global_object(ctx);
            duk_dup(ctx, objIdx);
            duk_put_prop_string(ctx, -2, key.c_str());
            duk_pop(ctx);
        }

        inline void PushObjectFromStash(duk_context * ctx, const std::string &key)
        {
            duk_push_global_object(ctx);
            duk_get_prop_string(ctx, -1, key.c_str());
            duk_swap(ctx, -1, -2);
            duk_pop(ctx);
        }

        inline void RemoveObjectFromStash(duk_context * ctx, const std::string &key)
        {
            duk_push_global_object(ctx);
            duk_del_prop_string(ctx, -1, key.c_str());
            duk_pop(ctx);
        }

        namespace Bindings
        {
            void CreateMap(duk_context * ctx);
            void CreateRide(duk_context * ctx, rct_ride * ride);
            void CreatePark(duk_context * ctx);
            void CreateUi(duk_context * ctx);
        }
    }
}
