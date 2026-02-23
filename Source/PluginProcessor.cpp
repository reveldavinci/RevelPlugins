#include "PluginProcessor.h"
#include "PluginEditor.h"

NewLouderSaturator_Feb21AudioProcessor::NewLouderSaturator_Feb21AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
        apvts (*this, nullptr, "Parameters", createParameterLayout())
#else
     : apvts (*this, nullptr, "Parameters", createParameterLayout())
#endif
{}
NewLouderSaturator_Feb21AudioProcessor::~NewLouderSaturator_Feb21AudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout NewLouderSaturator_Feb21AudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    juce::NormalisableRange<float> gainRange(-100.0f, 24.0f, 0.1f);
    gainRange.setSkewForCentre(0.0f); 

    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "bypass", 1 }, "Bypass", false));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "input", 1 }, "Input", gainRange, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "drive", 1 }, "Drive", 0.0f, 10.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "reverb", 1 }, "Reverb", 0.0f, 100.0f, 0.0f));
    
    // ---> THE FIX: Proper Bool parameter and renamed ID to bust Ableton's cache <---
    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "prePostSwitch", 1 }, "Pre/Post", false));
    
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "reverbType", 1 }, "Reverb Type", juce::StringArray { "Room", "Hall", "Plate" }, 0));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "decay", 1 }, "Decay", 0.0f, 100.0f, 50.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "damping", 1 }, "Damping", 0.0f, 100.0f, 50.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "tone", 1 }, "Tone", -100.0f, 100.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "width", 1 }, "Width", 0.0f, 200.0f, 100.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "mix", 1 }, "Mix", 0.0f, 100.0f, 100.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "output", 1 }, "Output", gainRange, 0.0f));
    return layout;
}

const juce::String NewLouderSaturator_Feb21AudioProcessor::getName() const { return JucePlugin_Name; }
bool NewLouderSaturator_Feb21AudioProcessor::acceptsMidi() const { return false; }
bool NewLouderSaturator_Feb21AudioProcessor::producesMidi() const { return false; }
bool NewLouderSaturator_Feb21AudioProcessor::isMidiEffect() const { return false; }
double NewLouderSaturator_Feb21AudioProcessor::getTailLengthSeconds() const { return 1.0; } 
int NewLouderSaturator_Feb21AudioProcessor::getNumPrograms() { return 1; }
int NewLouderSaturator_Feb21AudioProcessor::getCurrentProgram() { return 0; }
void NewLouderSaturator_Feb21AudioProcessor::setCurrentProgram (int index) {}
const juce::String NewLouderSaturator_Feb21AudioProcessor::getProgramName (int index) { return {}; }
void NewLouderSaturator_Feb21AudioProcessor::changeProgramName (int index, const juce::String& newName) {}

void NewLouderSaturator_Feb21AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    reverb.setSampleRate (sampleRate);
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = 1; 
    for (int i = 0; i < 2; ++i) {
        toneFilter[i].prepare (spec);
        toneFilter[i].setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        toneFilter[i].reset();
    }
    
    dryBuffer.setSize (2, samplesPerBlock);
}

void NewLouderSaturator_Feb21AudioProcessor::releaseResources()
{
    reverb.reset();
    for (int i = 0; i < 2; ++i) toneFilter[i].reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewLouderSaturator_Feb21AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif
    return true;
  #endif
}
#endif

