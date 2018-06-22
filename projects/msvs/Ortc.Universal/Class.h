#pragma once

#include "Class.g.h"

namespace winrt::Ortc::implementation
{
    struct Class : ClassT<Class>
    {
        Class() = default;

        int32_t Dummy();
        void Dummy(int32_t value);
    };
}

namespace winrt::Ortc::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
