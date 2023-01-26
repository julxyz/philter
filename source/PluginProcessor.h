/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Params {

    enum Names {
        Filter_Cutoff,
        Filter_Resonance,
        Enable_Autogain,
        Bypass,
        DryWet,
        Position,
    };

    inline const std::map<Names, juce::String>& GetParams() {
        static std::map<Names, juce::String> params = {
            {Filter_Cutoff, "Cutoff"},
            {Filter_Resonance, "Resonance"},
            {Enable_Autogain, "Autogain"},
            {Bypass, "Bypass"},
            {DryWet, "DryWet"},
            {Position, "Position"},
        };
        return params;
    };
}

typedef std::vector<float> PositionCoefficients;

//==============================================================================
/**
*/
class PhilterAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    PhilterAudioProcessor();
    ~PhilterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    void updateFilters();
    PositionCoefficients getPositionCoefficients();

    using APVTS = juce::AudioProcessorValueTreeState;
    static APVTS::ParameterLayout createParameterLayout();

    APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:
    double lastSampleRate;
    double autogain_previous;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lowpass_filter;
    juce::dsp::DryWetMixer<float> lowpass_filter_mixer;
    juce::dsp::DryWetMixer<float> dry_wet_mixer;
    juce::AudioParameterFloat* filter_cutoff { nullptr };
    juce::AudioParameterFloat* filter_resonance { nullptr };
    juce::AudioParameterFloat* drywet { nullptr };
    juce::AudioParameterFloat* position { nullptr };
    juce::AudioParameterBool* enable_autogain { nullptr };
    juce::AudioParameterBool* bypass { nullptr };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhilterAudioProcessor)
};
