#include "PluginProcessor.h"
#include "PluginEditor.h"

NewLouderSaturator_Feb21AudioProcessorEditor::NewLouderSaturator_Feb21AudioProcessorEditor (NewLouderSaturator_Feb21AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel (&customLookAndFeel);

    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& name, juce::Colour color) {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setColour (juce::Slider::thumbColourId, color); 
        addAndMakeVisible (slider);
        
        label.setText (name, juce::dontSendNotification);
        label.setFont (juce::FontOptions (11.0f).withStyle("Bold")); 
        label.setColour(juce::Label::textColourId, juce::Colours::grey);
        label.setJustificationType (juce::Justification::centredTop); 
        addAndMakeVisible (label);
    };

    juce::Colour satColor   = juce::Colour (0xFF0087FF); 
    juce::Colour revColor   = juce::Colour (0xFF4DB8FF); 
    juce::Colour utilColor  = juce::Colour (0xFF0055AA); 

    satSectionLabel.setText("SATURATION", juce::dontSendNotification);
    satSectionLabel.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    satSectionLabel.setColour(juce::Label::textColourId, satColor);
    satSectionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(satSectionLabel);

    revSectionLabel.setText("REVERB", juce::dontSendNotification);
    revSectionLabel.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    revSectionLabel.setColour(juce::Label::textColourId, revColor);
    revSectionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(revSectionLabel);

    setupSlider (driveSlider, driveLabel, "DRIVE", satColor);
    setupSlider (toneSlider, toneLabel, "TONE", satColor);
    setupSlider (reverbSlider, reverbLabel, "AMOUNT", revColor);
    setupSlider (decaySlider, decayLabel, "DECAY", revColor);
    setupSlider (dampingSlider, dampingLabel, "DAMPING", revColor);
    setupSlider (inputSlider, inputLabel, "INPUT", utilColor);
    setupSlider (mixSlider, mixLabel, "MIX", utilColor);
    setupSlider (widthSlider, widthLabel, "WIDTH", utilColor);
    setupSlider (outputSlider, outputLabel, "OUTPUT", utilColor);

    prePostButton.setName("PrePostButton");
    addAndMakeVisible (prePostButton);

    bypassButton.setName("BypassButton");
    addAndMakeVisible (bypassButton);

    reverbTypeCombo.addItem("Room", 1);
    reverbTypeCombo.addItem("Hall", 2);
    reverbTypeCombo.addItem("Plate", 3);
    reverbTypeCombo.setJustificationType(juce::Justification::centred);
    reverbTypeCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF2D2D2D));
    reverbTypeCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF3A3A3A));
    addAndMakeVisible(reverbTypeCombo);

    inputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "input", inputSlider);
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "drive", driveSlider);
    reverbAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "reverb", reverbSlider);
    toneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "tone", toneSlider);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "decay", decaySlider);
    dampingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "damping", dampingSlider);
    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "width", widthSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "mix", mixSlider);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "output", outputSlider);
    
    // ---> THE FIX: Hooking the ButtonAttachment up to the newly renamed ID <---
    prePostAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.apvts, "prePostSwitch", prePostButton);
    
    typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, "reverbType", reverbTypeCombo);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.apvts, "bypass", bypassButton);

    setSize (640, 520); 
    startTimerHz(30);
}

NewLouderSaturator_Feb21AudioProcessorEditor::~NewLouderSaturator_Feb21AudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr); 
}

void NewLouderSaturator_Feb21AudioProcessorEditor::timerCallback()
{
    smoothInputLevel = juce::jmax(audioProcessor.inputLevel.load(), smoothInputLevel * 0.85f);
    smoothOutputLevel = juce::jmax(audioProcessor.outputLevel.load(), smoothOutputLevel * 0.85f);
    repaint();
}

