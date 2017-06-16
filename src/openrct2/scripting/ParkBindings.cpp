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

#include "../core/Math.hpp"
#include "Bindings.hpp"

extern "C"
{
    #include "../management/finance.h"
    #include "../world/park.h"
}

namespace OpenRCT2 { namespace Scripting { namespace Bindings
{
    static duk_int_t GetRating() { return gParkRating; } 
    static void SetRating(duk_int_t value) { gParkRating = Math::Clamp(0, value, 999); } 

    void CreatePark(duk_context * ctx)
    {
        auto objIdx = duk_push_object(ctx);
        RegisterProperty<finance_get_current_cash, finance_set_current_cash>(ctx, objIdx, "cash");
        RegisterProperty<GetRating, SetRating>(ctx, objIdx, "rating");
    }
} } }
