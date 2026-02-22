/*
  ==============================================================================
    This file contains the basic framework code for a JUCE plugin editor.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        setColour (juce::Slider::thumbColourId, juce::Colour::fromFloatRGBA (0.0f, 0.53f, 1.0f, 1.0f));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto fill    = juce::Colour (0xFF0087FF); // #0087ff (Electric Blue)

        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (10);
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = 6.0f;
        auto arcRadius = radius - lineW * 0.5f;

        // Background circle (flat matte)
        g.setColour (juce::Colour (0xFF2D2D2D));
        g.fillEllipse (bounds.getCentreX() - radius, bounds.getCentreY() - radius, radius * 2.0f, radius * 2.0f);

        juce::Path backgroundArc;
        backgroundArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);

        g.setColour (juce::Colour (0xFF3A3A3A));
        g.strokePath (backgroundArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        if (slider.isEnabled())
        {
            juce::Path valueArc;
            valueArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);
            
            g.setColour (fill);
            g.strokePath (valueArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Indicator line
        g.setColour (juce::Colours::white);
        juce::Path p;
        auto pointerLength = radius * 0.3f;
        auto pointerThickness = 2.0f;
        p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform (juce::AffineTransform::rotation (toAngle).translated (bounds.getCentreX(), bounds.getCentreY()));
        g.fillPath (p);
    }
};

//==============================================================================
// ---> NEW: Inherit from juce::Timer <---
class NewLouderSaturator_Feb21AudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    NewLouderSaturator_Feb21AudioProcessorEditor (NewLouderSaturator_Feb21AudioProcessor&);
    ~NewLouderSaturator_Feb21AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // ---> NEW: Timer callback for meters <---
    void timerCallback() override;

private:
    CustomLookAndFeel customLookAndFeel;

    juce::Slider driveSlider, reverbSlider, toneSlider, widthSlider, mixSlider, outputSlider, sizeSlider, dampingSlider;
    juce::ToggleButton prePostButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment, reverbAttachment, toneAttachment, widthAttachment, mixAttachment, outputAttachment, sizeAttachment, dampingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> prePostAttachment;

    juce::Label driveLabel, reverbLabel, toneLabel, widthLabel, mixLabel, outputLabel, sizeLabel, dampingLabel;

    NewLouderSaturator_Feb21AudioProcessor& audioProcessor;

    // ---> NEW: Smoothing variables for bouncy LED meters <---
    float smoothInputLevel = 0.0f;
    float smoothOutputLevel = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewLouderSaturator_Feb21AudioProcessorEditor)
};