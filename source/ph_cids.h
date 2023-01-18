//------------------------------------------------------------------------
// Copyright(c) 2023 jul.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace MyCompanyName {
//------------------------------------------------------------------------
static const Steinberg::FUID kphilterProcessorUID (0xD58085F6, 0x11BE510E, 0xBAC77571, 0xE1491EE1);
static const Steinberg::FUID kphilterControllerUID (0xCE2277EB, 0x1E0E5139, 0xA06962CD, 0x77704BE6);

#define philterVST3Category "Fx"

//------------------------------------------------------------------------
} // namespace MyCompanyName
