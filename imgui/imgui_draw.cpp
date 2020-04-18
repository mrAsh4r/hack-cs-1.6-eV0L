// dear imgui, v1.63
// (drawing and font code)

// Contains implementation for
// - Default styles
// - ImDrawList
// - ImDrawData
// - ImFontAtlas
// - Internal Render Helpers
// - ImFont
// - Default font data

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"

#include <stdio.h>      // vsnprintf, sscanf, printf
#if !defined(alloca)
#if defined(__GLIBC__) || defined(__sun) || defined(__CYGWIN__)
#include <alloca.h>     // alloca (glibc uses <alloca.h>. Note that Cygwin may have _WIN32 defined, so the order matters here)
#elif defined(_WIN32)
#include <malloc.h>     // alloca
#if !defined(alloca)
#define alloca _alloca  // for clang with MS Codegen
#endif
#else
#include <stdlib.h>     // alloca
#endif
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4505) // unreferenced local function has been removed (stb stuff)
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

// Clang/GCC warnings with -Weverything
#ifdef __clang__
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"            // warning : comparing floating point with == or != is unsafe   // storing and comparing against same constants ok.
#pragma clang diagnostic ignored "-Wglobal-constructors"    // warning : declaration requires a global destructor           // similar to above, not sure what the exact difference it.
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning : implicit conversion changes signedness             //
#if __has_warning("-Wcomma")
#pragma clang diagnostic ignored "-Wcomma"                  // warning : possible misuse of comma operator here             //
#endif
#if __has_warning("-Wreserved-id-macro")
#pragma clang diagnostic ignored "-Wreserved-id-macro"      // warning : macro name is a reserved identifier                //
#endif
#if __has_warning("-Wdouble-promotion")
#pragma clang diagnostic ignored "-Wdouble-promotion"       // warning: implicit conversion from 'float' to 'double' when passing argument to function
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-function"          // warning: 'xxxx' defined but not used
#pragma GCC diagnostic ignored "-Wdouble-promotion"         // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"               // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#if __GNUC__ >= 8
#pragma GCC diagnostic ignored "-Wclass-memaccess"          // warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#endif
#endif

//-------------------------------------------------------------------------
// STB libraries implementation
//-------------------------------------------------------------------------

// Compile time options:
//#define IMGUI_STB_NAMESPACE           ImGuiStb
//#define IMGUI_STB_TRUETYPE_FILENAME   "my_folder/stb_truetype.h"
//#define IMGUI_STB_RECT_PACK_FILENAME  "my_folder/stb_rect_pack.h"
//#define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
//#define IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION

#ifdef IMGUI_STB_NAMESPACE
namespace IMGUI_STB_NAMESPACE
{
#endif

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4456)                             // declaration of 'xx' hides previous local declaration
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#pragma clang diagnostic ignored "-Wcast-qual"              // warning : cast from 'const xxxx *' to 'xxx *' drops const qualifier //
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"              // warning: comparison is always true due to limited range of data type [-Wtype-limits]
#pragma GCC diagnostic ignored "-Wcast-qual"                // warning: cast from type 'const xxxx *' to type 'xxxx *' casts away qualifiers
#endif

#ifndef STB_RECT_PACK_IMPLEMENTATION                        // in case the user already have an implementation in the _same_ compilation unit (e.g. unity builds)
#ifndef IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC
#define STBRP_ASSERT(x)     IM_ASSERT(x)
#define STBRP_SORT          ImQsort
#define STB_RECT_PACK_IMPLEMENTATION
#endif
#ifdef IMGUI_STB_RECT_PACK_FILENAME
#include IMGUI_STB_RECT_PACK_FILENAME
#else
#include "stb_rect_pack.h"
#endif
#endif

#ifndef STB_TRUETYPE_IMPLEMENTATION                         // in case the user already have an implementation in the _same_ compilation unit (e.g. unity builds)
#ifndef IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
#define STBTT_malloc(x,u)   ((void)(u), ImGui::MemAlloc(x))
#define STBTT_free(x,u)     ((void)(u), ImGui::MemFree(x))
#define STBTT_assert(x)     IM_ASSERT(x)
#define STBTT_fmod(x,y)     ImFmod(x,y)
#define STBTT_sqrt(x)       ImSqrt(x)
#define STBTT_pow(x,y)      ImPow(x,y)
#define STBTT_fabs(x)       ImFabs(x)
#define STBTT_ifloor(x)     ((int)ImFloorStd(x))
#define STBTT_iceil(x)      ((int)ImCeil(x))
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#else
#define STBTT_DEF extern
#endif
#ifdef IMGUI_STB_TRUETYPE_FILENAME
#include IMGUI_STB_TRUETYPE_FILENAME
#else
#include "stb_truetype.h"
#endif
#endif

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning (pop)
#endif

#ifdef IMGUI_STB_NAMESPACE
} // namespace ImGuiStb
using namespace IMGUI_STB_NAMESPACE;
#endif

//-----------------------------------------------------------------------------
// Style functions
//-----------------------------------------------------------------------------

void ImGui::StyleColorsDark(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.56f, 0.00f, 0.48f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.66f, 0.19f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.66f, 0.19f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.56f, 0.00f, 0.48f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.64f, 0.12f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.66f, 0.19f, 0.98f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.66f, 0.19f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.66f, 0.19f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.66f, 0.13f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.66f, 0.19f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.66f, 0.19f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.66f, 0.19f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.50f, 0.00f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.50f, 0.00f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.66f, 0.19f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.66f, 0.19f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.66f, 0.19f, 0.98f, 0.95f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.66f, 0.19f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.66f, 0.19f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void ImGui::StyleColorsClassic(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.11f, 0.11f, 0.14f, 0.92f);
    colors[ImGuiCol_Border]                 = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.43f, 0.43f, 0.43f, 0.39f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.47f, 0.47f, 0.69f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.42f, 0.41f, 0.64f, 0.69f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.27f, 0.27f, 0.54f, 0.83f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.32f, 0.32f, 0.63f, 0.87f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.40f, 0.40f, 0.55f, 0.80f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.40f, 0.40f, 0.80f, 0.30f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.80f, 0.40f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
    colors[ImGuiCol_Button]                 = ImVec4(0.35f, 0.40f, 0.61f, 0.62f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.40f, 0.48f, 0.71f, 0.79f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.46f, 0.54f, 0.80f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.40f, 0.40f, 0.90f, 0.45f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.53f, 0.53f, 0.87f, 0.80f);
    colors[ImGuiCol_Separator]              = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.60f, 0.60f, 0.70f, 1.00f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.70f, 0.70f, 0.90f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.78f, 0.82f, 1.00f, 0.60f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.78f, 0.82f, 1.00f, 0.90f);
    colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

// Those light colors are better suited with a thicker font than the default one + FrameBorder
void ImGui::StyleColorsLight(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.80f, 0.80f, 0.80f, 0.56f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_NavHighlight]           = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

void ImGui::Pink()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_Text] = ImVec4(0.92f, 0.92f, 0.85f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.92f, 0.92f, 0.85f, 0.58f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.20f, 0.19f, 0.14f, 0.55f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.49f, 0.47f, 0.20f, 0.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.92f, 0.92f, 0.85f, 0.30f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.49f, 0.47f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.78f, 0.17f, 0.75f, 0.68f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.78f, 0.17f, 0.75f, 1.00f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.78f, 0.17f, 0.75f, 0.45f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.78f, 0.17f, 0.75f, 0.35f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.78f, 0.17f, 0.75f, 0.78f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.49f, 0.47f, 0.20f, 0.57f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.49f, 0.47f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.78f, 0.17f, 0.75f, 0.31f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.78f, 0.17f, 0.75f, 0.78f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.78f, 0.17f, 0.75f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.78f, 0.17f, 0.75f, 0.80f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.78f, 0.17f, 0.75f, 0.24f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.78f, 0.17f, 0.75f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.78f, 0.17f, 0.75f, 0.44f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.78f, 0.17f, 0.75f, 0.86f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.78f, 0.17f, 0.75f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.78f, 0.17f, 0.75f, 0.76f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.78f, 0.17f, 0.75f, 0.86f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.78f, 0.17f, 0.75f, 1.00f);
	style.Colors[ImGuiCol_Column] = ImVec4(0.92f, 0.92f, 0.85f, 0.32f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.92f, 0.92f, 0.85f, 0.78f);
	style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.92f, 0.92f, 0.85f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.78f, 0.17f, 0.75f, 0.20f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.78f, 0.17f, 0.75f, 0.78f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.78f, 0.17f, 0.75f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.92f, 0.92f, 0.85f, 0.63f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.78f, 0.17f, 0.75f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.92f, 0.92f, 0.85f, 0.63f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.78f, 0.17f, 0.75f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.78f, 0.17f, 0.75f, 0.43f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

void ImGui::Purple()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_Text] = ImVec4(0.87f, 0.85f, 0.92f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.87f, 0.85f, 0.92f, 0.58f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.12f, 0.16f, 0.71f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.27f, 0.20f, 0.39f, 0.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.87f, 0.85f, 0.92f, 0.30f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.27f, 0.20f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.34f, 0.19f, 0.63f, 0.68f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.46f, 0.27f, 0.80f, 1.00f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.34f, 0.19f, 0.63f, 0.45f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.34f, 0.19f, 0.63f, 0.35f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.34f, 0.19f, 0.63f, 0.78f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.27f, 0.20f, 0.39f, 0.57f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.27f, 0.20f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.19f, 0.63f, 0.31f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.34f, 0.19f, 0.63f, 0.78f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.34f, 0.19f, 0.63f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.34f, 0.19f, 0.63f, 0.80f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.19f, 0.63f, 0.24f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.34f, 0.19f, 0.63f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.34f, 0.19f, 0.63f, 0.44f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.34f, 0.19f, 0.63f, 0.86f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.34f, 0.19f, 0.63f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.34f, 0.19f, 0.63f, 0.76f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.34f, 0.19f, 0.63f, 0.86f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.34f, 0.19f, 0.63f, 1.00f);
	style.Colors[ImGuiCol_Column] = ImVec4(0.87f, 0.85f, 0.92f, 0.32f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.87f, 0.85f, 0.92f, 0.78f);
	style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.87f, 0.85f, 0.92f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.34f, 0.19f, 0.63f, 0.20f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.34f, 0.19f, 0.63f, 0.78f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.34f, 0.19f, 0.63f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.87f, 0.85f, 0.92f, 0.63f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.34f, 0.19f, 0.63f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.87f, 0.85f, 0.92f, 0.63f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.34f, 0.19f, 0.63f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.34f, 0.19f, 0.63f, 0.43f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

void ImGui::CherryTheme() {
	// cherry colors, 3 intensities
#define HI(v)   ImVec4(0.502f, 0.075f, 0.256f, v)
#define MED(v)  ImVec4(0.455f, 0.198f, 0.301f, v)
#define LOW(v)  ImVec4(0.232f, 0.201f, 0.271f, v)
// backgrounds (@todo: complete with BG_MED, BG_LOW)
#define BG(v)   ImVec4(0.200f, 0.220f, 0.270f, v)
// text
#define TEXT(v) ImVec4(0.860f, 0.930f, 0.890f, v)

	auto &style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text] = TEXT(0.78f);
	style.Colors[ImGuiCol_TextDisabled] = TEXT(0.28f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
	style.Colors[ImGuiCol_ChildWindowBg] = BG(0.58f);
	style.Colors[ImGuiCol_PopupBg] = BG(0.9f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.31f, 0.31f, 1.00f, 0.00f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = BG(1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = MED(0.78f);
	style.Colors[ImGuiCol_FrameBgActive] = MED(1.00f);
	style.Colors[ImGuiCol_TitleBg] = LOW(1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = HI(1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = BG(0.75f);
	style.Colors[ImGuiCol_MenuBarBg] = BG(0.47f);
	style.Colors[ImGuiCol_ScrollbarBg] = BG(1.00f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = MED(0.78f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = MED(1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
	style.Colors[ImGuiCol_ButtonHovered] = MED(0.86f);
	style.Colors[ImGuiCol_ButtonActive] = MED(1.00f);
	style.Colors[ImGuiCol_Header] = MED(0.76f);
	style.Colors[ImGuiCol_HeaderHovered] = MED(0.86f);
	style.Colors[ImGuiCol_HeaderActive] = HI(1.00f);
	style.Colors[ImGuiCol_Column] = ImVec4(0.14f, 0.16f, 0.19f, 1.00f);
	style.Colors[ImGuiCol_ColumnHovered] = MED(0.78f);
	style.Colors[ImGuiCol_ColumnActive] = MED(1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
	style.Colors[ImGuiCol_ResizeGripHovered] = MED(0.78f);
	style.Colors[ImGuiCol_ResizeGripActive] = MED(1.00f);
	style.Colors[ImGuiCol_PlotLines] = TEXT(0.63f);
	style.Colors[ImGuiCol_PlotLinesHovered] = MED(1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = TEXT(0.63f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = MED(1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = MED(0.43f);
	// [...]
	style.Colors[ImGuiCol_ModalWindowDarkening] = BG(0.73f);

	style.WindowPadding = ImVec2(6, 4);
	style.WindowRounding = 0.0f;
	style.FramePadding = ImVec2(5, 2);
	style.FrameRounding = 3.0f;
	style.ItemSpacing = ImVec2(7, 1);
	style.ItemInnerSpacing = ImVec2(1, 1);
	style.TouchExtraPadding = ImVec2(0, 0);
	style.IndentSpacing = 6.0f;
	style.ScrollbarSize = 12.0f;
	style.ScrollbarRounding = 16.0f;
	style.GrabMinSize = 20.0f;
	style.GrabRounding = 2.0f;

	style.WindowTitleAlign.x = 0.50f;

	style.Colors[ImGuiCol_Border] = ImVec4(0.539f, 0.479f, 0.255f, 0.162f);
	style.FrameBorderSize = 0.0f;
	style.WindowBorderSize = 1.0f;
}

//-----------------------------------------------------------------------------
// ImDrawListData
//-----------------------------------------------------------------------------

ImDrawListSharedData::ImDrawListSharedData()
{
    Font = NULL;
    FontSize = 0.0f;
    CurveTessellationTol = 0.0f;
    ClipRectFullscreen = ImVec4(-8192.0f, -8192.0f, +8192.0f, +8192.0f);

    // Const data
    for (int i = 0; i < IM_ARRAYSIZE(CircleVtx12); i++)
    {
        const float a = ((float)i * 2 * IM_PI) / (float)IM_ARRAYSIZE(CircleVtx12);
        CircleVtx12[i] = ImVec2(ImCos(a), ImSin(a));
    }
}

//-----------------------------------------------------------------------------
// ImDrawList
//-----------------------------------------------------------------------------

void ImDrawList::Clear()
{
    CmdBuffer.resize(0);
    IdxBuffer.resize(0);
    VtxBuffer.resize(0);
    Flags = ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedFill;
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.resize(0);
    _TextureIdStack.resize(0);
    _Path.resize(0);
    _ChannelsCurrent = 0;
    _ChannelsCount = 1;
    // NB: Do not clear channels so our allocations are re-used after the first frame.
}

void ImDrawList::ClearFreeMemory()
{
    CmdBuffer.clear();
    IdxBuffer.clear();
    VtxBuffer.clear();
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.clear();
    _TextureIdStack.clear();
    _Path.clear();
    _ChannelsCurrent = 0;
    _ChannelsCount = 1;
    for (int i = 0; i < _Channels.Size; i++)
    {
        if (i == 0) memset(&_Channels[0], 0, sizeof(_Channels[0]));  // channel 0 is a copy of CmdBuffer/IdxBuffer, don't destruct again
        _Channels[i].CmdBuffer.clear();
        _Channels[i].IdxBuffer.clear();
    }
    _Channels.clear();
}

ImDrawList* ImDrawList::CloneOutput() const
{
    ImDrawList* dst = IM_NEW(ImDrawList(NULL));
    dst->CmdBuffer = CmdBuffer;
    dst->IdxBuffer = IdxBuffer;
    dst->VtxBuffer = VtxBuffer;
    dst->Flags = Flags;
    return dst;
}

// Using macros because C++ is a terrible language, we want guaranteed inline, no code in header, and no overhead in Debug builds
#define GetCurrentClipRect()    (_ClipRectStack.Size ? _ClipRectStack.Data[_ClipRectStack.Size-1]  : _Data->ClipRectFullscreen)
#define GetCurrentTextureId()   (_TextureIdStack.Size ? _TextureIdStack.Data[_TextureIdStack.Size-1] : NULL)

void ImDrawList::AddDrawCmd()
{
    ImDrawCmd draw_cmd;
    draw_cmd.ClipRect = GetCurrentClipRect();
    draw_cmd.TextureId = GetCurrentTextureId();

    IM_ASSERT(draw_cmd.ClipRect.x <= draw_cmd.ClipRect.z && draw_cmd.ClipRect.y <= draw_cmd.ClipRect.w);
    CmdBuffer.push_back(draw_cmd);
}

void ImDrawList::AddCallback(ImDrawCallback callback, void* callback_data)
{
    ImDrawCmd* current_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
    if (!current_cmd || current_cmd->ElemCount != 0 || current_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        current_cmd = &CmdBuffer.back();
    }
    current_cmd->UserCallback = callback;
    current_cmd->UserCallbackData = callback_data;

    AddDrawCmd(); // Force a new command after us (see comment below)
}

// Our scheme may appears a bit unusual, basically we want the most-common calls AddLine AddRect etc. to not have to perform any check so we always have a command ready in the stack.
// The cost of figuring out if a new command has to be added or if we can merge is paid in those Update** functions only.
void ImDrawList::UpdateClipRect()
{
    // If current command is used with different settings we need to add a new command
    const ImVec4 curr_clip_rect = GetCurrentClipRect();
    ImDrawCmd* curr_cmd = CmdBuffer.Size > 0 ? &CmdBuffer.Data[CmdBuffer.Size-1] : NULL;
    if (!curr_cmd || (curr_cmd->ElemCount != 0 && memcmp(&curr_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) != 0) || curr_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        return;
    }

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
    if (curr_cmd->ElemCount == 0 && prev_cmd && memcmp(&prev_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) == 0 && prev_cmd->TextureId == GetCurrentTextureId() && prev_cmd->UserCallback == NULL)
        CmdBuffer.pop_back();
    else
        curr_cmd->ClipRect = curr_clip_rect;
}

void ImDrawList::UpdateTextureID()
{
    // If current command is used with different settings we need to add a new command
    const ImTextureID curr_texture_id = GetCurrentTextureId();
    ImDrawCmd* curr_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
    if (!curr_cmd || (curr_cmd->ElemCount != 0 && curr_cmd->TextureId != curr_texture_id) || curr_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        return;
    }

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
    if (curr_cmd->ElemCount == 0 && prev_cmd && prev_cmd->TextureId == curr_texture_id && memcmp(&prev_cmd->ClipRect, &GetCurrentClipRect(), sizeof(ImVec4)) == 0 && prev_cmd->UserCallback == NULL)
        CmdBuffer.pop_back();
    else
        curr_cmd->TextureId = curr_texture_id;
}

#undef GetCurrentClipRect
#undef GetCurrentTextureId

// Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
void ImDrawList::PushClipRect(ImVec2 cr_min, ImVec2 cr_max, bool intersect_with_current_clip_rect)
{
    ImVec4 cr(cr_min.x, cr_min.y, cr_max.x, cr_max.y);
    if (intersect_with_current_clip_rect && _ClipRectStack.Size)
    {
        ImVec4 current = _ClipRectStack.Data[_ClipRectStack.Size-1];
        if (cr.x < current.x) cr.x = current.x;
        if (cr.y < current.y) cr.y = current.y;
        if (cr.z > current.z) cr.z = current.z;
        if (cr.w > current.w) cr.w = current.w;
    }
    cr.z = ImMax(cr.x, cr.z);
    cr.w = ImMax(cr.y, cr.w);

    _ClipRectStack.push_back(cr);
    UpdateClipRect();
}

void ImDrawList::PushClipRectFullScreen()
{
    PushClipRect(ImVec2(_Data->ClipRectFullscreen.x, _Data->ClipRectFullscreen.y), ImVec2(_Data->ClipRectFullscreen.z, _Data->ClipRectFullscreen.w));
}

void ImDrawList::PopClipRect()
{
    IM_ASSERT(_ClipRectStack.Size > 0);
    _ClipRectStack.pop_back();
    UpdateClipRect();
}

void ImDrawList::PushTextureID(ImTextureID texture_id)
{
    _TextureIdStack.push_back(texture_id);
    UpdateTextureID();
}

void ImDrawList::PopTextureID()
{
    IM_ASSERT(_TextureIdStack.Size > 0);
    _TextureIdStack.pop_back();
    UpdateTextureID();
}

void ImDrawList::ChannelsSplit(int channels_count)
{
    IM_ASSERT(_ChannelsCurrent == 0 && _ChannelsCount == 1);
    int old_channels_count = _Channels.Size;
    if (old_channels_count < channels_count)
        _Channels.resize(channels_count);
    _ChannelsCount = channels_count;

    // _Channels[] (24/32 bytes each) hold storage that we'll swap with this->_CmdBuffer/_IdxBuffer
    // The content of _Channels[0] at this point doesn't matter. We clear it to make state tidy in a debugger but we don't strictly need to.
    // When we switch to the next channel, we'll copy _CmdBuffer/_IdxBuffer into _Channels[0] and then _Channels[1] into _CmdBuffer/_IdxBuffer
    memset(&_Channels[0], 0, sizeof(ImDrawChannel));
    for (int i = 1; i < channels_count; i++)
    {
        if (i >= old_channels_count)
        {
            IM_PLACEMENT_NEW(&_Channels[i]) ImDrawChannel();
        }
        else
        {
            _Channels[i].CmdBuffer.resize(0);
            _Channels[i].IdxBuffer.resize(0);
        }
        if (_Channels[i].CmdBuffer.Size == 0)
        {
            ImDrawCmd draw_cmd;
            draw_cmd.ClipRect = _ClipRectStack.back();
            draw_cmd.TextureId = _TextureIdStack.back();
            _Channels[i].CmdBuffer.push_back(draw_cmd);
        }
    }
}

void ImDrawList::ChannelsMerge()
{
    // Note that we never use or rely on channels.Size because it is merely a buffer that we never shrink back to 0 to keep all sub-buffers ready for use.
    if (_ChannelsCount <= 1)
        return;

    ChannelsSetCurrent(0);
    if (CmdBuffer.Size && CmdBuffer.back().ElemCount == 0)
        CmdBuffer.pop_back();

    int new_cmd_buffer_count = 0, new_idx_buffer_count = 0;
    for (int i = 1; i < _ChannelsCount; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (ch.CmdBuffer.Size && ch.CmdBuffer.back().ElemCount == 0)
            ch.CmdBuffer.pop_back();
        new_cmd_buffer_count += ch.CmdBuffer.Size;
        new_idx_buffer_count += ch.IdxBuffer.Size;
    }
    CmdBuffer.resize(CmdBuffer.Size + new_cmd_buffer_count);
    IdxBuffer.resize(IdxBuffer.Size + new_idx_buffer_count);

    ImDrawCmd* cmd_write = CmdBuffer.Data + CmdBuffer.Size - new_cmd_buffer_count;
    _IdxWritePtr = IdxBuffer.Data + IdxBuffer.Size - new_idx_buffer_count;
    for (int i = 1; i < _ChannelsCount; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (int sz = ch.CmdBuffer.Size) { memcpy(cmd_write, ch.CmdBuffer.Data, sz * sizeof(ImDrawCmd)); cmd_write += sz; }
        if (int sz = ch.IdxBuffer.Size) { memcpy(_IdxWritePtr, ch.IdxBuffer.Data, sz * sizeof(ImDrawIdx)); _IdxWritePtr += sz; }
    }
    UpdateClipRect(); // We call this instead of AddDrawCmd(), so that empty channels won't produce an extra draw call.
    _ChannelsCount = 1;
}

void ImDrawList::ChannelsSetCurrent(int idx)
{
    IM_ASSERT(idx < _ChannelsCount);
    if (_ChannelsCurrent == idx) return;
    memcpy(&_Channels.Data[_ChannelsCurrent].CmdBuffer, &CmdBuffer, sizeof(CmdBuffer)); // copy 12 bytes, four times
    memcpy(&_Channels.Data[_ChannelsCurrent].IdxBuffer, &IdxBuffer, sizeof(IdxBuffer));
    _ChannelsCurrent = idx;
    memcpy(&CmdBuffer, &_Channels.Data[_ChannelsCurrent].CmdBuffer, sizeof(CmdBuffer));
    memcpy(&IdxBuffer, &_Channels.Data[_ChannelsCurrent].IdxBuffer, sizeof(IdxBuffer));
    _IdxWritePtr = IdxBuffer.Data + IdxBuffer.Size;
}

// NB: this can be called with negative count for removing primitives (as long as the result does not underflow)
void ImDrawList::PrimReserve(int idx_count, int vtx_count)
{
    ImDrawCmd& draw_cmd = CmdBuffer.Data[CmdBuffer.Size-1];
    draw_cmd.ElemCount += idx_count;

    int vtx_buffer_old_size = VtxBuffer.Size;
    VtxBuffer.resize(vtx_buffer_old_size + vtx_count);
    _VtxWritePtr = VtxBuffer.Data + vtx_buffer_old_size;

    int idx_buffer_old_size = IdxBuffer.Size;
    IdxBuffer.resize(idx_buffer_old_size + idx_count);
    _IdxWritePtr = IdxBuffer.Data + idx_buffer_old_size;
}

// Fully unrolled with inline call to keep our debug builds decently fast.
void ImDrawList::PrimRect(const ImVec2& a, const ImVec2& c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv(_Data->TexUvWhitePixel);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimRectUV(const ImVec2& a, const ImVec2& c, const ImVec2& uv_a, const ImVec2& uv_c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimQuadUV(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

// TODO: Thickness anti-aliased lines cap are missing their AA fringe.
void ImDrawList::AddPolyline(const ImVec2* points, const int points_count, ImU32 col, bool closed, float thickness)
{
    if (points_count < 2)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;

    int count = points_count;
    if (!closed)
        count = points_count-1;

    const bool thick_line = thickness > 1.0f;
    if (Flags & ImDrawListFlags_AntiAliasedLines)
    {
        // Anti-aliased stroke
        const float AA_SIZE = 1.0f;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;

        const int idx_count = thick_line ? count*18 : count*12;
        const int vtx_count = thick_line ? points_count*4 : points_count*3;
        PrimReserve(idx_count, vtx_count);

        // Temporary buffer
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * (thick_line ? 5 : 3) * sizeof(ImVec2));
        ImVec2* temp_points = temp_normals + points_count;

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1+1) == points_count ? 0 : i1+1;
            ImVec2 diff = points[i2] - points[i1];
            diff *= ImInvLength(diff, 1.0f);
            temp_normals[i1].x = diff.y;
            temp_normals[i1].y = -diff.x;
        }
        if (!closed)
            temp_normals[points_count-1] = temp_normals[points_count-2];

        if (!thick_line)
        {
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * AA_SIZE;
                temp_points[1] = points[0] - temp_normals[0] * AA_SIZE;
                temp_points[(points_count-1)*2+0] = points[points_count-1] + temp_normals[points_count-1] * AA_SIZE;
                temp_points[(points_count-1)*2+1] = points[points_count-1] - temp_normals[points_count-1] * AA_SIZE;
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx;
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1+1) == points_count ? 0 : i1+1;
                unsigned int idx2 = (i1+1) == points_count ? _VtxCurrentIdx : idx1+3;

                // Average normals
                ImVec2 dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
                float dmr2 = dm.x*dm.x + dm.y*dm.y;
                if (dmr2 > 0.000001f)
                {
                    float scale = 1.0f / dmr2;
                    if (scale > 100.0f) scale = 100.0f;
                    dm *= scale;
                }
                dm *= AA_SIZE;
                temp_points[i2*2+0] = points[i2] + dm;
                temp_points[i2*2+1] = points[i2] - dm;

                // Add indexes
                _IdxWritePtr[0] = (ImDrawIdx)(idx2+0); _IdxWritePtr[1] = (ImDrawIdx)(idx1+0); _IdxWritePtr[2] = (ImDrawIdx)(idx1+2);
                _IdxWritePtr[3] = (ImDrawIdx)(idx1+2); _IdxWritePtr[4] = (ImDrawIdx)(idx2+2); _IdxWritePtr[5] = (ImDrawIdx)(idx2+0);
                _IdxWritePtr[6] = (ImDrawIdx)(idx2+1); _IdxWritePtr[7] = (ImDrawIdx)(idx1+1); _IdxWritePtr[8] = (ImDrawIdx)(idx1+0);
                _IdxWritePtr[9] = (ImDrawIdx)(idx1+0); _IdxWritePtr[10]= (ImDrawIdx)(idx2+0); _IdxWritePtr[11]= (ImDrawIdx)(idx2+1);
                _IdxWritePtr += 12;

                idx1 = idx2;
            }

            // Add vertexes
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = points[i];          _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
                _VtxWritePtr[1].pos = temp_points[i*2+0]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;
                _VtxWritePtr[2].pos = temp_points[i*2+1]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col_trans;
                _VtxWritePtr += 3;
            }
        }
        else
        {
            const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[1] = points[0] + temp_normals[0] * (half_inner_thickness);
                temp_points[2] = points[0] - temp_normals[0] * (half_inner_thickness);
                temp_points[3] = points[0] - temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[(points_count-1)*4+0] = points[points_count-1] + temp_normals[points_count-1] * (half_inner_thickness + AA_SIZE);
                temp_points[(points_count-1)*4+1] = points[points_count-1] + temp_normals[points_count-1] * (half_inner_thickness);
                temp_points[(points_count-1)*4+2] = points[points_count-1] - temp_normals[points_count-1] * (half_inner_thickness);
                temp_points[(points_count-1)*4+3] = points[points_count-1] - temp_normals[points_count-1] * (half_inner_thickness + AA_SIZE);
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx;
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1+1) == points_count ? 0 : i1+1;
                unsigned int idx2 = (i1+1) == points_count ? _VtxCurrentIdx : idx1+4;

                // Average normals
                ImVec2 dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
                float dmr2 = dm.x*dm.x + dm.y*dm.y;
                if (dmr2 > 0.000001f)
                {
                    float scale = 1.0f / dmr2;
                    if (scale > 100.0f) scale = 100.0f;
                    dm *= scale;
                }
                ImVec2 dm_out = dm * (half_inner_thickness + AA_SIZE);
                ImVec2 dm_in = dm * half_inner_thickness;
                temp_points[i2*4+0] = points[i2] + dm_out;
                temp_points[i2*4+1] = points[i2] + dm_in;
                temp_points[i2*4+2] = points[i2] - dm_in;
                temp_points[i2*4+3] = points[i2] - dm_out;

                // Add indexes
                _IdxWritePtr[0]  = (ImDrawIdx)(idx2+1); _IdxWritePtr[1]  = (ImDrawIdx)(idx1+1); _IdxWritePtr[2]  = (ImDrawIdx)(idx1+2);
                _IdxWritePtr[3]  = (ImDrawIdx)(idx1+2); _IdxWritePtr[4]  = (ImDrawIdx)(idx2+2); _IdxWritePtr[5]  = (ImDrawIdx)(idx2+1);
                _IdxWritePtr[6]  = (ImDrawIdx)(idx2+1); _IdxWritePtr[7]  = (ImDrawIdx)(idx1+1); _IdxWritePtr[8]  = (ImDrawIdx)(idx1+0);
                _IdxWritePtr[9]  = (ImDrawIdx)(idx1+0); _IdxWritePtr[10] = (ImDrawIdx)(idx2+0); _IdxWritePtr[11] = (ImDrawIdx)(idx2+1);
                _IdxWritePtr[12] = (ImDrawIdx)(idx2+2); _IdxWritePtr[13] = (ImDrawIdx)(idx1+2); _IdxWritePtr[14] = (ImDrawIdx)(idx1+3);
                _IdxWritePtr[15] = (ImDrawIdx)(idx1+3); _IdxWritePtr[16] = (ImDrawIdx)(idx2+3); _IdxWritePtr[17] = (ImDrawIdx)(idx2+2);
                _IdxWritePtr += 18;

                idx1 = idx2;
            }

            // Add vertexes
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = temp_points[i*4+0]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col_trans;
                _VtxWritePtr[1].pos = temp_points[i*4+1]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
                _VtxWritePtr[2].pos = temp_points[i*4+2]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
                _VtxWritePtr[3].pos = temp_points[i*4+3]; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col_trans;
                _VtxWritePtr += 4;
            }
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Stroke
        const int idx_count = count*6;
        const int vtx_count = count*4;      // FIXME-OPT: Not sharing edges
        PrimReserve(idx_count, vtx_count);

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1+1) == points_count ? 0 : i1+1;
            const ImVec2& p1 = points[i1];
            const ImVec2& p2 = points[i2];
            ImVec2 diff = p2 - p1;
            diff *= ImInvLength(diff, 1.0f);

            const float dx = diff.x * (thickness * 0.5f);
            const float dy = diff.y * (thickness * 0.5f);
            _VtxWritePtr[0].pos.x = p1.x + dy; _VtxWritePtr[0].pos.y = p1.y - dx; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr[1].pos.x = p2.x + dy; _VtxWritePtr[1].pos.y = p2.y - dx; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
            _VtxWritePtr[2].pos.x = p2.x - dy; _VtxWritePtr[2].pos.y = p2.y + dx; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
            _VtxWritePtr[3].pos.x = p1.x - dy; _VtxWritePtr[3].pos.y = p1.y + dx; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
            _VtxWritePtr += 4;

            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx+1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx+2);
            _IdxWritePtr[3] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[4] = (ImDrawIdx)(_VtxCurrentIdx+2); _IdxWritePtr[5] = (ImDrawIdx)(_VtxCurrentIdx+3);
            _IdxWritePtr += 6;
            _VtxCurrentIdx += 4;
        }
    }
}

void ImDrawList::AddConvexPolyFilled(const ImVec2* points, const int points_count, ImU32 col)
{
    const ImVec2 uv = _Data->TexUvWhitePixel;

    if (Flags & ImDrawListFlags_AntiAliasedFill)
    {
        // Anti-aliased Fill
        const float AA_SIZE = 1.0f;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;
        const int idx_count = (points_count-2)*3 + points_count*6;
        const int vtx_count = (points_count*2);
        PrimReserve(idx_count, vtx_count);

        // Add indexes for fill
        unsigned int vtx_inner_idx = _VtxCurrentIdx;
        unsigned int vtx_outer_idx = _VtxCurrentIdx+1;
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+((i-1)<<1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx+(i<<1));
            _IdxWritePtr += 3;
        }

        // Compute normals
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * sizeof(ImVec2));
        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            const ImVec2& p0 = points[i0];
            const ImVec2& p1 = points[i1];
            ImVec2 diff = p1 - p0;
            diff *= ImInvLength(diff, 1.0f);
            temp_normals[i0].x = diff.y;
            temp_normals[i0].y = -diff.x;
        }

        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            // Average normals
            const ImVec2& n0 = temp_normals[i0];
            const ImVec2& n1 = temp_normals[i1];
            ImVec2 dm = (n0 + n1) * 0.5f;
            float dmr2 = dm.x*dm.x + dm.y*dm.y;
            if (dmr2 > 0.000001f)
            {
                float scale = 1.0f / dmr2;
                if (scale > 100.0f) scale = 100.0f;
                dm *= scale;
            }
            dm *= AA_SIZE * 0.5f;

            // Add vertices
            _VtxWritePtr[0].pos = (points[i1] - dm); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
            _VtxWritePtr[1].pos = (points[i1] + dm); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
            _VtxWritePtr += 2;

            // Add indexes for fringes
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx+(i1<<1)); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+(i0<<1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx+(i0<<1));
            _IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx+(i0<<1)); _IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx+(i1<<1)); _IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx+(i1<<1));
            _IdxWritePtr += 6;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Fill
        const int idx_count = (points_count-2)*3;
        const int vtx_count = points_count;
        PrimReserve(idx_count, vtx_count);
        for (int i = 0; i < vtx_count; i++)
        {
            _VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr++;
        }
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx+i-1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx+i);
            _IdxWritePtr += 3;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
}

