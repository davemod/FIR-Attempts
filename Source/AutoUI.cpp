#include "AutoUI.h"

AutoUI::AutoUI(AudioProcessor &processor)
:
processor (processor)
{
    addAndMakeVisible (infoButton);
    infoButton.onClick = [&](){
        AlertWindow::showMessageBoxAsync (MessageBoxIconType::InfoIcon, "Version", String (ProjectInfo::projectName) + String(" v") + String (ProjectInfo::versionString));
    };

    for (auto p : processor.getParameters ())
    {
        if (p->isAutomatable ())
        {
            // add a sepearor first
            if (auto name = p->getName (100); name.isNotEmpty ())
            {
                if (lastName.isNotEmpty ())
                {
                    if (! name.startsWithChar (lastName[0]))
                    {
                        class SeparatorComponent : public juce::Component
                        {
                        public:
                            void paint (juce::Graphics& g) override
                            {
                                g.setColour (juce::Colours::grey);
                                g.drawLine (0, getHeight () / 2, getWidth (), getHeight () / 2);
                            }
                        };

                        auto separator = components.add (new SeparatorComponent ());
                        addAndMakeVisible (separator);
                    }
                }

                lastName = name;

            }




            // then create param component
            if (auto param = dynamic_cast<AudioParameterBool*> (p))
            {
                auto button = static_cast<juce::ToggleButton*> (components.add (new juce::ToggleButton ()));

                button->setName (param->getName (100));
                addAndMakeVisible (button);

                auto attachment = attachments.add (new juce::ParameterAttachment (*param, [button](float val){
                    button->setToggleState (val >= 0.5f, juce::dontSendNotification);
                }));

                button->onClick = [attachment, button](){
                    attachment->setValueAsCompleteGesture (button->getToggleState() ? 1.f : 0.f);
                };

                attachment->sendInitialUpdate ();

                labels.add (new hill::AttachedNameLabel (*button));
            }
            else if (auto param = dynamic_cast<AudioParameterChoice*> (p))
            {
                auto combo = static_cast<juce::ComboBox*> (components.add (new juce::ComboBox ()));
                combo->addItemList (param->choices, 1);

                combo->setName (param->getName (100));
                addAndMakeVisible (combo);

                auto attachment = attachments.add (new juce::ParameterAttachment (*param, [combo](float val){
                    combo->setSelectedId (roundToInt (val) + 1, juce::dontSendNotification);
                }));

                combo->onChange = [combo, attachment](){
                    attachment->setValueAsCompleteGesture (combo->getSelectedId () - 1);
                };

                attachment->sendInitialUpdate ();

                labels.add (new hill::AttachedNameLabel (*combo));
            }
            else if (auto param = dynamic_cast<AudioParameterFloat*> (p))
            {
                auto slider = static_cast<juce::Slider*> (components.add (new juce::Slider ()));

                slider->setName (param->getName (100));
                slider->setSliderStyle (juce::Slider::LinearBar);
                
                auto range = param->getNormalisableRange ();
                slider->setNormalisableRange ({ range.start, range.end, range.interval, range.skew });
                
                addAndMakeVisible (slider);

                auto attachment = attachments.add (new juce::ParameterAttachment (*param, [slider](float val){
                    slider->setValue (val);
                }));

                slider->onValueChange = [attachment, slider](){
                    attachment->setValueAsCompleteGesture (slider->getValue ());
                };

                attachment->sendInitialUpdate ();

                labels.add (new hill::AttachedNameLabel (*slider));
            }
            else if (auto param = dynamic_cast<AudioParameterInt*> (p))
            {
                auto slider = static_cast<juce::Slider*> (components.add (new juce::Slider ()));

                slider->setName (param->getName (100));
                slider->setSliderStyle (juce::Slider::LinearBar);
                
                auto range = param->getNormalisableRange ();
                slider->setNormalisableRange ({ range.start, range.end, range.interval, range.skew });
                
                addAndMakeVisible (slider);

                auto attachment = attachments.add (new juce::ParameterAttachment (*param, [slider](float val){
                    slider->setValue (val);
                }));

                slider->onValueChange = [attachment, slider](){
                    attachment->setValueAsCompleteGesture (slider->getValue ());
                };

                attachment->sendInitialUpdate ();

                labels.add (new hill::AttachedNameLabel (*slider));
            }
        }
    }
}

void AutoUI::paint(Graphics &g)
{
}

void AutoUI::resized()
{
    auto area = getLocalBounds ().reduced (8);
    auto leftArea = area.removeFromLeft (150);

    infoButton.setBounds (leftArea.withSize (25, 25).reduced (1));

    auto h = area.getHeight () / components.size ();
    for (auto c : components)
        c->setBounds (area.removeFromTop (h));
}
