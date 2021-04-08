/*
  ==============================================================================

    ArcSpectrogram.h
    Created: 31 Mar 2021 10:18:28pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class ArcSpectrogram : public juce::Component
{
public:
  ArcSpectrogram();
  ~ArcSpectrogram() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void loadedBuffer(juce::AudioSampleBuffer* buffer);

private:
  static constexpr auto mFftOrder = 10;
  static constexpr auto mFftSize = 1 << mFftOrder;

  juce::dsp::FFT mForwardFFT;
  juce::AudioSampleBuffer* mAudioBuffer;
  std::array<float, mFftSize * 2> mFftFrame;
  std::vector<std::vector<float>> mFftData;
  bool mIsLoaded = false;

  void drawSpectrogram(juce::Graphics& g);
  void updateFft();
  juce::Range<float> getFftMinMax(int column);
  juce::Colour getRainbowColour(float value);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArcSpectrogram)
};