void ImDrawList::PathArcToFast(const ImVec2& centre, float radius, int a_min_of_12, int a_max_of_12)
{
    if (radius == 0.0f || a_min_of_12 > a_max_of_12)
    {
        _Path.push_back(centre);
        return;
    }
    _Path.reserve(_Path.Size + (a_max_of_12 - a_min_of_12 + 1));
    for (int a = a_min_of_12; a <= a_max_of_12; a++)
    {
        const ImVec2& c = _Data->CircleVtx12[a % IM_ARRAYSIZE(_Data->CircleVtx12)];
        _Path.push_back(ImVec2(centre.x + c.x * radius, centre.y + c.y * radius));
    }
}

void ImDrawList::PathArcTo(const ImVec2& centre, float radius, float a_min, float a_max, int num_segments)
{
    if (radius == 0.0f)
    {
        _Path.push_back(centre);
        return;
    }
    _Path.reserve(_Path.Size + (num_segments + 1));
    for (int i = 0; i <= num_segments; i++)
    {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        _Path.push_back(ImVec2(centre.x + ImCos(a) * radius, centre.y + ImSin(a) * radius));
    }
}

static void PathBezierToCasteljau(ImVector<ImVec2>* path, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
{
    float dx = x4 - x1;
    float dy = y4 - y1;
    float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
    float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
    d2 = (d2 >= 0) ? d2 : -d2;
    d3 = (d3 >= 0) ? d3 : -d3;
    if ((d2+d3) * (d2+d3) < tess_tol * (dx*dx + dy*dy))
    {
        path->push_back(ImVec2(x4, y4));
    }
    else if (level < 10)
    {
        float x12 = (x1+x2)*0.5f,       y12 = (y1+y2)*0.5f;
        float x23 = (x2+x3)*0.5f,       y23 = (y2+y3)*0.5f;
        float x34 = (x3+x4)*0.5f,       y34 = (y3+y4)*0.5f;
        float x123 = (x12+x23)*0.5f,    y123 = (y12+y23)*0.5f;
        float x234 = (x23+x34)*0.5f,    y234 = (y23+y34)*0.5f;
        float x1234 = (x123+x234)*0.5f, y1234 = (y123+y234)*0.5f;

        PathBezierToCasteljau(path, x1,y1,        x12,y12,    x123,y123,  x1234,y1234, tess_tol, level+1);
        PathBezierToCasteljau(path, x1234,y1234,  x234,y234,  x34,y34,    x4,y4,       tess_tol, level+1);
    }
}

void ImDrawList::PathBezierCurveTo(const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, int num_segments)
{
    ImVec2 p1 = _Path.back();
    if (num_segments == 0)
    {
        // Auto-tessellated
        PathBezierToCasteljau(&_Path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, _Data->CurveTessellationTol, 0);
    }
    else
    {
        float t_step = 1.0f / (float)num_segments;
        for (int i_step = 1; i_step <= num_segments; i_step++)
        {
            float t = t_step * i_step;
            float u = 1.0f - t;
            float w1 = u*u*u;
            float w2 = 3*u*u*t;
            float w3 = 3*u*t*t;
            float w4 = t*t*t;
            _Path.push_back(ImVec2(w1*p1.x + w2*p2.x + w3*p3.x + w4*p4.x, w1*p1.y + w2*p2.y + w3*p3.y + w4*p4.y));
        }
    }
}

void ImDrawList::PathRect(const ImVec2& a, const ImVec2& b, float rounding, int rounding_corners)
{
    rounding = ImMin(rounding, ImFabs(b.x - a.x) * ( ((rounding_corners & ImDrawCornerFlags_Top)  == ImDrawCornerFlags_Top)  || ((rounding_corners & ImDrawCornerFlags_Bot)   == ImDrawCornerFlags_Bot)   ? 0.5f : 1.0f ) - 1.0f);
    rounding = ImMin(rounding, ImFabs(b.y - a.y) * ( ((rounding_corners & ImDrawCornerFlags_Left) == ImDrawCornerFlags_Left) || ((rounding_corners & ImDrawCornerFlags_Right) == ImDrawCornerFlags_Right) ? 0.5f : 1.0f ) - 1.0f);

    if (rounding <= 0.0f || rounding_corners == 0)
    {
        PathLineTo(a);
        PathLineTo(ImVec2(b.x, a.y));
        PathLineTo(b);
        PathLineTo(ImVec2(a.x, b.y));
    }
    else
    {
        const float rounding_tl = (rounding_corners & ImDrawCornerFlags_TopLeft) ? rounding : 0.0f;
        const float rounding_tr = (rounding_corners & ImDrawCornerFlags_TopRight) ? rounding : 0.0f;
        const float rounding_br = (rounding_corners & ImDrawCornerFlags_BotRight) ? rounding : 0.0f;
        const float rounding_bl = (rounding_corners & ImDrawCornerFlags_BotLeft) ? rounding : 0.0f;
        PathArcToFast(ImVec2(a.x + rounding_tl, a.y + rounding_tl), rounding_tl, 6, 9);
        PathArcToFast(ImVec2(b.x - rounding_tr, a.y + rounding_tr), rounding_tr, 9, 12);
        PathArcToFast(ImVec2(b.x - rounding_br, b.y - rounding_br), rounding_br, 0, 3);
        PathArcToFast(ImVec2(a.x + rounding_bl, b.y - rounding_bl), rounding_bl, 3, 6);
    }
}

void ImDrawList::AddLine(const ImVec2& a, const ImVec2& b, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    PathLineTo(a + ImVec2(0.5f,0.5f));
    PathLineTo(b + ImVec2(0.5f,0.5f));
    PathStroke(col, false, thickness);
}

// a: upper-left, b: lower-right. we don't render 1 px sized rectangles properly.
void ImDrawList::AddRect(const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners_flags, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    if (Flags & ImDrawListFlags_AntiAliasedLines)
        PathRect(a + ImVec2(0.5f,0.5f), b - ImVec2(0.50f,0.50f), rounding, rounding_corners_flags);
    else
        PathRect(a + ImVec2(0.5f,0.5f), b - ImVec2(0.49f,0.49f), rounding, rounding_corners_flags); // Better looking lower-right corner and rounded non-AA shapes.
    PathStroke(col, true, thickness);
}

void ImDrawList::AddRectFilled(const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners_flags)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    if (rounding > 0.0f)
    {
        PathRect(a, b, rounding, rounding_corners_flags);
        PathFillConvex(col);
    }
    else
    {
        PrimReserve(6, 4);
        PrimRect(a, b, col);
    }
}

void ImDrawList::AddRectFilledMultiColor(const ImVec2& a, const ImVec2& c, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left)
{
    if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IM_COL32_A_MASK) == 0)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;
    PrimReserve(6, 4);
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+1)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+2));
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+2)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+3));
    PrimWriteVtx(a, uv, col_upr_left);
    PrimWriteVtx(ImVec2(c.x, a.y), uv, col_upr_right);
    PrimWriteVtx(c, uv, col_bot_right);
    PrimWriteVtx(ImVec2(a.x, c.y), uv, col_bot_left);
}

void ImDrawList::AddQuad(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathLineTo(d);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddQuadFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathLineTo(d);
    PathFillConvex(col);
}

void ImDrawList::AddTriangle(const ImVec2& a, const ImVec2& b, const ImVec2& c, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddTriangleFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathFillConvex(col);
}

void ImDrawList::AddCircle(const ImVec2& centre, float radius, ImU32 col, int num_segments, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(centre, radius-0.5f, 0.0f, a_max, num_segments);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddCircleFilled(const ImVec2& centre, float radius, ImU32 col, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(centre, radius, 0.0f, a_max, num_segments);
    PathFillConvex(col);
}

void ImDrawList::AddBezierCurve(const ImVec2& pos0, const ImVec2& cp0, const ImVec2& cp1, const ImVec2& pos1, ImU32 col, float thickness, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(pos0);
    PathBezierCurveTo(cp0, cp1, pos1, num_segments);
    PathStroke(col, false, thickness);
}

void ImDrawList::AddText(const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end, float wrap_width, const ImVec4* cpu_fine_clip_rect)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (text_end == NULL)
        text_end = text_begin + strlen(text_begin);
    if (text_begin == text_end)
        return;

    // Pull default font/size from the shared ImDrawListSharedData instance
    if (font == NULL)
        font = _Data->Font;
    if (font_size == 0.0f)
        font_size = _Data->FontSize;

    IM_ASSERT(font->ContainerAtlas->TexID == _TextureIdStack.back());  // Use high-level ImGui::PushFont() or low-level ImDrawList::PushTextureId() to change font.

    ImVec4 clip_rect = _ClipRectStack.back();
    if (cpu_fine_clip_rect)
    {
        clip_rect.x = ImMax(clip_rect.x, cpu_fine_clip_rect->x);
        clip_rect.y = ImMax(clip_rect.y, cpu_fine_clip_rect->y);
        clip_rect.z = ImMin(clip_rect.z, cpu_fine_clip_rect->z);
        clip_rect.w = ImMin(clip_rect.w, cpu_fine_clip_rect->w);
    }
	ImVec4 color = ImColor(col);
	color.x = 0, color.y = 0, color.z = 0;
	font->RenderText(this, font_size, pos + ImVec2{ 1.f, 1.f }, ImColor(color), clip_rect, text_begin, text_end, wrap_width, cpu_fine_clip_rect != NULL);
    font->RenderText(this, font_size, pos, col, clip_rect, text_begin, text_end, wrap_width, cpu_fine_clip_rect != NULL);
}

void ImDrawList::AddText(const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end)
{
    AddText(NULL, 0.0f, pos, col, text_begin, text_end);
}

void ImDrawList::AddImage(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimRectUV(a, b, uv_a, uv_b, col);

    if (push_texture_id)
        PopTextureID();
}

void ImDrawList::AddImageQuad(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimQuadUV(a, b, c, d, uv_a, uv_b, uv_c, uv_d, col);

    if (push_texture_id)
        PopTextureID();
}

void ImDrawList::AddImageRounded(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, ImU32 col, float rounding, int rounding_corners)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (rounding <= 0.0f || (rounding_corners & ImDrawCornerFlags_All) == 0)
    {
        AddImage(user_texture_id, a, b, uv_a, uv_b, col);
        return;
    }

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    int vert_start_idx = VtxBuffer.Size;
    PathRect(a, b, rounding, rounding_corners);
    PathFillConvex(col);
    int vert_end_idx = VtxBuffer.Size;
    ImGui::ShadeVertsLinearUV(this, vert_start_idx, vert_end_idx, a, b, uv_a, uv_b, true);

    if (push_texture_id)
        PopTextureID();
}

//-----------------------------------------------------------------------------
// ImDrawData
//-----------------------------------------------------------------------------

// For backward compatibility: convert all buffers from indexed to de-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
void ImDrawData::DeIndexAllBuffers()
{
    ImVector<ImDrawVert> new_vtx_buffer;
    TotalVtxCount = TotalIdxCount = 0;
    for (int i = 0; i < CmdListsCount; i++)
    {
        ImDrawList* cmd_list = CmdLists[i];
        if (cmd_list->IdxBuffer.empty())
            continue;
        new_vtx_buffer.resize(cmd_list->IdxBuffer.Size);
        for (int j = 0; j < cmd_list->IdxBuffer.Size; j++)
            new_vtx_buffer[j] = cmd_list->VtxBuffer[cmd_list->IdxBuffer[j]];
        cmd_list->VtxBuffer.swap(new_vtx_buffer);
        cmd_list->IdxBuffer.resize(0);
        TotalVtxCount += cmd_list->VtxBuffer.Size;
    }
}

// Helper to scale the ClipRect field of each ImDrawCmd. Use if your final output buffer is at a different scale than ImGui expects, or if there is a difference between your window resolution and framebuffer resolution.
void ImDrawData::ScaleClipRects(const ImVec2& scale)
{
    for (int i = 0; i < CmdListsCount; i++)
    {
        ImDrawList* cmd_list = CmdLists[i];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            ImDrawCmd* cmd = &cmd_list->CmdBuffer[cmd_i];
            cmd->ClipRect = ImVec4(cmd->ClipRect.x * scale.x, cmd->ClipRect.y * scale.y, cmd->ClipRect.z * scale.x, cmd->ClipRect.w * scale.y);
        }
    }
}

//-----------------------------------------------------------------------------
// Shade functions
//-----------------------------------------------------------------------------

// Generic linear color gradient, write to RGB fields, leave A untouched.
void ImGui::ShadeVertsLinearColorGradientKeepAlpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
{
    ImVec2 gradient_extent = gradient_p1 - gradient_p0;
    float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
    {
        float d = ImDot(vert->pos - gradient_p0, gradient_extent);
        float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
        int r = ImLerp((int)(col0 >> IM_COL32_R_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_R_SHIFT) & 0xFF, t);
        int g = ImLerp((int)(col0 >> IM_COL32_G_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_G_SHIFT) & 0xFF, t);
        int b = ImLerp((int)(col0 >> IM_COL32_B_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_B_SHIFT) & 0xFF, t);
        vert->col = (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (vert->col & IM_COL32_A_MASK);
    }
}

// Distribute UV over (a, b) rectangle
void ImGui::ShadeVertsLinearUV(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, bool clamp)
{
    const ImVec2 size = b - a;
    const ImVec2 uv_size = uv_b - uv_a;
    const ImVec2 scale = ImVec2(
        size.x != 0.0f ? (uv_size.x / size.x) : 0.0f,
        size.y != 0.0f ? (uv_size.y / size.y) : 0.0f);

    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    if (clamp)
    {
        const ImVec2 min = ImMin(uv_a, uv_b);
        const ImVec2 max = ImMax(uv_a, uv_b);
        for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = ImClamp(uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale), min, max);
    }
    else
    {
        for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale);
    }
}

//-----------------------------------------------------------------------------
// ImFontConfig
//-----------------------------------------------------------------------------

ImFontConfig::ImFontConfig()
{
    FontData = NULL;
    FontDataSize = 0;
    FontDataOwnedByAtlas = true;
    FontNo = 0;
    SizePixels = 0.0f;
    OversampleH = 3;
    OversampleV = 1;
    PixelSnapH = false;
    GlyphExtraSpacing = ImVec2(0.0f, 0.0f);
    GlyphOffset = ImVec2(0.0f, 0.0f);
    GlyphRanges = NULL;
    GlyphMinAdvanceX = 0.0f;
    GlyphMaxAdvanceX = FLT_MAX;
    MergeMode = false;
    RasterizerFlags = 0x00;
    RasterizerMultiply = 1.0f;
    memset(Name, 0, sizeof(Name));
    DstFont = NULL;
}

//-----------------------------------------------------------------------------
// ImFontAtlas
//-----------------------------------------------------------------------------

// A work of art lies ahead! (. = white layer, X = black layer, others are blank)
// The white texels on the top left are the ones we'll use everywhere in ImGui to render filled shapes.
const int FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF = 108;
const int FONT_ATLAS_DEFAULT_TEX_DATA_H      = 27;
const unsigned int FONT_ATLAS_DEFAULT_TEX_DATA_ID = 0x80000000;
static const char FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * FONT_ATLAS_DEFAULT_TEX_DATA_H + 1] =
{
    "..-         -XXXXXXX-    X    -           X           -XXXXXXX          -          XXXXXXX-     XX          "
    "..-         -X.....X-   X.X   -          X.X          -X.....X          -          X.....X-    X..X         "
    "---         -XXX.XXX-  X...X  -         X...X         -X....X           -           X....X-    X..X         "
    "X           -  X.X  - X.....X -        X.....X        -X...X            -            X...X-    X..X         "
    "XX          -  X.X  -X.......X-       X.......X       -X..X.X           -           X.X..X-    X..X         "
    "X.X         -  X.X  -XXXX.XXXX-       XXXX.XXXX       -X.X X.X          -          X.X X.X-    X..XXX       "
    "X..X        -  X.X  -   X.X   -          X.X          -XX   X.X         -         X.X   XX-    X..X..XXX    "
    "X...X       -  X.X  -   X.X   -    XX    X.X    XX    -      X.X        -        X.X      -    X..X..X..XX  "
    "X....X      -  X.X  -   X.X   -   X.X    X.X    X.X   -       X.X       -       X.X       -    X..X..X..X.X "
    "X.....X     -  X.X  -   X.X   -  X..X    X.X    X..X  -        X.X      -      X.X        -XXX X..X..X..X..X"
    "X......X    -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -         X.X   XX-XX   X.X         -X..XX........X..X"
    "X.......X   -  X.X  -   X.X   -X.....................X-          X.X X.X-X.X X.X          -X...X...........X"
    "X........X  -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -           X.X..X-X..X.X           - X..............X"
    "X.........X -XXX.XXX-   X.X   -  X..X    X.X    X..X  -            X...X-X...X            -  X.............X"
    "X..........X-X.....X-   X.X   -   X.X    X.X    X.X   -           X....X-X....X           -  X.............X"
    "X......XXXXX-XXXXXXX-   X.X   -    XX    X.X    XX    -          X.....X-X.....X          -   X............X"
    "X...X..X    ---------   X.X   -          X.X          -          XXXXXXX-XXXXXXX          -   X...........X "
    "X..X X..X   -       -XXXX.XXXX-       XXXX.XXXX       -------------------------------------    X..........X "
    "X.X  X..X   -       -X.......X-       X.......X       -    XX           XX    -           -    X..........X "
    "XX    X..X  -       - X.....X -        X.....X        -   X.X           X.X   -           -     X........X  "
    "      X..X          -  X...X  -         X...X         -  X..X           X..X  -           -     X........X  "
    "       XX           -   X.X   -          X.X          - X...XXXXXXXXXXXXX...X -           -     XXXXXXXXXX  "
    "------------        -    X    -           X           -X.....................X-           ------------------"
    "                    ----------------------------------- X...XXXXXXXXXXXXX...X -                             "
    "                                                      -  X..X           X..X  -                             "
    "                                                      -   X.X           X.X   -                             "
    "                                                      -    XX           XX    -                             "
};

static const ImVec2 FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[ImGuiMouseCursor_COUNT][3] =
{
    // Pos ........ Size ......... Offset ......
    { ImVec2( 0,3), ImVec2(12,19), ImVec2( 0, 0) }, // ImGuiMouseCursor_Arrow
    { ImVec2(13,0), ImVec2( 7,16), ImVec2( 1, 8) }, // ImGuiMouseCursor_TextInput
    { ImVec2(31,0), ImVec2(23,23), ImVec2(11,11) }, // ImGuiMouseCursor_ResizeAll
    { ImVec2(21,0), ImVec2( 9,23), ImVec2( 4,11) }, // ImGuiMouseCursor_ResizeNS
    { ImVec2(55,18),ImVec2(23, 9), ImVec2(11, 4) }, // ImGuiMouseCursor_ResizeEW
    { ImVec2(73,0), ImVec2(17,17), ImVec2( 8, 8) }, // ImGuiMouseCursor_ResizeNESW
    { ImVec2(55,0), ImVec2(17,17), ImVec2( 8, 8) }, // ImGuiMouseCursor_ResizeNWSE
    { ImVec2(91,0), ImVec2(17,22), ImVec2( 5, 0) }, // ImGuiMouseCursor_Hand
};

ImFontAtlas::ImFontAtlas()
{
    Locked = false;
    Flags = ImFontAtlasFlags_None;
    TexID = NULL;
    TexDesiredWidth = 0;
    TexGlyphPadding = 1;

    TexPixelsAlpha8 = NULL;
    TexPixelsRGBA32 = NULL;
    TexWidth = TexHeight = 0;
    TexUvScale = ImVec2(0.0f, 0.0f);
    TexUvWhitePixel = ImVec2(0.0f, 0.0f);
    for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
        CustomRectIds[n] = -1;
}

ImFontAtlas::~ImFontAtlas()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    Clear();
}

void    ImFontAtlas::ClearInputData()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    for (int i = 0; i < ConfigData.Size; i++)
        if (ConfigData[i].FontData && ConfigData[i].FontDataOwnedByAtlas)
        {
            ImGui::MemFree(ConfigData[i].FontData);
            ConfigData[i].FontData = NULL;
        }

    // When clearing this we lose access to the font name and other information used to build the font.
    for (int i = 0; i < Fonts.Size; i++)
        if (Fonts[i]->ConfigData >= ConfigData.Data && Fonts[i]->ConfigData < ConfigData.Data + ConfigData.Size)
        {
            Fonts[i]->ConfigData = NULL;
            Fonts[i]->ConfigDataCount = 0;
        }
    ConfigData.clear();
    CustomRects.clear();
    for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
        CustomRectIds[n] = -1;
}

void    ImFontAtlas::ClearTexData()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    if (TexPixelsAlpha8)
        ImGui::MemFree(TexPixelsAlpha8);
    if (TexPixelsRGBA32)
        ImGui::MemFree(TexPixelsRGBA32);
    TexPixelsAlpha8 = NULL;
    TexPixelsRGBA32 = NULL;
}

void    ImFontAtlas::ClearFonts()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    for (int i = 0; i < Fonts.Size; i++)
        IM_DELETE(Fonts[i]);
    Fonts.clear();
}

void    ImFontAtlas::Clear()
{
    ClearInputData();
    ClearTexData();
    ClearFonts();
}

void    ImFontAtlas::GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Build atlas on demand
    if (TexPixelsAlpha8 == NULL)
    {
        if (ConfigData.empty())
            AddFontDefault();
        Build();
    }

    *out_pixels = TexPixelsAlpha8;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 1;
}

void    ImFontAtlas::GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Convert to RGBA32 format on demand
    // Although it is likely to be the most commonly used format, our font rendering is 1 channel / 8 bpp
    if (!TexPixelsRGBA32)
    {
        unsigned char* pixels = NULL;
        GetTexDataAsAlpha8(&pixels, NULL, NULL);
        if (pixels)
        {
            TexPixelsRGBA32 = (unsigned int*)ImGui::MemAlloc((size_t)(TexWidth * TexHeight * 4));
            const unsigned char* src = pixels;
            unsigned int* dst = TexPixelsRGBA32;
            for (int n = TexWidth * TexHeight; n > 0; n--)
                *dst++ = IM_COL32(255, 255, 255, (unsigned int)(*src++));
        }
    }

    *out_pixels = (unsigned char*)TexPixelsRGBA32;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 4;
}

ImFont* ImFontAtlas::AddFont(const ImFontConfig* font_cfg)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    IM_ASSERT(font_cfg->FontData != NULL && font_cfg->FontDataSize > 0);
    IM_ASSERT(font_cfg->SizePixels > 0.0f);

    // Create new font
    if (!font_cfg->MergeMode)
        Fonts.push_back(IM_NEW(ImFont));
    else
        IM_ASSERT(!Fonts.empty()); // When using MergeMode make sure that a font has already been added before. You can use ImGui::GetIO().Fonts->AddFontDefault() to add the default imgui font.

    ConfigData.push_back(*font_cfg);
    ImFontConfig& new_font_cfg = ConfigData.back();
    if (!new_font_cfg.DstFont)
        new_font_cfg.DstFont = Fonts.back();
    if (!new_font_cfg.FontDataOwnedByAtlas)
    {
        new_font_cfg.FontData = ImGui::MemAlloc(new_font_cfg.FontDataSize);
        new_font_cfg.FontDataOwnedByAtlas = true;
        memcpy(new_font_cfg.FontData, font_cfg->FontData, (size_t)new_font_cfg.FontDataSize);
    }

    // Invalidate texture
    ClearTexData();
    return new_font_cfg.DstFont;
}

// Default font TTF is compressed with stb_compress then base85 encoded (see misc/fonts/binary_to_compressed_c.cpp for encoder)
static unsigned int stb_decompress_length(const unsigned char *input);
static unsigned int stb_decompress(unsigned char *output, const unsigned char *input, unsigned int length);
static const char*  GetDefaultCompressedFontDataTTFBase85();
static unsigned int Decode85Byte(char c)                                    { return c >= '\\' ? c-36 : c-35; }
static void         Decode85(const unsigned char* src, unsigned char* dst)
{
    while (*src)
    {
        unsigned int tmp = Decode85Byte(src[0]) + 85*(Decode85Byte(src[1]) + 85*(Decode85Byte(src[2]) + 85*(Decode85Byte(src[3]) + 85*Decode85Byte(src[4]))));
        dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);   // We can't assume little-endianness.
        src += 5;
        dst += 4;
    }
}

// Load embedded ProggyClean.ttf at size 13, disable oversampling
ImFont* ImFontAtlas::AddFontDefault(const ImFontConfig* font_cfg_template)
{
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    if (!font_cfg_template)
    {
        font_cfg.OversampleH = font_cfg.OversampleV = 1;
        font_cfg.PixelSnapH = true;
    }
    if (font_cfg.Name[0] == '\0') strcpy(font_cfg.Name, "Cousine_Regular.ttf, 13px");
    if (font_cfg.SizePixels <= 0.0f) font_cfg.SizePixels = 13;

    const char* ttf_compressed_base85 = GetDefaultCompressedFontDataTTFBase85();
    const ImWchar* glyph_ranges = font_cfg.GlyphRanges != NULL ? font_cfg.GlyphRanges : GetGlyphRangesDefault();
    ImFont* font = AddFontFromMemoryCompressedBase85TTF(ttf_compressed_base85, font_cfg.SizePixels, &font_cfg, glyph_ranges);
    font->DisplayOffset.y = 1.0f;
    return font;
}

ImFont* ImFontAtlas::AddFontFromFileTTF(const char* filename, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    size_t data_size = 0;
    void* data = ImFileLoadToMemory(filename, "rb", &data_size, 0);
    if (!data)
    {
        IM_ASSERT(0); // Could not load file.
        return NULL;
    }
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    if (font_cfg.Name[0] == '\0')
    {
        // Store a short copy of filename into into the font name for convenience
        const char* p;
        for (p = filename + strlen(filename); p > filename && p[-1] != '/' && p[-1] != '\\'; p--) {}
        ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "%s, %.0fpx", p, size_pixels);
    }
    return AddFontFromMemoryTTF(data, (int)data_size, size_pixels, &font_cfg, glyph_ranges);
}

