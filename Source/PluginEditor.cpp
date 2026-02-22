/*
  ==============================================================================
    This file contains the basic framework code for a JUCE plugin editor.
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewLouderSaturator_Feb21AudioProcessorEditor::NewLouderSaturator_Feb21AudioProcessorEditor (NewLouderSaturator_Feb21AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& name) {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setLookAndFeel (&customLookAndFeel);
        addAndMakeVisible (slider);

        label.setText (name, juce::dontSendNotification);
        label.setFont (juce::FontOptions (12.0f)); 
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
    };

    setupSlider (driveSlider, driveLabel, "DRIVE");
    setupSlider (reverbSlider, reverbLabel, "REVERB");
    setupSlider (toneSlider, toneLabel, "TONE");
    setupSlider (widthSlider, widthLabel, "WIDTH");
    setupSlider (mixSlider, mixLabel, "MIX");
    setupSlider (outputSlider, outputLabel, "OUTPUT");

    prePostButton.setButtonText ("PRE");
    prePostButton.setClickingTogglesState (true);
    prePostButton.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    prePostButton.setColour (juce::ToggleButton::tickColourId, juce::Colour (0xFF0087FF));
    addAndMakeVisible (prePostButton);

    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "drive", driveSlider);
    reverbAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "reverb", reverbSlider);
    toneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "tone", toneSlider);
    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "width", widthSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "mix", mixSlider);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "output", outputSlider);
    prePostAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.apvts, "prePost", prePostButton);

    setSize (500, 450);
    
    // ---> NEW: Start the UI refresh timer at 30 frames per second <---
    startTimerHz(30);
}

NewLouderSaturator_Feb21AudioProcessorEditor::~NewLouderSaturator_Feb21AudioProcessorEditor()
{

    stopTimer(); // <--- ADD THIS LINE TO PREVENT THE CRASH

    driveSlider.setLookAndFeel (nullptr);
    reverbSlider.setLookAndFeel (nullptr);
    toneSlider.setLookAndFeel (nullptr);
    widthSlider.setLookAndFeel (nullptr);
    mixSlider.setLookAndFeel (nullptr);
    outputSlider.setLookAndFeel (nullptr);
}

// ---> NEW: The Timer Loop <---
void NewLouderSaturator_Feb21AudioProcessorEditor::timerCallback()
{
    // A decay of 0.85f gives it that smooth, vintage LED drop-off
    smoothInputLevel = juce::jmax(audioProcessor.inputLevel.load(), smoothInputLevel * 0.85f);
    smoothOutputLevel = juce::jmax(audioProcessor.outputLevel.load(), smoothOutputLevel * 0.85f);
    repaint();
}

//==============================================================================
void NewLouderSaturator_Feb21AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF1A1A1A)); // Flat dark background

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (20.0f).withName ("Helvetica Neue").withStyle ("Bold")); 
    g.drawText ("a LOUDER Saturator", getLocalBounds().removeFromTop (50), juce::Justification::centred);

    // ---> NEW: Draw the Meters <---
    int meterWidth = 8;
    int meterHeight = getHeight() - 100;
    int meterY = 60;

    // 1. Input Meter Background (Left Edge)
    g.setColour (juce::Colour (0xFF2D2D2D));
    g.fillRect (10, meterY, meterWidth, meterHeight);
    
    // Input Meter Fill
    g.setColour (juce::Colour (0xFF0087FF));
    float inFill = meterHeight * juce::jmin (1.0f, smoothInputLevel);
    g.fillRect (10.0f, meterY + meterHeight - inFill, (float)meterWidth, inFill);

    // 2. Output Meter Background (Right Edge)
    g.setColour (juce::Colour (0xFF2D2D2D));
    g.fillRect (getWidth() - meterWidth - 10, meterY, meterWidth, meterHeight);

    // Output Meter Fill
    g.setColour (juce::Colour (0xFF0087FF));
    float outFill = meterHeight * juce::jmin (1.0f, smoothOutputLevel);
    g.fillRect ((float)(getWidth() - meterWidth - 10), meterY + meterHeight - outFill, (float)meterWidth, outFill);
}

void NewLouderSaturator_Feb21AudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (35, 0); // Increased margin to make room for meters
    area.removeFromTop (60); // Header space

    auto mainRow = area.removeFromTop (area.getHeight() * 0.6f);
    auto bottomRow = area;

    // Center Column (Large Drive)
    auto centerWidth = mainRow.getWidth() * 0.4f;
    auto driveArea = mainRow.withSizeKeepingCentre (centerWidth, centerWidth);
    driveSlider.setBounds (driveArea);
    driveLabel.setBounds (driveArea.translated (0, driveArea.getHeight() * 0.5f + 5).withHeight (20));

    // Flanking (Medium Reverb & Tone)
    auto sideWidth = (mainRow.getWidth() - centerWidth) * 0.5f;
    auto sideSize = sideWidth * 0.8f;

    auto reverbArea = mainRow.removeFromLeft (sideWidth).withSizeKeepingCentre (sideSize, sideSize);
    reverbSlider.setBounds (reverbArea);
    reverbLabel.setBounds (reverbArea.translated (0, reverbArea.getHeight() * 0.5f + 5).withHeight (20));

    auto toneArea = mainRow.removeFromRight (sideWidth).withSizeKeepingCentre (sideSize, sideSize);
    toneSlider.setBounds (toneArea);
    toneLabel.setBounds (toneArea.translated (0, toneArea.getHeight() * 0.5f + 5).withHeight (20));

    // Button Position
    prePostButton.setBounds (reverbArea.translated (0, -40).withHeight (30).withWidth (60));

    // Bottom Row (Small Width, Mix, Output)
    auto smallSize = bottomRow.getWidth() / 3.0f;
    auto sliderSize = smallSize * 0.6f;

    auto widthArea = bottomRow.removeFromLeft (smallSize).withSizeKeepingCentre (sliderSize, sliderSize);
    widthSlider.setBounds (widthArea);
    widthLabel.setBounds (widthArea.translated (0, widthArea.getHeight() * 0.5f + 5).withHeight (20));

    auto mixArea = bottomRow.removeFromLeft (smallSize).withSizeKeepingCentre (sliderSize, sliderSize);
    mixSlider.setBounds (mixArea);
    mixLabel.setBounds (mixArea.translated (0, mixArea.getHeight() * 0.5f + 5).withHeight (20));

    auto outputArea = bottomRow.removeFromLeft (smallSize).withSizeKeepingCentre (sliderSize, sliderSize);
    outputSlider.setBounds (outputArea);
    outputLabel.setBounds (outputArea.translated (0, outputArea.getHeight() * 0.5f + 5).withHeight (20));
}