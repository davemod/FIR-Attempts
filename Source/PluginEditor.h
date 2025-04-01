/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class FIRAttemptsAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    FIRAttemptsAudioProcessorEditor (FIRAttemptsAudioProcessor&);
    ~FIRAttemptsAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FIRAttemptsAudioProcessor& audioProcessor;
    std::unique_ptr<Component> ui;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FIRAttemptsAudioProcessorEditor)
};