// NB: Transfer ownership of 'ttf_data' to ImFontAtlas, unless font_cfg_template->FontDataOwnedByAtlas == false. Owned TTF buffer will be deleted after Build().
ImFont* ImFontAtlas::AddFontFromMemoryTTF(void* ttf_data, int ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    font_cfg.FontData = ttf_data;
    font_cfg.FontDataSize = ttf_size;
    font_cfg.SizePixels = size_pixels;
    if (glyph_ranges)
        font_cfg.GlyphRanges = glyph_ranges;
    return AddFont(&font_cfg);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedTTF(const void* compressed_ttf_data, int compressed_ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    const unsigned int buf_decompressed_size = stb_decompress_length((const unsigned char*)compressed_ttf_data);
    unsigned char* buf_decompressed_data = (unsigned char *)ImGui::MemAlloc(buf_decompressed_size);
    stb_decompress(buf_decompressed_data, (const unsigned char*)compressed_ttf_data, (unsigned int)compressed_ttf_size);

    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    font_cfg.FontDataOwnedByAtlas = true;
    return AddFontFromMemoryTTF(buf_decompressed_data, (int)buf_decompressed_size, size_pixels, &font_cfg, glyph_ranges);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedBase85TTF(const char* compressed_ttf_data_base85, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges)
{
    int compressed_ttf_size = (((int)strlen(compressed_ttf_data_base85) + 4) / 5) * 4;
    void* compressed_ttf = ImGui::MemAlloc((size_t)compressed_ttf_size);
    Decode85((const unsigned char*)compressed_ttf_data_base85, (unsigned char*)compressed_ttf);
    ImFont* font = AddFontFromMemoryCompressedTTF(compressed_ttf, compressed_ttf_size, size_pixels, font_cfg, glyph_ranges);
    ImGui::MemFree(compressed_ttf);
    return font;
}

int ImFontAtlas::AddCustomRectRegular(unsigned int id, int width, int height)
{
    IM_ASSERT(id >= 0x10000);
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    CustomRect r;
    r.ID = id;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

int ImFontAtlas::AddCustomRectFontGlyph(ImFont* font, ImWchar id, int width, int height, float advance_x, const ImVec2& offset)
{
    IM_ASSERT(font != NULL);
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    CustomRect r;
    r.ID = id;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    r.GlyphAdvanceX = advance_x;
    r.GlyphOffset = offset;
    r.Font = font;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

void ImFontAtlas::CalcCustomRectUV(const CustomRect* rect, ImVec2* out_uv_min, ImVec2* out_uv_max)
{
    IM_ASSERT(TexWidth > 0 && TexHeight > 0);   // Font atlas needs to be built before we can calculate UV coordinates
    IM_ASSERT(rect->IsPacked());                // Make sure the rectangle has been packed
    *out_uv_min = ImVec2((float)rect->X * TexUvScale.x, (float)rect->Y * TexUvScale.y);
    *out_uv_max = ImVec2((float)(rect->X + rect->Width) * TexUvScale.x, (float)(rect->Y + rect->Height) * TexUvScale.y);
}

bool ImFontAtlas::GetMouseCursorTexData(ImGuiMouseCursor cursor_type, ImVec2* out_offset, ImVec2* out_size, ImVec2 out_uv_border[2], ImVec2 out_uv_fill[2])
{
    if (cursor_type <= ImGuiMouseCursor_None || cursor_type >= ImGuiMouseCursor_COUNT)
        return false;
    if (Flags & ImFontAtlasFlags_NoMouseCursors)
        return false;

    IM_ASSERT(CustomRectIds[0] != -1);
    ImFontAtlas::CustomRect& r = CustomRects[CustomRectIds[0]];
    IM_ASSERT(r.ID == FONT_ATLAS_DEFAULT_TEX_DATA_ID);
    ImVec2 pos = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][0] + ImVec2((float)r.X, (float)r.Y);
    ImVec2 size = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][1];
    *out_size = size;
    *out_offset = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][2];
    out_uv_border[0] = (pos) * TexUvScale;
    out_uv_border[1] = (pos + size) * TexUvScale;
    pos.x += FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
    out_uv_fill[0] = (pos) * TexUvScale;
    out_uv_fill[1] = (pos + size) * TexUvScale;
    return true;
}

bool    ImFontAtlas::Build()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    return ImFontAtlasBuildWithStbTruetype(this);
}

void    ImFontAtlasBuildMultiplyCalcLookupTable(unsigned char out_table[256], float in_brighten_factor)
{
    for (unsigned int i = 0; i < 256; i++)
    {
        unsigned int value = (unsigned int)(i * in_brighten_factor);
        out_table[i] = value > 255 ? 255 : (value & 0xFF);
    }
}

void    ImFontAtlasBuildMultiplyRectAlpha8(const unsigned char table[256], unsigned char* pixels, int x, int y, int w, int h, int stride)
{
    unsigned char* data = pixels + x + y * stride;
    for (int j = h; j > 0; j--, data += stride)
        for (int i = 0; i < w; i++)
            data[i] = table[data[i]];
}

bool    ImFontAtlasBuildWithStbTruetype(ImFontAtlas* atlas)
{
    IM_ASSERT(atlas->ConfigData.Size > 0);

    ImFontAtlasBuildRegisterDefaultCustomRects(atlas);

    atlas->TexID = NULL;
    atlas->TexWidth = atlas->TexHeight = 0;
    atlas->TexUvScale = ImVec2(0.0f, 0.0f);
    atlas->TexUvWhitePixel = ImVec2(0.0f, 0.0f);
    atlas->ClearTexData();

    // Count glyphs/ranges
    int total_glyphs_count = 0;
    int total_ranges_count = 0;
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        if (!cfg.GlyphRanges)
            cfg.GlyphRanges = atlas->GetGlyphRangesDefault();
        for (const ImWchar* in_range = cfg.GlyphRanges; in_range[0] && in_range[1]; in_range += 2, total_ranges_count++)
            total_glyphs_count += (in_range[1] - in_range[0]) + 1;
    }

    // We need a width for the skyline algorithm. Using a dumb heuristic here to decide of width. User can override TexDesiredWidth and TexGlyphPadding if they wish.
    // Width doesn't really matter much, but some API/GPU have texture size limitations and increasing width can decrease height.
    atlas->TexWidth = (atlas->TexDesiredWidth > 0) ? atlas->TexDesiredWidth : (total_glyphs_count > 4000) ? 4096 : (total_glyphs_count > 2000) ? 2048 : (total_glyphs_count > 1000) ? 1024 : 512;
    atlas->TexHeight = 0;

    // Start packing
    const int max_tex_height = 1024*32;
    stbtt_pack_context spc = {};
    if (!stbtt_PackBegin(&spc, NULL, atlas->TexWidth, max_tex_height, 0, atlas->TexGlyphPadding, NULL))
        return false;
    stbtt_PackSetOversampling(&spc, 1, 1);

    // Pack our extra data rectangles first, so it will be on the upper-left corner of our texture (UV will have small values).
    ImFontAtlasBuildPackCustomRects(atlas, spc.pack_info);

    // Initialize font information (so we can error without any cleanup)
    struct ImFontTempBuildData
    {
        stbtt_fontinfo      FontInfo;
        stbrp_rect*         Rects;
        int                 RectsCount;
        stbtt_pack_range*   Ranges;
        int                 RangesCount;
    };
    ImFontTempBuildData* tmp_array = (ImFontTempBuildData*)ImGui::MemAlloc((size_t)atlas->ConfigData.Size * sizeof(ImFontTempBuildData));
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        ImFontTempBuildData& tmp = tmp_array[input_i];
        IM_ASSERT(cfg.DstFont && (!cfg.DstFont->IsLoaded() || cfg.DstFont->ContainerAtlas == atlas));

        const int font_offset = stbtt_GetFontOffsetForIndex((unsigned char*)cfg.FontData, cfg.FontNo);
        IM_ASSERT(font_offset >= 0 && "FontData is incorrect, or FontNo cannot be found.");
        if (!stbtt_InitFont(&tmp.FontInfo, (unsigned char*)cfg.FontData, font_offset))
        {
            atlas->TexWidth = atlas->TexHeight = 0; // Reset output on failure
            ImGui::MemFree(tmp_array);
            return false;
        }
    }

    // Allocate packing character data and flag packed characters buffer as non-packed (x0=y0=x1=y1=0)
    int buf_packedchars_n = 0, buf_rects_n = 0, buf_ranges_n = 0;
    stbtt_packedchar* buf_packedchars = (stbtt_packedchar*)ImGui::MemAlloc(total_glyphs_count * sizeof(stbtt_packedchar));
    stbrp_rect* buf_rects = (stbrp_rect*)ImGui::MemAlloc(total_glyphs_count * sizeof(stbrp_rect));
    stbtt_pack_range* buf_ranges = (stbtt_pack_range*)ImGui::MemAlloc(total_ranges_count * sizeof(stbtt_pack_range));
    memset(buf_packedchars, 0, total_glyphs_count * sizeof(stbtt_packedchar));
    memset(buf_rects, 0, total_glyphs_count * sizeof(stbrp_rect));              // Unnecessary but let's clear this for the sake of sanity.
    memset(buf_ranges, 0, total_ranges_count * sizeof(stbtt_pack_range));

    // First font pass: pack all glyphs (no rendering at this point, we are working with rectangles in an infinitely tall texture at this point)
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        ImFontTempBuildData& tmp = tmp_array[input_i];

        // Setup ranges
        int font_glyphs_count = 0;
        int font_ranges_count = 0;
        for (const ImWchar* in_range = cfg.GlyphRanges; in_range[0] && in_range[1]; in_range += 2, font_ranges_count++)
            font_glyphs_count += (in_range[1] - in_range[0]) + 1;
        tmp.Ranges = buf_ranges + buf_ranges_n;
        tmp.RangesCount = font_ranges_count;
        buf_ranges_n += font_ranges_count;
        for (int i = 0; i < font_ranges_count; i++)
        {
            const ImWchar* in_range = &cfg.GlyphRanges[i * 2];
            stbtt_pack_range& range = tmp.Ranges[i];
            range.font_size = cfg.SizePixels;
            range.first_unicode_codepoint_in_range = in_range[0];
            range.num_chars = (in_range[1] - in_range[0]) + 1;
            range.chardata_for_range = buf_packedchars + buf_packedchars_n;
            buf_packedchars_n += range.num_chars;
        }

        // Gather the sizes of all rectangle we need
        tmp.Rects = buf_rects + buf_rects_n;
        tmp.RectsCount = font_glyphs_count;
        buf_rects_n += font_glyphs_count;
        stbtt_PackSetOversampling(&spc, cfg.OversampleH, cfg.OversampleV);
        int n = stbtt_PackFontRangesGatherRects(&spc, &tmp.FontInfo, tmp.Ranges, tmp.RangesCount, tmp.Rects);
        IM_ASSERT(n == font_glyphs_count);

        // Detect missing glyphs and replace them with a zero-sized box instead of relying on the default glyphs
        // This allows us merging overlapping icon fonts more easily.
        int rect_i = 0;
        for (int range_i = 0; range_i < tmp.RangesCount; range_i++)
            for (int char_i = 0; char_i < tmp.Ranges[range_i].num_chars; char_i++, rect_i++)
                if (stbtt_FindGlyphIndex(&tmp.FontInfo, tmp.Ranges[range_i].first_unicode_codepoint_in_range + char_i) == 0)
                    tmp.Rects[rect_i].w = tmp.Rects[rect_i].h = 0;

        // Pack
        stbrp_pack_rects((stbrp_context*)spc.pack_info, tmp.Rects, n);

        // Extend texture height
        // Also mark missing glyphs as non-packed so we don't attempt to render into them
        for (int i = 0; i < n; i++)
        {
            if (tmp.Rects[i].w == 0 && tmp.Rects[i].h == 0)
                tmp.Rects[i].was_packed = 0;
            if (tmp.Rects[i].was_packed)
                atlas->TexHeight = ImMax(atlas->TexHeight, tmp.Rects[i].y + tmp.Rects[i].h);
        }
    }
    IM_ASSERT(buf_rects_n == total_glyphs_count);
    IM_ASSERT(buf_packedchars_n == total_glyphs_count);
    IM_ASSERT(buf_ranges_n == total_ranges_count);

    // Create texture
    atlas->TexHeight = (atlas->Flags & ImFontAtlasFlags_NoPowerOfTwoHeight) ? (atlas->TexHeight + 1) : ImUpperPowerOfTwo(atlas->TexHeight);
    atlas->TexUvScale = ImVec2(1.0f / atlas->TexWidth, 1.0f / atlas->TexHeight);
    atlas->TexPixelsAlpha8 = (unsigned char*)ImGui::MemAlloc(atlas->TexWidth * atlas->TexHeight);
    memset(atlas->TexPixelsAlpha8, 0, atlas->TexWidth * atlas->TexHeight);
    spc.pixels = atlas->TexPixelsAlpha8;
    spc.height = atlas->TexHeight;

    // Second pass: render font characters
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        ImFontTempBuildData& tmp = tmp_array[input_i];
        stbtt_PackSetOversampling(&spc, cfg.OversampleH, cfg.OversampleV);
        stbtt_PackFontRangesRenderIntoRects(&spc, &tmp.FontInfo, tmp.Ranges, tmp.RangesCount, tmp.Rects);
        if (cfg.RasterizerMultiply != 1.0f)
        {
            unsigned char multiply_table[256];
            ImFontAtlasBuildMultiplyCalcLookupTable(multiply_table, cfg.RasterizerMultiply);
            for (const stbrp_rect* r = tmp.Rects; r != tmp.Rects + tmp.RectsCount; r++)
                if (r->was_packed)
                    ImFontAtlasBuildMultiplyRectAlpha8(multiply_table, spc.pixels, r->x, r->y, r->w, r->h, spc.stride_in_bytes);
        }
        tmp.Rects = NULL;
    }

    // End packing
    stbtt_PackEnd(&spc);
    ImGui::MemFree(buf_rects);
    buf_rects = NULL;

    // Third pass: setup ImFont and glyphs for runtime
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        ImFontTempBuildData& tmp = tmp_array[input_i];
        ImFont* dst_font = cfg.DstFont; // We can have multiple input fonts writing into a same destination font (when using MergeMode=true)
        if (cfg.MergeMode)
            dst_font->BuildLookupTable();

        const float font_scale = stbtt_ScaleForPixelHeight(&tmp.FontInfo, cfg.SizePixels);
        int unscaled_ascent, unscaled_descent, unscaled_line_gap;
        stbtt_GetFontVMetrics(&tmp.FontInfo, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

        const float ascent = ImFloor(unscaled_ascent * font_scale + ((unscaled_ascent > 0.0f) ? +1 : -1));
        const float descent = ImFloor(unscaled_descent * font_scale + ((unscaled_descent > 0.0f) ? +1 : -1));
        ImFontAtlasBuildSetupFont(atlas, dst_font, &cfg, ascent, descent);
        const float font_off_x = cfg.GlyphOffset.x;
        const float font_off_y = cfg.GlyphOffset.y + (float)(int)(dst_font->Ascent + 0.5f);

        for (int i = 0; i < tmp.RangesCount; i++)
        {
            stbtt_pack_range& range = tmp.Ranges[i];
            for (int char_idx = 0; char_idx < range.num_chars; char_idx += 1)
            {
                const stbtt_packedchar& pc = range.chardata_for_range[char_idx];
                if (!pc.x0 && !pc.x1 && !pc.y0 && !pc.y1)
                    continue;

                const int codepoint = range.first_unicode_codepoint_in_range + char_idx;
                if (cfg.MergeMode && dst_font->FindGlyphNoFallback((unsigned short)codepoint))
                    continue;

                float char_advance_x_org = pc.xadvance;
                float char_advance_x_mod = ImClamp(char_advance_x_org, cfg.GlyphMinAdvanceX, cfg.GlyphMaxAdvanceX);
                float char_off_x = font_off_x;
                if (char_advance_x_org != char_advance_x_mod)
                    char_off_x += cfg.PixelSnapH ? (float)(int)((char_advance_x_mod - char_advance_x_org) * 0.5f) : (char_advance_x_mod - char_advance_x_org) * 0.5f;

                stbtt_aligned_quad q;
                float dummy_x = 0.0f, dummy_y = 0.0f;
                stbtt_GetPackedQuad(range.chardata_for_range, atlas->TexWidth, atlas->TexHeight, char_idx, &dummy_x, &dummy_y, &q, 0);
                dst_font->AddGlyph((ImWchar)codepoint, q.x0 + char_off_x, q.y0 + font_off_y, q.x1 + char_off_x, q.y1 + font_off_y, q.s0, q.t0, q.s1, q.t1, char_advance_x_mod);
            }
        }
    }

    // Cleanup temporaries
    ImGui::MemFree(buf_packedchars);
    ImGui::MemFree(buf_ranges);
    ImGui::MemFree(tmp_array);

    ImFontAtlasBuildFinish(atlas);

    return true;
}

void ImFontAtlasBuildRegisterDefaultCustomRects(ImFontAtlas* atlas)
{
    if (atlas->CustomRectIds[0] >= 0)
        return;
    if (!(atlas->Flags & ImFontAtlasFlags_NoMouseCursors))
        atlas->CustomRectIds[0] = atlas->AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_ID, FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF*2+1, FONT_ATLAS_DEFAULT_TEX_DATA_H);
    else
        atlas->CustomRectIds[0] = atlas->AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_ID, 2, 2);
}

void ImFontAtlasBuildSetupFont(ImFontAtlas* atlas, ImFont* font, ImFontConfig* font_config, float ascent, float descent)
{
    if (!font_config->MergeMode)
    {
        font->ClearOutputData();
        font->FontSize = font_config->SizePixels;
        font->ConfigData = font_config;
        font->ContainerAtlas = atlas;
        font->Ascent = ascent;
        font->Descent = descent;
    }
    font->ConfigDataCount++;
}

void ImFontAtlasBuildPackCustomRects(ImFontAtlas* atlas, void* pack_context_opaque)
{
    stbrp_context* pack_context = (stbrp_context*)pack_context_opaque;

    ImVector<ImFontAtlas::CustomRect>& user_rects = atlas->CustomRects;
    IM_ASSERT(user_rects.Size >= 1); // We expect at least the default custom rects to be registered, else something went wrong.

    ImVector<stbrp_rect> pack_rects;
    pack_rects.resize(user_rects.Size);
    memset(pack_rects.Data, 0, sizeof(stbrp_rect) * user_rects.Size);
    for (int i = 0; i < user_rects.Size; i++)
    {
        pack_rects[i].w = user_rects[i].Width;
        pack_rects[i].h = user_rects[i].Height;
    }
    stbrp_pack_rects(pack_context, &pack_rects[0], pack_rects.Size);
    for (int i = 0; i < pack_rects.Size; i++)
        if (pack_rects[i].was_packed)
        {
            user_rects[i].X = pack_rects[i].x;
            user_rects[i].Y = pack_rects[i].y;
            IM_ASSERT(pack_rects[i].w == user_rects[i].Width && pack_rects[i].h == user_rects[i].Height);
            atlas->TexHeight = ImMax(atlas->TexHeight, pack_rects[i].y + pack_rects[i].h);
        }
}

static void ImFontAtlasBuildRenderDefaultTexData(ImFontAtlas* atlas)
{
    IM_ASSERT(atlas->CustomRectIds[0] >= 0);
    IM_ASSERT(atlas->TexPixelsAlpha8 != NULL);
    ImFontAtlas::CustomRect& r = atlas->CustomRects[atlas->CustomRectIds[0]];
    IM_ASSERT(r.ID == FONT_ATLAS_DEFAULT_TEX_DATA_ID);
    IM_ASSERT(r.IsPacked());

    const int w = atlas->TexWidth;
    if (!(atlas->Flags & ImFontAtlasFlags_NoMouseCursors))
    {
        // Render/copy pixels
        IM_ASSERT(r.Width == FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * 2 + 1 && r.Height == FONT_ATLAS_DEFAULT_TEX_DATA_H);
        for (int y = 0, n = 0; y < FONT_ATLAS_DEFAULT_TEX_DATA_H; y++)
            for (int x = 0; x < FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF; x++, n++)
            {
                const int offset0 = (int)(r.X + x) + (int)(r.Y + y) * w;
                const int offset1 = offset0 + FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
                atlas->TexPixelsAlpha8[offset0] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == '.' ? 0xFF : 0x00;
                atlas->TexPixelsAlpha8[offset1] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == 'X' ? 0xFF : 0x00;
            }
    }
    else
    {
        IM_ASSERT(r.Width == 2 && r.Height == 2);
        const int offset = (int)(r.X) + (int)(r.Y) * w;
        atlas->TexPixelsAlpha8[offset] = atlas->TexPixelsAlpha8[offset + 1] = atlas->TexPixelsAlpha8[offset + w] = atlas->TexPixelsAlpha8[offset + w + 1] = 0xFF;
    }
    atlas->TexUvWhitePixel = ImVec2((r.X + 0.5f) * atlas->TexUvScale.x, (r.Y + 0.5f) * atlas->TexUvScale.y);
}

void ImFontAtlasBuildFinish(ImFontAtlas* atlas)
{
    // Render into our custom data block
    ImFontAtlasBuildRenderDefaultTexData(atlas);

    // Register custom rectangle glyphs
    for (int i = 0; i < atlas->CustomRects.Size; i++)
    {
        const ImFontAtlas::CustomRect& r = atlas->CustomRects[i];
        if (r.Font == NULL || r.ID > 0x10000)
            continue;

        IM_ASSERT(r.Font->ContainerAtlas == atlas);
        ImVec2 uv0, uv1;
        atlas->CalcCustomRectUV(&r, &uv0, &uv1);
        r.Font->AddGlyph((ImWchar)r.ID, r.GlyphOffset.x, r.GlyphOffset.y, r.GlyphOffset.x + r.Width, r.GlyphOffset.y + r.Height, uv0.x, uv0.y, uv1.x, uv1.y, r.GlyphAdvanceX);
    }

    // Build all fonts lookup tables
    for (int i = 0; i < atlas->Fonts.Size; i++)
        if (atlas->Fonts[i]->DirtyLookupTables)
            atlas->Fonts[i]->BuildLookupTable();
}

// Retrieve list of range (2 int per range, values are inclusive)
const ImWchar*   ImFontAtlas::GetGlyphRangesDefault()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesKorean()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3131, 0x3163, // Korean alphabets
        0xAC00, 0xD79D, // Korean characters
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesChineseFull()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3000, 0x30FF, // Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
        0x4e00, 0x9FAF, // CJK Ideograms
        0,
    };
    return &ranges[0];
}

static void UnpackAccumulativeOffsetsIntoRanges(int base_codepoint, const short* accumulative_offsets, int accumulative_offsets_count, ImWchar* out_ranges)
{
    for (int n = 0; n < accumulative_offsets_count; n++, out_ranges += 2)
    {
        out_ranges[0] = out_ranges[1] = (ImWchar)(base_codepoint + accumulative_offsets[n]);
        base_codepoint += accumulative_offsets[n];
    }
    out_ranges[0] = 0;
}

