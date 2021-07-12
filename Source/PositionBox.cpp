/*
  ==============================================================================

    PositionBox.cpp
    Created: 27 Jun 2021 3:49:17pm
    Author:  brady

  ==============================================================================
*/

#include "PositionBox.h"
#include "Utils.h"
#include <JuceHeader.h>

//==============================================================================
PositionBox::PositionBox() {

  mPositionChanger.onPositionChanged = [this](bool isRight) {
    if (onPositionChanged != nullptr) {
      onPositionChanged(isRight);
    }
  };
  addAndMakeVisible(mPositionChanger);

  mBtnEnabled.setColour(juce::ToggleButton::ColourIds::tickColourId, juce::Colours::darkgrey);
  mBtnEnabled.onClick = [this] {
    mIsActive = !mIsActive;
    setState(mState);
    parameterChanged(GranularSynth::ParameterType::ENABLED, mBtnEnabled.getToggleState());
  };
  addAndMakeVisible(mBtnEnabled);

  mBtnSolo.setColour(juce::ToggleButton::ColourIds::tickColourId,
                        juce::Colours::blue);
  mBtnSolo.onClick = [this] {
    setState(mBtnSolo.getToggleState() ? BoxState::SOLO : BoxState::READY);
    parameterChanged(GranularSynth::ParameterType::SOLO,
                     mBtnSolo.getToggleState());
  };
  addAndMakeVisible(mBtnSolo);

  /* Knob params */
  auto rotaryParams = juce::Slider::RotaryParameters();
  rotaryParams.startAngleRadians = 1.4f * juce::MathConstants<float>::pi;
  rotaryParams.endAngleRadians = 2.6f * juce::MathConstants<float>::pi; 
  rotaryParams.stopAtEnd = true; 

  mLabelRate.setEnabled(mState == BoxState::READY);
  mLabelDuration.setEnabled(mState == BoxState::READY);
  mLabelGain.setEnabled(mState == BoxState::READY);

  /* Rate */
  mSliderRate.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderRate.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderRate.setRotaryParameters(rotaryParams);
  mSliderRate.setRange(0.0, 1.0, 0.01);
  mSliderRate.onValueChange = [this] {
    mEnvelopeGrain.setRate(mSliderRate.getValue());
    parameterChanged(GranularSynth::ParameterType::RATE,
                     mSliderRate.getValue());
  };
  mSliderRate.setValue(PARAM_RATE_DEFAULT, juce::sendNotification);
  addAndMakeVisible(mSliderRate);

  mLabelRate.setText("Rate", juce::dontSendNotification);
  mLabelRate.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelRate);

  /* Duration */
  mSliderDuration.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderDuration.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderDuration.setRotaryParameters(rotaryParams);
  mSliderDuration.setRange(0.0, 1.0, 0.01);
  mSliderDuration.onValueChange = [this] {
    mEnvelopeGrain.setDuration(mSliderDuration.getValue());
    parameterChanged(GranularSynth::ParameterType::DURATION,
                     mSliderDuration.getValue());
  };
  mSliderDuration.setValue(PARAM_DURATION_DEFAULT, juce::sendNotification);
  addAndMakeVisible(mSliderDuration);

  mLabelDuration.setText("Duration", juce::dontSendNotification);
  mLabelDuration.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelDuration);

  /* Gain */
  mSliderGain.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderGain.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderGain.setRotaryParameters(rotaryParams);
  mSliderGain.setRange(0.0, 1.0, 0.01);
  mSliderGain.onValueChange = [this] {
    mEnvelopeGrain.setGain(mSliderGain.getValue());
    parameterChanged(GranularSynth::ParameterType::GAIN,
                     mSliderGain.getValue());
  };
  mSliderGain.setValue(PARAM_GAIN_DEFAULT, juce::sendNotification);
  addAndMakeVisible(mSliderGain);

  mLabelGain.setText("Gain", juce::dontSendNotification);
  mLabelGain.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelGain);

  /* Grain envelope viz */
  addAndMakeVisible(mEnvelopeGrain);

  /* Amp envelope viz */
  addAndMakeVisible(mEnvelopeAmp);

  /* Attack */
  mSliderAttack.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderAttack.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderAttack.setRotaryParameters(rotaryParams);
  mSliderAttack.setRange(0.0, 1.0, 0.01);
  mSliderAttack.onValueChange = [this] {
    mEnvelopeAmp.setAttack(mSliderAttack.getValue());
    parameterChanged(GranularSynth::ParameterType::ATTACK,
                     mSliderAttack.getValue());
  };
  mSliderAttack.setValue(PARAM_ATTACK_DEFAULT, juce::sendNotification);
  addAndMakeVisible(mSliderAttack);

  mLabelAttack.setText("Attack", juce::dontSendNotification);
  mLabelAttack.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelAttack);

  /* Decay */
  mSliderDecay.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderDecay.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderDecay.setRotaryParameters(rotaryParams);
  mSliderDecay.setRange(0.0, 1.0, 0.01);
  mSliderDecay.onValueChange = [this] {
    mEnvelopeAmp.setDecay(mSliderDecay.getValue());
    parameterChanged(GranularSynth::ParameterType::DECAY,
                     mSliderDecay.getValue());
  };
  mSliderDecay.setValue(PARAM_DECAY_DEFAULT, juce::sendNotification);
  addAndMakeVisible(mSliderDecay);

  mLabelDecay.setText("Decay", juce::dontSendNotification);
  mLabelDecay.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelDecay);

  /* Sustain */
  mSliderSustain.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderSustain.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderSustain.setRotaryParameters(rotaryParams);
  mSliderSustain.setRange(0.0, 1.0, 0.01);
  mSliderSustain.onValueChange = [this] {
    mEnvelopeAmp.setSustain(mSliderSustain.getValue());
    parameterChanged(GranularSynth::ParameterType::SUSTAIN,
                     mSliderSustain.getValue());
  };
  mSliderSustain.setValue(PARAM_SUSTAIN_DEFAULT, juce::sendNotification);
  addAndMakeVisible(mSliderSustain);

  mLabelSustain.setText("Sustain", juce::dontSendNotification);
  mLabelSustain.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelSustain);

  /* Release */
  mSliderRelease.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
  mSliderRelease.setSliderStyle(juce::Slider::SliderStyle::Rotary);
  mSliderRelease.setRotaryParameters(rotaryParams);
  mSliderRelease.setRange(0.0, 1.0, 0.01);
  mSliderRelease.onValueChange = [this] {
    mEnvelopeAmp.setRelease(mSliderRelease.getValue());
    parameterChanged(GranularSynth::ParameterType::RELEASE,
                     mSliderRelease.getValue());
  };
  mSliderRelease.setValue(PARAM_RELEASE_DEFAULT, juce::sendNotification);
  addAndMakeVisible(mSliderRelease);

  mLabelRelease.setText("Release", juce::dontSendNotification);
  mLabelRelease.setJustificationType(juce::Justification::centredTop);
  addAndMakeVisible(mLabelRelease);
}

