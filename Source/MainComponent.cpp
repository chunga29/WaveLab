//==============================================================================

#include "MainComponent.h"
#include "MainApplication.h"
#include <cmath>

using namespace juce;

MainComponent::MainComponent()
: deviceManager (MainApplication::getApp().audioDeviceManager), audioVisualizer(2) {
    setSize(600, 400);
    addAndMakeVisible(settingsButton);
    settingsButton.setButtonText("Audio Settings...");
    settingsButton.addListener(this);

    addAndMakeVisible(waveformMenu);
    waveformMenu.addListener(this);
    waveformMenu.setTextWhenNothingSelected("Waveforms");
    waveformMenu.addItem("White", 1);
    waveformMenu.addItem("Brown", 2);
    waveformMenu.addItem("Dust", 3);

    waveformMenu.addSeparator();

    waveformMenu.addItem("Sine", 4);

    waveformMenu.addSeparator();

    waveformMenu.addItem("LF Impulse", 5);
    waveformMenu.addItem("LF Square", 6);
    waveformMenu.addItem("LF Saw", 7);
    waveformMenu.addItem("LF Triangle", 8);

    waveformMenu.addSeparator();

    waveformMenu.addItem("BL Impulse", 9);
    waveformMenu.addItem("BL Square", 10);
    waveformMenu.addItem("BL Saw", 11);
    waveformMenu.addItem("BL Triangle", 12);

    waveformMenu.addSeparator();

    waveformMenu.addItem("WT Sine", 13);
    waveformMenu.addItem("WT Impulse", 14);
    waveformMenu.addItem("WT Square", 15);
    waveformMenu.addItem("WT Saw", 16);
    waveformMenu.addItem("WT Triangle", 17);

    addAndMakeVisible(playButton);
    playButton.addListener(this);
    drawPlayButton(playButton, true);
    playButton.setEnabled(false);

    addAndMakeVisible(levelLabel);
    levelLabel.setText("Level:", dontSendNotification);

    addAndMakeVisible(levelSlider);
    
    levelSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    levelSlider.setRange(0.0, 1.0);
    levelSlider.addListener(this);
    levelLabel.attachToComponent(&levelSlider, true);

    addAndMakeVisible(freqLabel);
    freqLabel.setText("Frequency:", dontSendNotification);


    addAndMakeVisible(freqSlider);
    
    freqSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    freqSlider.setRange(0.0, 5000.0);
    freqSlider.addListener(this);
    freqLabel.attachToComponent(&freqSlider, true);
    freqSlider.setSkewFactorFromMidPoint(500.0);

    addAndMakeVisible(audioVisualizer);
    audioSourcePlayer.setSource(nullptr);
    deviceManager.addAudioCallback(&audioSourcePlayer);
    startTimer(100);

    cpuLabel.setText("CPU:", juce::dontSendNotification);
    cpuUsage.setJustificationType(juce::Justification::right);
    addAndMakeVisible(cpuLabel);
    addAndMakeVisible(cpuUsage);
    

    setVisible(true);

    createWaveTables();
}

MainComponent::~MainComponent() {
    audioSourcePlayer.setSource(nullptr);
    deviceManager.removeAudioCallback(&audioSourcePlayer);
    deviceManager.closeAudioDevice();
}

//==============================================================================
// Component overrides
//==============================================================================

