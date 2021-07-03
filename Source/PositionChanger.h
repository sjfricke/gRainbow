/*
  ==============================================================================

    PositionChanger.h
    Created: 3 Jul 2021 10:08:25pm
    Author:  brady

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
 */
class PositionChanger : public juce::Component {
 public:
  PositionChanger();
  ~PositionChanger() override;

  void paint(juce::Graphics&) override;
  void resized() override;
  void mouseMove(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;
  void mouseEnter(const juce::MouseEvent&) override;
  void mouseExit(const juce::MouseEvent&) override;

  void setActive(bool isActive);
  void setColour(juce::Colour colour);
  void setPosition(int position);

  std::function<void(bool isRight)> onPositionChanged = nullptr;

 private:

  static constexpr auto TITLE_PERC = 0.6;
  static constexpr auto ARROW_PERC = (1.0 - TITLE_PERC) / 2.0;

  bool mIsActive = false;
  juce::Colour mColour;
  int mPosition = 0;
  bool mIsClickingArrow = false;
  bool mIsOverLeftArrow = false;
  bool mIsOverRightArrow = false;

  void updateMouseOver(const juce::MouseEvent& e, bool isDown);
  bool isLeftArrow(juce::Point<float> point);
  bool isRightArrow(juce::Point<float> point);
  void positionChanged(bool isRight);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PositionChanger)
};