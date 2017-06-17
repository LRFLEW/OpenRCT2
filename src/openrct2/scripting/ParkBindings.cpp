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

#include <array>
#include "../core/Math.hpp"
#include "../core/String.hpp"
#include "../localisation/LanguagePack.h"
#include "Bindings.hpp"

extern "C"
{
    #include "../management/finance.h"
    #include "../management/news_item.h"
    #include "../world/park.h"
}

namespace OpenRCT2 { namespace Scripting { namespace Bindings
{
    static duk_int_t GetRating() { return gParkRating; } 
    static void SetRating(duk_int_t value) { gParkRating = Math::Clamp(0, value, 999); } 

    static uint8 GetParkMessageType(const std::string &type)
    {
        static std::array<std::string, 9> TypeMapping =
            { "attraction", "peep_on_attraction", "peep", "money", "blank", "research", "guests", "award", "chart" };

        auto result = (uint8)NEWS_ITEM_NULL;
        auto findResult = std::find(TypeMapping.begin(), TypeMapping.end(), type);
        if (findResult != TypeMapping.end())
        {
            result = (uint8)(findResult - TypeMapping.begin()) + 1;
        }
        return result;
    }

    static duk_int_t PostMessage(duk_context * ctx)
    {
        sint32 numArgs = duk_get_top(ctx);
        if (numArgs == 0)
        {
            return DUK_RET_TYPE_ERROR;
        }

        duk_get_prop_string(ctx, 0, "type");
        duk_get_prop_string(ctx, 0, "text");
        std::string text = String::ToStd(duk_get_string(ctx, -1));
        std::string type = String::ToStd(duk_get_string(ctx, -2));
        duk_pop_3(ctx);
        duk_push_undefined(ctx);

        auto parkMessageType = GetParkMessageType(type);
        if (parkMessageType != NEWS_ITEM_NULL)
        {
            auto formattedText = EncodeString(text);
            news_item_add_to_queue_raw(parkMessageType, formattedText.c_str(), 0);
            return 1;
        }
        else
        {
            return duk_error(ctx, DUK_ERR_TYPE_ERROR, "invalid type: %s", type.c_str());
        }
    }

    void CreatePark(duk_context * ctx)
    {
        auto objIdx = duk_push_object(ctx);
        RegisterProperty<finance_get_current_cash, finance_set_current_cash>(ctx, objIdx, "cash");
        RegisterProperty<GetRating, SetRating>(ctx, objIdx, "rating");
        RegisterFunction(ctx, objIdx, "postMessage", PostMessage);
    }
} } }