PositionBox::~PositionBox() {}

void PositionBox::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  bool borderLit = (mIsActive || mState == BoxState::SOLO);
  g.setColour(borderLit ? juce::Colour(Utils::POSITION_COLOURS[mColour])
                        : juce::Colours::darkgrey);
  g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 10.0f, 2.0f);
}

void PositionBox::resized() {
  auto r = getLocalBounds();
  // Add insets
  r.removeFromTop(PADDING_SIZE);
  r.removeFromLeft(PADDING_SIZE);
  r.removeFromRight(PADDING_SIZE);
  r.removeFromBottom(PADDING_SIZE);

  // Enable and solo buttons
  auto btnPanel = r.removeFromTop(TOGGLE_SIZE);
  mBtnEnabled.setBounds(btnPanel.removeFromLeft(TOGGLE_SIZE));
  mBtnSolo.setBounds(btnPanel.removeFromRight(TOGGLE_SIZE));
  mPositionChanger.setBounds(btnPanel.withSizeKeepingCentre(
      btnPanel.getWidth() * 0.7, btnPanel.getHeight()));

  r.removeFromTop(PADDING_SIZE);
  
  // Amp envelope
  mEnvelopeAmp.setBounds(r.removeFromTop(ENVELOPE_HEIGHT));
  r.removeFromTop(PADDING_SIZE);

  // Amp env knobs
  auto knobWidth = r.getWidth() / NUM_AMP_ENV_PARAMS;
  auto knobPanel = r.removeFromTop(knobWidth / 2);
  mSliderAttack.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderDecay.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderSustain.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderRelease.setBounds(knobPanel.removeFromLeft(knobWidth));

  auto labelPanel = r.removeFromTop(LABEL_HEIGHT);
  mLabelAttack.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelDecay.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelSustain.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelRelease.setBounds(labelPanel.removeFromLeft(knobWidth));

  // Grain envelopes
  mEnvelopeGrain.setBounds(r.removeFromTop(ENVELOPE_HEIGHT));
  r.removeFromTop(PADDING_SIZE);

  // Grain env knobs
  knobWidth = r.getWidth() / NUM_GRAIN_ENV_PARAMS;
  knobPanel = r.removeFromTop(knobWidth / 2);
  mSliderRate.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderDuration.setBounds(knobPanel.removeFromLeft(knobWidth));
  mSliderGain.setBounds(knobPanel.removeFromLeft(knobWidth));

  labelPanel = r.removeFromTop(LABEL_HEIGHT);
  mLabelRate.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelDuration.setBounds(labelPanel.removeFromLeft(knobWidth));
  mLabelGain.setBounds(labelPanel.removeFromLeft(knobWidth));
}

