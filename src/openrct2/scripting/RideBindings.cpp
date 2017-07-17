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

#include "Bindings.hpp"

extern "C"
{
    #include "../localisation/localisation.h"
    #include "../ride/ride.h"
}

#define AddPropertyInt32(TWrapper, name, getter, setter)      \
{                                                             \
    duk_push_string(Context, name);                           \
    duk_push_c_function(Context, [](duk_context * ctx) -> int \
    {                                                         \
        auto wrapper = GetNativeReference<TWrapper>(ctx);     \
        if (wrapper != nullptr)                               \
        {                                                     \
            sint32 result = wrapper->getter();                \
            duk_push_int(ctx, result);                        \
        }                                                     \
        else                                                  \
        {                                                     \
            duk_push_int(ctx, 0);                             \
        }                                                     \
        return 1;                                             \
    }, 0);                                                    \
    duk_push_c_function(Context, [](duk_context * ctx) -> int \
    {                                                         \
        auto wrapper = GetNativeReference<TWrapper>(ctx);     \
        if (wrapper != nullptr)                               \
        {                                                     \
            sint32 numArgs = duk_get_top(ctx);                \
            if (numArgs == 0) return DUK_RET_TYPE_ERROR;      \
            wrapper->setter(duk_to_int32(ctx, 0));            \
        }                                                     \
        return 0;                                             \
    }, 1);                                                    \
    duk_def_prop(Context, -4, DUK_DEFPROP_HAVE_GETTER |       \
                              DUK_DEFPROP_HAVE_SETTER |       \
                              DUK_DEFPROP_SET_ENUMERABLE);    \
}((void)(0))

namespace OpenRCT2 { namespace Scripting { namespace Bindings
{
    template<typename TWrapper, typename TNative>
    class BindingObject
    {
    public:

    protected:
        duk_context * const Context;
        TNative *     const Native;

        virtual void AddProperties() { }
    };

    class RideBindingObject : BindingObject<RideBindingObject, rct_ride>
    {
        void AddProperties() override
        {
            AddPropertyInt32(RideBindingObject, "totalCustomers", GetTotalCustomers, SetTotalCustomers);
        }

        sint32 GetTotalCustomers() { return Native->total_customers; }
        void SetTotalCustomers(sint32 value) { Native->total_customers = value; }
    };

    void CreateRide(duk_context * ctx, rct_ride * ride)
    {
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
            duk_put_prop_string(ctx, objidx, PROP_NATIVE_REF);

            duk_push_string(ctx, "totalCustomers");
            duk_push_c_function(ctx, [](duk_context * ctx2) -> int
            {
                auto ride2 = GetNativeReference<rct_ride>(ctx2);
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
                auto ride2 = GetNativeReference<rct_ride>(ctx2);
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
    }
} } }
