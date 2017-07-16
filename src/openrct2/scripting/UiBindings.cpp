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

#include <algorithm>
#include "../core/Memory.hpp"
#include "../core/String.hpp"
#include "../localisation/string_ids.h"
#include "Bindings.hpp"
#include "WindowBindings.h"

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
            class UiBinding final
            {
            private:
                std::vector<CustomWindow *> _windows;

            public:
                ~UiBinding()
                {
                    window_close_by_class(WC_CUSTOM);
                    RemoveClosedWindows();
                }

                void AddWindow(CustomWindow * customWindow)
                {
                    _windows.push_back(customWindow);
                    RemoveClosedWindows();
                }

                void RemoveClosedWindows()
                {
                    auto it = _windows.begin();
                    while (it != _windows.end())
                    {
                        auto cw = *it;
                        if (cw->IsClosed())
                        {
                            delete cw;
                            it = _windows.erase(it);
                        }
                        else
                        {
                            it++;
                        }
                    }
                }

                void CreateBindingObject(duk_context * ctx)
                {
                    auto objIdx = PushBindingObject(ctx, this);
                    RegisterFunction(ctx, objIdx, "openWindow", CreateWindow);
                    RegisterFunction(ctx, objIdx, "closeWindows", CloseWindows);
                    RegisterFunction(ctx, objIdx, "closeAllWindows", CloseAllWindows);
                }
            private:
                rct_windownumber _nextWindowNumber = 0;

                constexpr static struct { const char * name; rct_windowclass c; } ClassificationMap[] =
                {
                    { "$footpath",  WC_FOOTPATH },
                    { "$land",      WC_LAND }
                };

                std::vector<CustomWindow *> GetCustomWindows()
                {
                    return _windows;
                }

                rct_windownumber AllocateWindowNumber()
                {
                    auto result = _nextWindowNumber;
                    _nextWindowNumber++;
                    return result;
                }

                static rct_windowclass GetBuiltInClassification(const std::string &classification)
                {
                    if (!classification.empty() && classification[0] == '$')
                    {
                        for (auto entry : ClassificationMap)
                        {
                            if (String::Equals(classification.c_str(), entry.name))
                            {
                                return entry.c;
                            }
                        }
                    }
                    return WC_NULL;
                }

                static duk_int_t CreateWindow(duk_context * ctx)
                {
                    sint32 numArgs = duk_get_top(ctx);
                    if (numArgs == 0 || !duk_is_object(ctx, 0))
                    {
                        return DUK_RET_TYPE_ERROR;
                    }

                    WindowDesc desc;
                    desc.Classification = GetObjectPropString(ctx, 0, "classification");
                    desc.Title = GetObjectPropString(ctx, 0, "title");
                    desc.Width = GetObjectPropInt32(ctx, 0, "width");
                    desc.Height = GetObjectPropInt32(ctx, 0, "height");
                    if (ObjectPropExists(ctx, 0, "x"))
                    {
                        desc.X = GetObjectPropInt32(ctx, 0, "x");
                        desc.Y = GetObjectPropInt32(ctx, 0, "y");
                    }
                    else
                    {
                        desc.X = -1;
                        desc.Y = -1;
                    }

                    auto uiBinding = GetNativeReference<UiBinding>(ctx);
                    auto windowNumber = uiBinding->AllocateWindowNumber();
                    auto customWindow = new CustomWindow(ctx, windowNumber, desc);
                    if (customWindow->IsClosed())
                    {
                        duk_push_null(ctx);
                    }
                    else
                    {
                        customWindow->PushES();
                    }
                    uiBinding->AddWindow(customWindow);
                    return 1;
                }

                static duk_int_t CloseWindows(duk_context * ctx)
                {
                    sint32 numArgs = duk_get_top(ctx);
                    if (numArgs == 0)
                    {
                        return DUK_RET_TYPE_ERROR;
                    }

                    bool hasId = false;
                    sint32 id = 0;
                    if (numArgs >= 2)
                    {
                        id = duk_to_number(ctx, 1);
                    }

                    auto szClassification = String::ToStd(duk_get_string(ctx, 0));
                    auto builtInClass = GetBuiltInClassification(szClassification);
                    if (builtInClass != WC_NULL)
                    {
                        if (hasId)
                        {
                            window_close_by_number(builtInClass, id);
                        }
                        else
                        {
                            window_close_by_class(builtInClass);
                        }
                    }
                    else
                    {
                        auto uiBinding = GetNativeReference<UiBinding>(ctx);
                        auto customWindows = uiBinding->GetCustomWindows();
                        for (auto w : customWindows)
                        {
                            w->Close();
                        }
                    }
                    return 0;
                }

                static duk_int_t CloseAllWindows(duk_context * ctx)
                {
                    window_close_all();
                    return 0;
                }
            };

            void CreateUi(duk_context * ctx)
            {
                auto binding = new UiBinding();
                binding->CreateBindingObject(ctx);
            }
        }
    }
}