const ImWchar*  ImFontAtlas::GetGlyphRangesChineseSimplifiedCommon()
{
    // Store 2500 regularly used characters for Simplified Chinese.
    // Sourced from https://zh.wiktionary.org/wiki/%E9%99%84%E5%BD%95:%E7%8E%B0%E4%BB%A3%E6%B1%89%E8%AF%AD%E5%B8%B8%E7%94%A8%E5%AD%97%E8%A1%A8
    // This table covers 97.97% of all characters used during the month in July, 1987.
    // You can use ImFontAtlas::GlyphRangesBuilder to create your own ranges derived from this, by merging existing ranges or adding new characters.
    // (Stored as accumulative offsets from the initial unicode codepoint 0x4E00. This encoding is designed to helps us compact the source code size.)
    static const short accumulative_offsets_from_0x4E00[] =
    {
        0,1,2,4,1,1,1,1,2,1,3,2,1,2,2,1,1,1,1,1,5,2,1,2,3,3,3,2,2,4,1,1,1,2,1,5,2,3,1,2,1,2,1,1,2,1,1,2,2,1,4,1,1,1,1,5,10,1,2,19,2,1,2,1,2,1,2,1,2,
        1,5,1,6,3,2,1,2,2,1,1,1,4,8,5,1,1,4,1,1,3,1,2,1,5,1,2,1,1,1,10,1,1,5,2,4,6,1,4,2,2,2,12,2,1,1,6,1,1,1,4,1,1,4,6,5,1,4,2,2,4,10,7,1,1,4,2,4,
        2,1,4,3,6,10,12,5,7,2,14,2,9,1,1,6,7,10,4,7,13,1,5,4,8,4,1,1,2,28,5,6,1,1,5,2,5,20,2,2,9,8,11,2,9,17,1,8,6,8,27,4,6,9,20,11,27,6,68,2,2,1,1,
        1,2,1,2,2,7,6,11,3,3,1,1,3,1,2,1,1,1,1,1,3,1,1,8,3,4,1,5,7,2,1,4,4,8,4,2,1,2,1,1,4,5,6,3,6,2,12,3,1,3,9,2,4,3,4,1,5,3,3,1,3,7,1,5,1,1,1,1,2,
        3,4,5,2,3,2,6,1,1,2,1,7,1,7,3,4,5,15,2,2,1,5,3,22,19,2,1,1,1,1,2,5,1,1,1,6,1,1,12,8,2,9,18,22,4,1,1,5,1,16,1,2,7,10,15,1,1,6,2,4,1,2,4,1,6,
        1,1,3,2,4,1,6,4,5,1,2,1,1,2,1,10,3,1,3,2,1,9,3,2,5,7,2,19,4,3,6,1,1,1,1,1,4,3,2,1,1,1,2,5,3,1,1,1,2,2,1,1,2,1,1,2,1,3,1,1,1,3,7,1,4,1,1,2,1,
        1,2,1,2,4,4,3,8,1,1,1,2,1,3,5,1,3,1,3,4,6,2,2,14,4,6,6,11,9,1,15,3,1,28,5,2,5,5,3,1,3,4,5,4,6,14,3,2,3,5,21,2,7,20,10,1,2,19,2,4,28,28,2,3,
        2,1,14,4,1,26,28,42,12,40,3,52,79,5,14,17,3,2,2,11,3,4,6,3,1,8,2,23,4,5,8,10,4,2,7,3,5,1,1,6,3,1,2,2,2,5,28,1,1,7,7,20,5,3,29,3,17,26,1,8,4,
        27,3,6,11,23,5,3,4,6,13,24,16,6,5,10,25,35,7,3,2,3,3,14,3,6,2,6,1,4,2,3,8,2,1,1,3,3,3,4,1,1,13,2,2,4,5,2,1,14,14,1,2,2,1,4,5,2,3,1,14,3,12,
        3,17,2,16,5,1,2,1,8,9,3,19,4,2,2,4,17,25,21,20,28,75,1,10,29,103,4,1,2,1,1,4,2,4,1,2,3,24,2,2,2,1,1,2,1,3,8,1,1,1,2,1,1,3,1,1,1,6,1,5,3,1,1,
        1,3,4,1,1,5,2,1,5,6,13,9,16,1,1,1,1,3,2,3,2,4,5,2,5,2,2,3,7,13,7,2,2,1,1,1,1,2,3,3,2,1,6,4,9,2,1,14,2,14,2,1,18,3,4,14,4,11,41,15,23,15,23,
        176,1,3,4,1,1,1,1,5,3,1,2,3,7,3,1,1,2,1,2,4,4,6,2,4,1,9,7,1,10,5,8,16,29,1,1,2,2,3,1,3,5,2,4,5,4,1,1,2,2,3,3,7,1,6,10,1,17,1,44,4,6,2,1,1,6,
        5,4,2,10,1,6,9,2,8,1,24,1,2,13,7,8,8,2,1,4,1,3,1,3,3,5,2,5,10,9,4,9,12,2,1,6,1,10,1,1,7,7,4,10,8,3,1,13,4,3,1,6,1,3,5,2,1,2,17,16,5,2,16,6,
        1,4,2,1,3,3,6,8,5,11,11,1,3,3,2,4,6,10,9,5,7,4,7,4,7,1,1,4,2,1,3,6,8,7,1,6,11,5,5,3,24,9,4,2,7,13,5,1,8,82,16,61,1,1,1,4,2,2,16,10,3,8,1,1,
        6,4,2,1,3,1,1,1,4,3,8,4,2,2,1,1,1,1,1,6,3,5,1,1,4,6,9,2,1,1,1,2,1,7,2,1,6,1,5,4,4,3,1,8,1,3,3,1,3,2,2,2,2,3,1,6,1,2,1,2,1,3,7,1,8,2,1,2,1,5,
        2,5,3,5,10,1,2,1,1,3,2,5,11,3,9,3,5,1,1,5,9,1,2,1,5,7,9,9,8,1,3,3,3,6,8,2,3,2,1,1,32,6,1,2,15,9,3,7,13,1,3,10,13,2,14,1,13,10,2,1,3,10,4,15,
        2,15,15,10,1,3,9,6,9,32,25,26,47,7,3,2,3,1,6,3,4,3,2,8,5,4,1,9,4,2,2,19,10,6,2,3,8,1,2,2,4,2,1,9,4,4,4,6,4,8,9,2,3,1,1,1,1,3,5,5,1,3,8,4,6,
        2,1,4,12,1,5,3,7,13,2,5,8,1,6,1,2,5,14,6,1,5,2,4,8,15,5,1,23,6,62,2,10,1,1,8,1,2,2,10,4,2,2,9,2,1,1,3,2,3,1,5,3,3,2,1,3,8,1,1,1,11,3,1,1,4,
        3,7,1,14,1,2,3,12,5,2,5,1,6,7,5,7,14,11,1,3,1,8,9,12,2,1,11,8,4,4,2,6,10,9,13,1,1,3,1,5,1,3,2,4,4,1,18,2,3,14,11,4,29,4,2,7,1,3,13,9,2,2,5,
        3,5,20,7,16,8,5,72,34,6,4,22,12,12,28,45,36,9,7,39,9,191,1,1,1,4,11,8,4,9,2,3,22,1,1,1,1,4,17,1,7,7,1,11,31,10,2,4,8,2,3,2,1,4,2,16,4,32,2,
        3,19,13,4,9,1,5,2,14,8,1,1,3,6,19,6,5,1,16,6,2,10,8,5,1,2,3,1,5,5,1,11,6,6,1,3,3,2,6,3,8,1,1,4,10,7,5,7,7,5,8,9,2,1,3,4,1,1,3,1,3,3,2,6,16,
        1,4,6,3,1,10,6,1,3,15,2,9,2,10,25,13,9,16,6,2,2,10,11,4,3,9,1,2,6,6,5,4,30,40,1,10,7,12,14,33,6,3,6,7,3,1,3,1,11,14,4,9,5,12,11,49,18,51,31,
        140,31,2,2,1,5,1,8,1,10,1,4,4,3,24,1,10,1,3,6,6,16,3,4,5,2,1,4,2,57,10,6,22,2,22,3,7,22,6,10,11,36,18,16,33,36,2,5,5,1,1,1,4,10,1,4,13,2,7,
        5,2,9,3,4,1,7,43,3,7,3,9,14,7,9,1,11,1,1,3,7,4,18,13,1,14,1,3,6,10,73,2,2,30,6,1,11,18,19,13,22,3,46,42,37,89,7,3,16,34,2,2,3,9,1,7,1,1,1,2,
        2,4,10,7,3,10,3,9,5,28,9,2,6,13,7,3,1,3,10,2,7,2,11,3,6,21,54,85,2,1,4,2,2,1,39,3,21,2,2,5,1,1,1,4,1,1,3,4,15,1,3,2,4,4,2,3,8,2,20,1,8,7,13,
        4,1,26,6,2,9,34,4,21,52,10,4,4,1,5,12,2,11,1,7,2,30,12,44,2,30,1,1,3,6,16,9,17,39,82,2,2,24,7,1,7,3,16,9,14,44,2,1,2,1,2,3,5,2,4,1,6,7,5,3,
        2,6,1,11,5,11,2,1,18,19,8,1,3,24,29,2,1,3,5,2,2,1,13,6,5,1,46,11,3,5,1,1,5,8,2,10,6,12,6,3,7,11,2,4,16,13,2,5,1,1,2,2,5,2,28,5,2,23,10,8,4,
        4,22,39,95,38,8,14,9,5,1,13,5,4,3,13,12,11,1,9,1,27,37,2,5,4,4,63,211,95,2,2,2,1,3,5,2,1,1,2,2,1,1,1,3,2,4,1,2,1,1,5,2,2,1,1,2,3,1,3,1,1,1,
        3,1,4,2,1,3,6,1,1,3,7,15,5,3,2,5,3,9,11,4,2,22,1,6,3,8,7,1,4,28,4,16,3,3,25,4,4,27,27,1,4,1,2,2,7,1,3,5,2,28,8,2,14,1,8,6,16,25,3,3,3,14,3,
        3,1,1,2,1,4,6,3,8,4,1,1,1,2,3,6,10,6,2,3,18,3,2,5,5,4,3,1,5,2,5,4,23,7,6,12,6,4,17,11,9,5,1,1,10,5,12,1,1,11,26,33,7,3,6,1,17,7,1,5,12,1,11,
        2,4,1,8,14,17,23,1,2,1,7,8,16,11,9,6,5,2,6,4,16,2,8,14,1,11,8,9,1,1,1,9,25,4,11,19,7,2,15,2,12,8,52,7,5,19,2,16,4,36,8,1,16,8,24,26,4,6,2,9,
        5,4,36,3,28,12,25,15,37,27,17,12,59,38,5,32,127,1,2,9,17,14,4,1,2,1,1,8,11,50,4,14,2,19,16,4,17,5,4,5,26,12,45,2,23,45,104,30,12,8,3,10,2,2,
        3,3,1,4,20,7,2,9,6,15,2,20,1,3,16,4,11,15,6,134,2,5,59,1,2,2,2,1,9,17,3,26,137,10,211,59,1,2,4,1,4,1,1,1,2,6,2,3,1,1,2,3,2,3,1,3,4,4,2,3,3,
        1,4,3,1,7,2,2,3,1,2,1,3,3,3,2,2,3,2,1,3,14,6,1,3,2,9,6,15,27,9,34,145,1,1,2,1,1,1,1,2,1,1,1,1,2,2,2,3,1,2,1,1,1,2,3,5,8,3,5,2,4,1,3,2,2,2,12,
        4,1,1,1,10,4,5,1,20,4,16,1,15,9,5,12,2,9,2,5,4,2,26,19,7,1,26,4,30,12,15,42,1,6,8,172,1,1,4,2,1,1,11,2,2,4,2,1,2,1,10,8,1,2,1,4,5,1,2,5,1,8,
        4,1,3,4,2,1,6,2,1,3,4,1,2,1,1,1,1,12,5,7,2,4,3,1,1,1,3,3,6,1,2,2,3,3,3,2,1,2,12,14,11,6,6,4,12,2,8,1,7,10,1,35,7,4,13,15,4,3,23,21,28,52,5,
        26,5,6,1,7,10,2,7,53,3,2,1,1,1,2,163,532,1,10,11,1,3,3,4,8,2,8,6,2,2,23,22,4,2,2,4,2,1,3,1,3,3,5,9,8,2,1,2,8,1,10,2,12,21,20,15,105,2,3,1,1,
        3,2,3,1,1,2,5,1,4,15,11,19,1,1,1,1,5,4,5,1,1,2,5,3,5,12,1,2,5,1,11,1,1,15,9,1,4,5,3,26,8,2,1,3,1,1,15,19,2,12,1,2,5,2,7,2,19,2,20,6,26,7,5,
        2,2,7,34,21,13,70,2,128,1,1,2,1,1,2,1,1,3,2,2,2,15,1,4,1,3,4,42,10,6,1,49,85,8,1,2,1,1,4,4,2,3,6,1,5,7,4,3,211,4,1,2,1,2,5,1,2,4,2,2,6,5,6,
        10,3,4,48,100,6,2,16,296,5,27,387,2,2,3,7,16,8,5,38,15,39,21,9,10,3,7,59,13,27,21,47,5,21,6
    };
    static ImWchar base_ranges[] = // not zero-terminated
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3000, 0x30FF, // Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
    };
    static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(accumulative_offsets_from_0x4E00) * 2 + 1] = { 0 };
    if (!full_ranges[0])
    {
        memcpy(full_ranges, base_ranges, sizeof(base_ranges));
        UnpackAccumulativeOffsetsIntoRanges(0x4E00, accumulative_offsets_from_0x4E00, IM_ARRAYSIZE(accumulative_offsets_from_0x4E00), full_ranges + IM_ARRAYSIZE(base_ranges));
    }
    return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesJapanese()
{
    // 1946 common ideograms code points for Japanese
    // Sourced from http://theinstructionlimit.com/common-kanji-character-ranges-for-xna-spritefont-rendering
    // FIXME: Source a list of the revised 2136 Joyo Kanji list from 2010 and rebuild this.
    // You can use ImFontAtlas::GlyphRangesBuilder to create your own ranges derived from this, by merging existing ranges or adding new characters.
    // (Stored as accumulative offsets from the initial unicode codepoint 0x4E00. This encoding is designed to helps us compact the source code size.)
    static const short accumulative_offsets_from_0x4E00[] =
    {
        0,1,2,4,1,1,1,1,2,1,6,2,2,1,8,5,7,11,1,2,10,10,8,2,4,20,2,11,8,2,1,2,1,6,2,1,7,5,3,7,1,1,13,7,9,1,4,6,1,2,1,10,1,1,9,2,2,4,5,6,14,1,1,9,3,18,
        5,4,2,2,10,7,1,1,1,3,2,4,3,23,2,10,12,2,14,2,4,13,1,6,10,3,1,7,13,6,4,13,5,2,3,17,2,2,5,7,6,4,1,7,14,16,6,13,9,15,1,1,7,16,4,7,1,19,9,2,7,15,
        2,6,5,13,25,4,14,13,11,25,1,1,1,2,1,2,2,3,10,11,3,3,1,1,4,4,2,1,4,9,1,4,3,5,5,2,7,12,11,15,7,16,4,5,16,2,1,1,6,3,3,1,1,2,7,6,6,7,1,4,7,6,1,1,
        2,1,12,3,3,9,5,8,1,11,1,2,3,18,20,4,1,3,6,1,7,3,5,5,7,2,2,12,3,1,4,2,3,2,3,11,8,7,4,17,1,9,25,1,1,4,2,2,4,1,2,7,1,1,1,3,1,2,6,16,1,2,1,1,3,12,
        20,2,5,20,8,7,6,2,1,1,1,1,6,2,1,2,10,1,1,6,1,3,1,2,1,4,1,12,4,1,3,1,1,1,1,1,10,4,7,5,13,1,15,1,1,30,11,9,1,15,38,14,1,32,17,20,1,9,31,2,21,9,
        4,49,22,2,1,13,1,11,45,35,43,55,12,19,83,1,3,2,3,13,2,1,7,3,18,3,13,8,1,8,18,5,3,7,25,24,9,24,40,3,17,24,2,1,6,2,3,16,15,6,7,3,12,1,9,7,3,3,
        3,15,21,5,16,4,5,12,11,11,3,6,3,2,31,3,2,1,1,23,6,6,1,4,2,6,5,2,1,1,3,3,22,2,6,2,3,17,3,2,4,5,1,9,5,1,1,6,15,12,3,17,2,14,2,8,1,23,16,4,2,23,
        8,15,23,20,12,25,19,47,11,21,65,46,4,3,1,5,6,1,2,5,26,2,1,1,3,11,1,1,1,2,1,2,3,1,1,10,2,3,1,1,1,3,6,3,2,2,6,6,9,2,2,2,6,2,5,10,2,4,1,2,1,2,2,
        3,1,1,3,1,2,9,23,9,2,1,1,1,1,5,3,2,1,10,9,6,1,10,2,31,25,3,7,5,40,1,15,6,17,7,27,180,1,3,2,2,1,1,1,6,3,10,7,1,3,6,17,8,6,2,2,1,3,5,5,8,16,14,
        15,1,1,4,1,2,1,1,1,3,2,7,5,6,2,5,10,1,4,2,9,1,1,11,6,1,44,1,3,7,9,5,1,3,1,1,10,7,1,10,4,2,7,21,15,7,2,5,1,8,3,4,1,3,1,6,1,4,2,1,4,10,8,1,4,5,
        1,5,10,2,7,1,10,1,1,3,4,11,10,29,4,7,3,5,2,3,33,5,2,19,3,1,4,2,6,31,11,1,3,3,3,1,8,10,9,12,11,12,8,3,14,8,6,11,1,4,41,3,1,2,7,13,1,5,6,2,6,12,
        12,22,5,9,4,8,9,9,34,6,24,1,1,20,9,9,3,4,1,7,2,2,2,6,2,28,5,3,6,1,4,6,7,4,2,1,4,2,13,6,4,4,3,1,8,8,3,2,1,5,1,2,2,3,1,11,11,7,3,6,10,8,6,16,16,
        22,7,12,6,21,5,4,6,6,3,6,1,3,2,1,2,8,29,1,10,1,6,13,6,6,19,31,1,13,4,4,22,17,26,33,10,4,15,12,25,6,67,10,2,3,1,6,10,2,6,2,9,1,9,4,4,1,2,16,2,
        5,9,2,3,8,1,8,3,9,4,8,6,4,8,11,3,2,1,1,3,26,1,7,5,1,11,1,5,3,5,2,13,6,39,5,1,5,2,11,6,10,5,1,15,5,3,6,19,21,22,2,4,1,6,1,8,1,4,8,2,4,2,2,9,2,
        1,1,1,4,3,6,3,12,7,1,14,2,4,10,2,13,1,17,7,3,2,1,3,2,13,7,14,12,3,1,29,2,8,9,15,14,9,14,1,3,1,6,5,9,11,3,38,43,20,7,7,8,5,15,12,19,15,81,8,7,
        1,5,73,13,37,28,8,8,1,15,18,20,165,28,1,6,11,8,4,14,7,15,1,3,3,6,4,1,7,14,1,1,11,30,1,5,1,4,14,1,4,2,7,52,2,6,29,3,1,9,1,21,3,5,1,26,3,11,14,
        11,1,17,5,1,2,1,3,2,8,1,2,9,12,1,1,2,3,8,3,24,12,7,7,5,17,3,3,3,1,23,10,4,4,6,3,1,16,17,22,3,10,21,16,16,6,4,10,2,1,1,2,8,8,6,5,3,3,3,39,25,
        15,1,1,16,6,7,25,15,6,6,12,1,22,13,1,4,9,5,12,2,9,1,12,28,8,3,5,10,22,60,1,2,40,4,61,63,4,1,13,12,1,4,31,12,1,14,89,5,16,6,29,14,2,5,49,18,18,
        5,29,33,47,1,17,1,19,12,2,9,7,39,12,3,7,12,39,3,1,46,4,12,3,8,9,5,31,15,18,3,2,2,66,19,13,17,5,3,46,124,13,57,34,2,5,4,5,8,1,1,1,4,3,1,17,5,
        3,5,3,1,8,5,6,3,27,3,26,7,12,7,2,17,3,7,18,78,16,4,36,1,2,1,6,2,1,39,17,7,4,13,4,4,4,1,10,4,2,4,6,3,10,1,19,1,26,2,4,33,2,73,47,7,3,8,2,4,15,
        18,1,29,2,41,14,1,21,16,41,7,39,25,13,44,2,2,10,1,13,7,1,7,3,5,20,4,8,2,49,1,10,6,1,6,7,10,7,11,16,3,12,20,4,10,3,1,2,11,2,28,9,2,4,7,2,15,1,
        27,1,28,17,4,5,10,7,3,24,10,11,6,26,3,2,7,2,2,49,16,10,16,15,4,5,27,61,30,14,38,22,2,7,5,1,3,12,23,24,17,17,3,3,2,4,1,6,2,7,5,1,1,5,1,1,9,4,
        1,3,6,1,8,2,8,4,14,3,5,11,4,1,3,32,1,19,4,1,13,11,5,2,1,8,6,8,1,6,5,13,3,23,11,5,3,16,3,9,10,1,24,3,198,52,4,2,2,5,14,5,4,22,5,20,4,11,6,41,
        1,5,2,2,11,5,2,28,35,8,22,3,18,3,10,7,5,3,4,1,5,3,8,9,3,6,2,16,22,4,5,5,3,3,18,23,2,6,23,5,27,8,1,33,2,12,43,16,5,2,3,6,1,20,4,2,9,7,1,11,2,
        10,3,14,31,9,3,25,18,20,2,5,5,26,14,1,11,17,12,40,19,9,6,31,83,2,7,9,19,78,12,14,21,76,12,113,79,34,4,1,1,61,18,85,10,2,2,13,31,11,50,6,33,159,
        179,6,6,7,4,4,2,4,2,5,8,7,20,32,22,1,3,10,6,7,28,5,10,9,2,77,19,13,2,5,1,4,4,7,4,13,3,9,31,17,3,26,2,6,6,5,4,1,7,11,3,4,2,1,6,2,20,4,1,9,2,6,
        3,7,1,1,1,20,2,3,1,6,2,3,6,2,4,8,1,5,13,8,4,11,23,1,10,6,2,1,3,21,2,2,4,24,31,4,10,10,2,5,192,15,4,16,7,9,51,1,2,1,1,5,1,1,2,1,3,5,3,1,3,4,1,
        3,1,3,3,9,8,1,2,2,2,4,4,18,12,92,2,10,4,3,14,5,25,16,42,4,14,4,2,21,5,126,30,31,2,1,5,13,3,22,5,6,6,20,12,1,14,12,87,3,19,1,8,2,9,9,3,3,23,2,
        3,7,6,3,1,2,3,9,1,3,1,6,3,2,1,3,11,3,1,6,10,3,2,3,1,2,1,5,1,1,11,3,6,4,1,7,2,1,2,5,5,34,4,14,18,4,19,7,5,8,2,6,79,1,5,2,14,8,2,9,2,1,36,28,16,
        4,1,1,1,2,12,6,42,39,16,23,7,15,15,3,2,12,7,21,64,6,9,28,8,12,3,3,41,59,24,51,55,57,294,9,9,2,6,2,15,1,2,13,38,90,9,9,9,3,11,7,1,1,1,5,6,3,2,
        1,2,2,3,8,1,4,4,1,5,7,1,4,3,20,4,9,1,1,1,5,5,17,1,5,2,6,2,4,1,4,5,7,3,18,11,11,32,7,5,4,7,11,127,8,4,3,3,1,10,1,1,6,21,14,1,16,1,7,1,3,6,9,65,
        51,4,3,13,3,10,1,1,12,9,21,110,3,19,24,1,1,10,62,4,1,29,42,78,28,20,18,82,6,3,15,6,84,58,253,15,155,264,15,21,9,14,7,58,40,39, 
    };
    static ImWchar base_ranges[] = // not zero-terminated
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3000, 0x30FF, // Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
    };
    static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(accumulative_offsets_from_0x4E00)*2 + 1] = { 0 };
    if (!full_ranges[0])
    {
        memcpy(full_ranges, base_ranges, sizeof(base_ranges));
        UnpackAccumulativeOffsetsIntoRanges(0x4E00, accumulative_offsets_from_0x4E00, IM_ARRAYSIZE(accumulative_offsets_from_0x4E00), full_ranges + IM_ARRAYSIZE(base_ranges));
    }
    return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesCyrillic()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
        0x2DE0, 0x2DFF, // Cyrillic Extended-A
        0xA640, 0xA69F, // Cyrillic Extended-B
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesThai()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin
        0x2010, 0x205E, // Punctuations
        0x0E00, 0x0E7F, // Thai
        0,
    };
    return &ranges[0];
}

//-----------------------------------------------------------------------------
// ImFontAtlas::GlyphRangesBuilder
//-----------------------------------------------------------------------------

void ImFontAtlas::GlyphRangesBuilder::AddText(const char* text, const char* text_end)
{
    while (text_end ? (text < text_end) : *text)
    {
        unsigned int c = 0;
        int c_len = ImTextCharFromUtf8(&c, text, text_end);
        text += c_len;
        if (c_len == 0)
            break;
        if (c < 0x10000)
            AddChar((ImWchar)c);
    }
}

void ImFontAtlas::GlyphRangesBuilder::AddRanges(const ImWchar* ranges)
{
    for (; ranges[0]; ranges += 2)
        for (ImWchar c = ranges[0]; c <= ranges[1]; c++)
            AddChar(c);
}

void ImFontAtlas::GlyphRangesBuilder::BuildRanges(ImVector<ImWchar>* out_ranges)
{
    for (int n = 0; n < 0x10000; n++)
        if (GetBit(n))
        {
            out_ranges->push_back((ImWchar)n);
            while (n < 0x10000 && GetBit(n + 1))
                n++;
            out_ranges->push_back((ImWchar)n);
        }
    out_ranges->push_back(0);
}

//-----------------------------------------------------------------------------
// ImFont
//-----------------------------------------------------------------------------

ImFont::ImFont()
{
    Scale = 1.0f;
    FallbackChar = (ImWchar)'?';
    DisplayOffset = ImVec2(0.0f, 0.0f);
    ClearOutputData();
}

ImFont::~ImFont()
{
    // Invalidate active font so that the user gets a clear crash instead of a dangling pointer.
    // If you want to delete fonts you need to do it between Render() and NewFrame().
    // FIXME-CLEANUP
    /*
    ImGuiContext& g = *GImGui;
    if (g.Font == this)
        g.Font = NULL;
    */
    ClearOutputData();
}

void    ImFont::ClearOutputData()
{
    FontSize = 0.0f;
    Glyphs.clear();
    IndexAdvanceX.clear();
    IndexLookup.clear();
    FallbackGlyph = NULL;
    FallbackAdvanceX = 0.0f;
    ConfigDataCount = 0;
    ConfigData = NULL;
    ContainerAtlas = NULL;
    Ascent = Descent = 0.0f;
    DirtyLookupTables = true;
    MetricsTotalSurface = 0;
}

void ImFont::BuildLookupTable()
{
    int max_codepoint = 0;
    for (int i = 0; i != Glyphs.Size; i++)
        max_codepoint = ImMax(max_codepoint, (int)Glyphs[i].Codepoint);

    IM_ASSERT(Glyphs.Size < 0xFFFF); // -1 is reserved
    IndexAdvanceX.clear();
    IndexLookup.clear();
    DirtyLookupTables = false;
    GrowIndex(max_codepoint + 1);
    for (int i = 0; i < Glyphs.Size; i++)
    {
        int codepoint = (int)Glyphs[i].Codepoint;
        IndexAdvanceX[codepoint] = Glyphs[i].AdvanceX;
        IndexLookup[codepoint] = (unsigned short)i;
    }

    // Create a glyph to handle TAB
    // FIXME: Needs proper TAB handling but it needs to be contextualized (or we could arbitrary say that each string starts at "column 0" ?)
    if (FindGlyph((unsigned short)' '))
    {
        if (Glyphs.back().Codepoint != '\t')   // So we can call this function multiple times
            Glyphs.resize(Glyphs.Size + 1);
        ImFontGlyph& tab_glyph = Glyphs.back();
        tab_glyph = *FindGlyph((unsigned short)' ');
        tab_glyph.Codepoint = '\t';
        tab_glyph.AdvanceX *= 4;
        IndexAdvanceX[(int)tab_glyph.Codepoint] = (float)tab_glyph.AdvanceX;
        IndexLookup[(int)tab_glyph.Codepoint] = (unsigned short)(Glyphs.Size-1);
    }

    FallbackGlyph = FindGlyphNoFallback(FallbackChar);
    FallbackAdvanceX = FallbackGlyph ? FallbackGlyph->AdvanceX : 0.0f;
    for (int i = 0; i < max_codepoint + 1; i++)
        if (IndexAdvanceX[i] < 0.0f)
            IndexAdvanceX[i] = FallbackAdvanceX;
}

void ImFont::SetFallbackChar(ImWchar c)
{
    FallbackChar = c;
    BuildLookupTable();
}

void ImFont::GrowIndex(int new_size)
{
    IM_ASSERT(IndexAdvanceX.Size == IndexLookup.Size);
    if (new_size <= IndexLookup.Size)
        return;
    IndexAdvanceX.resize(new_size, -1.0f);
    IndexLookup.resize(new_size, (unsigned short)-1);
}

// x0/y0/x1/y1 are offset from the character upper-left layout position, in pixels. Therefore x0/y0 are often fairly close to zero.
// Not to be mistaken with texture coordinates, which are held by u0/v0/u1/v1 in normalized format (0.0..1.0 on each texture axis).
void ImFont::AddGlyph(ImWchar codepoint, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x)
{
    Glyphs.resize(Glyphs.Size + 1);
    ImFontGlyph& glyph = Glyphs.back();
    glyph.Codepoint = (ImWchar)codepoint;
    glyph.X0 = x0;
    glyph.Y0 = y0;
    glyph.X1 = x1;
    glyph.Y1 = y1;
    glyph.U0 = u0;
    glyph.V0 = v0;
    glyph.U1 = u1;
    glyph.V1 = v1;
    glyph.AdvanceX = advance_x + ConfigData->GlyphExtraSpacing.x;  // Bake spacing into AdvanceX

    if (ConfigData->PixelSnapH)
        glyph.AdvanceX = (float)(int)(glyph.AdvanceX + 0.5f);

    // Compute rough surface usage metrics (+1 to account for average padding, +0.99 to round)
    DirtyLookupTables = true;
    MetricsTotalSurface += (int)((glyph.U1 - glyph.U0) * ContainerAtlas->TexWidth + 1.99f) * (int)((glyph.V1 - glyph.V0) * ContainerAtlas->TexHeight + 1.99f);
}

void ImFont::AddRemapChar(ImWchar dst, ImWchar src, bool overwrite_dst)
{
    IM_ASSERT(IndexLookup.Size > 0);    // Currently this can only be called AFTER the font has been built, aka after calling ImFontAtlas::GetTexDataAs*() function.
    int index_size = IndexLookup.Size;

    if (dst < index_size && IndexLookup.Data[dst] == (unsigned short)-1 && !overwrite_dst) // 'dst' already exists
        return;
    if (src >= index_size && dst >= index_size) // both 'dst' and 'src' don't exist -> no-op
        return;

    GrowIndex(dst + 1);
    IndexLookup[dst] = (src < index_size) ? IndexLookup.Data[src] : (unsigned short)-1;
    IndexAdvanceX[dst] = (src < index_size) ? IndexAdvanceX.Data[src] : 1.0f;
}

const ImFontGlyph* ImFont::FindGlyph(ImWchar c) const
{
    if (c >= IndexLookup.Size)
        return FallbackGlyph;
    const unsigned short i = IndexLookup[c];
    if (i == (unsigned short)-1)
        return FallbackGlyph;
    return &Glyphs.Data[i];
}

const ImFontGlyph* ImFont::FindGlyphNoFallback(ImWchar c) const
{
    if (c >= IndexLookup.Size)
        return NULL;
    const unsigned short i = IndexLookup[c];
    if (i == (unsigned short)-1)
        return NULL;
    return &Glyphs.Data[i];
}

const char* ImFont::CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) const
{
    // Simple word-wrapping for English, not full-featured. Please submit failing cases!
    // FIXME: Much possible improvements (don't cut things like "word !", "word!!!" but cut within "word,,,,", more sensible support for punctuations, support for Unicode punctuations, etc.)

    // For references, possible wrap point marked with ^
    //  "aaa bbb, ccc,ddd. eee   fff. ggg!"
    //      ^    ^    ^   ^   ^__    ^    ^

    // List of hardcoded separators: .,;!?'"

    // Skip extra blanks after a line returns (that includes not counting them in width computation)
    // e.g. "Hello    world" --> "Hello" "World"

    // Cut words that cannot possibly fit within one line.
    // e.g.: "The tropical fish" with ~5 characters worth of width --> "The tr" "opical" "fish"

    float line_width = 0.0f;
    float word_width = 0.0f;
    float blank_width = 0.0f;
    wrap_width /= scale; // We work with unscaled widths to avoid scaling every characters

    const char* word_end = text;
    const char* prev_word_end = NULL;
    bool inside_word = true;

    const char* s = text;
    while (s < text_end)
    {
        unsigned int c = (unsigned int)*s;
        const char* next_s;
        if (c < 0x80)
            next_s = s + 1;
        else
            next_s = s + ImTextCharFromUtf8(&c, s, text_end);
        if (c == 0)
            break;

        if (c < 32)
        {
            if (c == '\n')
            {
                line_width = word_width = blank_width = 0.0f;
                inside_word = true;
                s = next_s;
                continue;
            }
            if (c == '\r')
            {
                s = next_s;
                continue;
            }
        }

        const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX[(int)c] : FallbackAdvanceX);
        if (ImCharIsBlankW(c))
        {
            if (inside_word)
            {
                line_width += blank_width;
                blank_width = 0.0f;
                word_end = s;
            }
            blank_width += char_width;
            inside_word = false;
        }
        else
        {
            word_width += char_width;
            if (inside_word)
            {
                word_end = next_s;
            }
            else
            {
                prev_word_end = word_end;
                line_width += word_width + blank_width;
                word_width = blank_width = 0.0f;
            }

            // Allow wrapping after punctuation.
            inside_word = !(c == '.' || c == ',' || c == ';' || c == '!' || c == '?' || c == '\"');
        }

        // We ignore blank width at the end of the line (they can be skipped)
        if (line_width + word_width >= wrap_width)
        {
            // Words that cannot possibly fit within an entire line will be cut anywhere.
            if (word_width < wrap_width)
                s = prev_word_end ? prev_word_end : word_end;
            break;
        }

        s = next_s;
    }

    return s;
}

ImVec2 ImFont::CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end, const char** remaining) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // FIXME-OPT: Need to avoid this.

    const float line_height = size;
    const float scale = size / FontSize;

    ImVec2 text_size = ImVec2(0,0);
    float line_width = 0.0f;

    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    const char* s = text_begin;
    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - line_width);
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                if (text_size.x < line_width)
                    text_size.x = line_width;
                text_size.y += line_height;
                line_width = 0.0f;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (ImCharIsBlankA(c)) { s++; } else if (c == '\n') { s++; break; } else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        const char* prev_s = s;
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                text_size.x = ImMax(text_size.x, line_width);
                text_size.y += line_height;
                line_width = 0.0f;
                continue;
            }
            if (c == '\r')
                continue;
        }

        const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX[(int)c] : FallbackAdvanceX) * scale;
        if (line_width + char_width >= max_width)
        {
            s = prev_s;
            break;
        }

        line_width += char_width;
    }

    if (text_size.x < line_width)
        text_size.x = line_width;

    if (line_width > 0 || text_size.y == 0.0f)
        text_size.y += line_height;

    if (remaining)
        *remaining = s;

    return text_size;
}

void ImFont::RenderChar(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, unsigned short c) const
{
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') // Match behavior of RenderText(), those 4 codepoints are hard-coded.
        return;
    if (const ImFontGlyph* glyph = FindGlyph(c))
    {
        float scale = (size >= 0.0f) ? (size / FontSize) : 1.0f;
        pos.x = (float)(int)pos.x + DisplayOffset.x;
        pos.y = (float)(int)pos.y + DisplayOffset.y;
        draw_list->PrimReserve(6, 4);
        draw_list->PrimRectUV(ImVec2(pos.x + glyph->X0 * scale, pos.y + glyph->Y0 * scale), ImVec2(pos.x + glyph->X1 * scale, pos.y + glyph->Y1 * scale), ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V1), col);
    }
}

void ImFont::RenderText(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, const ImVec4& clip_rect, const char* text_begin, const char* text_end, float wrap_width, bool cpu_fine_clip) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // ImGui functions generally already provides a valid text_end, so this is merely to handle direct calls.

    // Align to be pixel perfect
    pos.x = (float)(int)pos.x + DisplayOffset.x;
    pos.y = (float)(int)pos.y + DisplayOffset.y;
    float x = pos.x;
    float y = pos.y;
    if (y > clip_rect.w)
        return;

    const float scale = size / FontSize;
    const float line_height = FontSize * scale;
    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    // Fast-forward to first visible line
    const char* s = text_begin;
    if (y + line_height < clip_rect.y && !word_wrap_enabled)
        while (y + line_height < clip_rect.y)
        {
            while (s < text_end)
                if (*s++ == '\n')
                    break;
            y += line_height;
        }

    // For large text, scan for the last visible line in order to avoid over-reserving in the call to PrimReserve()
    // Note that very large horizontal line will still be affected by the issue (e.g. a one megabyte string buffer without a newline will likely crash atm)
    if (text_end - s > 10000 && !word_wrap_enabled)
    {
        const char* s_end = s;
        float y_end = y;
        while (y_end < clip_rect.w)
        {
            while (s_end < text_end)
                if (*s_end++ == '\n')
                    break;
            y_end += line_height;
        }
        text_end = s_end;
    }

    // Reserve vertices for remaining worse case (over-reserving is useful and easily amortized)
    const int vtx_count_max = (int)(text_end - s) * 4;
    const int idx_count_max = (int)(text_end - s) * 6;
    const int idx_expected_size = draw_list->IdxBuffer.Size + idx_count_max;
    draw_list->PrimReserve(idx_count_max, vtx_count_max);

    ImDrawVert* vtx_write = draw_list->_VtxWritePtr;
    ImDrawIdx* idx_write = draw_list->_IdxWritePtr;
    unsigned int vtx_current_idx = draw_list->_VtxCurrentIdx;

    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - (x - pos.x));
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                x = pos.x;
                y += line_height;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (ImCharIsBlankA(c)) { s++; } else if (c == '\n') { s++; break; } else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                x = pos.x;
                y += line_height;
                if (y > clip_rect.w)
                    break; // break out of main loop
                continue;
            }
            if (c == '\r')
                continue;
        }

        float char_width = 0.0f;
        if (const ImFontGlyph* glyph = FindGlyph((unsigned short)c))
        {
            char_width = glyph->AdvanceX * scale;

            // Arbitrarily assume that both space and tabs are empty glyphs as an optimization
            if (c != ' ' && c != '\t')
            {
                // We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
                float x1 = x + glyph->X0 * scale;
                float x2 = x + glyph->X1 * scale;
                float y1 = y + glyph->Y0 * scale;
                float y2 = y + glyph->Y1 * scale;
                if (x1 <= clip_rect.z && x2 >= clip_rect.x)
                {
                    // Render a character
                    float u1 = glyph->U0;
                    float v1 = glyph->V0;
                    float u2 = glyph->U1;
                    float v2 = glyph->V1;

                    // CPU side clipping used to fit text in their frame when the frame is too small. Only does clipping for axis aligned quads.
                    if (cpu_fine_clip)
                    {
                        if (x1 < clip_rect.x)
                        {
                            u1 = u1 + (1.0f - (x2 - clip_rect.x) / (x2 - x1)) * (u2 - u1);
                            x1 = clip_rect.x;
                        }
                        if (y1 < clip_rect.y)
                        {
                            v1 = v1 + (1.0f - (y2 - clip_rect.y) / (y2 - y1)) * (v2 - v1);
                            y1 = clip_rect.y;
                        }
                        if (x2 > clip_rect.z)
                        {
                            u2 = u1 + ((clip_rect.z - x1) / (x2 - x1)) * (u2 - u1);
                            x2 = clip_rect.z;
                        }
                        if (y2 > clip_rect.w)
                        {
                            v2 = v1 + ((clip_rect.w - y1) / (y2 - y1)) * (v2 - v1);
                            y2 = clip_rect.w;
                        }
                        if (y1 >= y2)
                        {
                            x += char_width;
                            continue;
                        }
                    }

                    // We are NOT calling PrimRectUV() here because non-inlined causes too much overhead in a debug builds. Inlined here:
                    {
                        idx_write[0] = (ImDrawIdx)(vtx_current_idx); idx_write[1] = (ImDrawIdx)(vtx_current_idx+1); idx_write[2] = (ImDrawIdx)(vtx_current_idx+2);
                        idx_write[3] = (ImDrawIdx)(vtx_current_idx); idx_write[4] = (ImDrawIdx)(vtx_current_idx+2); idx_write[5] = (ImDrawIdx)(vtx_current_idx+3);
                        vtx_write[0].pos.x = x1; vtx_write[0].pos.y = y1; vtx_write[0].col = col; vtx_write[0].uv.x = u1; vtx_write[0].uv.y = v1;
                        vtx_write[1].pos.x = x2; vtx_write[1].pos.y = y1; vtx_write[1].col = col; vtx_write[1].uv.x = u2; vtx_write[1].uv.y = v1;
                        vtx_write[2].pos.x = x2; vtx_write[2].pos.y = y2; vtx_write[2].col = col; vtx_write[2].uv.x = u2; vtx_write[2].uv.y = v2;
                        vtx_write[3].pos.x = x1; vtx_write[3].pos.y = y2; vtx_write[3].col = col; vtx_write[3].uv.x = u1; vtx_write[3].uv.y = v2;
                        vtx_write += 4;
                        vtx_current_idx += 4;
                        idx_write += 6;
                    }
                }
            }
        }

        x += char_width;
    }

    // Give back unused vertices
    draw_list->VtxBuffer.resize((int)(vtx_write - draw_list->VtxBuffer.Data));
    draw_list->IdxBuffer.resize((int)(idx_write - draw_list->IdxBuffer.Data));
    draw_list->CmdBuffer[draw_list->CmdBuffer.Size-1].ElemCount -= (idx_expected_size - draw_list->IdxBuffer.Size);
    draw_list->_VtxWritePtr = vtx_write;
    draw_list->_IdxWritePtr = idx_write;
    draw_list->_VtxCurrentIdx = (unsigned int)draw_list->VtxBuffer.Size;
}

