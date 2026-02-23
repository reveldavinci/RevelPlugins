#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel() {}

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto fill = slider.findColour (juce::Slider::thumbColourId); 
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (12);
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = 6.0f;
        auto arcRadius = radius - lineW * 0.5f;

        int numTicks = 11;
        g.setColour (juce::Colour (0xFF444444)); 
        for (int i = 0; i < numTicks; ++i)
        {
            float tickPos = i / (float)(numTicks - 1);
            float angle = rotaryStartAngle + tickPos * (rotaryEndAngle - rotaryStartAngle);
            juce::Path tick;
            tick.addLineSegment (juce::Line<float> (0, -(radius + 3.0f), 0, -(radius + 7.0f)), 1.5f);
            g.fillPath (tick, juce::AffineTransform::rotation (angle).translated (bounds.getCentreX(), bounds.getCentreY()));
        }

        g.setColour (juce::Colour (0xFF2D2D2D));
        g.fillEllipse (bounds.getCentreX() - radius, bounds.getCentreY() - radius, radius * 2.0f, radius * 2.0f);

        juce::Path backgroundArc;
        backgroundArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (juce::Colour (0xFF3A3A3A));
        g.strokePath (backgroundArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        if (slider.isEnabled()) {
            juce::Path valueArc;
            valueArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);
            g.setColour (fill);
            g.strokePath (valueArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        g.setColour (juce::Colours::white);
        juce::Path p;
        auto pointerLength = radius * 0.3f;
        auto pointerThickness = 2.0f;
        p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform (juce::AffineTransform::rotation (toAngle).translated (bounds.getCentreX(), bounds.getCentreY()));
        g.fillPath (p);
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, 
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (2.0f);
        bool state = button.getToggleState(); 

        if (button.getName() == "BypassButton")
        {
            g.setColour (state ? juce::Colour (0xFFE53935) : juce::Colour (0xFF2D2D2D)); 
            g.fillRoundedRectangle (bounds, 4.0f);
            
            g.setColour (juce::Colour (0xFF3A3A3A));
            g.drawRoundedRectangle (bounds, 4.0f, 2.0f);
            
            g.setFont (juce::FontOptions (10.0f).withStyle ("Bold"));
            g.setColour (state ? juce::Colour (0xFF1A1A1A) : juce::Colours::grey);
            g.drawText ("BYPASS", bounds, juce::Justification::centred);
        }
        else 
        {
            g.setColour (state ? juce::Colour (0xFF4DB8FF) : juce::Colour (0xFF2D2D2D));
            g.fillRoundedRectangle (bounds, bounds.getHeight() * 0.5f);
            
            g.setColour (juce::Colour (0xFF3A3A3A));
            g.drawRoundedRectangle (bounds, bounds.getHeight() * 0.5f, 2.0f);

            auto thumbRadius = bounds.getHeight() * 0.5f - 2.0f;
            auto thumbX = state ? bounds.getRight() - thumbRadius * 2.0f - 2.0f : bounds.getX() + 2.0f;
            g.setColour (juce::Colours::white);
            g.fillEllipse (thumbX, bounds.getY() + 2.0f, thumbRadius * 2.0f, thumbRadius * 2.0f);

            g.setFont (juce::FontOptions (10.0f).withStyle ("Bold"));
            g.setColour (state ? juce::Colour (0xFF1A1A1A) : juce::Colours::grey);
            if (state) {
                g.drawText ("POST", bounds.withTrimmedRight (bounds.getHeight()), juce::Justification::centred);
            } else {
                g.drawText ("PRE", bounds.withTrimmedLeft (bounds.getHeight()), juce::Justification::centred);
            }
        }
    }
};

class NewLouderSaturator_Feb21AudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    NewLouderSaturator_Feb21AudioProcessorEditor (NewLouderSaturator_Feb21AudioProcessor&);
    ~NewLouderSaturator_Feb21AudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    // IMPORTANT: This must remain at the top of the private section!
    CustomLookAndFeel customLookAndFeel;
    
    juce::Slider inputSlider, driveSlider, reverbSlider, toneSlider;
    juce::Slider decaySlider, dampingSlider, widthSlider, mixSlider, outputSlider; 
    juce::ToggleButton prePostButton, bypassButton;
    juce::ComboBox reverbTypeCombo; 

    juce::Label satSectionLabel, revSectionLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputAttachment, driveAttachment, reverbAttachment, toneAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment, dampingAttachment, widthAttachment, mixAttachment, outputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> prePostAttachment, bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment;

    juce::Label inputLabel, driveLabel, reverbLabel, toneLabel;
    juce::Label decayLabel, dampingLabel, widthLabel, mixLabel, outputLabel;

    NewLouderSaturator_Feb21AudioProcessor& audioProcessor;
    float smoothInputLevel = 0.0f;
    float smoothOutputLevel = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewLouderSaturator_Feb21AudioProcessorEditor)
};