/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PhilterAudioProcessor::PhilterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), lowpass_filter(juce::dsp::IIR::Coefficients<float>::makeLowPass(44100, 1000.0f, 1.0f))
#endif
{
    using namespace Params;
    const auto& params = GetParams();

    auto castBool = [&apvts = this->apvts, &params](auto &param, const auto &paramName) {
        param = dynamic_cast<juce::AudioParameterBool*> (apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    auto castFloat = [&apvts = this->apvts, &params](auto& param, const auto& paramName) {
        param = dynamic_cast<juce::AudioParameterFloat*> (apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };
    castFloat(filter_cutoff, Names::Filter_Cutoff);
    castFloat(filter_resonance, Names::Filter_Resonance);
    castBool(enable_autogain, Names::Enable_Autogain);
}

PhilterAudioProcessor::~PhilterAudioProcessor()
{
}

//==============================================================================
const juce::String PhilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}


bool PhilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PhilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PhilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PhilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PhilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PhilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PhilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PhilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void PhilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PhilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {

    lastSampleRate = sampleRate;
    autogain_previous = 0.0;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    lowpass_filter.prepare(spec);
    lowpass_filter.reset();

}

void PhilterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PhilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PhilterAudioProcessor::updateFilter() {
    *lowpass_filter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(lastSampleRate, filter_cutoff->get(), 0.1f*filter_resonance->get());
}

void PhilterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // clear dead channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    // autogain
    double inputGain = 0.0;
    for (int i = 0; i < buffer.getNumChannels(); ++i) {
        inputGain += buffer.getRMSLevel(i, 0, buffer.getNumSamples());
    }

    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    updateFilter();
    lowpass_filter.process(context);



    // autogain
    if (enable_autogain->get()) {
        double outputGain = 0.0;
        for (int i = 0; i < buffer.getNumChannels(); ++i)
            outputGain += buffer.getRMSLevel(i, 0, buffer.getNumSamples());

        auto makeup = (inputGain > 0.0) ? outputGain / inputGain : 1.0;
        for (int i = 0; i < buffer.getNumChannels(); ++i)
            buffer.applyGainRamp(i, 0, buffer.getNumSamples(), autogain_previous, makeup);

        DBG(enable_autogain->getCurrentValueAsText() << ": " << makeup);
        autogain_previous = makeup;
    }
}

//==============================================================================
bool PhilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PhilterAudioProcessor::createEditor()
{
    //return new PhilterAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void PhilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void PhilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        apvts.replaceState(tree);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout PhilterAudioProcessor::createParameterLayout() {
    // Create plugin parameters

    APVTS::ParameterLayout layout;
    using namespace juce; 
    using namespace Params;
    const auto& params = GetParams();

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Filter_Cutoff),
                                                     params.at(Names::Filter_Cutoff),
                                                     NormalisableRange<float>(10, 20000, 1, 0.25f),
                                                     600));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Filter_Resonance),
                                                     params.at(Names::Filter_Resonance),
                                                     NormalisableRange<float>(1.0f, 100, 1, 1),
                                                     10));
    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::Enable_Autogain), params.at(Names::Enable_Autogain), true));

    return layout;
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhilterAudioProcessor();
}