//-----------------------------------------------------------------------------
// Internals Render Helpers
// (progressively moved from imgui.cpp to here when they are redesigned to stop accessing ImGui global state)
//-----------------------------------------------------------------------------
// RenderMouseCursor()
// RenderArrowPointingAt()
// RenderRectFilledRangeH()
//-----------------------------------------------------------------------------

void ImGui::RenderMouseCursor(ImDrawList* draw_list, ImVec2 pos, float scale, ImGuiMouseCursor mouse_cursor)
{
    if (mouse_cursor == ImGuiMouseCursor_None)
        return;
    IM_ASSERT(mouse_cursor > ImGuiMouseCursor_None && mouse_cursor < ImGuiMouseCursor_COUNT);

    const ImU32 col_shadow = IM_COL32(0, 0, 0, 48);
    const ImU32 col_border = IM_COL32(0, 0, 0, 255);          // Black
    const ImU32 col_fill   = IM_COL32(255, 255, 255, 255);    // White

    ImFontAtlas* font_atlas = draw_list->_Data->Font->ContainerAtlas;
    ImVec2 offset, size, uv[4];
    if (font_atlas->GetMouseCursorTexData(mouse_cursor, &offset, &size, &uv[0], &uv[2]))
    {
        pos -= offset;
        const ImTextureID tex_id = font_atlas->TexID;
        draw_list->PushTextureID(tex_id);
        draw_list->AddImage(tex_id, pos + ImVec2(1,0)*scale, pos + ImVec2(1,0)*scale + size*scale, uv[2], uv[3], col_shadow);
        draw_list->AddImage(tex_id, pos + ImVec2(2,0)*scale, pos + ImVec2(2,0)*scale + size*scale, uv[2], uv[3], col_shadow);
        draw_list->AddImage(tex_id, pos,                     pos + size*scale,                     uv[2], uv[3], col_border);
        draw_list->AddImage(tex_id, pos,                     pos + size*scale,                     uv[0], uv[1], col_fill);
        draw_list->PopTextureID();
    }
}

// Render an arrow. 'pos' is position of the arrow tip. half_sz.x is length from base to tip. half_sz.y is length on each side.
void ImGui::RenderArrowPointingAt(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, ImGuiDir direction, ImU32 col)
{
    switch (direction)
    {
    case ImGuiDir_Left:  draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), pos, col); return;
    case ImGuiDir_Right: draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), pos, col); return;
    case ImGuiDir_Up:    draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), pos, col); return;
    case ImGuiDir_Down:  draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), pos, col); return;
    case ImGuiDir_None: case ImGuiDir_COUNT: break; // Fix warnings
    }
}

static inline float ImAcos01(float x)
{
    if (x <= 0.0f) return IM_PI * 0.5f;
    if (x >= 1.0f) return 0.0f;
    return ImAcos(x);
    //return (-0.69813170079773212f * x * x - 0.87266462599716477f) * x + 1.5707963267948966f; // Cheap approximation, may be enough for what we do.
}

// FIXME: Cleanup and move code to ImDrawList.
void ImGui::RenderRectFilledRangeH(ImDrawList* draw_list, const ImRect& rect, ImU32 col, float x_start_norm, float x_end_norm, float rounding)
{
    if (x_end_norm == x_start_norm)
        return;
    if (x_start_norm > x_end_norm)
        ImSwap(x_start_norm, x_end_norm);

    ImVec2 p0 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_start_norm), rect.Min.y);
    ImVec2 p1 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_end_norm), rect.Max.y);
    if (rounding == 0.0f)
    {
        draw_list->AddRectFilled(p0, p1, col, 0.0f);
        return;
    }

    rounding = ImClamp(ImMin((rect.Max.x - rect.Min.x) * 0.5f, (rect.Max.y - rect.Min.y) * 0.5f) - 1.0f, 0.0f, rounding);
    const float inv_rounding = 1.0f / rounding;
    const float arc0_b = ImAcos01(1.0f - (p0.x - rect.Min.x) * inv_rounding);
    const float arc0_e = ImAcos01(1.0f - (p1.x - rect.Min.x) * inv_rounding);
    const float x0 = ImMax(p0.x, rect.Min.x + rounding);
    if (arc0_b == arc0_e)
    {
        draw_list->PathLineTo(ImVec2(x0, p1.y));
        draw_list->PathLineTo(ImVec2(x0, p0.y));
    }
    else if (arc0_b == 0.0f && arc0_e == IM_PI*0.5f)
    {
        draw_list->PathArcToFast(ImVec2(x0, p1.y - rounding), rounding, 3, 6); // BL
        draw_list->PathArcToFast(ImVec2(x0, p0.y + rounding), rounding, 6, 9); // TR
    }
    else
    {
        draw_list->PathArcTo(ImVec2(x0, p1.y - rounding), rounding, IM_PI - arc0_e, IM_PI - arc0_b, 3); // BL
        draw_list->PathArcTo(ImVec2(x0, p0.y + rounding), rounding, IM_PI + arc0_b, IM_PI + arc0_e, 3); // TR
    }
    if (p1.x > rect.Min.x + rounding)
    {
        const float arc1_b = ImAcos01(1.0f - (rect.Max.x - p1.x) * inv_rounding);
        const float arc1_e = ImAcos01(1.0f - (rect.Max.x - p0.x) * inv_rounding);
        const float x1 = ImMin(p1.x, rect.Max.x - rounding);
        if (arc1_b == arc1_e)
        {
            draw_list->PathLineTo(ImVec2(x1, p0.y));
            draw_list->PathLineTo(ImVec2(x1, p1.y));
        }
        else if (arc1_b == 0.0f && arc1_e == IM_PI*0.5f)
        {
            draw_list->PathArcToFast(ImVec2(x1, p0.y + rounding), rounding, 9, 12); // TR
            draw_list->PathArcToFast(ImVec2(x1, p1.y - rounding), rounding, 0, 3);  // BR
        }
        else
        {
            draw_list->PathArcTo(ImVec2(x1, p0.y + rounding), rounding, -arc1_e, -arc1_b, 3); // TR
            draw_list->PathArcTo(ImVec2(x1, p1.y - rounding), rounding, +arc1_b, +arc1_e, 3); // BR
        }
    }
    draw_list->PathFillConvex(col);
}

//-----------------------------------------------------------------------------
// DEFAULT FONT DATA
//-----------------------------------------------------------------------------
// Compressed with stb_compress() then converted to a C array and encoded as base85.
// Use the program in misc/fonts/binary_to_compressed_c.cpp to create the array from a TTF file.
// The purpose of encoding as base85 instead of "0x00,0x01,..." style is only save on _source code_ size.
// Decompression from stb.h (public domain) by Sean Barrett https://github.com/nothings/stb/blob/master/stb.h
//-----------------------------------------------------------------------------

static unsigned int stb_decompress_length(const unsigned char *input)
{
    return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

static unsigned char *stb__barrier_out_e, *stb__barrier_out_b;
static const unsigned char *stb__barrier_in_b;
static unsigned char *stb__dout;
static void stb__match(const unsigned char *data, unsigned int length)
{
    // INVERSE of memmove... write each byte before copying the next...
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_out_b) { stb__dout = stb__barrier_out_e+1; return; }
    while (length--) *stb__dout++ = *data++;
}

static void stb__lit(const unsigned char *data, unsigned int length)
{
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_in_b) { stb__dout = stb__barrier_out_e+1; return; }
    memcpy(stb__dout, data, length);
    stb__dout += length;
}

#define stb__in2(x)   ((i[x] << 8) + i[(x)+1])
#define stb__in3(x)   ((i[x] << 16) + stb__in2((x)+1))
#define stb__in4(x)   ((i[x] << 24) + stb__in3((x)+1))

static const unsigned char *stb_decompress_token(const unsigned char *i)
{
    if (*i >= 0x20) { // use fewer if's for cases that expand small
        if (*i >= 0x80)       stb__match(stb__dout-i[1]-1, i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)  stb__match(stb__dout-(stb__in2(0) - 0x4000 + 1), i[2]+1), i += 3;
        else /* *i >= 0x20 */ stb__lit(i+1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    } else { // more ifs for cases that expand large, since overhead is amortized
        if (*i >= 0x18)       stb__match(stb__dout-(stb__in3(0) - 0x180000 + 1), i[3]+1), i += 4;
        else if (*i >= 0x10)  stb__match(stb__dout-(stb__in3(0) - 0x100000 + 1), stb__in2(3)+1), i += 5;
        else if (*i >= 0x08)  stb__lit(i+2, stb__in2(0) - 0x0800 + 1), i += 2 + (stb__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)  stb__lit(i+3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
        else if (*i == 0x06)  stb__match(stb__dout-(stb__in3(1)+1), i[4]+1), i += 5;
        else if (*i == 0x04)  stb__match(stb__dout-(stb__in3(1)+1), stb__in2(4)+1), i += 6;
    }
    return i;
}

static unsigned int stb_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen)
{
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen, i;

    blocklen = buflen % 5552;
    while (buflen) {
        for (i=0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0], s2 += s1;
            s1 += buffer[1], s2 += s1;
            s1 += buffer[2], s2 += s1;
            s1 += buffer[3], s2 += s1;
            s1 += buffer[4], s2 += s1;
            s1 += buffer[5], s2 += s1;
            s1 += buffer[6], s2 += s1;
            s1 += buffer[7], s2 += s1;

            buffer += 8;
        }

        for (; i < blocklen; ++i)
            s1 += *buffer++, s2 += s1;

        s1 %= ADLER_MOD, s2 %= ADLER_MOD;
        buflen -= blocklen;
        blocklen = 5552;
    }
    return (unsigned int)(s2 << 16) + (unsigned int)s1;
}

static unsigned int stb_decompress(unsigned char *output, const unsigned char *i, unsigned int /*length*/)
{
    unsigned int olen;
    if (stb__in4(0) != 0x57bC0000) return 0;
    if (stb__in4(4) != 0)          return 0; // error! stream is > 4GB
    olen = stb_decompress_length(i);
    stb__barrier_in_b = i;
    stb__barrier_out_e = output + olen;
    stb__barrier_out_b = output;
    i += 16;

    stb__dout = output;
    for (;;) {
        const unsigned char *old_i = i;
        i = stb_decompress_token(i);
        if (i == old_i) {
            if (*i == 0x05 && i[1] == 0xfa) {
                IM_ASSERT(stb__dout == output + olen);
                if (stb__dout != output + olen) return 0;
                if (stb_adler32(1, output, olen) != (unsigned int) stb__in4(2))
                    return 0;
                return olen;
            } else {
                IM_ASSERT(0); /* NOTREACHED */
                return 0;
            }
        }
        IM_ASSERT(stb__dout <= output + olen);
        if (stb__dout > output + olen)
            return 0;
    }
}

