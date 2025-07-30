#include "MainComponent.h"

MainComponent::MainComponent()
{
    setSize(800, 600);

    addAndMakeVisible(myButton);
    myButton.setButtonText("Change Background Color");
    myButton.addListener(this);

    addAndMakeVisible(frequencySlider);
    frequencySlider.setRange(100.0, 1000.0);
    frequencySlider.setValue(frequency);
    frequencySlider.addListener(this);
    frequencySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);

    addAndMakeVisible(volumeSlider);
    volumeSlider.setRange(0.0, 1.0);
    volumeSlider.setValue(currentVolume);
    volumeSlider.addListener(this);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);

    addAndMakeVisible(powerButton);
    powerButton.setButtonText("On/Off sound");
    powerButton.setToggleState(isPlaying, juce::dontSendNotification);
    if (isPlaying)
        startTone();
    powerButton.addListener(this);

    addAndMakeVisible(waveSelector);
    waveSelector.addItem("Sine", 1);
    waveSelector.addItem("Square", 2);
    waveSelector.addItem("Saw", 3);
    waveSelector.setSelectedId(1);

    waveSelector.onChange = [this]
    {
        switch (waveSelector.getSelectedId())
        {
        case 1:
            currentWaveType = WaveType::Sine;
            break;
        case 2:
            currentWaveType = WaveType::Square;
            break;
        case 3:
            currentWaveType = WaveType::Saw;
            break;
        }
    };

    attackSlider.setRange(0.01, 5.0);
    attackSlider.setValue(0.1);
    attackSlider.onValueChange = [this]
    {
        adsrParams.attack = attackSlider.getValue();
        adsr.setParameters(adsrParams);
    };
    addAndMakeVisible(attackSlider);
    attackLabel.setText("Attack", juce::dontSendNotification);
    addAndMakeVisible(attackLabel);

    decaySlider.setRange(0.01, 2.0);
    decaySlider.setValue(0.1);
    decaySlider.onValueChange = [this]
    {
        adsrParams.decay = decaySlider.getValue();
        adsr.setParameters(adsrParams);
    };
    addAndMakeVisible(decaySlider);
    decayLabel.setText("Decay", juce::dontSendNotification);
    addAndMakeVisible(decayLabel);

    sustainSlider.setRange(0.01, 1.0);
    sustainSlider.setValue(0.1);
    sustainSlider.onValueChange = [this]
    {
        adsrParams.sustain = sustainSlider.getValue();
        adsr.setParameters(adsrParams);
    };
    addAndMakeVisible(sustainSlider);
    sustainLabel.setText("Sustain", juce::dontSendNotification);
    addAndMakeVisible(sustainLabel);

    releaseSlider.setRange(0.01, 5.0);
    releaseSlider.setValue(0.5);
    releaseSlider.onValueChange = [this]
    {
        adsrParams.release = releaseSlider.getValue();
        adsr.setParameters(adsrParams);
    };
    addAndMakeVisible(releaseSlider);
    releaseLabel.setText("Release", juce::dontSendNotification);
    addAndMakeVisible(releaseLabel);

    addAndMakeVisible(playButton);
    playButton.setButtonText("Play");
    playButton.onClick = [this]()
    {
        if (!isPlaying)
            startTone();
        else
            stopTone();
    };

    setAudioChannels(0, 2);

    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio) && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
                                          [&](bool granted)
                                          { setAudioChannels(granted ? 2 : 0, 2); });
    }
    else
    {
        setAudioChannels(2, 2);
    }
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    currentAngle = 0.0;
    angleDelta = 2.0 * juce::MathConstants<double>::pi * frequency / currentSampleRate;
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill)
{
     if (!isPlaying)
     {
         bufferToFill.clearActiveBufferRegion();
         return;
     }

    auto *buffer = bufferToFill.buffer;
    auto startSample = bufferToFill.startSample;
    auto numSamples = bufferToFill.numSamples;
    float sampleRate = static_cast<float>(deviceManager.getCurrentAudioDevice()->getCurrentSampleRate());

    for (int channel = 0; channel < buffer->getNumChannels(); ++channel)
    {
        auto *writePointer = buffer->getWritePointer(channel, startSample);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float env = adsr.getNextSample();

            float sampleValue = 0.0f;

            switch (currentWaveType)
            {
            case WaveType::Sine:
                sampleValue = std::sin(phase); // instead of t
                break;

            case WaveType::Square:
                sampleValue = std::sin(phase) > 0.0f ? 1.0f : -1.0f;
                break;

            case WaveType::Saw:
                sampleValue = 2.0f * (phase / juce::MathConstants<float>::twoPi) - 1.0f;
                break;
            }

            sampleValue *= env * volume;

            writePointer[sample] = sampleValue;

            phase += juce::MathConstants<float>::twoPi * frequency / sampleRate;
            if (phase >= juce::MathConstants<float>::twoPi)
                phase -= juce::MathConstants<float>::twoPi;
        }
    }
}

void MainComponent::releaseResources()
{
}

void MainComponent::paint(juce::Graphics &g)
{
    g.fillAll(backgroundColor);
}

void MainComponent::resized()
{
    myButton.setBounds(10, 10, 150, 40);
    frequencySlider.setBounds(10, 60, getWidth() - 20, 40);
    volumeSlider.setBounds(10, 110, getWidth() - 20, 40);
    powerButton.setBounds(10, 160, 150, 30);
    waveSelector.setBounds(10, 200, 120, 30);

    attackSlider.setBounds(10, 240, 160, 60);
    attackLabel.setBounds(10, 300, 160, 20);

    decaySlider.setBounds(180, 240, 160, 60);
    decayLabel.setBounds(180, 300, 160, 20);

    sustainSlider.setBounds(350, 240, 160, 60);
    sustainLabel.setBounds(350, 300, 160, 20);

    releaseSlider.setBounds(520, 240, 160, 60);
    releaseLabel.setBounds(520, 300, 160, 20);

    playButton.setBounds(10, 350, 80, 60);
}

void MainComponent::buttonClicked(juce::Button *button)
{
    if (button == &myButton)
    {
        // DBG("Кнопка нажата");
        // backgroundColor = juce::Colour::fromHSV(juce::Random::getSystemRandom().nextFloat(), 0.8f, 0.8f, 1.0f);
        backgroundColor = juce::Colour::fromHSV(juce::Random::getSystemRandom().nextFloat(), 0.4f, 0.5f, 1.0f);
        repaint();
    }
    if (button == &powerButton)
    {
        isPlaying = powerButton.getToggleState();
        DBG("Volume " << (isPlaying ? "is On" : "is Off"));

        if (isPlaying)
            startTone();
        else
            stopTone();
    }
}

void MainComponent::sliderValueChanged(juce::Slider *slider)
{
    if (slider == &volumeSlider)
    {
        volume = (float)volumeSlider.getValue();
        DBG("Volume: " << volume);
    }
    else if (slider == &frequencySlider)
    {
        frequency = (float)frequencySlider.getValue();
        // angleDelta = 2.0 * juce::MathConstants<double>::pi * currentFrequency / currentSampleRate;
        DBG("Частота: " << frequency);
    }
}

void MainComponent::startTone()
{
    playButton.setButtonText("Stop");
    isPlaying = true;
    adsr.noteOn();
}

void MainComponent::stopTone()
{
    playButton.setButtonText("Play");
    isPlaying = false;
    adsr.noteOff();
}
