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

#include "../core/Memory.hpp"
#include "../core/String.hpp"
#include "../localisation/string_ids.h"
#include "Bindings.hpp"
#include "WindowBindings.h"

extern "C"
{
    #include "../localisation/localisation.h"
}

namespace OpenRCT2
{
    namespace Scripting
    {
        namespace Bindings
        {
            CustomWindow::CustomWindow(duk_context * ctx, rct_windownumber windowNumber, const WindowDesc &desc)
                : _context(ctx),
                  _windowNumber(windowNumber)
            {
                _stashKey = "window:" + std::to_string(_windowNumber);

                _events.close = OnClose;
                _events.mouse_up = OnMouseUp;
                _events.invalidate = OnInvalidate;
                _events.paint = OnPaint;

                _widgets.push_back({ WWT_FRAME,     0, 0, 0, 0, 0,  STR_NONE,       STR_NONE }),
                _widgets.push_back({ WWT_CAPTION,   0, 1, 0, 1, 14, STR_STRINGID,   STR_WINDOW_TITLE_TIP }),
                _widgets.push_back({ WWT_CLOSEBOX,  0, 0, 0, 2, 13, STR_CLOSE_X,    STR_CLOSE_WINDOW_TIP }),
                _widgets.push_back({ WIDGETS_END });

                auto w = Open(desc);
                if (w == nullptr)
                {
                    _closed = true;
                }
                else
                {
                    CreateES();
                }
            }

            CustomWindow::~CustomWindow()
            {
                RemoveObjectFromStash(_context, _stashKey);
            }

            bool CustomWindow::IsClosed() const
            {
                return _closed;
            }

            rct_window * CustomWindow::GetWindow()
            {
                return window_find_by_number(WC_CUSTOM, _windowNumber);
            }

            rct_window * CustomWindow::Open(const WindowDesc &desc)
            {
                rct_window * window;
                if (desc.X == -1)
                {
                    window = window_create_auto_pos(desc.Width, desc.Height, &_events, WC_CUSTOM, 0);
                }
                else
                {
                    window = window_create(desc.X, desc.Y, desc.Width, desc.Height, &_events, WC_CUSTOM, 0);
                }
                if (window != nullptr)
                {
                    _classification = desc.Classification;
                    _title = desc.Title;

                    window->number = _windowNumber;
                    window->widgets = _widgets.data();
                    window->enabled_widgets = 0b111;
                    window->tag = this;
                }
                return window;
            }

            void CustomWindow::Close()
            {
                auto window = GetWindow();
                if (window != nullptr)
                {
                    window_close(window);
                }
                _closed = true;
            }

            void CustomWindow::CreateES()
            {
                auto ctx = _context;
                PushBindingObject(ctx, this);
                RegisterProperty<GetWidth, SetWidth>(ctx, -1, "width");
                RegisterProperty<GetHeight, SetHeight>(ctx, -1, "height");
                StashObject(ctx, -1, _stashKey);
                duk_pop(ctx);
            }

            void CustomWindow::PushES()
            {
                PushObjectFromStash(_context, _stashKey);
            }

            ///////////////////////////////////////////////////////////////
            // Events
            ///////////////////////////////////////////////////////////////
            void CustomWindow::OnClose(rct_window * w)
            {
                auto customWindow = static_cast<CustomWindow *>(w->tag);
                if (customWindow != nullptr)
                {
                    customWindow->_closed = true;
                }
            }

            void CustomWindow::OnMouseUp(rct_window * w, rct_widgetindex widgetIndex)
            {
                switch (widgetIndex) {
                case 2:
                    window_close(w);
                    break;
                }
            }

            void CustomWindow::OnInvalidate(rct_window * w)
            {
                w->widgets[2].left = w->width - 13;
                w->widgets[0].right = w->width - 1;
                w->widgets[1].right = w->width - 2;
                w->widgets[2].right = w->width - 3;
                w->widgets[0].bottom = w->height - 1;

                auto customWindow = static_cast<CustomWindow *>(w->tag);
                if (customWindow != nullptr)
                {
                    set_format_arg(0, rct_string_id, STR_STRING);
                    set_format_arg(2, const utf8 *, customWindow->_title.c_str());
                }
            }

            void CustomWindow::OnPaint(rct_window * w, rct_drawpixelinfo * dpi)
            {
                window_draw_widgets(w, dpi);
            }

            ///////////////////////////////////////////////////////////////
            // ES properties
            ///////////////////////////////////////////////////////////////
            duk_int_t CustomWindow::GetWidth(duk_context * ctx)
            {
                auto customWindow = GetNativeReference<CustomWindow>(ctx);
                sint32 result = 0;
                auto w = customWindow->GetWindow();
                if (w != nullptr)
                {
                    result = w->width;
                }
                duk_push_int(ctx, result);
                return 1;
            }

            void CustomWindow::SetWidth(duk_context * ctx, duk_int_t value)
            {
                auto customWindow = GetNativeReference<CustomWindow>(ctx);
                auto w = customWindow->GetWindow();
                if (w != nullptr)
                {
                    window_invalidate(w);
                    w->width = duk_get_int(ctx, 0);
                    window_invalidate(w);
                }
            }

            duk_int_t CustomWindow::GetHeight(duk_context * ctx)
            {
                auto customWindow = GetNativeReference<CustomWindow>(ctx);
                sint32 result = 0;
                auto w = customWindow->GetWindow();
                if (w != nullptr)
                {
                    result = w->height;
                }
                duk_push_int(ctx, result);
                return 1;
            }

            void CustomWindow::SetHeight(duk_context * ctx, duk_int_t value)
            {
                auto customWindow = GetNativeReference<CustomWindow>(ctx);
                auto w = customWindow->GetWindow();
                if (w != nullptr)
                {
                    window_invalidate(w);
                    w->height = duk_get_int(ctx, 0);
                    window_invalidate(w);
                }
            }
        }
    }
}
