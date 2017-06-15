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
    #include "../ride/ride.h"
}

namespace OpenRCT2 { namespace Scripting { namespace Bindings
{
    static duk_int_t GetRide(duk_context * ctx)
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

        CreateRide(ctx, ride);
        return 1;
    }

    void CreateMap(duk_context * ctx)
    {
        auto objIdx = duk_push_object(ctx);
        RegisterFunction(ctx, objIdx, "getRide", GetRide);
    }
} } }
