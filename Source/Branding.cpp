// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "Branding.h"

#include <BinaryData.h>

namespace
{
    const juce::Colour warm1 { 0xffffb35c };
    const juce::Colour warm2 { 0xffff7a3d };

    juce::Font boldFont (float height, float kerning = 0.0f)
    {
        juce::Font f (juce::FontOptions (height).withStyle ("Bold"));
        f.setExtraKerningFactor (kerning);
        return f;
    }

    /** Draws text with the forward slant of the FS logo. */
    void drawSlanted (juce::Graphics& g, const juce::String& text, juce::Rectangle<float> area, const juce::Font& font)
    {
        juce::GlyphArrangement glyphs;
        glyphs.addFittedText (font, text, area.getX(), area.getY(), area.getWidth(), area.getHeight(),
                              juce::Justification::centredLeft, 1, 1.0f);

        juce::Path path;
        glyphs.createPath (path);
        path.applyTransform (juce::AffineTransform::shear (-0.21f, 0.0f)
                                 .translated (0.21f * area.getCentreY(), 0.0f));
        g.fillPath (path);
    }
}

namespace Branding
{
    std::unique_ptr<juce::Drawable> createIcon()
    {
        return juce::Drawable::createFromImageData (BinaryData::fgp_icon_svg, BinaryData::fgp_icon_svgSize);
    }

    juce::Image artistLogo()
    {
        return juce::ImageCache::getFromMemory (BinaryData::fs_pitch_logo_png, BinaryData::fs_pitch_logo_pngSize);
    }

    void drawWordmark (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const float h = area.getHeight();
        auto top = area.removeFromTop (h * 0.64f);
        area.removeFromTop (h * 0.06f);

        g.setGradientFill (juce::ColourGradient (warm1, top.getX(), top.getY(),
                                                 warm2, top.getX() + top.getHeight() * 3.2f, top.getBottom(), false));
        drawSlanted (g, "FASE", top, boldFont (top.getHeight() * 1.18f, 0.04f));

        g.setColour (juce::Colours::white);
        drawSlanted (g, "GROOVE PIANO", area, boldFont (area.getHeight() * 1.05f, 0.32f));
    }

    void drawFullLogo (juce::Graphics& g, juce::Rectangle<float> area)
    {
        auto iconArea = area.removeFromLeft (area.getHeight());
        if (auto icon = createIcon())
            icon->drawWithin (g, iconArea, juce::RectanglePlacement::centred, 1.0f);

        area.removeFromLeft (area.getHeight() * 0.18f);
        drawWordmark (g, area.reduced (0.0f, area.getHeight() * 0.08f));
    }
}