void PositionBox::setActive(bool isActive) {
  mIsActive = isActive;
  mBtnEnabled.setToggleState(isActive, juce::dontSendNotification);
  setState(mState);
}

void PositionBox::setPositions(std::vector<int> positions) {
  mPositionChanger.setGlobalPositions(positions);
}

void PositionBox::setNumPositions(int numPositions) {
  mPositionChanger.setNumPositions(numPositions);
}

void PositionBox::setState(BoxState state) {
  mState = state;

  if (state == BoxState::SOLO_WAIT) {
    mBtnSolo.setToggleState(false, juce::dontSendNotification);
  }

  juce::Colour enabledColour = (state != BoxState::SOLO_WAIT)
          ? juce::Colour(Utils::POSITION_COLOURS[mColour])
                                     : juce::Colours::darkgrey;
  mBtnEnabled.setColour(juce::ToggleButton::ColourIds::tickColourId,
                        enabledColour);
  juce::Colour soloColour = (state != BoxState::SOLO_WAIT)
                                   ? juce::Colours::blue
                                   : juce::Colours::darkgrey;
  mBtnSolo.setColour(juce::ToggleButton::ColourIds::tickColourId, soloColour);

  bool componentsLit = (mIsActive && state == BoxState::READY || state == BoxState::SOLO);
  juce::Colour knobColour = componentsLit
                                ? juce::Colour(Utils::POSITION_COLOURS[mColour])
                                : juce::Colours::darkgrey;
  mPositionChanger.setActive(componentsLit);

  mEnvelopeAmp.setActive(componentsLit);
  mSliderAttack.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                          knobColour);
  mSliderDecay.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                         knobColour);
  mSliderSustain.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                           knobColour);
  mSliderRelease.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                           knobColour);
  mSliderAttack.setColour(
      juce::Slider::ColourIds::rotarySliderOutlineColourId,
      componentsLit
          ? juce::Colour(Utils::SECONDARY_POSITION_COLOURS[mColour][0])
          : juce::Colours::darkgrey);
  mSliderDecay.setColour(
      juce::Slider::ColourIds::rotarySliderOutlineColourId,
      componentsLit
          ? juce::Colour(Utils::SECONDARY_POSITION_COLOURS[mColour][1])
          : juce::Colours::darkgrey);
  mSliderSustain.setColour(
      juce::Slider::ColourIds::rotarySliderOutlineColourId,
      componentsLit
          ? juce::Colour(Utils::SECONDARY_POSITION_COLOURS[mColour][2])
          : juce::Colours::darkgrey);
  mSliderRelease.setColour(
      juce::Slider::ColourIds::rotarySliderOutlineColourId,
      componentsLit
          ? juce::Colour(Utils::SECONDARY_POSITION_COLOURS[mColour][3])
          : juce::Colours::darkgrey);
  mLabelAttack.setEnabled(componentsLit);
  mLabelDecay.setEnabled(componentsLit);
  mLabelSustain.setEnabled(componentsLit);
  mLabelRelease.setEnabled(componentsLit);

  mEnvelopeGrain.setActive(componentsLit);
  mSliderRate.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                        knobColour);
  mSliderDuration.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                            knobColour);
  mSliderGain.setColour(juce::Slider::ColourIds::rotarySliderFillColourId,
                        knobColour);
  mSliderRate.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                        knobColour);
  mSliderDuration.setColour(
      juce::Slider::ColourIds::rotarySliderOutlineColourId, knobColour);
  mSliderGain.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId,
                        knobColour);
  mLabelRate.setEnabled(componentsLit);
  mLabelDuration.setEnabled(componentsLit);
  mLabelGain.setEnabled(componentsLit);
  repaint();
}

GranularSynth::PositionParams PositionBox::getParams() {
  return GranularSynth::PositionParams(
      mSliderRate.getValue(), mSliderDuration.getValue(),
      mSliderGain.getValue(), mSliderAttack.getValue(), mSliderDecay.getValue(),
      mSliderSustain.getValue(), mSliderRelease.getValue());
}

void PositionBox::setColour(Utils::PositionColour colour) {
  mColour = colour;
  juce::Colour newColour = juce::Colour(Utils::POSITION_COLOURS[colour]);
  if (mState == BoxState::READY) {
    mBtnEnabled.setColour(juce::ToggleButton::ColourIds::tickColourId,
                          newColour);
    mBtnSolo.setColour(juce::ToggleButton::ColourIds::tickColourId,
                       juce::Colours::blue);
  }
  mPositionChanger.setColour(colour, newColour);
  mEnvelopeGrain.setColour(newColour);
  mEnvelopeAmp.setColour(colour);
  repaint();
}

void PositionBox::parameterChanged(GranularSynth::ParameterType type,
                                   float value) {
  if (onParameterChanged != nullptr) {
    onParameterChanged(mColour, type, value);
  }
}