void NewLouderSaturator_Feb21AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels(); 

    if (numChannels == 0 || numSamples == 0) return;

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        if (i < numChannels) buffer.clear (i, 0, numSamples);
    }

    float maxInput = 0.0f;
    for (int ch = 0; ch < numChannels; ++ch) {
        maxInput = juce::jmax(maxInput, buffer.getMagnitude(ch, 0, numSamples));
    }
    inputLevel.store(maxInput);

    bool isBypassed = apvts.getRawParameterValue("bypass")->load() > 0.5f;

    if (!isBypassed) 
    {
        float inDB = apvts.getRawParameterValue("input")->load();
        float inputGain = (inDB <= -99.0f) ? 0.0f : juce::Decibels::decibelsToGain(inDB);
        buffer.applyGain(inputGain);

        float drive = apvts.getRawParameterValue("drive")->load();
        float reverbAmount = apvts.getRawParameterValue("reverb")->load() / 100.0f;
        
        // ---> THE FIX: Reading the new ID <---
        float prePost = apvts.getRawParameterValue("prePostSwitch")->load();
        
        float type = apvts.getRawParameterValue("reverbType")->load();
        float decay = apvts.getRawParameterValue("decay")->load() / 100.0f;
        float damping = apvts.getRawParameterValue("damping")->load() / 100.0f;
        float tone = apvts.getRawParameterValue("tone")->load();
        float width = apvts.getRawParameterValue("width")->load() / 100.0f; 
        float mix = apvts.getRawParameterValue("mix")->load() / 100.0f;
        float outDB = apvts.getRawParameterValue("output")->load();
        float outputGain = (outDB <= -99.0f) ? 0.0f : juce::Decibels::decibelsToGain(outDB);

        if (type == 0.0f) { // Room
            reverbParameters.roomSize = decay * 0.5f; 
            reverbParameters.damping = damping * 0.6f;
            reverbParameters.width = 0.8f;
        } else if (type == 1.0f) { // Hall
            reverbParameters.roomSize = 0.5f + (decay * 0.5f); 
            reverbParameters.damping = damping * 0.8f;
            reverbParameters.width = 1.0f;
        } else { // Plate
            reverbParameters.roomSize = decay * 0.7f;
            reverbParameters.damping = 0.2f + (damping * 0.7f); 
            reverbParameters.width = 0.5f; 
        }
        reverbParameters.wetLevel = reverbAmount;
        reverbParameters.dryLevel = 1.0f;
        reverbParameters.freezeMode = 0.0f;

        if (dryBuffer.getNumSamples() < numSamples) {
            dryBuffer.setSize (2, numSamples, false, false, true); 
        }
        for (int ch = 0; ch < numChannels; ++ch) {
            if (ch < 2) dryBuffer.copyFrom (ch, 0, buffer, ch, 0, numSamples);
        }

        if (prePost < 0.5f) // PRE
        {
            reverb.setParameters(reverbParameters);
            if (numChannels > 1) {
                reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), numSamples);
            } else {
                reverb.processMono(buffer.getWritePointer(0), numSamples);
            }

            if (drive > 0.0f) {
                for (int channel = 0; channel < numChannels; ++channel) {
                    auto* channelData = buffer.getWritePointer(channel);
                    for (int sample = 0; sample < numSamples; ++sample)
                        channelData[sample] = std::tanh(channelData[sample] * (1.0f + drive));
                }
            }
        }
        else // POST
        {
            if (drive > 0.0f) {
                for (int channel = 0; channel < numChannels; ++channel) {
                    auto* channelData = buffer.getWritePointer(channel);
                    for (int sample = 0; sample < numSamples; ++sample)
                        channelData[sample] = std::tanh(channelData[sample] * (1.0f + drive));
                }
            }

            reverb.setParameters(reverbParameters);
            if (numChannels > 1) {
                reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), numSamples);
            } else {
                reverb.processMono(buffer.getWritePointer(0), numSamples);
            }
        }

        if (tone != 0.0f)
        {
            float cutoffFrequency;
            if (tone < 0.0f) {
                cutoffFrequency = juce::jmap (tone, -100.0f, 0.0f, 200.0f, 20000.0f); 
                for (int i = 0; i < 2; ++i) toneFilter[i].setType (juce::dsp::StateVariableTPTFilterType::lowpass);
            } else {
                cutoffFrequency = juce::jmap (tone, 0.0f, 100.0f, 20.0f, 2000.0f); 
                for (int i = 0; i < 2; ++i) toneFilter[i].setType (juce::dsp::StateVariableTPTFilterType::highpass);
            }

            for (int i = 0; i < 2; ++i) toneFilter[i].setCutoffFrequency (cutoffFrequency);

            for (int channel = 0; channel < numChannels; ++channel)
            {
                if (channel < 2) 
                {
                    auto* channelData = buffer.getWritePointer(channel);
                    for (int sample = 0; sample < numSamples; ++sample)
                        channelData[sample] = toneFilter[channel].processSample (0, channelData[sample]);
                }
            }
        }

        if (numChannels > 1 && width != 1.0f) 
        {
            auto* leftChannel = buffer.getWritePointer (0);
            auto* rightChannel = buffer.getWritePointer (1);
            for (int sample = 0; sample < numSamples; ++sample)
            {
                float mid = (leftChannel[sample] + rightChannel[sample]) * 0.5f;
                float side = (leftChannel[sample] - rightChannel[sample]) * 0.5f;
                side *= width; 
                leftChannel[sample] = mid + side;
                rightChannel[sample] = mid - side;
            }
        }

        for (int channel = 0; channel < numChannels; ++channel)
        {
            buffer.applyGain(channel, 0, numSamples, mix);
            if (channel < 2) buffer.addFrom(channel, 0, dryBuffer, channel, 0, numSamples, 1.0f - mix);
        }

        buffer.applyGain(outputGain);
    } 

    float maxOutput = 0.0f;
    for (int ch = 0; ch < numChannels; ++ch) {
        maxOutput = juce::jmax(maxOutput, buffer.getMagnitude(ch, 0, numSamples));
    }
    outputLevel.store(maxOutput);
}

bool NewLouderSaturator_Feb21AudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* NewLouderSaturator_Feb21AudioProcessor::createEditor() { return new NewLouderSaturator_Feb21AudioProcessorEditor (*this); }

void NewLouderSaturator_Feb21AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void NewLouderSaturator_Feb21AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewLouderSaturator_Feb21AudioProcessor();
}