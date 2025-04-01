#pragma once

#include <JuceHeader.h>

class AutoUI : public juce::Component
{
public:
    AutoUI (AudioProcessor& processor);

    void paint (Graphics& g) override;
    void resized () override;

private:
    AudioProcessor& processor;

    OwnedArray<Component> components;
    OwnedArray<ParameterAttachment> attachments;
    OwnedArray<hill::AttachedNameLabel> labels;

    String lastName;
    
    TextButton infoButton { "i", "Info" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutoUI)
};