void MainComponent::paint (Graphics& g) {
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void MainComponent::resized() {
    auto bounds = getLocalBounds().reduced(8);
    auto twoLines = bounds.removeFromTop(56);
    auto area = twoLines.removeFromLeft(118);

    settingsButton.setBounds(area.removeFromTop(24));

    area.removeFromTop(8);

    waveformMenu.setBounds(area);

    twoLines.removeFromLeft(8);

    playButton.setBounds(twoLines.removeFromLeft(56));
    

    auto secArea = twoLines.removeFromRight(300);
    auto sliderSection = secArea.removeFromTop(56);

    levelSlider.setBounds(sliderSection.removeFromTop(24));
    sliderSection.removeFromTop(8);
    freqSlider.setBounds(sliderSection.removeFromTop(24));

    secArea.removeFromRight(8);
    
    bounds.removeFromTop(8);
    auto cpuArea = bounds.removeFromBottom(20);
    auto cpuLabelArea2 = cpuArea.removeFromRight(200);
    auto cpuUsageArea = cpuLabelArea2.removeFromRight(100);
    cpuLabel.setBounds(cpuLabelArea2);
    cpuUsage.setBounds(cpuUsageArea);

    audioVisualizer.setBounds(bounds);

}

void MainComponent::drawPlayButton(juce::DrawableButton& button, bool showPlay) {
  juce::Path path;
  if (showPlay) {
    // draw the triangle
      float f1 = 0;
      float f2 = 0;
      float f3 = 0;
      float f4 = 100;
      float f5 = 86.6;
      float f6 = 50;
      path.addTriangle(f1, f2, f3, f4, f5, f6);
  }
  else {
    // draw the two bars
      float f1 = 0;
      float f2 = 0;
      float f3 = 42;
      float f4 = 100;
      float f5 = 100 - button.getWidth();
      float f6 = 0;
      path.addRectangle(f1, f2, f3, f4);
      path.addRectangle(f5, f6, f3, f4);
  }
  juce::DrawablePath drawable;
  drawable.setPath(path);
  juce::FillType fill (Colours::white);
  drawable.setFill(fill);
  button.setImages(&drawable);
}

//==============================================================================
// Listener overrides
//==============================================================================

void MainComponent::buttonClicked (Button *button) {
    if (button == &playButton) {
        if (isPlaying()) {
            audioSourcePlayer.setSource(nullptr);
        } else {
            audioSourcePlayer.setSource(this);
        }
        drawPlayButton(playButton, !isPlaying());
    }
    else if (button == &settingsButton) {
        openAudioSettings();
    }
}

void MainComponent::sliderValueChanged (Slider *slider) {
    if (slider == &levelSlider) {
        level = levelSlider.getValue();
    }
    else if (slider == &freqSlider) {
        freq = freqSlider.getValue();
        phaseDelta = freqSlider.getValue() / srate;
        for (auto& o : oscillators) {
            o->setFrequency(freq, srate);
        }
    }
}

void MainComponent::comboBoxChanged (ComboBox *menu) {
    if (menu == &waveformMenu) {
        waveformId = (WaveformId)waveformMenu.getSelectedId();
        playButton.setEnabled(true);
        /*
        int num = waveformMenu.getSelectedItemIndex();
        std::cout << num << std::endl;
        waveformId = WaveformId(num);
        */

    }
}

//==============================================================================
// Timer overrides
//==============================================================================

void MainComponent::timerCallback() {
    auto cpu = deviceManager.getCpuUsage() * 100;
    cpuUsage.setText(juce::String(cpu, 3) + " %", juce::dontSendNotification);
}

//==============================================================================
// AudioSource overrides
//==============================================================================

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate) {
    audioVisualizer.setBufferSize(samplesPerBlockExpected);
    audioVisualizer.setSamplesPerBlock(8);
    srate = sampleRate;
    phaseDelta = freq / srate;
    phase = 0.0;
}

void MainComponent::releaseResources() {
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) {
  bufferToFill.clearActiveBufferRegion();
  switch (waveformId) {
    case WhiteNoise:      whiteNoise(bufferToFill);   break;
    case DustNoise:       dust(bufferToFill);         break;
    case BrownNoise:      brownNoise(bufferToFill);   break;
    case SineWave:        sineWave(bufferToFill);     break;
    case LF_ImpulseWave:  LF_impulseWave(bufferToFill);  break;
    case LF_SquareWave:   LF_squareWave(bufferToFill);   break;
    case LF_SawtoothWave: LF_sawtoothWave(bufferToFill); break;
    case LF_TriangeWave:  LF_triangleWave(bufferToFill); break;
    case BL_ImpulseWave:  BL_impulseWave(bufferToFill);  break;
    case BL_SquareWave:   BL_squareWave(bufferToFill);   break;
    case BL_SawtoothWave: BL_sawtoothWave(bufferToFill); break;
    case BL_TriangeWave:  BL_triangleWave(bufferToFill); break;
    case WT_SineWave:
    case WT_ImpulseWave:
    case WT_SquareWave:
    case WT_SawtoothWave:
    case WT_TriangleWave:
      WT_wave(bufferToFill);
      break;
    case Empty:
      break;
  }
  audioVisualizer.pushBuffer(bufferToFill);
}

//==============================================================================
// Audio Utilities
//==============================================================================

double MainComponent::phasor() {
  double p = phase;
  phase = std::fmod(phase + phaseDelta, 1.0);
  return p;
}

float MainComponent::ranSamp() {
    /*float random = ((float)rand()) / (float)RAND_MAX;
    float r = random * 2;
    return r - 1;
    */
    return random.nextFloat() * 2 - 1;

}

float MainComponent::ranSamp(const float mul) {
  return (ranSamp() * mul);
}

