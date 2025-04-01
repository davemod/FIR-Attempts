/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "AutoUI.h"

//==============================================================================
FIRAttemptsAudioProcessorEditor::FIRAttemptsAudioProcessorEditor (FIRAttemptsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{    
    ui = std::make_unique<AutoUI> (audioProcessor);
    addAndMakeVisible (ui.get());

    setSize (400, 300);
    setResizable (true, true);
}

FIRAttemptsAudioProcessorEditor::~FIRAttemptsAudioProcessorEditor()
{
}

//==============================================================================
void FIRAttemptsAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void FIRAttemptsAudioProcessorEditor::resized()
{
    ui->setBounds (getLocalBounds ());
}
