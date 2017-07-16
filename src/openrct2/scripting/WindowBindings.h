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

#include <string>
#include <vector>
#include "Bindings.hpp"

extern "C"
{
    #include "../interface/widget.h"
    #include "../interface/window.h"
}

namespace OpenRCT2
{
    namespace Scripting
    {
        namespace Bindings
        {
            struct WindowDesc
            {
                std::string Classification;
                sint32      X;
                sint32      Y;
                sint32      Width;
                sint32      Height;
                std::string Title;
            };

            class CustomWindow final
            {
            private:
                duk_context * const _context;

                // Identity into global stash
                std::string         _stashKey;
                // Identity into window manager
                rct_windownumber    _windowNumber;

                // Internal custom classification
                bool                        _closed = false;
                std::string                 _classification;
                std::string                 _title;
                rct_window_event_list       _events = { nullptr };
                std::vector<rct_widget>     _widgets;

            public:
                CustomWindow(duk_context * ctx, rct_windownumber windowNumber, const WindowDesc &desc);
                ~CustomWindow();
                bool IsClosed() const;
                rct_window * GetWindow();
                void PushES();
                void Close();

            private:
                rct_window * Open(const WindowDesc &desc);
                void CreateES();

                // Window events
                static void OnClose(rct_window * w);
                static void OnMouseUp(rct_window * w, rct_widgetindex widgetIndex);
                static void OnInvalidate(rct_window * w);
                static void OnPaint(rct_window * w, rct_drawpixelinfo * dpi);

                // ES properties
                static duk_int_t GetWidth(duk_context * ctx);
                static void SetWidth(duk_context * ctx, duk_int_t value);
                static duk_int_t GetHeight(duk_context * ctx);
                static void SetHeight(duk_context * ctx, duk_int_t value);
            };
        }
    }
}