float MainComponent::lowPass(const float value, const float prevout, const float alpha) {
  return (value - prevout) * alpha + prevout;
}

bool MainComponent::isPlaying() {
    if (audioSourcePlayer.getCurrentSource() == nullptr) {
        return false;
    }
    return true;
}

void MainComponent::openAudioSettings() {
    adsComp = std::make_unique<AudioDeviceSelectorComponent>(deviceManager, 0, 2, 0, 2, false, false, false, false);
    adsComp.get()->setSize(500, 270);
    new_options = std::make_unique<DialogWindow::LaunchOptions>();
    new_options->useNativeTitleBar = true;
    new_options->resizable = true;
    new_options->dialogTitle = "Audio Settings";
    new_options->dialogBackgroundColour = juce::Colours::black;

    new_options->content.setOwned(adsComp.get());
    new_options->launchAsync();
}

void MainComponent::createWaveTables() {
  createSineTable(sineTable);
  oscillators.push_back(std::make_unique<WavetableOscillator>(sineTable));
  createImpulseTable(impulseTable);
  oscillators.push_back(std::make_unique<WavetableOscillator>(impulseTable));
  createSquareTable(squareTable);
  oscillators.push_back(std::make_unique<WavetableOscillator>(squareTable));
  createSawtoothTable(sawtoothTable);
  oscillators.push_back(std::make_unique<WavetableOscillator>(sawtoothTable));
  createTriangleTable(triangleTable);
  oscillators.push_back(std::make_unique<WavetableOscillator>(triangleTable));
}

//==============================================================================
// Noise
//==============================================================================

// White Noise

void MainComponent::whiteNoise (const AudioSourceChannelInfo& bufferToFill) {
    // loop over every channel in buffer to fill
    for (auto chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan) {
        auto channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        for (auto i = 0; i < bufferToFill.numSamples; ++i) {
            channelData[i] = ranSamp(level);
        }
    }
}

// Dust

void MainComponent::dust (const AudioSourceChannelInfo& bufferToFill) {
    for (auto chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan) {
        auto channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        for (auto i = 0; i < bufferToFill.numSamples; ++i) {
            if (i % bufferToFill.numSamples == 0) {
                channelData[i] = ranSamp(level);
            }
        }
    }
}

// Brown Noise

void MainComponent::brownNoise (const AudioSourceChannelInfo& bufferToFill) {
    for (auto chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan) {
        auto channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        float brownNoise = 0.0;
        for (auto i = 0; i < bufferToFill.numSamples; ++i) {
            brownNoise = lowPass(ranSamp(), brownNoise, 0.025);
            channelData[i] = 3.0 * level * brownNoise;
        }
    }
}

//==============================================================================
// Sine Wave
//==============================================================================

void MainComponent::sineWave (const AudioSourceChannelInfo& bufferToFill) {
    auto startingPhase = phase;
    for (auto chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan) {
        // set the phase for each channel to  startingPhase
        phase = startingPhase;
        // get the pointer to the first sample in the buffer
        auto channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        // iterate samples
        for (auto i = 0; i < bufferToFill.numSamples; ++i) {
            // calculate the next sample for the current phase
            channelData[i] = level * std::sin(phase * TwoPi);
            // increment phase for the next dample
            phase += phaseDelta;
        }
    }
}

//==============================================================================
// Low Frequency Waveforms
//==============================================================================

/// Impulse wave

void MainComponent::LF_impulseWave (const AudioSourceChannelInfo& bufferToFill) {
    auto startingPhase = phase;
    for (auto chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan) {
        phase = startingPhase;
        double lastPhasor = 0.0;
        auto channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        for (auto i = 0; i < bufferToFill.numSamples; ++i) {
            double phasorValue = phasor();
            if (lastPhasor - phasorValue > 0.9) {
                channelData[i] = level * (phasorValue * -2 + 1);
            }
            lastPhasor = phasorValue;
        }
    }
}

/// Square wave

void MainComponent::LF_squareWave (const AudioSourceChannelInfo& bufferToFill) {
    auto startingPhase = phase;
    for (auto chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan) {
        phase = startingPhase;
        auto channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        for (auto i = 0; i < bufferToFill.numSamples; ++i) {
            if (level * (phasor() * 2 - 1) > 0) {
                channelData[i] = level * 1.0;
            }
            else {
                channelData[i] = level * -1.0;
            }
            
        }
    }
}

/// Sawtooth wave