void NewLouderSaturator_Feb21AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF1A1A1A)); 
    
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (22.0f).withName ("Helvetica Neue").withStyle ("Bold")); 
    g.drawText ("a LOUDER Saturator", getLocalBounds().removeFromTop (50), juce::Justification::centred);

    int meterWidth = 10;
    int meterHeight = getHeight() - 120;
    int meterY = 70;

    g.setFont (juce::FontOptions (10.0f).withStyle ("Bold"));
    g.setColour(juce::Colours::grey);
    g.drawText("IN", 15, meterY - 20, 20, 20, juce::Justification::centred);
    g.drawText("OUT", getWidth() - 35, meterY - 20, 20, 20, juce::Justification::centred);

    g.setColour (juce::Colour (0xFF2D2D2D));
    g.fillRect (20, meterY, meterWidth, meterHeight);
    g.setColour (juce::Colour (0xFF0087FF));
    float inFill = meterHeight * juce::jmin (1.0f, smoothInputLevel);
    g.fillRect (20.0f, meterY + meterHeight - inFill, (float)meterWidth, inFill);

    g.setColour (juce::Colour (0xFF2D2D2D));
    g.fillRect (getWidth() - 30, meterY, meterWidth, meterHeight);
    g.setColour (juce::Colour (0xFF0087FF));
    float outFill = meterHeight * juce::jmin (1.0f, smoothOutputLevel);
    g.fillRect ((float)(getWidth() - 30), meterY + meterHeight - outFill, (float)meterWidth, outFill);
}

void NewLouderSaturator_Feb21AudioProcessorEditor::resized()
{
    auto bindKnob = [](juce::Slider& s, juce::Label& l, juce::Rectangle<int> bounds) {
        l.setBounds (bounds.removeFromBottom (20)); 
        s.setBounds (bounds);                       
    };

    bypassButton.setBounds (15, 15, 60, 20); 

    auto area = getLocalBounds().reduced (55, 0); 
    area.removeFromTop (60); 
    auto bottomRow = area.removeFromBottom (100); 

    auto satArea = area.removeFromLeft (area.getWidth() / 2.0f).reduced(10);
    auto revArea = area.reduced(10);

    satSectionLabel.setBounds(satArea.removeFromTop(30));
    revSectionLabel.setBounds(revArea.removeFromTop(30));

    bindKnob (driveSlider, driveLabel, satArea.removeFromTop (160).withSizeKeepingCentre (140, 160));
    bindKnob (toneSlider, toneLabel, satArea.removeFromTop (100).withSizeKeepingCentre (90, 110));   

    reverbTypeCombo.setBounds(revArea.removeFromTop(20).withSizeKeepingCentre(75, 20));
    revArea.removeFromTop(12); 
    
    bindKnob (reverbSlider, reverbLabel, revArea.removeFromTop (140).withSizeKeepingCentre (130, 150));
    
    auto revMedRow = revArea.removeFromTop(100);
    bindKnob (decaySlider, decayLabel, revMedRow.removeFromLeft(revMedRow.getWidth() / 2.0f).withSizeKeepingCentre (90, 110)); 
    bindKnob (dampingSlider, dampingLabel, revMedRow.withSizeKeepingCentre (90, 110)); 
    
    revArea.removeFromTop(5); 
    prePostButton.setBounds(revArea.removeFromTop(30).withSizeKeepingCentre(60, 20)); 

    float smallW = bottomRow.getWidth() / 4.0f;
    bindKnob (inputSlider, inputLabel, bottomRow.removeFromLeft(smallW).withSizeKeepingCentre(70, 90));   
    bindKnob (mixSlider, mixLabel, bottomRow.removeFromLeft(smallW).withSizeKeepingCentre(70, 90));       
    bindKnob (widthSlider, widthLabel, bottomRow.removeFromLeft(smallW).withSizeKeepingCentre(70, 90));   
    bindKnob (outputSlider, outputLabel, bottomRow.withSizeKeepingCentre(70, 90));                        
}