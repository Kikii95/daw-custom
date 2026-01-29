#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

namespace Theme
{
    //=========================================================================
    // BACKGROUNDS
    //=========================================================================
    constexpr juce::uint32 bgDark       = 0xff1a1a1a;  // Main background
    constexpr juce::uint32 bgPanel      = 0xff252525;  // Panel backgrounds
    constexpr juce::uint32 bgSlot       = 0xff2a2a2a;  // Effect slots, cards
    constexpr juce::uint32 bgHover      = 0xff333333;  // Hover state
    constexpr juce::uint32 bgSelected   = 0xff3a3a3a;  // Selected state
    constexpr juce::uint32 bgHeader     = 0xff1f1f1f;  // Headers, toolbars

    //=========================================================================
    // ACCENTS
    //=========================================================================
    constexpr juce::uint32 accentBlue   = 0xff64b5f6;  // Primary accent
    constexpr juce::uint32 accentOrange = 0xffff9800;  // Bypass, warnings
    constexpr juce::uint32 accentRed    = 0xfff44336;  // Delete, errors
    constexpr juce::uint32 accentGreen  = 0xff4caf50;  // Success, play
    constexpr juce::uint32 accentPurple = 0xffba68c8;  // Synth, creative

    //=========================================================================
    // TEXT COLORS
    //=========================================================================
    constexpr juce::uint32 textPrimary   = 0xffe0e0e0;  // Main text
    constexpr juce::uint32 textSecondary = 0xff9e9e9e;  // Secondary text
    constexpr juce::uint32 textDim       = 0xff616161;  // Disabled, hints
    constexpr juce::uint32 textOnAccent  = 0xff1a1a1a;  // Text on accent bg

    //=========================================================================
    // WAVEFORM COLORS
    //=========================================================================
    constexpr juce::uint32 waveformFill     = 0xff64b5f6;  // Waveform fill
    constexpr juce::uint32 waveformOutline  = 0xff90caf9;  // Waveform outline
    constexpr juce::uint32 waveformBg       = 0xff2a2a2a;  // Waveform background

    //=========================================================================
    // METER COLORS
    //=========================================================================
    constexpr juce::uint32 meterLow     = 0xff4caf50;  // Green (safe)
    constexpr juce::uint32 meterMid     = 0xffffeb3b;  // Yellow (warning)
    constexpr juce::uint32 meterHigh    = 0xfff44336;  // Red (clipping)
    constexpr juce::uint32 meterBg      = 0xff1a1a1a;  // Meter background

    //=========================================================================
    // BORDERS & DIVIDERS
    //=========================================================================
    constexpr juce::uint32 border       = 0xff3a3a3a;  // Subtle borders
    constexpr juce::uint32 borderLight  = 0xff4a4a4a;  // Lighter borders
    constexpr juce::uint32 divider      = 0xff2a2a2a;  // Divider lines

    //=========================================================================
    // SPACING (pixels)
    //=========================================================================
    constexpr int spacing_xs = 4;
    constexpr int spacing_sm = 8;
    constexpr int spacing_md = 16;
    constexpr int spacing_lg = 24;
    constexpr int spacing_xl = 32;

    //=========================================================================
    // SIZING (pixels)
    //=========================================================================
    constexpr int buttonHeight      = 32;
    constexpr int buttonHeightSmall = 24;
    constexpr int sliderWidth       = 60;
    constexpr int sliderHeight      = 80;
    constexpr int headerHeight      = 40;
    constexpr int channelWidth      = 80;
    constexpr int trackHeight       = 100;
    constexpr int effectSlotHeight  = 120;

    //=========================================================================
    // CORNER RADIUS
    //=========================================================================
    constexpr float cornerRadius    = 6.0f;
    constexpr float cornerRadiusSm  = 4.0f;
    constexpr float cornerRadiusLg  = 8.0f;

    //=========================================================================
    // HELPER FUNCTIONS
    //=========================================================================
    inline juce::Colour colour(juce::uint32 argb)
    {
        return juce::Colour(argb);
    }

    inline juce::Colour withAlpha(juce::uint32 argb, float alpha)
    {
        return juce::Colour(argb).withAlpha(alpha);
    }

    //=========================================================================
    // PRESET COLOURS (for convenience)
    //=========================================================================
    namespace Colours
    {
        inline juce::Colour background()     { return colour(bgDark); }
        inline juce::Colour panel()          { return colour(bgPanel); }
        inline juce::Colour slot()           { return colour(bgSlot); }
        inline juce::Colour hover()          { return colour(bgHover); }
        inline juce::Colour selected()       { return colour(bgSelected); }

        inline juce::Colour accent()         { return colour(accentBlue); }
        inline juce::Colour warning()        { return colour(accentOrange); }
        inline juce::Colour error()          { return colour(accentRed); }
        inline juce::Colour success()        { return colour(accentGreen); }

        inline juce::Colour text()           { return colour(textPrimary); }
        inline juce::Colour textMuted()      { return colour(textSecondary); }
        inline juce::Colour textDisabled()   { return colour(textDim); }
    }
}