//-----------------------------------------------------------------------------
// ProggyClean.ttf
// Copyright (c) 2004, 2005 Tristan Grimmer
// MIT license (see License.txt in http://www.upperbounds.net/download/ProggyClean.ttf.zip)
// Download and more information at http://upperbounds.net
//-----------------------------------------------------------------------------
// File: 'ProggyClean.ttf' (41208 bytes)
// Exported using misc/fonts/binary_to_compressed_c.cpp (with compression + base85 string encoding).
// The purpose of encoding as base85 instead of "0x00,0x01,..." style is only save on _source code_ size.
//-----------------------------------------------------------------------------
//Counter-Strike
static const char Cousine_Regular_ttf_compressed_data_base85[47040 + 1] =
"7])#######WNWrN'/###I),##bw9hLQXH##j$1S:'>pgL@Kt(9668X%QUQl;1TmEd;Q^rEN@4',:a`T9-V:;$wmZJ;V'q?-'Tx-31s(U<h2TlSQv$sAqV,-G+D1>V,nHS[J)-5J<gXG-"
"EXd;^*1kHVn[<?6$jg0Ffc:jLH^^/1/%@W$%J%/GDhno%IU%d3-q+5(,5LsC8%JsKtc0=QFp<%9b';9C&u(rNAWC&,r1dD4AJr9.afNMB)a4&>V3dW.;h^`I8bo#e#,Ncs8cB3_o6Q<B"
"uY3Ugi[2bj:8&9]0iR/GcI,,DID^VIeQi)3+>00Fcc<;5gfCfUW*35&S$S+HV2Z,v?to=Y-5YY#@cFVCSY)TspD/2'[G:;$gmRfL_C*j0%ow'XodPirJ[m##>hjYdswgZNHM]Cs6%_fh"
"dbaw'OMYY#0]Wh#bYbw$ehG3#J]35vlubU$KdW3M<DT;-:-Xn/vTq/#=M=.#e<151c>$(#+?uu#Nkc+#?KXV.Nc3T#&rpo/%V%h$W,>>#]A<Z5C8SfCDbIW$9L<p%.Y1Z#wI)4#kap)#"
"t,k(N*Uwt7O1HS7)3W)+7NCiKP^Q>YBH;-#%4GCibGrY5l[$C/k7?>#9Kel8(V5qVvZ&.$PDY>#oid(#7>5=6GQSX#v_P+#<@_J#wZYF#1+1/#4Lg2#4)[0#<ME/1Y3m3#7+BM#vl7-#"
"?B#s-0q/kLwb<4#e8cwL6lh[MmLS8#.$)t-ks5tL5RX3NFsH+#S'02#abtK2lQk&#^sG3#P&M$#Pq4Y#_Ui].Y5n0#Xb%^.=.92#4whP/0`2<#c$S-#<)NS]+,%jq4([(a(el59A`w4A"
"].g;Rr3B,)GQ)dE5re34>wE'S_X]w';*BD<gs_4fjq`?T>b9e?(VaKc[A))3e-U>QF5[Ee<BE>,r@5DEOdGJ(sVg1Thk=i#-x)X_/NB,Wvb_L#I-b9`0aIcM;Ce3#q8o7N@J=jLnuWuL"
"YX&nN2?VhLc@[tLU9T;-%1G@MHvHTM^.@tL:J5s-3T3rLC7lZMaTNmLcTNmL;O)L)[ra]+C/Buup.bCsB6euP3(Lo[32k4]%Ikr?`O+Seqb].qDib@t)2r=cv^H#$@MR&,Z2c'&T7'L5"
")Eu'&hF;;$f,?X(viY&#G;G##NqH0#VYQC#A<eo#R'^R-<Bx=-Nsk>-7@KB-)>+u-jle+M;KG&#4Mx>-C':W.96f=-ZKx>-H$o;7])+##xp[xF^_aOoJ,*i]E[fO[(nhLSCp)M/X,*(`"
"bu[>-7f4k#G-E##>$S-#B0f-#F<x-#JH4.#NTF.#RaX.#Lfb.#Xst.#])1/#a5C/#eAU/#iMh/#mY$0#qf60#urH0##)[0#'5n0#+A*1#/M<1#3YN1#7fa1#;rs1#?(02#C4B2#G@T2#"
"KLg2#OX#3#Se53#WqG3#['Z3#`3m3#d?)4#hK;4#lWM4#pd`4#tpr4#x&/5#&3A5#*?S5#.Kf5#2Wx5#6d46#:pF6#>&Y6#B2l6#F>(7#JJ:7#9.>>#As(bnXPsA##J,W-^v&2#pI1##"
"/r$W$Aeap%QWG3'bJ.L(r=ke),1Q(+<$8A,LmtY-]`Zs.mRA60'F(O179eh2G,K+4Wu1D5hhn]6xZTv72N;99BAxQ:R4_k;c'E.=sp+G>-dh`?=VN#AMI5<B`D<RNr<i>UWPZqWiUfOY"
"%LLiZ5?3,]JA>a^_OI?`_HOuB5fZ2IFFA.#2te(N-$(/#.oWSM,q?@JWA4'L-Fl$':P10#kUQ'S6#]4dN`Old`&qO.i0N,gsa8>I>)QfLCk8oN[#v1P3cWrQ_^Wparlr.#85@sLLND,#"
"WTe^?#eJf=gqdL<V((4;F5Aq96BZW8&Ot>7l[7&6[iPc4EHRL2/Ll31ixms.X/1Z-*)SL(^pY3'IkNT%9xh;$)/,##S@*Q9%XqG/_RW5/YF(R1OU^;.7/v<-AYVt-,<tiL)&IEP8$#N-"
"`NB5Bv2%A0/iv?0b#:A02%<@0`^#l=4Sq<-t$dP1-bP?.[tX,9v8vQ#TonoL&Pc%9q)TV[lnG<-wucW-3ur$'4s_$'_;xiLJ0t>--Ggw8ULglA*>.a9'fe@9w9m31@,H6&2_BQC1``'#"
"msXgL0_YgL-,R)0jU8Q#e96GM(hwl/VgQm#fwkgL>JNZ[GK=9B/MH.=v-pd+A7mS1WO:a'?R<nL2;CP8$.#W[$S&K%<am>-%19t_[6&J-LgNUO#]f$#saIq&qVW<B6pe2$)0LA6+YBHO"
"L'9^(KlV<.g)A9J$bk31U_lA1+uNZ[H0Z`-[ID:D5H#iGX$Cj$FHt>-&LRfL%k>5p:@<VO<4+3N.XIp.WSn+M]81Q9xp:KO7Bv>-4>=KOi.,JL.5P2(9^e)'p-$>9TUg5#(^.Q#N[$k_"
"lk,a9u=rA('rYY5a'?R9/.n>).-R][[Z^B-oSqgucI/<7fxUc+C0,nalF^#$PAxE7W-V'#$]:Z[OrQK1kD=/M/ZO=_HaqpL.9WALb-$x''P9MB1+LS72LY][x=x3+O#J4+2Zl4MeMaQN"
"a'6Z%JC<Z-q[Le=fe,N$oBmB.nu6g7AKYALcgF5Bpta9.K&lT[e-NY-IvDL(..MkL?(l?-MT$*0ZBIF-ZQ2W[n-&cO**)N$uSU@-UE#W-Hm`-6jRoV[h*HiL5iLN(]n8>.b;Zw9I7e59"
".N,s-G2(iLhcfM1toJq07/H59+<%D?0obmLCwe8BOrh8.$[212_a#6B@MglAZC@']QdQ][Aj*`[4q+<8xPQ/;<<lBSH]_h=c>hhMUrMZ[+jh5#MW744kn*79K&w596ZCHO0Z,<-%LuD3"
";=aC?f[d$v`Bj@-%uN(#JN2v8L:p/10n5B?B.(69&%7qCoemg+d^+pAs6$g+94;^?:Aba?^_3b?e0Hp80BmD0n-uC.5tqY?@/=_#_0@5;6/l-=;nJlL6GVPM[6D.MhF7p.`UB$.?.?fM"
"O^Dj$&bap.PS&d#K(X][462hLflcP9HFg;.)mLe$#[Wp.)PP8.eTNp.g,P?gWh'?-Bun%#6$X6NBPZ?-CV%0NjQ<$P)2M9.,^-6:xrfv$wB3p.T27T8VbqY?>3_h#Kr@_#YbX>-NPa;."
"54.I-GT=Z-RmaO+8-0?-C?3W%I`3[9:)@T.:0Q?.lPg=-D)cC-',uB.(#>F-h:XC?94;^?^_3b?*#ZQ1Mg5t$,hjmLbBOg$=s'D?)cv51U-l31x.wiLu3EiM%'Hm#omQ][o6>W84.m<-"
"?YCq7h8ZV[8RJ>--P,hLmFHDOB@Le=OtB%@v4jn<*imW_A&i5#MrkNXfv;hLLNl31)C*Q9NLF9BA[r<-,R`m$*4Ag7N.$>9SSTP1sVpF.)%sF.:BPMMj@1I=9;k7JY:Gf%UUr+;[U&>8"
"0<Qm#:S)K.Z=/x[@Jwk=JqnB(w]fcN/s[m#uwdsA?MEg.>r$?\?R0$##sRM=-A$).MRHo>-VC,-Mba5oLT_uGMLkoL_9Ti.LK3`>$KNnw'dXxQ#@-MeMVeRU.@N79_3I;=-WsF?-0@;=-"
"5I;=-qF;=-7Rqx4/x%h*K/l-=YjG59Cf6g7L?Zd?%.^a?)BRi.AHGm#YTh]4N$@8.j[Fm#7j6Q#=aET[MCk?8Dof88seN;7seN;7;b?v$K&K=%:p-Q#>oj31(Ae39,0$M14tvk%oNuv#"
"r#@crX:MiqM12MqN.m1qIiZ`OHLx8Zq&Ytp]It+`S#a$q5NPQ@r&N9$E3WsobDEg9eYBSm4Q9SmJK0Sm'-=rlwk8Zl8]Jf9qdI:l%*E6l74p9l%O3]3rc8f9nMDR#wLsD#6-DG#kTdW6"
"C8;o6s42o6R0a-8CU@%#PaFoL/x.qLYqQuuYUGR]V-^B-PL,7]8#uF`?^O2(+YmPfCD#7]ko#2_.XEpBUrN2(?T#l7Z(HFL@PS4%MffhjdT%sjj3hU#[[?%-.0`Fd#D_PAD5A(Lm48Ub"
"]Ob=/sNYR#F:g-alk,#G#5vn#vf?J`PY1L7ghqK[,dck$%.$$-:Gn3_Uu_:_pD;/_@?k),_$`Q9btC;-dBCBLGXnN$k(TD^'n3K7(K03$*egT7aEY'-@+Sc[G&*saX0j2$F5V>[$SZf["
"A$[kZ(;:,2iX$m#0NE`aKXml#.4pi'e>UM'#$&##B3,/(>fW>-99.&G=aSA$C<5/(u<X`<-?E3GHl0ESG:R^90N;US*;bfLoltGStasmL##8T0u./O9H<,;-a=sfRf9gS7LsRR9)]$`+"
">RD^4>)9q'ddgG7e8uxp0wqg_J3E]F;@&B-l16D/''7(v_3fM#YJ'v,v)%j#3BPHQphA>(fT6/$+U^p.]bQ%,H:KV2;XDFPchmIP9`:M#kx#&PxufS7o@AO9T#_P&tYk-$N5l-$fMH%b"
"NF?W*rIQ?MTo1Y73FJO9wUx#-R1>&-:EI&-56@&-iu-;-YM9/('TEU2_Ep0M'h7L#POkm&Se_=/-NF-46Fn-$1Qe`*2',f*a2E%$,KYuu@JOk+(om-$:@a=lR-o2-X,ZpLcEb&-LqI<A"
"AN]V@kT@;@')-v?Tbf2)m2]`$+Q@D$gXc7_Rtrp+;LSw#M:0&+$(q84i(-$M:86o/-[B5&[xDn<Ok:i<IU@?=bjZS7(:>S9Wbbp/5,tF#GlBG;F27/;w6lc<8d;V7e_5U94'9nLQ%0nL"
"fs&nLka]=[;7-f916(/L^p;+3J>1@#uH^*359S>#$*/##rn+m'5lrs'pF5j'k+iE<@08wPwx#wuPL2mLS71mpOuk.#up@>#5E`V$ol+;QRGVr0YFpK%rp0[[3sm][-3Y,/]l.T%1QB%@"
"iNYb[.%[A@/JM>#tO3LPfd;E,OTnb=T,SH,2a_ACd@jV7O.72L?,h3(^jF%#QU<GHVu7v0sCbA#M>t#HCmU?Hbe:$HCLBI$%3c'&dwo'/s@(kL(rJfL1Y:GHfIMiLolbkLGv+v0CQ,W-"
"]fAR*[nd*._WH&#UOUm0*N=GHEb]6E;eVJ#O&KgLEV:GH78J&#L;X>HM`2R3pI>#HI8a'JeD'XLB%%v0@`xA#cs1R3&i,#H+Mu$HN$)-#O.Bm$sDsd$(M7%#-Hg;--:.20v1EG)E5'##"
"Soeq)'JO]u,%dWdObm##35+q]6a;YY7nk:d^KLcM6I:'#*27MK44<`aBO,+%.*2Vd[p`xO&+NiTH=xt(A)FM_4twoJxxjVRdHAF%9%LB#;nk.#nDuwLl3ItL]mv6#dq&M#1g7-#o.e0#"
"dkdm3-+1N$2E5+#nZx5#F_u##x<6_#5;7I-nhM=/a>T2#eBnqL-mx#Mhw><MoVEfqKo&s69Pn%XJv`%O$l]lAa>7R*kq)##m/?`W33$5Ac[PcDKxJ&#7XTJDJh9i#hEsf:&j:PJL>kYH"
"KY`.250PD3#<%cr01c%OIvo4S#`pAG&r2A=RnXh#Re7v#b#7BPVrY+`2Ag1^(iX&#8gbe$0kMw$B9Uf#=LP>u&(suL]EPi;*?r.C`;;JLlHYxX&6pIUZn129O`t:Q03HD*=xT29)#ml&"
"Zs+mSuT.AF'*;MBR)$Sn(SwJ(=YT21^b%p%MA5MK1sPG28@wS%V=fS@vgFm&CL3sHmdH;Zfp(T@P'[s-M*p)3Upr/C?Qs)E&gda3x#u/U7JL6/aeVaj@dCBbDL-tZbBk01>7UHrY8sHM"
"A-LO'Hg#u$=ZEnf/m^*jEt0Cb=kPOKj%UbsZ-I=Z5:=V%S3uUI7ds[Y1Z'ih?``Ck7nh=dE6;]cknpu?-A;]u68x7o3a^ihs8^V[D)ePTg*i8A/b;vd]fIpSek#-373WK)ub^&cPK1q/"
"LK'3h^^6X%x#I3Ux%BR'QMgQpq3V-W[Pl-N9-sk(4.MX@%*b3qmLBRT%i+FkWK7:JpbV_Y%Ps-j4PBr&A:F@QHrbeaUr_kqcYDx-pYA(>EZWXnP<=f*[*Y4:hlcXeGXBxQRF_FbbL[Lr"
"Ds3;&C,l(5Vca(Gp=?fW^jVrSc9<`ch_wLr#`=;/DQCMD93WM2@_<;ACr@GO+09Spc-w(cjX[lqw@A#.X@1/aKlnMD`6_5:6$-B-v#i5:UQ@sSv]4<&K&:gW@+VT'Be&<Aw@x#.v5/N`"
"CWt/W%5YY#C%2I$sq]>>-l68%Y86Q#Af2;]88S4ph6=W$G(w2$)rus-[B1,M7C>,Mp`7K%*5&V6rd=R*kNAX-6IuM(UtH;-5MUlS:*^>>h>VB$B>m`-WOW-?X^<m8QNLg:O)6x#pO:##"
"=()Z#<[ap%,`hV$uW1v#*^W6pX/JO(I')22)o8B3SL;8.<Y7&4uZ[o[[a(ocx@#/$]L9kdE:P>#6n1($0Xm]Mr&DT/gj05S7I39%E5w>#0r$s$.Yqr$e]*87@Hw-3d+V#A6^)22i2BOM"
"l_7iP@.p(*iXn7$Eo6Ku&)R0#sAA#c[id(#cMGf$.rn%#BUVd_nWcIqq/nK(OV,,*1a(#$A_[@#`0(g(O_I<$f=,?,m/iG)15,##^B0Y$(vH;%WlN.)@>D;ZZO$s*YRG87E&WA+un&x$"
"kNI%%oZe=-6JRx*B/VG)`2Lu$f=UW-^a,n&8095&:@.<$_3`?,MHx%#c'm'+TxlR(9)/RA8xmw#a(2v-2U&wP5nEw#AS;<'P&v3'diN.)5`P##M]#K3,&+22Llhu06X-F31Y7B3eu/gL"
"aTS^3qsIO(DE*I-jbn--X6s52PeDL#Jqg.$dxgt@Zr)v#(EL#$59t-$-EsI3ZL3]-Eq,,MQOqkL<&H[-u[Zb8cC.rm?r5cu%^R.>Y;+QqH4TJF@tKF?gn6Tujds(ETYs`ufk=cDCvx-$"
"FBVV6&5ST%liYkE$LYV6&>uu#.Sq:HpU*#cK6`l87O,&+W?g^6r$IJC(ACu9rrdD#30HVHTp1_#s;D@0DgE,Nf_7X'1Oh&O]Tf&&1=hoI`:$Z#g0X.-C25V1j0<L3_L4Z.b%Yv,tF31#"
"?lk.#3lSB-)1-*GN<Pb%ScHb%Q?EA9T6iD9[q5-=?MXT7,;waPEw]Q&A6su,/86##c4l1qAV8,3EKLj:qAT=7671O1)^BW7-EMw.*qF1:BMev$PUj`%KjPn&wABu&n]=,MdSr''2_H0'"
"H-kT%n)@8%O;Na*t7bI3C;=hL&/2;(a[#K3eB%O(B&CN(WE&7pcX@U.x5wGG'3]Y-9I@F.Modp(mr5H33,B.3c[Rq.%oSL2`wZ1#aa7H#NCI8%jm?l1KII`,u&t6/M.RM0YVI&4$2f]4"
"#*ahLs62O%7]TN([BLG)lMnL(j^^`36YTN(<dVMKJ&]JI<Gm6.SdAh;E'-9%2YuY#[d[P;-a3aGZ26tCp:8N)Bi8<7d;S`GHQIZ.>F@69a?lTBwLO`WHLke5Hhvq0m9n6LcIpcV-(#qX"
"tRFv8;rEon*AcY#uVPS/D.KB7e'7u//64I$b1dS;R?XF5H6Pu>a;+_5o_KA4C%W_,(J`0$hT(EuFL8i3&l2M0%a.G=(AP##r`FU$HsY+#=XI%#8Ja)#G:U__:.-ci3XGN'*f0t%/-A1M"
"2&V3_2.Z#MBKihL>q%>-AOM=-,>^;-4=2=-W)Zl0*x8i#+EcY>vCf(NJQHd;8@%@#$&a<#'E#i7iwI0:tJ?C#jS:v#;31,)9W'36/Bt5G&;Oa#bX0@>Y4d2^R&,49cpbW<Pk-S/hPfQ,"
"<nCZ#A_R%#$j>8p7$&.$L/KkLvLSh(<)XGu:B4F%9?%12rGl(5G1EpLRaQL2YJ/B4XaCD3E,m]#]q.[#C]R_#xH:a#qnCkL4qugL'^l]#tB:a#?YrV%iEO/$N$wbV?nVR1CL9F6CqD71"
"%9LecW@w[-^BVj(]XNx-XBr/)C%Mh-IEjJ:o5BRLe*Gn<<a54MahAq;4IsG81`:+*2QLZ74xhF*3u$cix<Ia3&5>##?0ST$mY5+#ZUG+#c'In7+L/4M>6c2=fbIJC$LllM;@]h=.c;B#"
":0:on6vI0:(%AT%:FAn1ci-n7ih70:b@%s$`9S[#F)m:.o1rw6;wXD#j<PO:fRpF#thBJ)$2*m//gH=$ie.i3afC%G[3UAQ6)D#7%tu2vtHl5(C+(N21s+++S028:?LPE*^+VT1oG%X1"
"2@pF#MHbV$.J+@$CP<3;uda6;a0KI2TtT8@.>3#$]u]+#-J+87aWO=7t?Y@57B:I$Yw:H#[V=K9WMnm0P4_t@_k2MMn.=3p?,V#AuZC3p7S]o@W.>n04Xr,#+@ZZ$Gq[P/K;mPhb/Rl1"
"#.@x6g%$Ze-O?Z,'=tD#+ri?#'QCD3-&Ui)uxjo7f^V-*)b<v#2;XQ':03787*%WZxZXK;(U<%,dUk*6r$Tn2Hq5(7o`)o&.mMe?fAPP2g`V8<elnm:v(-N)W;rl2G%VE+dCG%5#.xL1"
"U7xN:_3EH*[5cl2UQ9F87DmV74Vjm9]ixd)S0e(+#4)]5]a/l;%goC-oX'O;EML1=sZ06Cn:we>%V[4Cgf^$?p*vaO&U(J3q%r_&HqX$.]U?(+U77t7fck+3nmZReb(/s1Y%sX@kqV[,"
"w3n0#Z3R(sLRLfhMQ&v#7pw8'jO_Y#,YL;$*5YY#@mwJ3;lq;-cH6a$:ex%:M':'#&6PY>MBTPJ].as-KW]W7@H2/qh1Gv,4u-W$+wV<#JN:0(D-o%#'<-,2s4[k9Pw5K)(FW:.(Z-T%"
"s&sR'cWo30$ho2a6Z9s06Jfb@$IaI?@wrBOWVb4ISZ]ut$$[qmr6DJlG`KkF0h<kFW8PkFpip2)*IfQ&AE)/qAw33(A3c/(q+cnECn8B3.Y3T%pq.[#9tT&$h=S_#I82#$tHU?\?(&/+B"
"ZRqQ0''Pja4KkV0qMFltwE,@B2TeUM%l3nmi'%`%Qd+u#R[5n$R$M$#0SM:@GG>Z#$*M($Cr+j'YeD^F=4.w#@?u2v&kYX&7vGT%C7i;$L+ST%lY(Z#VT53',8s/L?Fws$NWL0(LBP3'"
"NeE5&9i1$#6+cdM=73p%eG(_&v`aP&F-gm&EXwW$x'pE%p5R8%9&tR^29r?%7J###E_cn02P:F*O'BT(u4QLMOk)K:]1;teT*as0D/:e;Fscn0//+wpt]nH3BD.?A;1_o@fJ(Y$M.M?#"
"3ENa4ar)`#ND4K1:ofKI6#&p`9Q5C#rJgI$RcEX:jmeY#eFegs`GgI#o%K9r&4Il]w,lfUx-;J:LTZN']VU271F>YYlGYY#3fuY#@R%[#A1@W$j<*?#1XUVZhWR#MdD8B3sAX'#7Rium"
"HR`9MjG8iL,.oB0m9JC#4t@X-jscNb=no[#iO6V?bI)N#-i7duc^q.R$vu=>Gs0E7(G(v#pYx1^Rkvu#o];W/Ee[%#CW1O'2EXZ#BGlY#0FhB#Y*)0$]IF]FINEb.M/vN#(m+/(8L3,2"
"ZG]LpcV9t@kC4,)3O5[.+XuYu4^,%.O9nqLEwZF#x8*h#qD/r/=ZTw#+2###6#f>/d$FC#RiIfLB]W#.v-MkLLg3]-4eub#q`5^V('Fk48N+##k8f-QXa[s&QF]t(PG:;$pxP3#pFEaM"
"bODpLni#L/W6MW#QuVBM1IM+vsx*a$;'%`%4[f),>1;>5^WboI*DcY#<chV$8r>A4[?xrH,>>##/H)-_RN`#$+/BP8OUU&5=[FpLPH9J3?mSL2pcdC#+cF:]_=iX$u/O&#&5>##0q=^%"
"5`9s?^hHP/s@VoIb-#A-'h9[>4EW9''AMJ:7mFZ#waI)$*s^?-e'6T0JY6##KAR60?T5&#62]b<M3CG)$pLL1fx65&IRE5&@n=D<*Ft^#l(R<.qnn9%-*R&+:]EQ7:Tw*.XFJ]8pNIAG"
"0[Ns-l,74:p$nC#H1i?#BS7C#KJ=l(stC.3['0:%2+'U%XKsI3.pwk;+Ph6DErv3;,VUUC^KOq^SD';:[t6u/WVKV:W33r0iwcs#,*u_sOkTKWjemhefICe7LKwbr8$ndK41dvg[p;EL"
"<=ZZg9H]=#J,Mj9dJg2#A1qb$%;r$#K=,)#W&7L(WrhS%L:H>#tU&E#)nIH#TE;O#1[(?#Dk@[#QTsxFUx5v#NPEI)[[np%JWc/(ZW_`*7te,3`=3u7-OIAGFw%?5TVK12-rF12kQEI3"
"Cm;O'[;W.3#C.J3^<>LUO#anAgR_m:TuTK+i;UaQ>>$n9-2Z,.8.FbtsL<_SC=*_STs$^$N_''#h30aHb@?Z#GN,;H5hS]#T1VO+9$@FNsi`5*,)`cDT<O:(9D(@#j$75/I+Y+-u#OZ#"
"cZsh%HPF(#4Hh7)fVa^.]00b.>emSfhjS&ZtR]h(nNk:%5Qbt_DxO12d+V#Ax7`5/nt+@5N)RrL<G]v&C.Ix64:KvR%n3Q/x^OZe'blB$Fr1=Q).)7Xl:km9SfHu@USH/_)<P4Wjt1iH"
"9xbnWc%,^#sF_jW@r6qM:?LG5Gcg]?B3l0NP>ht@]oEmV8AlWL)vNfLcCx,v79JnLLUD'6X1MpB;LDL5nYHD#&bbF5Z@veNO6,l3TwU=@R9B824J/s$'Z^-NJne>0B(]`Et>E#/Eo:@#"
"@R5>>^HJa%0:hoI[<a,;[;Ha3..Ki#QtFf2PTbSA0bW>-:%LU7J'(,)Eau2vvd-j)[Em.)BlOW6[gYT/wU>K&]@_N&?Zrm$pnAxG>ac01[$G#H/TCwGY=-<Qcd8I$_V6w#WDN&+piI12"
"E:?3p^xO12YZ)22%,-,2meb$'3Nt;-iQkr.:gZ=pNJRm:2l&K%1MngsjPaa4T$D.3FH7l115%&4tB:a#+87<.HXJmCLxte#a21#$+t?vI,F>$^@G<g_d=]-F+,VB9Ha$lC-8fC@4VwEI"
".Fjd&<w;h?9'mO3K;Kr:ssar0.lx`Qkm)KE?c_$5e8'QKqEP/4&.:`4HA;CI#h3q@oA_'50TCe402Zp'of/&?f4n0#RI6##9H:;$()B*#:rC$#W01iD27C&O`&G'&>hNT%QL@t$XW`k#"
"+jVm/ivt:(,>G##R1MW$jo.p&/R]x&@CV?#;cu>#e.i>7&[)22*R&7pVoJg%^>p,3glSL2;rYl$1mCD308.r%G<6>YJ9qf#x;/nukv_?#WIa`u]AB)P&S=%OK^jDu%vr&-lWT993W<YP"
"3Wg_%6k1c%f&CpIi[dv#KXtM*(h0@>E9&W7V<P$.t#Ho)WpF#-2KW8&$j,##4@Znffise)gW::q^b:K&PFto7c[.u.F1aD+/oXB#RB4U.#=d/(bg.jiIKEn8eVMs@b;W6p7n/s@Xgf;-"
"gY,-0S]<.3-rF12l#]3%ZKxW-xk9</sx;M.80fX-C]DD3$YM;$>$,9`&`#lG6Qsd&>6e9:YHa71UD';:iqm,4sn&%$:IQL,[7,nD.k3E#l_NcJt5Y>JGP30;Re,e<0e:2DrXZ$,7d+Gi"
"0+3#/Qhj:U%5YY#REP1p$I9s?*5nx47kd8K.4^.$,+:AO=c'+$*(K.$(ca;IITc#$F7R).kc?29>-Q`#rhMl2L@Pw?pNHX/oM0#,mj%>6p8>CFeB*AF93PS7ougo.Tv#0/U)J^5o`]FQ"
";s%61P(-M&,bW5&P>+G-u5X>-0bW>-GT(v'3(ic)-Q?a36o&H2Ht&9(6^)22x1ZW#eF]=pYiG<->N#<-rbJZ$`7`39GD$_#qV6C##F/[#V$G[$*u8U%cMM'6,s.]J.wR89vWX;:oW_L*"
"d<v2=hvAS10XuxA$Vho^@K[X9=^tL3C0D@8:fk</Hs:*ZF=hDZe&:Q#jFfd,DlJ(-Fv-0N%,(ZA5`7UETEsR1).5$83R^H5m,F/#:FD^4n1m3#YlD[$wxL$#Xs>X$TrJm&SnaQ&OQC0("
"KGu/_/PFW&N5Ot&ZsF>'FnR%#m<9B31S9t@^IRr0f+aw$h9E:.Upd]4IYi>$4;cxLXP@1NRYr:-u(hf_1BBquXos@VT?WD#U^8gM^Vn`%R5FcMZ+lM9VP,20]cl%=C?*TKo)v$0.7(*N"
"VeHg#(uKSIiUQZ#][[L,J5B)O'KQs(*&%)Ek>a_70@pF#9F@JCQqxK4x^Vw,O]1x,Se`hLeW*$#geuD#a@;17t#-s6l_gHF_FhN&ks]O&H,j0(L0=D527R+N7U^$Hn?Nd$VMec;RwIs-"
"2#FaMd(OP(G$gO&<^5B5*s<3pXFUm8IW,'$dS8f3%Ue+4;2d9.]U;v#BYI*<[)j>PrqF]H3F#?^Rb6I<Wuvu@-*i(GTs4JY)tX;7dN_MD@+>u+U.0U9MR%,*kD#c>Jnx9.M[af>G5WB-"
"M.%2=GjQE,/Y8`QvjZJEn1;'Hdq8<$(cn7C%Hw#Bv4IWKTjK->%fW5&v2bf#jjpC6M2SG=eGe:9l4PH=d:9[?a=]3:MROPCb@=0;c4n0#.%v=PSAW87V_l>5PYXAJk[QZ#6euF%$PE;I"
"CBG#$82,N+^Tt)>l:#k2qLMY&@XKL(F$nO(R?*AF<A))/hXL,/3KApf?Wcs.IM:BF*@+FF/,nW,iLMA#n6v?,?p6%HLDp2)]g7c*F6.).[0nc$*X1_&-E8_%pl5W-O/v,OWdH127c$<-"
"++'`&;Hl20LII`,sq-x6.A-4M^)E.3.v;U%$&'T9.aV2=k'c0*j)bD@mJcl25eIw@n5_sT-;$ZIw3*dqmkrKE[<*::r5ZNDUmKh=w@j-]*F;-AJf1P<3Igu8s(k92UsTo8k1$RD1n*au"
"U=Y3MB*<b==Ac&Hh;$T869P>#l7YY#=:3)#-Mc##J2mt$u#b7%^=.<$s+4h:20N;7IfMm/?\?RL2D$nO(+:eM0B&-jo:&U^u9*C-MlK>w.o0[4S`(6:;tun+MG'SX-ID?9/RhEP#%>vxM"
"+B:bt@O^Mu8$###C4C/#sbRD$)IY##QPD>#.#%W$*egT7M?CG)4xHs$$k8A=/YC;$B&2.2c9LG)+JuY#5SG##5fYY#uR:A$n+X87ti2jKs='T7CKV$%8fP>#]2]t@9t]5/Qp<I3GO$iL"
"H*OBkEkdI$9Y4GM<x$]MWL<l0wURx2GPwC#WWhc)&pT:u)`7d#%Ol(WN:^iuqLY9V&Cx:H-X4v>l]'v#w<7G;N[EZ#18>>#6ms,7NcZ>$Jj7L$E+[O$)TSm(0kI##R:Y`<K@mxFPTxnL"
"%QxK#QnNT#N=G_HtsvGG2aYBGx1X0Pta31MIl6g)T4YJM:n(#$L'U]RK`<SRq8kk'Iw8A=?&%hui4rr$>Pu>#-Sc>#*ghV$)2ZX$gWqG)3iU;$VhAN02PlY#:rc>#`Z:j;upfW7=k^*#"
"0S9t@?4q&8&W@&cDPO4(_69r.DX<?#x[<wTPWm[X>0>BTX[6AF>EN#f+_AG2.Z%-#^.j[$Oe0'#QM&`76p(*%;k'pIcIHv#S36g+i<;H*Y*Ah=+r@Z5O?GE&<Q5&#%K@@#>s'C,$TLT7"
"Cmn<&mfINUKVEvHHQc#$jKuhU4j*87&%_t@5Z*@5XT,,2W2Vf<o(#H39L=Z8@*LB#R[n8%+QBg:^C]v5L`0i)grbv6S2R@7RX=m#'4mh<P+'60gows8t3V=%[&*UKcD$%]tKX:QpxvJ9"
"[ko&6--Gf4`7Fx7nUI+4[3^`,47T'78>UpExhX/Op5XldADJa<8XiS$EXe8#QV-K#1YiM#jXpa#,s2A71S[>#^49a#u.km3SN]D*G$J0:rqfa%'vd`EojSe#?h1pCT%9H#Gb@JC[F8kM"
".fYE73)?87=JOK24G*v#gsuZ07svY#&]QW0Dk,Z%TUi:Ju+o%$*q',;?tSJ;j2Sb>W`%80;BemA=-;XgtU6N)vN:_4W2o:7+%(;&CJW^4EG[5_$HRE'<O8pgHrN%blB]=-dAebF#B4X$"
">/En-^5h7/%:>m'FS7.2G],:[Dmof-^jcV-k-0I*wJV3;:b6g)IGVB67u2?7;xg,#IgE7_s4+87jT%-1`j)22`S9t@x),u@.s<3p%TGs-[$[HNpYU#An'dhLkV*22g]]o@8lSL2a17ZA"
"c;'L5#Mq0#'W$QMM7%b%oepr6lRHp8V.E@$S,m]#-,0J3Ls$d)^^D.32P-<-:m/_%mSSIM%YdY#Rg,A#48sq/pM-a3G.,Q'WbrE%%0bx6N0^C0J><b,@'^@#h/:h1g7V<72uhF*DGrUB"
"DH@p0n(c;-h]]j'fCKJ)*;$++^6I&BoW]B8,5mwCpMB/=skQ6B/,@qKvE(+>ten;MPte`u,U-v.0Qpa+.vq11AR*b5jOnS%$'`+r[j()Bl5I51F<K6&v-iH4%WehO.bEpN9;eb+Mg$TS"
"Me=*=H1lb3HEVfdP%pXXF%`ru1R$Qt'a+P0P8+#/-(BAG''+Q^Z1oH#NaGGdQEM5EmKq6C,X9A68Q0b4,8E5)jC8(#%/5##43s@$:*B*#>F.%#+v-3).Sl>#3A@w#oRE7%*dK@#'w8A="
"9Gw4AKBB:%JHP/(9:w8%Lt)vP4+[8%DqNQ)[Fd;'(^b?#>oLv#9:r$#]Y#K3Jr.H5Pn8B32L6MjMw,@53Z.x0$Pata'vYXN(V.%'-5SL2ldK#$.V=u%'weP/>r_v#2L)BO=e41cSITU#"
"lFvK'lH+=$hkG(+CB2Pf$5W+M$u?F`UQ#-<b#.n&ir/x.)Nxu#qb(##CE,kbBVC%#s8q'#:T[RC5Ag_.VweA#..Ki#)o=HM&@3w#P6N]F>8cn(O_uf(v9tM*xN+1:W7Z##KOtA#_S((,"
"Wjtr$51x-%90@s$.,*-,8H3SAa)]TAvN_^;@Y+5pM%Y&;f0l5ptpcn0dOcu@->IQ/Hq.[#Dn@X-7jLQ/T*gm0END.3-_)gLF8xiL9a@D$Cv.V`_t^B#ocMBL/O'6CCW)I?,lZjS#Gs=="
"PLC)=1wFn<uN@K:0ugC#HMGG@i*L</(G*,5#QgS'--@GGjXIC@0]OA55#AO7#hg[uHcu:AXIVS%*&&>G530sHVP,205,8Q1vnHM9pqZiCq;i-,_6[>#]f$%-kJ(&GY]u0--^O>-B5i2:"
"#$UQ&?'J?#QqN%biAim&UXfj0vL<1#[O$Q237fb<P?_c)`,rc)IeKJ+ofHK+,E(>$h]V8&s4H?&(ikG23<Imfr(H<$;5p6(>8iV$Pr%%#6D;N3av2D7.s<3pc3c'&Podp(9M(kLIl*F3"
"qQ;O+hBlgEqYl]#tB:a#eS><--$Or$ICa:%D3Tv-o;VY5*#Ab@Q>vb5F=4D=']qHPO3EZ7cY$YC<'7.+XvjO:Dp$sKGF->YGcw=:>`.L4D%`qD-`xQ#lH+P#gJ>g4(miv7nC^T0bm;8#"
"v_B^#j#t(<PEi/:FXI%#*S_^HY(qY#H&k=(-1>1LhrJF*/?$O*Da8wP_S[l'-_4u'gaN?#H^ru5,T_r?306C'X%B'+Bjtr$Ax-s$J2cv#J:S;%PM)H;DfD'Q.;dX$Ix@a3*6qB#V8+l("
"v@'uSq#IuPCoB<rnQC7515/&Jl=`p#&wL:vvkS29f[UUZtF1K5teD5&S(4GM%W)b$tf1$#U3R>%uE+1:;=35&3135&EH53')>S8:`*/X%+TX87<$3vPte<rLF*Th$xMDo$+b%t%5d+v#"
"#B*581_[AG9;rAde/I12YZ)228QEI3BH`hL1>YD3_XgMu=s>Nu;a_V$_&n7@71<4UjqdDbecnV6cmWt%-i4Q15@3T%<x_V$]2f;6rr)E#4:TM#+)P]Fs$.v#M+@40sY_V$72cv#Feh)."
":JKhL;8X/Ono/Z$`+X;.0X?C#IN2Oui%XPJ7mq+)&jf(Md-+k%$Vg<C?1E'#>BK[%Y_h[,OO(g,vBQ>>/sUT/mqCv)c@XI)Rajia@4]^#'B))/Ka9B#$*jE4ZHn+NnrH>$+^@:&eVB.="
"'@CW-OH(q/aTu2vp[[3'I>oRAli%l'VCca$dT<8p*k.4(Xv]?TRCi;-5V*30)$Q;p<9I12rV5b[*gZw'1&kQ1%VJ,&9h0T%$93F3x1f]4VF3]-M@1TAKU,1N@bZ#7]DU=C8h_h*Ev7lF"
"oeY@.R=[b=NUEZufs4%Nh:D_,IfM>#g,M>#f>v[?3**GF1,p-dC4/tuOqG(+tZROVIUb,2B(B*#qs^]7xV0^%r>$(#Oc.T%l%l.#2.92#<=G##::`v#HBg>#;_w<$'&(N$7ANPAr;.GV"
"9tIZ#*4v29(mqA5QDNoMPVUh$Jes&3Qn@X-Fl*F3(0(<-+1IDEBNDXaF)%o#'e1g$3K<p#f*(7#$),##GI:;$_+.u1Pnqx$<Ie<$18###-4FrLwL/s?.bUv%ePk*&4N;QA8d,C&kW0^k"
"K<Tq%B6gm&Ff/QAJ$kP&06>##ZaokLk=Sh(]#%uLSEU.A>*CD3su:e;E>[h22nb,OV[-lLdMP5$uv?S#'9nS#(EWl$J'@Frn4Y2'xm>kF:<Q1peZ;kOl_r+;q.(XHw0aw#%0:onNsiK("
"DnTU%J3:L(FdH9M8uSS%E:eS%W+.x#I]Hp%DO%%#*k.4(?.YW#x,iJj#g%38[Uwm0NtUO';]d8/oRAT%`RG)4UsO[3-D'*Z.R0I&OSo90i#8rdk.N`W2M,)7KhgV7j@axPV<`/U3i$<-"
"cn;d%dE0PSl,UA&OsPM9ISXgCFrpB%*@>F%`+C>#``-0#s$FV?i0:;&COD?#a`VK(,VWP&*Lc'&<Bu2v'Z?U&6)dp%Euu##nvNh#u_aP&E:`v#<xO/(/S(Z#VsbrHeIHZ#uLTp7^&.B5"
"+%nO(vlw2.,g;hLA:9J3F&9J3KQ%@5d75h2ew:H#;ou,#F7BZ-`0w9.OVDW-bQ<?#*t1A+HN+o#Hgco%@tFMKQU;[$spnd#/SC7uo7x^f5E+##qjq^fu4-5/,M`;$)9o/2eH[Y#UvFs-"
"oO#m8$&jEI#g_v7ZMlw0Xfj3p5t[h$XU.8U6?TMLTVA/;H[M41wMh6#k6:m7G<f0:g(@`,?v%iLY$gQ0?q'm&ci-n7K6f0:s89I$Qg>W.GO6W0C#X%,Do[f1l4bc3KRi$#Zb6V@l*Z$,"
"O&v2vBv^N1vU3N0M4.],MZ@d)QA=jLZ>]s&YN1[./NbB,o.waP`8*22544E,`.5s0wA:B3tZ)22+Cbd()X`I30*RL24]Zs0k#fF4#Ihx$@=@8%%?#g1E4NT/p&PA#(o0N(3qOA#H.^C4"
"&,]]4(?gw#'FlY#T;;O'+X37OBi:k'aSw-)YIVS%R'Ad_Zm#n&UNgQ&59MQl(D;.E@,gb4f_h?/$%2M^gvZh24;bM2wGLO3]6V29HY$f;+$7120L=.+]+QWAJg2,WQbiM0+J*,)UUAZ."
"Th^`*0_^*#VVgY(L:?c(#Q0@#x,ST%?b9V(TP^Y(^%Oe)$0:onW2WK(.W*T@GB,##9+oGV=64<$e.L>#_8UcD7@.@#iPL_&P%Q?#KYWP&=Rr$#&;-u@i7je$)6]t@A'HX$$mxGG6x26p"
"Q/mA#wUKB#Jc7C#5SB9r;ntDu'fCv#_sDr#@o_;$CW<)EPaIZ?6sLg$q2$&0dabN3#=xNbe0[Y#w[N1pBN:s?Q]8>,3s<a>ZURL,xxGA#R%;4+t`#A#TuuR*#jg@#LD^:)uPB@#%v,29"
"WU:)Hb.&8@RXlf(6Nu1:1*g,2sHU:@9.5/(Mvdd)[^&^+bhDm8xa^j(jhZs0`5R#'W#1s@@#<U%$rF/(ml:^#483F3'cufC&r_hI$[/b7&:w*KCttVd>wI?U>[ZeR&0Plu2SQ>A/-g]X"
"+H(;ZsV`ku30i?#xP]tug=#s-VQ$iL,,W,W1AaPA&sLM'BP%8@ul7O*wCf(N(jSq'YRxt'Uj+sK(@<[#H>ne/Q^Ck'_cIs?7$_g12STU7Di(O)[[>RA(.Dj9'5F<S,,9t@fQ6a$9;#F>"
"-X1N(A,m]#x--]%gtbPA`4_dJvjgR#OdH*KlgnqW:[HxIM?fo[)Xj;:k_Y29g9rDAdVXLpTkq-#Tk0H;`:aH$IRV,#p,_'#1kC04wA0hMZ+Fr/o)v$0V]w8JRp($$rHn).<SicDH$R'O"
"0e1C$u`PM9eIw]#M'-,,C[wP)?&%8@qhnA#0)`66%uZX-XRAU/>a$A#gV*&+tqN>$q>PQ)S/QZAk@9'#2cFQ7p2Kv705RB5`hZs0pY7B3=?mw$%Y-1#A4aE$CJUv-S`jj1i$d&%(7:a'"
"&_HHP=ZOXChrU7/1H6SLV:X%Gbncu7foG=R?1a4*w/MW$H[ZE3p5lY&)KDIUf/c=-6<V2%n,ug$$5i$#:<,Z_.EdIq3aRc;x<a=lQ,&L(OLvL#0M>>#crrK(7qq0(J+D;$Tmh0(.1u)N"
"@Dns?YqN%brtqb*7tSwPIwbf(fw1@,0/wi$(XL&4_)%*-&:&]#Svm4*4&PB*S?A%#Bv`m_Pw&*)Z6wGGnrc;-^7`T.L0v2AO&DI2E&p'5<vU#AZcV'#>u:8.*0'W$vg_I);](?#/l`B#"
"q:-x6:E1nuf(hR#TR+KI],HoL_OIos8iaIqV_pFGtr=87-QW29DGN.?;`:R;4C=2$O@Ui0>cX,#$),##d0ST$@RV,#S73g7e:lD#`X&,;fLe]#UjalA8i/)7]Z6;H4&;^69Ov(N65DB6"
",gmrHplR`#kmP_/jeN>HB?P#$cxg%OWE8E%^1U>#uVMM0%BD'#<J@@#Td&s7d$/b%p;eS%&Lru,tO,v,I$P,2Q`oQ0d4gQAmCMa<UaTHOdH&9(oCW/1S0GD#ucS)i<OZh(]S,<-FA0S'"
"Y@v&d*oCp.k@C8.U^ogWO2&n$aeVEm[/E.3$.4@7ueXI)4$*QJ^LUGD+NU=N-VBo.ncq'>^b[?ITQd/E0fokC2hC#.P7<ACFxhx8*#vj0dkhg<$%%)4u#ud4AuT>8h,:J>d=q4_3eTx?"
"3@D]5[ORB?Gjv3'=*U1;a_u/Ot$QE=AUiG3<EPY-j=v?6'r1j(_,]B59q&##qB,+#3YDp.`$=[7cZ;gtbFjr?vbAa#VQVk#btap#/$QPA52D['_-F]k=R@[#0(eS%:AaPA>*lf(##[$."
"v=d+Bs)ah)S@AnuO8j@$$M[D$4KL<MTDn^PQ/wV61+#&+[(Rs8(93p)&^PJC<=(eM:Z_r?cI`S*X-8]*pnX87)Et$M9IbiK8q/]#$,&5'bYj,V?6+w#Y+.8@#4Dd*3_r39=JPH3d+V#A"
"#b3@5>KIAGIH+W-OT$_[:i*F3AY7.%K5+P('W#a5?A*HH?I/Ja3)ck<GjQE,W.[>O$vr+44NH.$GJMDBV4(q/x`#+$GjfE[QA###rWdE[>8c$#Jonr'U?f0:N0no'a^s?#&r8.$6g7/C"
"b4a%$=)_S.(/>>#Gecj'P_CG)D]>>#-)6p%2TS>#=J(v#@0>/(Tmh0(FhmuP8hxi'1)tx%s/=?#tLGM9t2N;7W3bO(J0^p(.&R%bcQYHN^&/o*,aLh$'W$QM?l-[-&ddC#d]VT%*m)`#"
"ecFS%>;s5(hgY3'hOlZ,<b`]d:4jOJbMjK2.@570>+))NYp*Y844]C#7wAX-/URc;&nL-MgJ$n&8?F0_Dn)?M.'K1_ed%6(;[7G;((j$#=?A.MC?VhLY;ml05=a=lDBvj'TY=d*8T03("
"&RPF#TJQt1=03/MOrJfL)DBE'WB4OMcv06&YcEhL&N9Q(J5:j(^k.Y-Kq^q)Ss=_/qMj-MpUOs%c]lY#2MG>#psKm0]?F8%$2;?#e+:8.THU%.H1+=(nxEK(ZdCK(R%Cl2f^x23FQ;.3"
"3E9A=o<-7A5tq-M21bn0]/D,)C/))3P/rq%;E;%.u%2s-NE<1)lAg%#d5,A#Rx(v#1>+8713BT(M),u@D,LB#q3H12@13r7_p4ZI426T%ZZ5ZA:&RBPPaQL2L=tY-v=.Q/ARIw#Sp^#$"
"txls-H)NhLpx[D*lr,x6f6fT%nk#1gXe:k'[87L(]^Pj'Z*@)EHG$:F@:.W$NHt9%T0Ft$_RW(%&io>GKh%@#Ek3t$#]u+M:_E>7w$_k;WfQ&6;pvLK(=3(aKqIC5Q9GE,@J6J*$$4YG"
"?Xe]6dP2t8=Ke%$h,>>#G:$##qZV29LUf:d'mmp%6ru>#6aI)$PXH>#,G(v#-A>>#EksP&;OWp%;O[W$s=Wt(AM-p%-cCZ#*j^S%<>*5ABne[#;[a5&0,L<$@o/6APZ>7&Mg10(Y'aZ#"
";RE5&51no%A),/_<qRg*@NvB$'i9T7Dp7[%517s$4%.<$Cc(v#*OR=7d=Y@5:N%O(3M(kLn@:D55[-A8<5pe$V&/o*F7s_&.NataABx-$l`&i%7.Te$_7JC#;1_o@='>?#.,:8.,TZp."
"kD4I#5Ec(<#6vou[T=(fMrc(<:0pa#GR$KuJ53DuWu6o#EA1?%nE*)Yv;TS%cm'NDR[1$%23[S@ntO>%+A*p%taj200NI`$LE7m8-;>##^=c;-<LnW$ACi?#<U*X$[YC;$8F#**mL@[#"
"2:E5&v4bS7)$ZlAfRa)#8%Pc-0s+n(?p3l(F,V#AiT0(6N97`ciOxd+^,wh2ona#AdS$1#Q^7H#A?V-Mig+`#kW,>,jNqJ#Z'9/:$?8/:jM1fUa`$L>;U%##FN,kbgEw##3Me)OH=d_%"
"P9k8I_irW%a<@[%Fj6H)6c1?#(T1v#Ot6H)<7R8%KqN%bBV+1:@]Ss6B0HN'wYc##1lCv#HF587x2+v#9VaPA5+eS%9Ug*<0dtA#Dw3]nQee+%jV/'c=i9BH<7aB#ZxF)4&>YV-nCwtc"
"n+A`*`P[OfqvSJ$bvskt*=>6sPmGju@$ek$LOc##[JZ]#?FRW$%_ZuPeOa)#ms[fLpk;;$uUUV$ba?iLRf@sA&$^G3=h3N(RmwiLG+7MT.TArQ?-p:?%C;4t(bNfLJF-##21ST$/Q^S1"
"P7.Z7x%C>#-.;>5bV(O=#S[w#^gjh#nmdfCKY5v#4*u2vxWD>#'25##&S+87,jem-MAvE?<BSm27[BD3tFm3u31<&4nK7gL-4NZu3T>LMs?hW7?QH0B*DlY#0/###]Ax^.x%Y##1El,/"
"&[)22MBF']UX_E[vRkA#,7aM-h@1-/lGxPA1eF']</FcM)<jM9t)tr$K$m22(+_oIb@-Z#e'dxO%ajjL2Ds/#*D(v#?4rr$Zem(%_/@s$ugoQadEK3b5eF3bA?X$#wj`*.]vV6NG,>>#"
"pifn.@Hw-3W,`AMe*bl(,8Fn-CG1I$]*$u%L*rWhgJP._wKWN#<GblJZNR8.7ib]OnuMfLLOHuuG&6?$Q2ZtL?)rm]7jX+`AIU6p_2[h2[M2]-n/]fhD,(6Mq`^rQlXoY57q[V$6s<T'"
"xe&.$Urg>#c*vxOKo/870=0ue?#ht@]JsI3f%i(aKQM4m2WmV,:3_-6i67##?1ST$S]>+#AVs)#(CdGO6lTg,7A`cDK.8,OjI1E4N)<,)Kh70:nh^F*#H>K1v,tv#BlVL1>,i^oZ1[G2"
"53LE<JQZ`#ATXR8qfm2_4-;Q13WRP/&9V*#2SF@53;0G3NNt#-KG.:/TG@P/>PUZ-8,xE3,7k%=O#%h(VmwJ3..FH(rGT^32+OV(m.G,M&k9P:u/,H3PrF12Mv0W-.Mq0#=TSF4)li?#"
"C]R_#)RJw#oNv)4,%?8713W:'1N.f).30r.(i^V6<RIw#g^X=$V/]P'7K@H3aZPx#7.?B?ueqWV@w64>bf[(>oZ0c5kS8I&mS:TE3t7Bus2o2=[D@cF((rfFMnM`%$H`+=s&h(6G'4TE"
"q)[A#Yg/<AY0'R/@h7U7oY80`RdOe<pl?sA*WgD$jS/=@&x_-#._F2v[BY^$f;F&#NZd^7m+:AOI;OK3B<uN3Eff;@-Ub#$Cn#&4e]Fv,A[C,-wDH##BlnA#rQLo0.[K*#w*a[#rdkI+"
"eq%%#qWYe<B[@%#n8EQ7di:p.IBO;pG`%TgR%]o@'%_B#%0Tv-Rw-K)5]/^$P$nO(J@[s$G<uD#Q?n3B-MW@$dm7OHW)nY#.XUrZ)>P>#[8n0#TxdG2vbA7cEW%WA[TWU:&a;kDDi]^7"
"hEX]uDF)s>Z_Ql1'@]j0(cJ?$1ElG,N(-G*,9*lolxCoVPl[0D`T.sU>JIkCs*,##,6Do%IQ>)<;u)JU0>a3*Df(@#ct3i,OL.A#;r[l)2Nm9@_3&a%iVaP&E&#$,jO+G<3kY.VdYQY$"
"_WFu+LCtlAvhL`+O_T*#%e:N0Z&-##uZMoAJIc1Lv@nu$cqN%bd*J],eFQc*d5jO'MQiS&H^e`&I_;mLwD_Q&x7+87dS9t@7ke;-/FKr%WF]=p95;6/Go$6)8<0,;W>BYA&e`[,aPYD4"
"dt4T%FE<h>'NJWB+;[9'@0<*IUJN'-ThWJ>CDJ*a,6TrUHxCLIsK2GW9WZ.3CLeT'3dDiD'MF6VN-+1FM,7BAnJO)fZ_#AO'-M&+jNvu,m//DEk).$.voO'.TkC=@KZ?%$uYXI#f^E?#"
"KKK:%,FGF%ODaA+07LAO/q@O9ZS:o(h?#IV[8[h(k0U*#J5sA+$Xx+2A[u/(0k)588ufG35':,&s$dI3BuQW-VukZgvvq0(Ne_F*:'[d3nL+F3g<Z1/9in$$rv(bn9FKT%:9<5A;aD)e"
":N+(JP#R>#1[iY#)=]P#*)p=ukQ.sAadaU:vYM0EHcAB7f[S[ASDe`*jCIVB]g_O1x1Hnt7kjB6*GqS.U#-A#_>;XDAtg-YdEF'B0.oNW%5YY#j`.T%b[>+#o&U'#i%_9CZGX*$ErfZ?"
"7tB$$dDx].=<RfM+JX/,AnLSIu$3[#BWn@'kI%#>9#5v#%#6#-Q;=K&eYu:?jO4K1EYN5&+;)'#a7]$lOR7%#pS[Z$jBq1(iVl^k)DhZ#dM<5&S-A%#xx^t@D@fD?n#cc%:2aa#aYU#A"
"?Yxo7lf(jrmn`:88GrJUe`tr-[(:W-B2058QP^r:4kn03tN2U7Wtj*H@VDK<=p*6M(VL_u2p5e>Vo1/:e]LUBVre(Q_Ja11471O1?R#4:])o=U]Mi(>(.En*_?fi:7M8WK:/$##Y[#AO"
"s6@,;jNvu,;Ff6#_g-n7bh70:44,b#nf_V$NPfY,./[_#+/Lf#6Fpi#)a4N)=e[Y#9LI[#7qmG*?+?e6lFaY$HO7%#nv&7pEY5o$<?%12pqAT(u:+F3YKsjt7?RL2aDQJ(XRG)4oFFn-"
"OW?Jt1D?D,]mo[#llp[tgx^3CMUZc<7r#i(lqe3)'<d&-ke9;Z$J4;Zk4D*Q]@8s$o`G/$EN_G)c_6=&Pqnh%S05##d[eD$;I#+#DoA*#dVQ;JfMi_#eX.29KSOKCcRMb5_fse5(>mA#"
"xTM@,XOIL,cbcC+VTNCO_o?R9+A$V'd'THVOQuN'&BxM9))jIq>SH%bZ`=e)]p[;6631A#(DMW+w?(n'APQ#-)R@r'QmKa<$ZuG27.Rs$(Y+87<:F(Mo4K;-dMIl$?+D-4*/Vm&tvwr-"
"4H'3C5&wY6E?Q-&fwC.3cs%f3#C.J3ZD[L()O3R2d9ng#'D6xB7;(_(0u.>@GE;,)iAb::L2Us0`f?QFmGj$$/Sl>#)FlY#JsqJ2PH(V8WTg],;bw,4`79_uKekTBkfk-=`99g>?vWb5"
"/7c*e3&gSPQ7B0+o+8$$2N_P:G#X4mMd-TG)F*O1avGcHq[qm0vVC>#2R)##S*5T%Np;W/0'.['J2eH)6%PHM6e.O9nM-Z$4$L*#9tx]#2POe)FA)Q&3####nv&7p_mUh$n*-@5sQ8W-"
"ppeTirG5x6TOr_,oZw9.rs,<%@jx9.F2]v#&)xG=Ms?oL.wlj-pLv$6ds-q]o-+PJnis02CV/21t)OHu]M&t<cYsL1ExBFuI`a7%Q@$##_RXSe9_/2')>Ws/:%i;$,heg:=VG>#e>_h$"
"<$#N'xk=:&XiRf$Ud*]%1es`<7c@w[,D^t@VgMh$3);B3L6W6poqLkL[xQL2J0fX-c0;hLhG**4WUOons[?i06=a@bTu5Pd'F1/$d7_vacP*b.GL@)$rH0b%5v8>,0lo51D,9:)L>WA+"
"X=5c*QGCi:NI?M9N&ic)*>N8&DSs.LtbC`+]-Cl'(&1g:=IE/2J%gA#(V-G#[XMa%e@1%IXg)*4T$D.3v.`IN>hd$$mEB;UA>#P;$fL=6qS0'5JDE^uMUfBdcbg[Sk9Um&xcPf$]i]#,"
"[G/'$:9j..RTq28Ok*H;t0*g73XFsHfRdv#dL8&&/UQPJ^4hY#sHiG#/#VJ:9sOZ#``-0#PvUJ:)B]Y#$0:onR#WK()VWT%4?.w%6G*p%,FKb&S*pe&4(=j$%O^Q&+oYM9,YC;$PW]e]"
"4EgNM'Uv_+(*MY7E<f0::Xs5&41[S%;V:;$xF+87cbrD5bPnI%*7O;pt3H12I;6N3q2h,5O8bi29vU#A;IT#AY>mV?rNCW-w?fVMj^$Dj@<5F#IY2hBdtXSe;xD2'<V1lS$,>>#jGUEn"
"*8XSe`a9D38@FT%5u$W$E`?T.:B9a<d^*s7et^N(I')229sF12:MHX-<EkMr2<.V#O0MiP>(^uL.bDE-Pr>Z%O,@)<@)=YP]%h#vkUij'D3v2vAQD#&)#kA5#D/2'3'E@2Q`5G2&Ve'G"
"M8</2E,b22Y3@UA;x[$Q@9_-2''.12k7l$6;O^*#O_%[#10pi')B'[#BYtI'CU`?#gf1a<4A7$$uMik'u^_c)r(=.3'PRCFVODs@/B/>BJ@c'&l])*)^KLW5Z9bi0W;AK<;qjj0I82#$"
"WNU&$ae_F*rKU*3Q0J[#3pUl0jn]:/9:iY#?J];Mj>C2'DY0n1=Z_[#6-u4:nWSi36/l'/3)Wj;8x<1EfG,u@BlB(W<<T/;MbAW$#vBM)<X]%-aW?O:ROm2V2;eW%P[]Ou]->>#,@)##"
"=H=)<0fJV6rnh(.wfM;$C@O>HCHl>$9L%##a0WN9ioh?'QmTu$jHZ&NZBj>7?)i]$Bi[2TC]l]#[F5/Cx-Z,MmfKP#-S1v#<Rp1E;6H-?&'eU&_`k21Eakn8JUF&##r^^#=[O1pIqG-Z"
"(HX$#5*q]7=(xA#Yp%&.QX@A#M'-,,TUr@#K_0j*N1;@#?l7p(rSC?[Db+/(_'A9(lWNq.9T55&>>db<Z7jc<+'anU9$bs>5g`w]+4_[%]gk-;iPl;H9L3?^2ZgDKC8]`rHs5h;(Q&HF"
"t<6I<wDAdFlrgY,W`%A9W'26QUP*&+H]Q7n%puZ/%0ebJ'&;w/S5d4#.'*P]kDN58J)gl/pj6JCGcN*6Qec`>L4.u0G(#[?'Pd5/QaaA/^0%a33(lY%*%(&OZ5P56,o3201x%`,7Def("
"u_uf(9f`V.l`%L(&`c=&Yc[9p9v-I%Jq[P/WU?29it9WJnMl]#TMNk0+k[>S.0-e>.;m>#]$7W?3FlY#-U.v#F&E&+ULK'7W)Yv/=?78B^W]n;(ZM0ENf<*61OlSJIKNP)u6#`#shID,"
".mOv#P8aS037Pd3O[gL;O(G@Rq(K9UMpp5BiTDVUV32GFH5n0#3B8##^U[D$6Cp*#*vv(#]8O3;DduHEt,oj2>$>n2(/?A#b`_V$K_0ZHJ^1?$2Bx=-i<tI-ON:w.7;C@#%%/=)01P[5"
"g-nr-vA)f<.S+i(i]/`$s7+87:iO'vCR,@53`u#n0W()3Xjtve#KEs@0=g;.vhXDQS+i4&&=L)34*TmCu)6]#/Sl>#-FlY#^9YfLi[s`#2]nRC]:QqbT=nj:tE59B+C.>6mZLMDBb&g:"
"1MB32u3F=.FAup8Pm%]40K@]5UI9G3mj,A#*=_hLDnSF,n.2sUF1thD=:(7VN2dmB%+,##pI:;$5[>+#tbnZ7U+(aNI,aA+WLM;$l^=]#CeQ,*7o30(wSK^FFBavPQ2***OT-2(aD[d)"
"9R'?RT+#rJi,<3pbZiN8sSbA#RwAj9?i1T/gL$m8+$]v5j>.d)LtH)>Aj[q]WKkt$Hi(O'PDJ#,(rRM=J-L$$g;mm87_IcuuB%75j#Ii*V/ms8)_$X9Zlc###p6bX8M#T.UUox4AStA?"
"IZWv8BDwhE19PL4LG'1M$<4k2'``S.M*bB,K>WA+Mib;--Owl/f/2'#`O.x,gWw<&Od_*#2K(&G;0Y>$6^lX$8Ym>-[/v^#POc2(>A)Q&Ib@%#nr$T.Ht&9(PT]+%A9W6p3'@C;DV)s@"
"r)B<-3xe+%7paS_x,?i;UuT#$@m^VKlS#/$L>%)G&E6ZS?nZ-VhS:UBB_Q9:QmxxI7^&wBN,bY<rL<U'>X:U8rOn-1a>`v7i;0F#u?0K>h9K2D]oVu75to30Y`h[6:5JL(>Q8n;eX8;7"
"cYNL1v%IQSwS^a+`uhQ9<->>#(5jIq$*G#>A;);?ruwk*X1Do*pX/t%0WTs$j5_Y$sTa=QYIVS%T_2V7h0xH+p$J0:XcEA+WgM+*'elF#/Sl>#OmwJ3.s<3p.,YW#[p?n0X`k;6&d<I3"
"_./(#I9v'4X*nO(6otM(^^D.3Z3Si)OmAm%mp*P(?nOYI$^^%=W;FB>;LV+*^%/L4n47;Zwie[ua8'VH%9v5&oMP+%*bd&OY?v3#@WJ^4#2ST$+83)##]eZ7+);#Q9:Zd-1DF5&T`rP0"
"G,$),<<jN9b8LB#d-[2L1:i)+YAA,*XU;$#HGG,3_8p;-O%$h-U?ZvISo8H##$l(&:X*<-Ckgw1DX)P(3,E(4`^_2D:WZa9<&:G+ej[X9&_IY/]Y1]=96Ud+uH43=C7,rZ/-tM9K1wYQ"
"4d)BM@2C*#1KmtLhKk2(:es;8suCJ:EA1[#cRMs'>Lr?#NH%)$Dd?)*Q#UB#=XUV?GF?/;]XeW-(ATY]9Y);(qF>/(2G#i7LwI0:W'S[#p$J0:E9#j'i?Bm/DC(v%/G>>#OR0g%q6-W?"
",,*s@-VGv5)OT&$;m)`#iqC`<XdjT%aQbt$f=ke)`d6XdTFXX7QaB?7$4f;8=M*Ab99_c)R922__Ao)36X0]7J6@JMR4&b,i%`S.@2m]#/=a=l.O`b*p9KA#8DcJ,B)iS.>,d]#$[V@,"
"9&oiL1:r3,'i8SJ?6##$P/c',5CkP&,Vbj+<wj]+q<7B2pliK(iBpN0'BKw#Jt82'4?\?#-B:Y?Qd%+6'=-Y>'fmc>#nMaB#4`WY5M`,v#?Nx>-T;BY%x]ho.[3wu#->>>#?Pl7(NJgq'"
"dADk'<69A=ii6J;RWB:%/h8A=x@@J;F3B:%HOd##nNS<.mXN'#/428.RT,j'U/gY0T1bM0^)$O0oX')34rH8%EpG40pB,j'64]XH,L&]#GP,##0G+87kv2;6YpP=7d--@5BDi#A5b,,2"
"Ix^t@c0/JNqV/'cH[^g%?.Y:@K'&TA;GxMN&Y5<-AYcdMAS_;.'kRP/(Ro8%Q5>Gcjl%d)%IuD#Yf$oB*&,d3.LHl$X3pc%T''q%5d9dML@PGeC6u)$L-BQ&RKB:%J^m*%T0B6&FU7w#"
"h,c+itX7s.[/c#-akJf1V$B(WMRW^@e4R<.*V^xX*(ik9goL_,L,3/+/B&##-#w4A?k2&Fm5PP&8uW)G;bbrHeIHZ#-`6K#'ftrHMZ5V%;ME5&8.`v#BX*t$3.7<$3YP##iMc'&gx$<$"
"2i:?#-auY#-Qnw%K<kp%#e8A==#x/;AtaX$&?]'/Uu_;$g8rg%*6p#GhX2w#KkwW$<[s5&41wS%:c>##Td[/3;LZ@5a8I12Ix^t@pc*Q)Q;:..v,Ia-YJVs%3Ach#LnX`WIhI(a07vju"
"6h5/1mXJou=diO#%'#b#_xLZu#Q;OulXv&M+q'au5X)uLoaZ--fF'H=%j@h(X9:XH%=sw#UEdG,UcR5'W'g$$;iri0eHnY#l.UK:tF31#]0QfCJ9F:(H&k=(.0-X?Ee+/(683rI5nSx#"
"aKgm&UpmV%$$+#-a/v2vb$SK(vgkY-TZu2vphNK(q*S0:%Fs@#*=a=l8*M0(N^i0(nsGn&IQ]n$@j*22559H#>p8J3EVUe$sZ)22Lr9t@M:-u@Svfg21q7t@/DTfLR#r`-$3Tv-o=7`,"
"k(TF4BOBD3]`[D*Y$s1)?ZD./G7a5<^?M/)?sdeO9(Ni0MgAQ&%q+]#%6+#-MdXbBpQWA$c=?v#PKR@$?c0k+])Ov%[k5L<o3e0#GgPm8cD^2#W]v?$gYu##Nhhx$n<n5/6v'<$*'Dc#"
"x#r$%Knn8%<Y0'%5C3T%7c:v#)9w7[=0/v#&5G>#+3iC$5$9a<OQYw$Tj,,2m7Zfk<V[#8AfJGN,b/W-j2alipE)h)am`OuE>gEu*8f;Ob_P4O1Ur1#^+DulM>9p.&G`c2@TWm(>W*x'"
"w[03%qaGr%DKk0_&?Jd4@Ld&4C,VC#.c[@-liGp.u2X?#w*>kK.SUV$YuIT&.gKE<X+;w#%+N)'nAUH2JC[%#eQ`]+a,xF>pgvGGn1`9V3EI12j='5:_h=u@c./(#4vU#Aw[R_#w06l1"
"k_Hx67BPd2c2v[-6.,Q'`(4I)OgHd)$0Gb%124x5Xr8#,Jq)r072mN37$A42J<;%800*b1v2-40cE-n00Xhd<Ucs:?SpRx8p&N&?TUq%Oxr<`>_8SW9COkaO<U.I4__LY@KLaZ#Q9sv@"
"mqN:E_&C>#9ml:?bf9-vgQ?D*r_luYT(ofL#T+L71x7@#UXs&3CZ)F3E$vBXK$+WHRu'##wb8]Xg%[onE3ti29w$9'e<0+.1I#tIW)D$$.=a=lb2.3'50&iLA`MH2>Tcs.sE0e($HF++"
"GxQ0#9-+,Mua=6/'1t/2,3CaFV2()/uD%p.W`^er+wb(&kZ)22#4?gLu&&7pju^t@EvQ,2&:v'4I5^+4SH:a#lIo$$cYpk115%&4A,m]#&4`[,`f`O';gh&4S8x`X#6Hu/a<_60JBfK)"
"<)dNh[qw;8iP$_uuJA2Em2ou@KOaZ#P3aZ@C.jJ4<mT_OS/=Y9W4,a>UUq%OxuN%?GVv'9<c2kLWOio%0>?20]cgP/Kx[o@BqX=$2D&A$(3;K$9;c/(qb.%#`K^e$u,v2vxgc/MK>FJM"
"$4L7'taFA#iJWD'AmE%,`L_v%?X:>,R4b-)gZx+2*I$+kD3s6pls&9()aln0O->n0-W0wg(91#$g)THkG6_W/G.,Q'JSGx6mN@>00vjw.&gaI3/[bm3UZ5s8lHfY:e`is7<@(,349=6'"
"fTX9%&Bh)3n-<x,/0xE+>I[=-rXfj1Jc0`#t/RCs-R#Z5-l68%O=Qj)D]]vLx`u9%;U<T%/>uu#2ZUj'C)ZM'e6Eg*ZA7U%Ln4:.NkY)4BvIp*tvFo#rYL_$P@P8#T.FcM1ZjM9`l,20"
"WqMsA4pW=(;/*'>i?$=/lAk]+H[b4'jLl;-^/Iv$)(Y/(4u-W$g?L*#:[J$l>:E=$nK4i*YZ*e-MplV--j)S&WUm>#[gPqAtoD/LIgOs$([N`<x/4L#1/T'+Ib;$#F0^p(I?%12S7nX$"
"(mGh:g(1+vXd1x$Jn8B37S]o@O8mxF38Z;%(;4d`iB7f3%Ue+4731#$qV6C#.m@d)vuHPJ0C`32&7PZ5CqK8L/9rL4/PVXD6d0@B8A1_(`H&tVKt=$,BvWr&q3Y&7fQHi%'QeTF7@:l="
"O:tK%`TvC7n1[bdr8P8#]uv(#k(;g$_jd(#fPH*$I-S0:YYa]+aobN1je:%.&jaiKx-wZ#jjtl&g(&Q08PY##us)D#O2t.3anCk'O`uU.,.c/(^0]/1B9=$-,*Np'PCd$(FrT+(p/*I-"
"sTY#H>tZ,*,w#%HFB_x$i^l[$AY#K361Ykis%GO/-Tat@tM]IMZ+@92ws<K9sE2s@1Oo.D(uNn0'GL8.fTU&$e.Y)4[ZBo%4w/[#2/u/*mMGA#:=6g)(_Xq%nGSF4PMR)&Zg0c5#fV3:"
">q1$`vCIqED8g9:]S/b*W[,-3xU:=J4b5juPIw>6:R4)6.s>-1.gaQTlJ$nChBBoj[9Lh4`2CG5@O[W$jh.Z7&fCqPB(jm;63S+4oiCfU87N,2if48.EdY4om?_$.#.TJ1Qf;^#UEdG,"
"W,SY6Ood-+phSJ1?/?]#=rRP)8r*+3:aS6(X>(s-x,B[#+Z^Y&/;.*3(e_?%O#(s-fKEZ#uT.d#<4^V6KfkV-lIta*uepb*gIH##p7+i(m=:C#;Tgf19JAA$2`(^Y3.`$#Q6>##GURw#"
":'/],l$8s$c[5G*;[NW[OWmg++86A#U@.W$fn%$#MxO12@,V#A8ZBN(AZNP(S;^V-A'(u(r8s0,0N<I3S^br.?/V#AXLq0#ldug+j3ST%p)fF4%a5J*stC.3I/5J*_Aic)[/*E*aKj?#"
"a%NT/b/Rl1?k=/O@7TU^>FlQD0ZlW:+Tc7B<U_m0@75qC/B,w94A*iD>U(60HhZnCJEJn;/jdnCI?/R;Fex%:OrP6DZoxQ0u&mwAh':q9Q+2nDV]fQ0-veqCBd_'/uI1kL8@WiMPrJfL"
"-KMG#Wk$##`'V29Bd*P]Ec[m)08U@#:n)h)rZKA=%&bS7_J3.)LBW1)gfES7w/xu#NxlR(M+0m&4+Rs$]T5R&_M[d)bvUg('j$.2]G<I)6>3T@G2K%StlQ/(?Xjp%eN9Q&Jx8m&P$A9("
"SEPn&9Z4Y%U_nS%_.i>70a9D5q:+F31Y7B33J#W-g^a^I%k8B3[cU.AD/f`*B0=A3YZ)22XfuS.BD.?A$-TV-cMq0#'V5g)3S?MjdBF]-XZCD3cl)`#bUs`u$vQgu/2uFM&AafLaO.Z7"
"PgUS#nl*+d:Lq4O`0G>#wkuhPYM5lS9a&##:6:,)mh6J_/L%v#O1ho.)SR8%,b_;$9]P##-MC;$miIfLu/EM-$Jm-.iJ)o9-eB#$-6qJ1mGT`W$9@Csn^oP&>A:R<;kf+VRuL/1BDYc;"
"5&cV$ZqF;(xl_cDPM+#BN[WP87ohd#qcq@@lP+g=$@G5MXJr#7wT&)ERH<CO$4mK)E4xP)A(9o:51L1EHVxd<Dqb4'D0K3:F#]rJ%>+&$7XaV$#0u]#:s9V.7lqk)Bb1H,(2Rw7otiU'"
"]jtJ:v`Z$.=rN%b$4r'+R/X_$cNJ,;Wh@e*v5G7(fm31<Reh*#%u0b=lF*E6.@80NrxXX92P-&#C%E?-UsL_#ZO^T&+)Rs$)H-<7-s<3pSP<J%i]CT(d5Ph#nu(9._j)22Hw?m0PeDL#"
"$a9D5lY*@571xf$Al9gL&;gV-FAYR2K&1a<k/dg)NCI8%#$p*%_8^SU+jUD3Eh]f$rar,b.+i5/hU5l120vgLkwGZ%JYwY#7%4oE%fbe%n9eo1Rrlb=FZVdO-=R`4,eHI<#'r(G+a)WT"
"G6EeVnuqqBJHJn;dfCZJ<V1UDj5+)>]k3F5PEt=l`TQc6h^>&-Oq^F6x7bW/R%C;7bYNL1Eu4X?[#Om1:NCh1PqMR1^m&8'Rr],=i[wi=iN#NDdOOo9IjUL1l6(P:CvMk',ESs?4#Or9"
"NE1h1pvKbOcD)^2xe5Q'l0S79@DBN2n/e`*CL%##$'`C$lGjd$L6>##F8YX7vN31#k@<=(1l$W$sv9j'm<>PA=IwCaiuES7J0_*#0S9t@e/-r8S)c5pfD%&4emwiL*SGci>E7<X&`1$M"
"qqJfLhY,/$i-.5/Q,[Se4rJV6<q(#,QE^m<v;7;H>.Ub#j[OR<n^f0N+`_Y9l]JPJ1]Oa#[JZ@9jE],Oeej`<6<4v#x-Qb#pObi(h1;v#4*b:.O>o-#vL<1#U*Wx)3G%5-2=l+-)l#j'"
"-Nu]#gGGj)5vs5(_>0x,u@?x6D<f0:`M2O0w8va=k#jIqMw#=(<mFu3Rt$<&S$^I&3jkG22gZp.?U[W$AO'ZdB0aD+RDB>(3?>J(B?sU[quhx#hh#C+HcuN+mVGb%W9V[8W2$`8lRQS5"
"MSo$IGO6($9KGqfXlv2A;,Hk(I?%12aDDT(.s<3pSi_8.mw$-11Y)s@JGq0,+vU#AL;u2(A(=3pT@[29uIF&#JA;>-j:Ms$v.gb*a8*W%w(*2,g;*80j:Bp&(50l:Tl&mM'_%_>N?dpB"
"uu)[Gcc'@6cQf.4et>;@4l9aIPr5^@4rT&JOi,^@f0T#Dci`q;cw8^Cfurq;7`'[IGD8M1q%Jm0p`LW7JZ@WB9tiY.Gr`Z-WpEH*f.6s:+hBN],a@Q#Wu@R#=K]c>*d7E`Dbu#:Ds`vX"
"E[Y^9G<S<#KfWp#`QUr$;xK'#=8$W_eb3onYbxn$K$Jd4N0P$.X0f0:IRlgLGcbjLV2g],G3lv$9S%&4M_Z8'N)5k0ma+/(nnAT.%),/0Hv5W$Nl;p/R_cn],@f;mZinc).ET^3M8bI3"
"6KW6phZBd%:;@['f=cI3N>[h2WsA0M6%Xb3'O7a*@?KT.t(4I)hl=o$:-Tv-L@5Q'4+FWo.2[b6<RIw#hm&qL4QPd>ukapR)KSxI'J[1-*Bmr%1=XC=Ku.j0u`Y2O(D7B5#MS#8mP+x?"
"NImfCke[f*[&8t@nrG=&[#i>.b-Y@kLhNv#9bl>>2-ocGkSJ:/Qx+-=+4e>#.i[(#5$*l%vI$>P7l4;Z<*GJ(4rSp0p4qv#[M+6%$01,)3cMd(k9cmo(D;5&;@iZ#Ck/q%5exi'5uUv#"
"WmD<#iTX]6X6nO(rd;hLm1f`*Fo#Wu:,hd(x?.LMc)RL2*pvC#vL;?#>Svb%M4Alu-bN=]5uZ(>@U:o[%;rlu`bVFP'G*E#Ard=P[l0DOi*%##C4C/#5Prc#qCP##AQt$#/_c##9rlY#"
"V23O+YXF2$j<EDuxB.,2^E:IpU,UO(V$vM(C;J7[knWQ##[r[bTAW&#)&dT%[C)%#*dZ(#9LUm7$gGaO5]TG-@hlrHKTY^#c0ke-:*.JM(,/6*@ehoI:wS]#TuuR*c`v2v]teR&-E6r/"
".kY'$/T_^#$n7=-V*K60ci-n742(%6h<Pq/(tR+4kYR(GhSI`#=dsE4_5m3'TY@_b)uV..Z:#/MxIhkL1ZB_,En<310p156S:o'#:vH_#:'2O1[LM8.8,p2+7e<&+bBPfCgmN,N5gN>d"
"SxBl%qj)222Ov1/A[7B30[^/j1MDpLlJo'5V]c8.bfFP(,/Dp.4vU#A5#M)Zp#sDHvQH+<EIN[$(0x9.H&B.*Ynn8%=O<cm0I'A$6uTM-Rh87d##GD3]b9I>Qv%0=)&E+>U4>QU/&.Gt"
"$J9o[SqmjFmY)[u^(Q:9>taT>#,Y:vqc+U$`tLo(m[&*#Mfw8#&8P>#;_B#$^RV,#^d0'#8BMJ(^m#N0'0:onI9hm&cb$iL#7,,M'o#=ZCA'>ZGQp6&fYN5&Pb[V$G2N&+;kG##^xO12"
"A3,e4xaai0Ocgisgr83%U[0U%i;QX1Q00&66qVk1P-0&6vNU*F+?)</+])(,+6dv.+Sdb+^ALMDntBh;lBUMDlk9h;/ahs7i1Xf2w(Lg2%Dvt7C>F(#FH:;$Bth7#=(V$#jd]o'HP`C="
"07?v$x0&974JlY#TmRWZ*(^M'A*#N'JIu>%LYruP[er]k%D-<$1XUVZ?6u2vl&-+*J1x+2Ln8B39nhu0C/:e;A[B4p:A*f%^^7m06wwq.vPwh2F%KEu@(+F3ldS@#;no[#@faf%Eq1Y,"
"`xnxX$xK]O?cahu,C-V?G9C>#<PMD3MQR>PAsK]=JU')EJ(r0_eI]s/LnS'-UMFuFpkQZ#Uokr-0UKk'`25&#5fYY#JZ#R&QQ$V#YNS%kq9'Q/aa,,2ItIO(tSp;-)[gs$Sa7H#n1J&4"
"?qD>&/DXI)/Ras6A&WY5Rf;8CX-6o9hLv]%)Q/L;=%8eQ$hUtBYCl7emU;1DWR5r9/%/x6Y8kZ%iP[v.^^hSBV%_0<@TMfLKOH>#,0-$$TF.h%Wr7PJ]%h#vvN=s%Osxl/[VxN2k&DAl"
"IT5K11tI1*[97)*$WQV%xPB0_Fvc#.PJum'THb>li31hLuIkh)@Y2o&YpO#-qFP.'sajp%]:)x@B<+x@St&*)9M(kL`>*22XR>3p=*og2TV<n02nrJ:N]kA#_DA4;X`0'%a21#$?HojP"
"]+>[.if)<?>P^x.NpSY6>ZJ/3ol;t8bNYF%F7Oe5:A]?.SjM1;2ms]+*+XW7EReQ8*`LR1B6b&FW*Gr/,<6w7N2CG5@aMJ<uR/S)0O1E4j2G>#f.F01..Ki#Tl^>#`wlxO=/Lk&?L6c4"
"r4m>#_r,oumh9Z)m>R;#$),##.m)H$H83)#H/N_7f,Mk'i>@>%<^)_$<Z=#-ftSca5o@#$gqN%bZk>a=(ZD50G*/O9EjZa=//e),[sZ>5=k^*#i_-=-s[P_+I5qW$W#`c)*+$K3v$8I$"
"46]t@=_26p(+-@5L*wGGC5)T.Kcn$$P-bV$X&>^,CF?s?oxTp7jX-v#c#?&4fGW7LKJY>#@(';705p)GZQZQ#.f$s$:]h2)$UBa5G)q[uvhLRuY]NJ>aYqVK,6S-u/`LX8fbiQj3dSgu"
"wZS]$&A%%#mM2Y$pQu>#=:rZ#:)6MT<R7%#MXrZ#Tx_0_SF:$)D6lf(-;,##G'4mLLX+o$jp=5pnZ;hL@nN;p%K-T%e0x9.1]R_#xH:a#:w4x#UbLxM9.404AAN.?<>38e$UOj'&s#)a"
"2sDI?7Vr23H*P:vfuM`W(l=PfeT;ig_l5d.4s]E#E-ST%`Z0A=Lp*xu55jn'2-TH+AD2v#NHx0:/+$G*92im&J3qU%e,B_/^4[S%xa.V7aH/nL]M0?PRu/87S`*22mI3,2&:W6p?[330"
"#G+F3Z9bi0'4%ElC(NT/Ok4@7YPN>/#=A8%^L?Z,':gr,,-GO)k7jX.b$Hm')Xe?,Z#o[/37/uu3]QF+:]B'%$(L/).s9M^3xl+#D<b]#OV.h$c;r$#YAc;7W&7L(trlS%;USM'7F@[#"
"8#Q0)-B42&pFZaj.$eW&VX61M>V8@#E&6-.&j,f*,p#<-tbsFP7+pG3^W'B#f3Kl:,0o$?/%GqI['SIEwd4P0'6Cr(Bs5Au#v&qrwa7##A8g.$Vu53#W?O&#aV':J7nA]#RcGr)ATwfM"
"->F/,r/6##LN5IVeP1x#3t%E+,fG?#GQp6&5YN5&<ZB0_aWo2-r>uQ#bACW-8>.^dm,lg$OMH5&7fF.3)*?a3?;Rv$m&l]#mqb%$42&'R*f?AB%'s&R3rC)A%m5gGu=.Z6KR#vKvto(Q"
"g*:MDC4V(Qj6CMD&rn8NP6s-FdQ41#;F-DEV?aG;1gHT%dW:Pou(-s$U-tOoBH57&nK(no6YQg'3kTK(QZPj';@<T%8Y:v#:AHQ)MO<Hp.+n<A6naj0x%.<.B$$&4nXR@#6X?_#$,?A="
"/j1Q#Pt$-^;J]9]tk>M,_</;HdZ7G7HrdC&j6@uu>tn%#O1Km$J?1'#CGA=#Y8('#[jFJ(X=9U#qlO6#P5Hx#XmOx#:`$A#AW*&+DrL@#W9F&#=(Kq@XYcS.@Y](#l`/,Mx-;hLqgO;N"
"VsV&NX#EEMq_(U#-a=6#5E'[##mx(8WEH&#cPtS&&8%se.CuQN=-jr-s$F)+$i4?-%)vY#5?p_/.a>a5IkGS75`(xKnK[Y#F%;xk&0Z>5vsai0gJ+H4b4N)%o8lfC(]4)N5lvd+T7`cD"
"&ib%O9esp%:xAn0/i63#NUu1L(7@A,)5^vL<nfg3PqN%bEj[-)K]-9.*c.hDm;K)Odg9s.aTK2_i^'X8(k)b$2^cp.6KW6p*.%+>gfJf3'O.J3[Y9H44.QB?SRQ>#*5J7Ta%@R,/ZSlE"
"DNta#k'1i*t4Ap/TsSld#)P:vXIVS%v4^2%X@('#XhFJ(neJ&vf4s<#Ycwc)_xp._<n;<*mRQp./:](#`^M:2NRS:2sv_o@+[0,)#jDU2%?sb*oH&`AkWW`bvB6:251[5/U.Tf)OUMMU"
"FP:AcD5rHQ@AaJ2eh*s@e)rfD29&^l2aw8'_Ypc.Aav%+u((1Mg^-lLV#)P-nIhL2F2n4$$AdU7m2)#5.S1v#H)^D8hP.K:&2>>#(FuuY'&ii'ltFcMo)L<$GPiK$@+dY#''Dc#B&x)$"
"B@V;$6:RW$FtsT%^QCl'OEP3'K8EPAWu^^%,U:Z%B9ic%sQ3I$SWPe%(s@l%_i3E*OB':%@Y/m&Q*@h(.v&7p?l1l(I?%12Z=?3pQI3,2IMYh#hN$r)K_9K1;T3x2^a7H#l^'B#cmoO9"
"(r/QA1S4Q'QukV-]KV4]BGOrmp#wR[$+.f_#Ho_jDgje)v/0M#1#Zjuk2ksS`K&juLQ85]WYDt8Nlt.#?'/>G67*sHfL)7/I@$##_v9SI1K&##;BcQ#Zv3h)*tji0sEf5/uH:;$boCxL"
"muBKMmWL&v';&=#Oa#m&^rg._-`FE'*`,68mI8;2kd(8@pnYGMlsRs&_G(@-i@(.Qpq4[$2%V5'e%L>m3xksQ3/=jNDm5.#pw,;##L(,M')^fL0on0^M+jq25&h8.CLOrd5<iA#clO21"
"=c<;[ClWV[6<iA#0%?;2w%p+DN,@/Du=tX1*%?;2(H0<-7p1P-PkOLRuP:xMh`uh%96M&#sCgq2:K%'#%/5##78Fi%i8T29Rlou,Tt.TBD>p6*ng..*Z'VG)v&#]#ZCrO+MG>xPsXl(+"
"g`:,+hwlJ(?d_r?R>ID*7=:/)>Sws$X)%a-eUBj(se-#-1ZXZ#9.a%&Osu/&Z4C3&<.a&-P/mp%iqZZ&HGhI;fVem0ic(7p=fGqTMY<3p%[)22`7o04^YU#AU=>w$Z*x9.iNAX-w=Q>#"
"fND.3S06l1cDQJ(0VLh3A.DIN,_8lUhl.cumtR@WrBF`Gkj:K#J?A5Y*i'j?hdSj#j_&Rtc@l.:9L<5&tF1K5p;u&-ChFw$O@('#fhFJ(Es$8#o>/=#-0O?#dvBB#$:TX%7.#L%K8v92"
"G]N1p<l8#Y'8C,3NJsU8nrnFrD^lm#o)_^#;1Z(^oq)$#?4evP>FD/#cJ_p$]NpPBoXwX1DpqVonU,v,(j[&#)Og;2N@2.Q%<j,MTS*20@v:@-[m6=.:6>##wv2gL'`vY#BSkfMe+JV%"
"bt-##QA;S$t::/#RjRc7q#DVdY/9v#Q7I8#bPAM^&:2gh@W?0%x#d3%;<u2vu39h(=>7/()LV8&$l^>$6pMS],7$Z$.jIP^>D`)+Y%ffL`au;/,.U#$)</F.gO;?#54r5/_o7/LrA`R:"
")#*`#=3n4]YcxJ3d+V#AkBRL21H&hMIn=n0R@(kL'vFu@x5Z]4MW>r%5x:?##rIdu6c-5A@^7eu8[epC^1rIU'*DMB@W%eu7qWmDge3cVCk@duC5n0#X_?##:oYR$+-u.#`EX&#TaGi7"
"Y+KA#0jJ2+/A.)E@h?BO`^4^&8T&HN+#^n$Ih9[>PvQ/tMj[f3aNeJM3cK,-F<IPJB9p]#BH,;HT+4;/[1kTJ^;V$$TD$s.(_Qc*OLI<$NJ%8@epa]+6Nu1:`t@T%hlv<.<N#V'C[i?#"
"<d4B,AdL50XWja*WjQ)*eK`SADO%%#n8EQ7d=Y@5s<Y@5@vWBG:a3@5L>[BG+_x<(D@Pn*_uq9(Unu,#DDAs?(Tu)4S.tD#OV1/bYHsD%@g.)*F09@#(eff1ll:^#XRVNMxAcFZoc&HP"
"I9bmLe?%FL^?;V2:2;]uYj6TWgL8j,l'0?HTf@k#uWp[td?3t8m$9O?LNkP#YCw1NhR*R>]oqkGTh)M#B@#BX38EG8^W]=##)>>#M1ST$w#]x(p(@V8qDa0V-ngc)MGBY$X*u2v+=#88"
"`^S;2f6KY>iUYGM;'LJ+TW<c'#k@?.i+b]+(j[&#wBg;2d)0.QY%PgL1QQ>#J&'88::dERl:$##nu?kbs.<kbui8JCgH@s%=f*NCDRwS%J#Xa*t=aT%'HW]+_%r;8>W*x'pi-YH5nfx#"
",3es(nm>##HnJwP2(6U7K*_*#W&7L(i<F9%3B,P&q41'%Ti-t$'@)<%HNLg(l?t5/uZC3pTNi2<EPDs@0u'38PG$C#k@C8.8%1N(jCK,3B`NG)j5?2'W+6b#7HVc#H,:eI_]Y29WQW:U"
"K-YMuvNSLE$,>>#@.[:m3'ZSe$F<c`CCcg5bWQVHqo[`#27[fC>P@?7k-MD7P7[JC<(Iq1%snt1Zo.t%d[/4_r_av.2#<Q/^p?A4C9OZ6+f^V61sLg*4U%p*TH>v,fMHr%p)L'#5[u2v"
"l)?F*6;8N0NTHN'(.j39T=%X7=k^*#6o#O(2lrV$t<9(#xL-$&&@P'&FHGN'.Uth:n.#=7$-AF*?6]t@2a3@5Z8Le$p]q5/14u2Atq_l8iA]t@40_o@Hq.[#in'02@dk`<9,$?$M$+mL"
"L1CkLoQ/[#(o0N(dfW]$K=^I)_cWI).U%vP5`b_AAAS`#nkg0<=r`K4[jpI46&ki2T1H#@UaaU:bR(ZIBSAtTlXhG3>w)a4_#?f4YR^#I61@c*X<Oi(m4;(65hco9DkcO:V([=74KwP1"
"(X.=J*T_;$f8-5GEl=.F7A]Y80]R@-X'XY7<$T:v;UO1pj8xl]JR_l8:kat$JF22#D^lm#IN_^#V(Jx]p;6x##+svP:ato;w-HU7Z]$x-dc)-Ofc4Z/+J+##9bZP8IJN?-%+u2v3Mh<8"
"Ww<s@%(S3M/jDE-awGD/7i+##n8WHM'u^h;=^QF#=)Eh5#Xlv;a5IA-wFjfNb9NmN59d9q,qVV7Eb,I6lYF;-F5*>.5S6t85l?)4KaaDN7e0Q&s#t%cbxVI*vouY#OW$$$q#1=8JD;B#"
"d%bX%[7CG)x24/_bZdo7x9FV?V5'QL2B&J?-2[+PLUB7)NJg:)v1EnJ*Ljw#c,aX6w;Si)+gkV-u1k]+UtI0:q(4,#'1g/;4A;6&([>YfFGbb#,,(,;;#oc,&m#O2-ipK<_@t.VicQ>$"
"c>%e<Tfub>:Q<^>(H?T.Pa[12iV`_#r,f4(-dP.+5nL1L?Xjp%<J+87I'BT(2)=3p`=ip.I?%12PeDL#Yj,,2&#P12cg_-MhVU#AWa`$'078B5i./(#1`61#j-.1C_CMH*$3Tv-UsSgM"
"+G4J)M=tD#S,m]#_t;&FcpDmJw$5N'Wr*Q'&J3j1$x.P#1`,k2crcBOg7S-FN1h8:alJC4GTqr8qEEA52;rcXV[&Y._sEH*Ud]m/5>V@@k^]6KJh]6<)?i>5p68i3&gUj:#5WfU)OX=."
"CF)%&0>fe(BU'h4$O2O1eJ_e*%oLj2]Fp21Pn1LE4b^R($5xm:+AQ^g*xrV=b6]x%vtxF,PS&*Zl*>>#B8YB$kS(o(:uJ*#YI8j0u####]$`V->>tx+X4*6/D1ST$P_[#M5$0nL/:[##"
"n:)##q2>/1kxp._ckX^1v&^S8_O8;2l(9]8LP;U2f%b025_[9MnQa`bP/l?-Qa`&.qA:wQs8NmN>Me_%xAbV$%#k#$6Rgk1%=*($4D:2#g:nQMAkqwM&C37#7d@W&fTs6/;;P?#8wf._"
"7rfd&cuRfLoqs;&*VP>H(*kVM051>&gib0M*sJfLqBEq/FC###?Bk?#;]b?/?DY7(<W2hLXb[0^NOXk4CE8;DFWw9V2XF'#]Kb&-A*JR2p_j]'_Xm?#XdalA;2#V't;D].Cw3t$3LZxL"
")j<Z8m^Np@MnZ,vh5r_$wYt&#V$Bh7v;pC>Mad%$vt`l2]hsf:D^c-ErTZt/81*x/;(9v5DDGv#0TbU.NHXt$J9KU%Uvcn&`>,l9idFB#@H-##5H]pACj19/)eq&vuHNh#@*RV-f#p9("
">^<=(g,U5JhXZv#U<tt$k#Pk0w5>##3.`$#3x68%^Nue<#m@L(@G###MQ<I3EZ*@5nl^t@f(=3pA=Y@5dA.,2(BqH%]KKP3pfgG33ZEV&@q@.*;kRP/_'@lLM7%`,xm;H*p-2@,$ZcW-"
",b@*58p]c$OG//C2?)U/*DM=u;F?YIcDAb=8HD^4'_(P5Z9%@Kg]Vf3[Qt4]MxUl<RcA_-cD?%IjmWigW,55VSjd@A+B/K)TWWlE<N%MK]TF/crjqNMoiMX^W-Vi'<p^81<Yob@qwJw?"
"&`9SIei;[$Z@('#0iFJ(Nx4J&P:<m/,ECB#]@$3)RKA?&d[O1p*%vl]CMTD=F&NU8H8_c)eH9N(Y-u2v69fp/kQ+wPM*VVCc[GU2K9>T/rYuvP3vi7#oa6s.$^[&#/1=#1h)>RNf$+x0"
"Q7^&#O->#1PuY_]ZKXw0Uaw8'RgWb.P=dl/,kp%'4HA##&5)S$IGL/#-Mc##'gv^7)C#R&Y/5$G1`Lv#ob087/TOZ#6F<5&C6#3''Fwf7>oc>#:tLrZ>V&E#qXND<Txm+VouK;$0q'j$"
"YV<8pbH]=pwD[1(fB^MW564N3t'@(Fl<pg2h3rI3:;2]-jL7J3I*D$Yosful2ju1$PCVmu?IYiuhHV6#uW*?5-1ST$(]>+#bWt&#q:fk7[#wA>)Ik#$`Xaf:5hg_#l`VK(v=L&Ot@0T7"
"Or]BH1[A]#aqN%bRv-@'I1KU)u@.@':6(@'Y2(_,;U@E+9fc87LFfB-s>4V7HCY1LCTcu$R?kI+S/*T+X<Q##sTDoAJ:Pg1>.tA+3%am9SQPN'W=o3:Oj3`+lQbB,:luV-&Wja*T$XX$"
"xH>e<cC&d<Y8u;HZ-Y5pSfSh(OVu-$Lran0aree%.,;B3Z6j#&'59H#/3i+MQ%]o@0nAgC(@Y)49EKG;oer-*V[NT/wPCD3f9&p.I_i$@Y^FY'x.8W9@hwVCNgKO;tFjU8[iqVh/Smh<"
"dVx0(n9t^Gvq&g>CVAY(^CDi0?c_^Ht%hY,W`%A9*;xu8T^xHJUiI]9#:x#@jmK[IE1M:4(rEKX;j2s1jsgU/T,Q4##)P:vuVQ1pUPtl]_]);?>=9d208_c)fQKN(X*u2vqVXQ0sdqW/"
"eoU[-3cZd=t1+>/@OFwQ:a:%M>IvD-MjrA.%Ro9/YQ]SJl?ufDfbW)+A4X0PU5kKPv&`a0HVVR9DexQa5Pn:?JH'Ra9l%RaTfnnMv3oiL65YF-.kM^mP@6##=O3X%4?WSeoVmx4c=Rs8"
"iB0f-Gr:@#9Yro(>&T>#uT.d#rP^@#5(&s$;4OuH9*5>$>=:-#_/MY5*,@R9YkwT2118(#OC(n'lrbI)L<h1(.ET^3^%N;7uo6H;T@)s@A-dDI@ror6No#f*e'v)39^1#$(o0N(,['u$"
":%`v##nouI*fFAu34>vIY8J2b^#QQ#tR0^#Ld(['8HS/JMZQ8`/J)WH';=A5kQJ%#eujo7O_uSJG7co7J`VhD?H?D*t`P>#'N_^#p>^v-5.;p7c0UB#^((##v#vY5Zb-jKt^Ijkb#C6&"
"d0I3K(uarnQ316/'F1/$RiNfLjb&a#3A9xt=;P,;c'el/wB0Q:mmGC/v6FZ>&[(C$?4uf(huN**l,Hg+q+B+*)Gc&%b6(L(YDwW(7]j]F-'g1V:W#_k[27L(jKZRAWdZD*n6qQAUZYN'"
"/7uV&w$$@5$4g=pS?'*)f)Ph#Y].Vg*T'W$$:j?#&0ZJ(b[k>>2ocs.)W=P(v[*e+Y(Xp#<'PF+tcjVJrrMJ3p]KuG$:DrQkXo:6EI,.)7]Y_HfUKH<HEY>#2Y5v]iNOX;_w-J:51[rS"
"eg_;$/KiY54:XiP_0f)#B)sx+tdrl]Fg/5/K0DP8,cS&HY2qC8i?]GOhAI/5^vioIX&2_#[fQ,$js4/C-aTLMI[E723Mk:2ch>g)g2jA+_-F5&jWo[#;Rk)#'_jd*ME[w%T+h6(=P[v7"
"15pK<Rr<.V]>q=$wHt[7Vd@51w'a`+jK:f<hSR-)`5[S%XhUwIx)&7p%[)22lV0o%UYbw?[jJ,3Ofc0&#Mq0#(ovM3R(6J*$3Tv-';:8.UD.&4UQD.3E-+<-Shp0O7*N^%KSGA)O:It7"
"kG7`&>9n9:0vrgMB1tJ>Y=x<`wU&4=doj'-&3Y11G&qs7`]Ri1Cv#t7c$AG;n[7@-s^hZ%mOJX>IZ&2;_B$o^Vls)PIVMD*MBnGA=AQ^g[w@QKAW=''fr;0;_$b)<$,>>#J#oo[<`#m]"
"Q7ho.F3cA#+tRH#THc>#WPrS&biXYPkO6Z#)owJ3;T#C#PaXm277E:.BIr8.qnF%$rH'r#n(I%X6%6CO4Fv)MMoZY#oP$mJ:`2Q&^hHP/pjUZ]W8T0:'0:on2]fj'cb$iL#7,,M/#>N)"
"c`+/(FIvu#ort.#TJf5#`b39#5NA>#Q3=&#s,<JM8L<5&YVDw'(;#j'lh70:i90;2Aru]Fg]p;$l%W[$''ra3LvRh$J<IW$03Y1:-oan0`8tGhR'Aq$.u[#A&$v&3)dkMBd>@?\?l%3a6"
"lZ#a,U()tIGfH[-H.B5'.)6F%:LxW$eFR*5;et4:F=7e4:_k4:9.dc*w<i@,8Qw.MtY5&#$)>uuS,ISenl-kbB+v5(06]x,`IS?pui?j'W0^M'Z8:m7R$S0:@S9_+s:@d+itnl+ir)8&"
"7oKH2UVJB+T&F$P'f-$#(f[/301[^3&nj;-B<@i.`59H#pQc[$_&c59>Bd&4tB:a#I-Kw#Qt9'>PZ+H*cq[W$.CLL1)-N'>KEoG*dk@<$-1lk0HU^,/NX;[,GQn?\?$p2C&[2Y',rquQ1"
"0%^m/A:*P$>I,+#@8>3^4WxI_v?187qB#12x[t'4]*,2$?BM:OX-Guu9gF?-+GG?-Gv'w-B*>%NvgtA#k*x)$4We8#[046^A_[@#4*u2vtw<O'p*JMTUYu1Te1Wt(7P(##E_qXHJfHC#"
"KR)v#X/t_s-sVSe$$AAF=Uc&#7s`uLTb&NM[]mX75L$iLA%w##G+*p%x*4uHA_F&#<d[#$Ka3P=&ItB#rAsB8V$'p%M?C5A$:V7R#sWB#l'[4S9r(v#M=N#6#'im=H[Ak=W.`$#_43uH"
"VEp*[-B,s@#NR]ul6o>//kk2$$xmIMXvd##;C=W$ElhV$#4Ds%V?OW&JVOjLWOnMT^rC2TS+o-%;-c/(sCBp7w4O2(SfOZ$vCV0$v0*H3_8g0MDB^t@vQim0PdkV-v]k-$ZxIHYR^c]4"
"#xl,4Wm_]$Otd4]ir)a+iB(KDvL,a*Gb)t%FC)Z##e-iLA(2hLG6p_3`llPStn?;$wcYJ$pGp1TZoLMT?>CW-(13TBs79sMx$xVuHaln077E0M[82,)oqo-$aF+IYKU4rdUdVvIm)=AF"
"6xNfL&:>oRE7dY#-l68%p?,N;W_`?$qwq>#,#,/(TWtY$0`T]ui3#E8(9-E3_WG/$_F(7#X]:Y7>nWq.(Egw#xI+6'-HWA+@.9Z5J9bx#8P/&$6_+/(=:[W$gNx+2MN9W-0Fvp:D:SPJ"
"=+Fe56Cdq06`eH5=b;71qsflClk'L;T,R4CdF4k:43G>#d<#/$0;A5#D&>X73$gb%AwI>#9LuA#XW[eGc8aI33T7<&b;5HpraXW-4pSO<oIwA-:IvD-AbV&.eDb4B,7C'#VOKlf0%[S%"
"^UV]&(ONNEJAT?&sW@5BH)8)=q**97$),##H@uu#m*B*#Z0Om^;R[w#,;>>#ci-n7>e4/(Fu,a#A_R%#'<-,2SN4l(L&9J3tc_;.En158$g'4HQ3H&#Zw&HDp0ST$;RV,#6EFd7j<UC#"
"RIP'$Vo%j0du3'4h3UV&@wdu6<0;>5#:G&#j+gF*8+C+*CP[N'JNID*WTuf*dehc-TSNl*0]mA#@B+Y-E^*&/Po3<%3B1I2Ew]Q&7S+87Ht&9(dCg;-[Kq0%%U4[GW*CB#5-I12a<Ee-"
"w%=3pr3;%%<5^t(1A^E[[HT#AgV1#$eWUvfJr?<.f18C#OM>c4191P2hP_:%+J*T%0i5g)ighc)sJL#$tV=Y%`Frl<<D*=8fxS?\?QtrYIPfe9'M0afLE@1@IN)jqmu@`)?fY6GO3J[i="
"04LoeIOH>#XRTU#gAk/>N^#s.^Y`i1m_-9K@NcS*?3t:.FV;/pB;:_,0T.),'#0HPFJZO0C####=KRA%M5>##Q;Qx##ABO0,i$W$$R[$#Y72mLYG=T''R%##D7GmD(pYKbApnO(W6MW#"
"H%U^u.hOO1CWfc;7aW29$l^l8pi,-l$woo%9:rv#QDVJ(xji:mDMrf(-<9B3MEI129A+C&)K2T%a7JC#UM<O'[;W.3#C.J3(ET@#0_G-2OM/h(rsIO(aGic;VqDA+5/%9IBS6[-f&VA."
"^n?u#'s$W$?:F&#Agec)]V-B-OwS'-OmM%E9k+UANNuN'5#q0_AHGQ#x/4J-(EZ(AQTI0W[mZh2<(Z0)UjvT)tvwBt.Kf+>VJU@6&I?gLCb;Z@+_Xs1W(Ku8^$ng#_@$D>2<Mp9]-X#7"
":xOe<V$i7D@Aew7O+MgM9(Xs#&Xc)M+tI(#<%KM0a9pV7$nQ#$Y3G)3l*Z$,38RPJX<CAlKaGK1gM%iLW0*$#X9s>l/Ga'44rSg1CiK+*$ESTA5'Kv-'_ijLj+av%lq.a<Ew]Q&K`RG)"
"IR%<-=t5L/YZ)22`HQlLXODpLKG<3pNm4dD)VX&#.3;GYrlLv'.j_%'?29f3?;Rv$OtKd6X-<)%_vOA#)Btp8V:3Z@h'%aN)GBF43OVE4(;Zp8EUcN1ONYF%oNEP2b9KD,I4_4;uP7c="
"0$+?65]P<6XZdv.93k&F[DX.33VOB48'^x,gUR98Qwuf(46YY#6)5,;PQR29Dg?D*(bVm(>x1?#':9mA+jwL%,.,M(4KmU7?'w8'+w<;$=`L;$j,ol%h$`w,jaP-V`MLX$QkWP&KJVJ("
"FWbrH#YjUID47m0FOio%1>cI3O>@12E#jc)'kH)4FF+F3LFIw#`7tM(D,E`#<$l@O@XxCJO:pb*QH(R#r3KlovjtH#5UUKC&Wsv,pnDvu5_S2'($p%#>cE^,>#B>#4ij-$<?J`E/(1B#"
"2-e4],o'^#F*kud1:h#$.s*`s2C-?$F2a]+3LHZ$nuRV60=?v$7Y-v?3Rm;%-oZKl4fGW%d=aiKB9Ws%ft9DN]+E?#F*A7Nj'3$#I5p,N_-<$#,r$:N_0E$#Gq__Ar8<A+-=%v#>2Bs%"
"9m;,E9___&VOJM'dZ_R*(HC_&:7o_&orKS.2*p_&YXJM'l%i_&eaZ`*.oi_&8l4R*u9k_&D5%GVn,l_&xOdl/bZk_&m;sx+Kom_&pLAYc#Lo_&5?mx4S&q_&Z2@S@U'k_&j_h(E8'p_&"
":$U`38;bl8IYo+D=^E>HIcu8JB2s.CtYk-$sb%;HfTf'&t:H]FcgXoI?<s'/C,Wq)<Cel/Zl,20mig,3/pYc2@rj>-hX_'/%Ir?9r_#F7S7CP8Eje]Gkrj`FP^Yw9>N_w''ed.#@5^l8"
"'D7FH&*a9C(`CEHxqvgF>M_oD2<XG-MhW?-<9RL2j.:iF'C%F-dnr.G05DEHBN7rLNk>%.9sarLa`4rLEVCP8:GhoDLU&##)ArT.U;L/#1v;M-JG%U.'DvlE/T,<-2'l?-Dr.>-Q*MP-"
"[Ln*.Hv8kL#;M90KgiTC#g1eGa@Z3FUM%,.60x]>qK<3M5)m.#)80C-oG%Q-Hvxn.Q:1eG7)8+/(*7L2%PlQsB%Qi>XG@-d(LV$9*um-$-uwWU$:c3XNn=kbD#w9)qcSfC=Ce,=4fipL"
"K#/qLVZ4?-FN#<-d3XJ-#D3#.u;*RM4@LSMq)Tk$ZKXw0tXTiBM2F>H:%$x'%1AVHcA@gDP]s.C]Z5mBujNe$]_1#HXAl,=:.rfD8ESE-%O;9CuicdG/iCrC8&9W6xBrE-6E0H-(th]G"
"x:r*H*BFVC*7<LF)c1eG-D/F%O-ofG#<4nD)>ZhF3@u'#M<>X(YUI=-HXnb%.p#t-NZ=oLJBg-#$;_S-,h(P-MAMt-3Z<rL+JL]=.ZkVCgoBEH5M`iFX8Ev'2du7Il=>#HYrmanp#iZp"
"lYo+D558H+W.K-#'^skTFW0,NcfipLCKG&#4Bg;-S5YG-l:.I-m9XJ-3V3B-'ngg-hhSe$w+5GDTN:/DN5+#HY8NDFW][>-.+XMCNT-gDHdVrMW>%hc&w&:;)gxQW'h3X:h#lw0@/8R*"
"t:L`ExK'mBS'H)FAZ4@0g,<MB@bXMC1LD;IN<I>HdmhsABEUSM)T=o$Y:q?9W0Fv-%^E#nK2oiLv.AqL]D#l$Tw5)F:Ih]G%,*3):e=rL,.:kL20]QMR<oQMEX'4.lm-qLRmrpLRfXRM"
"bLCsL/s%qLOgipL#)B-#XsqxMs)crL8@]qLLa%n$j`i34o0O']Q.,EF@q$29u9aM-YS1H-)o,D-,V>[.m%T*#vBqC-rj*J-)o,D-gvX?-MLx>->JA/.3/'sLd(jE-weh9M:`4rL:(8qL"
"k+IR84'=2C`hf'/b#q92A?G_&,w3X:=%g+Mm'm.#(k2f%Io0L,88%L>N5@X(<[d9D#cW9Vm=(.M^_4rL`CcG-iW_@-Am+G-*'KV-QqU>8o5i`F7Z&ZH+W]3Ff-Y>--Kt;-x&55(Ne@_/"
"](N'J$#$FRwuG'S+)<e?md6)F^p3j1HR[b%I:)*H+oL*Hs/xUC.q'$J,RSK'XE.L,tdHKF41mLF4ADEH?#m91,#AqC,dKSD-U7F-/GI+H9=2eGg+DC8WBRSD-OtfD4SlUC,witB,=0%J"
".rY1F0@/,4xB%F-6vTSDlX2cH,v1eGoOdBIMh4t&<Xj`Fsl1eG8.w0FAFO(IrJaaF@7muB$EpKFHx.6B`NwmDEanrL^u&=B$dUEH^&x#H-s:*Hxi[:CShkNC.MKKF(imfG@q:qL=wsr9"
"%JXNN72-L-h<qs.>+4(IX3LX1+d%UCD9uKGH=?vHVQMn3X36dEFE77MC5<hF7)sI.<8MU1Dg?jCMpnrL#1)G-k`(eG:WOG-%AraH.]eFH@q7fG4QmH.u@ViFT>J&G=HnrLq;>+He%1GD"
"qE7L#G-QY5+v4L#?,j+MuIr'#_BWR'u:H]F[qdxF>X(58;GE/2':M?Hr(622d8_DI2H^E5v&wgFb$FkLe#3P^fFD/#P6/F-0sX`'r;4hPKx=hYu=L`E(-f>H`-n?8Br<^GL<,FHeQqw-"
"[5[qLcBg-#H]oW8LnaYH6@F2CBw4[$*wOm/k1g*#::F&#B.rH-=Z5<-$t7O8h(f(8cWMk+17TU%xb$##LHiTC$k`9C6P]:C:WI>HeZr`=(@YG<2ewF-g$fFHaNxY-5N[90&>Qu7Is*#H"
";Mk]GAYF&Gkb5nsDHG_&5``f1uY8^GeX%,2XEhP9q?U&#SvCe$J1vdG:C+,HEu^kEEr'8D4ibN;r+TV-T?*1#iKk&#xS,<-7e?>PME0_MEa%d#*P:;$.iqr$2+RS%6C35&:[jl&>tJM'"
"B6,/(FNcf(JgCG)N)%)*RA[`*VY<A+Zrsx+_4TY,cL5;-gelr-k'MS.o?.5/sWel/wpEM0%3'/1)K^f1;j'<6/K6(#3WH(#7dZ(#;pm(#?&*)#C2<)#G>N)#KJa)#OVs)#Sc/*#WoA*#"
"[%T*#`1g*#d=#+#hI5+#TeMM/pbY+#K[=oLM4D?>&:fu>*RFV?.k'8@2-_o@6E?PA:^v1BKe0GMgc>c`Mq&Sn8h?YPLS-JUrBB`W3#T(j:6s7R*J(v#H#5PSpL7]b]AK;$mW:YYdQqFr"
"hod(WEH1VdDMSrZ>vViBn_t.CTp;JCbMMrdku.Sek+f4ft(XfCsOFlfOuo7[&+T.q6j<fh#+$JhxUwOoErf%OLoOcDQ@h%FSL-AF3HJ]FZndxF_6auGcH&;Hggx7I1$BSIm/YoIrVq1K"
"Xpa._D1SiKx%n.L<U=lox/Ff_V:5S[X1uFi0k;ci4;0P]<lGi^G%Ll]I<p=cjrY(aJ_%G`0o$s$41[S%8I<5&x':oDM]#03Ys]G3er-B5ThmfGUqUE3G;GC5rFxUClEvsB48vLF4</jF"
"<?x5'-KFVCRmdgD,#Wx&6=ofG:4G&#+IZ$$-MYY#5`>>#&D(v#,ubA#vOX&#VdJe$,$k-$6Bk-$:Ld---&<X(K.r-$.sZw'uue+MAg'W.E]=o[i)'k0Z`/c.#####Nr@'cPV=817;(##"
"";


static const char* GetDefaultCompressedFontDataTTFBase85()
{
    return Cousine_Regular_ttf_compressed_data_base85;
}