void MainComponent::LF_sawtoothWave (const AudioSourceChannelInfo& bufferToFill) {
    auto startingPhase = phase;
    for (auto chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan) {
        phase = startingPhase;
        auto channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        for (auto i = 0; i < bufferToFill.numSamples; ++i) {
            channelData[i] = level * (phasor() * 2 - 1);
        }
    }
}

/// Triangle wave

void MainComponent::LF_triangleWave (const AudioSourceChannelInfo& bufferToFill) {
    auto startingPhase = phase;
    for (auto chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan) {
        phase = startingPhase;
        auto channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
        for (auto i = 0; i < bufferToFill.numSamples; ++i) {
            double phasorValue = phasor();
            if (phasorValue <= 0.5) { // first half
                // phasor goes from 0.0 to 0.5
                // need -1 to 1
                channelData[i] = level * (phasorValue * 4 - 1);
            }
            else { // second half
                // 0.5 to 1.0
                channelData[i] = level * (phasorValue * 4 - 3) * -1;
            }
        }
    }

}

//==============================================================================
// Band Limited Waveforms
//==============================================================================

/// Impulse (pulse) wave

/// Synthesized by summing sin() over frequency and all its harmonics at equal
/// amplitude. To make it band limited only include harmonics that are at or
/// below the nyquist limit.
void MainComponent::BL_impulseWave (const AudioSourceChannelInfo& bufferToFill) {
    auto channelData = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    double numHarmonics = srate / 2 / freq;
    double nyquist = srate / 2;

    for (auto i = 0; i < bufferToFill.numSamples; ++i) {
        // amplitude = level / numHarmonics
        auto freqLevel = level / numHarmonics;
        auto phasorValue = phasor();
        for (auto h = 1; h < numHarmonics; h++) {
            auto newFreq = phasorValue * TwoPi * h;
            if (newFreq < nyquist) {
                channelData[i] += sin(phasorValue * TwoPi * h) * freqLevel;
            }
        }
    }

    memcpy(bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample), bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample), sizeof(float)  * bufferToFill.numSamples);
}

/// Square wave

/// Synthesized by summing sin() over all ODD harmonics at 1/harmonic amplitude.
/// To make it band limited only include harmonics that are at or below the
/// nyquist limit.
void MainComponent::BL_squareWave (const AudioSourceChannelInfo& bufferToFill) {
    auto channelData = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    double numHarmonics = srate / 2 / freq;
    double nyquist = srate / 2;

    for (auto i = 0; i < bufferToFill.numSamples; ++i) {
        // amplitude = level / numHarmonics
        auto freqLevel = level;
        auto phasorValue = phasor();
        for (auto h = 1; h < numHarmonics; h++) {
            auto newFreq = phasorValue * TwoPi * h;
            if (h % 2 == 1) {
                channelData[i] += sin(phasorValue * TwoPi * (h)) * freqLevel * ((float)1/h);
            }
        }
    }

    memcpy(bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample), bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample), sizeof(float) * bufferToFill.numSamples);

}

/// Sawtooth wave
///
/// Synthesized by summing sin() over all harmonics at 1/harmonic amplitude. To make
/// it band limited only include harmonics that are at or below the nyquist limit.
void MainComponent::BL_sawtoothWave (const AudioSourceChannelInfo& bufferToFill) {
    auto channelData = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    double numHarmonics = srate / 2 / freq;
    double nyquist = srate / 2;

    for (auto i = 0; i < bufferToFill.numSamples; ++i) {
        // amplitude = level / numHarmonics
        auto freqLevel = level;
        auto phasorValue = phasor();
        for (auto h = 1; h < numHarmonics; h++) {
            auto newFreq = phasorValue * TwoPi * h;
            if (newFreq < nyquist) {
                auto newData = sin(phasorValue * TwoPi * h) * freqLevel / h;
                channelData[i] += newData;
            }
        }
    }

    memcpy(bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample), bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample), sizeof(float) * bufferToFill.numSamples);

}

/// Triangle wave
///
/// Synthesized by summing sin() over all ODD harmonics at 1/harmonic**2 amplitude.
/// To make it band limited only include harmonics that are at or below the
/// Nyquist limit.
void MainComponent::BL_triangleWave (const AudioSourceChannelInfo& bufferToFill) {
    auto channelData = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    double numHarmonics = srate / 2 / freq;
    double nyquist = srate / 2;

    for (auto i = 0; i < bufferToFill.numSamples; ++i) {
        // amplitude = level / numHarmonics
        auto freqLevel = level;
        auto phasorValue = phasor();
        for (auto h = 1; h < numHarmonics; h++) {
            auto newFreq = phasorValue * TwoPi * h;
            if (newFreq < nyquist && h % 2 == 1) {
                channelData[i] += sin(phasorValue * TwoPi * (h)) * freqLevel * ((float)1 / (h*h));
            }
        }
    }

    memcpy(bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample), bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample), sizeof(float) * bufferToFill.numSamples);

}

