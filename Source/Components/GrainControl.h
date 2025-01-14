/*
  ==============================================================================

    GrainControl.h
    Created: 23 Jun 2021 8:34:54pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Parameters.h"
#include "RainbowSlider.h"
#include "MeterLookAndFeel.h"
#include "ff_meters/ff_meters.h"

//==============================================================================
/*
 */
class GrainControl : public juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer {
 public:
  GrainControl(Parameters& parameters, foleys::LevelMeterSource& meterSource);
  ~GrainControl();

  void paint(juce::Graphics&) override;
  void resized() override;

  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}

  void timerCallback() override;

  void updateSelectedParams();
  
  std::function<void(void)> onRefToneOn = nullptr;
  std::function<void(void)> onRefToneOff = nullptr;

 private:
  static constexpr const char* SECTION_TITLE = "controls";

  // Bookkeeping
  Parameters& mParameters;
  std::atomic<bool> mParamHasChanged;
  ParamCommon* mCurSelectedParams;
  juce::Colour mParamColour;
  MeterLookAndFeel mMeterLookAndFeel;

  // Components
  // -- Generator Adjustments
  RainbowSlider mSliderPitchAdjust;
  juce::Label mLabelPitchAdjust;
  RainbowSlider mSliderPitchSpray;
  juce::Label mLabelPitchSpray;
  RainbowSlider mSliderPosAdjust;
  juce::Label mLabelPosAdjust;
  RainbowSlider mSliderPosSpray;
  juce::Label mLabelPosSpray;
  RainbowSlider mSliderPanAdjust;
  juce::Label mLabelPanAdjust;
  RainbowSlider mSliderPanSpray;
  juce::Label mLabelPanSpray;
  RainbowSlider mSliderGain;
  juce::Label mLabelGain;
  foleys::LevelMeter mMeter{foleys::LevelMeter::Horizontal};
  juce::TextButton mBtnRefTone;

  // UI values saved on resize
  juce::Rectangle<float> mTitleRect;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrainControl)
};
