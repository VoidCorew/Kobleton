#pragma once

#include <JuceHeader.h>

class MainComponent : public juce::AudioAppComponent,
                      private juce::Button::Listener,
                      private juce::Slider::Listener
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    juce::Colour backgroundColor = juce::Colour::fromRGB(30, 30, 30);

    juce::TextButton myButton;
    juce::Slider frequencySlider;

    juce::Slider volumeSlider;
    juce::ToggleButton powerButton;

    enum class WaveType
    {
        Sine,
        Square,
        Saw
    };
    WaveType currentWaveType = WaveType::Sine;
    juce::ComboBox waveSelector;

    double phase = 0.0;
    float volume = 0.2f;
    float frequency = 440.0f;

    bool isPlaying = true;
    float currentVolume = 0.2f;

    void buttonClicked(juce::Button *button) override;
    void sliderValueChanged(juce::Slider *slider) override;

    double currentSampleRate = 0.0;
    double currentAngle = 0.0;
    double angleDelta = 0.0;

    // ADSR
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;

    void startTone();
    void stopTone();

    juce::TextButton playButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent);
};