//==============================================================================
// WaveTable Synthesis
//==============================================================================

// The audio block loop
void inline MainComponent::WT_wave(const AudioSourceChannelInfo& bufferToFill) {
    auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
    bufferToFill.clearActiveBufferRegion();
    auto oscillatorIndex = waveformId - WT_START;
    auto* oscil = oscillators[oscillatorIndex].get();
    // add each oscillator's samples ot the output buffer
    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample) {
        auto levelSample = oscil->getNextSample() * level;
        leftBuffer[sample] += levelSample;
        rightBuffer[sample] += levelSample;
    }
}

// Create a sine wave table
void MainComponent::createSineTable(AudioSampleBuffer& waveTable) {
    waveTable.setSize (1, tableSize + 1);
    waveTable.clear();
    auto* samples = waveTable.getWritePointer (0);
    auto phase = 0.0;
    auto phaseDelta = MathConstants<double>::twoPi / (double) (tableSize - 1);
    for (auto i = 0; i < tableSize; ++i) {
        samples[i] += std::sin(phase);
        phase += phaseDelta;
    }
    samples[tableSize] = samples[0];
}

// Create an inpulse wave table
void MainComponent::createImpulseTable(AudioSampleBuffer& waveTable) {
    waveTable.setSize(1, tableSize + 1);
    waveTable.clear();
    auto* samples = waveTable.getWritePointer(0);
    auto phase = 0.0;
    auto phaseDelta = MathConstants<double>::twoPi / (double)(tableSize - 1);

    double numHarmonics = 15;

    for (auto i = 0; i < tableSize; ++i) {
        for (auto h = 1; h < numHarmonics; h++) {
            samples[i] += std::sin(phase * h) / numHarmonics;
            //channelData[i] += sin(phasorValue * TwoPi * h) * freqLevel;
        }
        phase += phaseDelta;
    }
    samples[tableSize] = samples[0];

}

// Create a square wave table
void MainComponent::createSquareTable(AudioSampleBuffer& waveTable) {
    waveTable.setSize(1, tableSize + 1);
    waveTable.clear();
    auto* samples = waveTable.getWritePointer(0);
    auto phase = 0.0;
    auto phaseDelta = MathConstants<double>::twoPi / (double)(tableSize - 1);

    double numHarmonics = 15;

    for (auto i = 0; i < tableSize; ++i) {
        for (auto h = 1; h < numHarmonics; h++) {
            if (h % 2 == 1) {
                samples[i] += std::sin(phase * h) / numHarmonics / h;
                //channelData[i] += sin(phasorValue * TwoPi * h) * freqLevel;
            }
        }
        phase += phaseDelta;
    }
    samples[tableSize] = samples[0];
}

// Create a sawtooth wave table
void MainComponent::createSawtoothTable(AudioSampleBuffer& waveTable) {
    waveTable.setSize(1, tableSize + 1);
    waveTable.clear();
    auto* samples = waveTable.getWritePointer(0);
    auto phase = 0.0;
    auto phaseDelta = MathConstants<double>::twoPi / (double)(tableSize - 1);

    double numHarmonics = 15;

    for (auto i = 0; i < tableSize; ++i) {
        for (auto h = 1; h < numHarmonics; h++) {
            samples[i] += std::sin(phase * h) / numHarmonics / h;
            //channelData[i] += sin(phasorValue * TwoPi * h) * freqLevel;
        }
        phase += phaseDelta;
    }
    samples[tableSize] = samples[0];
}

// Create a triangle wave table
void MainComponent::createTriangleTable(AudioSampleBuffer& waveTable) {
    waveTable.setSize(1, tableSize + 1);
    waveTable.clear();
    auto* samples = waveTable.getWritePointer(0);
    auto phase = 0.0;
    auto phaseDelta = MathConstants<double>::twoPi / (double)(tableSize - 1);

    double numHarmonics = 15;

    for (auto i = 0; i < tableSize; ++i) {
        for (auto h = 1; h < numHarmonics; h++) {
            if (h % 2 == 1) {
                samples[i] += std::sin(phase * h) / numHarmonics / (h * h);
                //channelData[i] += sin(phasorValue * TwoPi * h) * freqLevel;
            }
        }
        phase += phaseDelta;
    }
    samples[tableSize] = samples[0];
}
