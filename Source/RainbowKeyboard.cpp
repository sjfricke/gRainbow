/*
  ==============================================================================

    RainbowKeyboard.cpp
    Created: 2 Jun 2021 10:27:38pm
    Author:  brady

  ==============================================================================
*/

#include "RainbowKeyboard.h"

#include <JuceHeader.h>

//==============================================================================
RainbowKeyboard::RainbowKeyboard(juce::MidiKeyboardState& state) : mState(state) {}

RainbowKeyboard::~RainbowKeyboard() {}

void RainbowKeyboard::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  // Draw each white key, but rainbowy, then black key, but rainbowy, so the
  // black keys rect are fully on top of white key rect visually
  for (Utils::PitchClass pitchClass : WHITE_KEYS_PITCH_CLASS) {
    drawKey(g, pitchClass);
  }
  for (Utils::PitchClass pitchClass : BLACK_KEYS_PITCH_CLASS) {
    drawKey(g, pitchClass);
  }
}

float RainbowKeyboard::getPitchXRatio(Utils::PitchClass pitchClass) {
  float noteMiddle = mNoteRectangleMap[pitchClass].getCentreX() / static_cast<float>(getWidth());
  bool needsLeftShift = ((1 << (pitchClass)) & 0x021) != 0;
  bool needsRightShift = ((1 << (pitchClass)) & 0x810) != 0;
  float shiftAmount = (1.0f / 7.0f) * (BLACK_NOTE_SIZE_RATIO / 4.0f);
  if (needsLeftShift) {
    return noteMiddle - shiftAmount;
  } else if (needsRightShift) {
    return noteMiddle + shiftAmount;
  } else {
    return noteMiddle;
  }
}

void RainbowKeyboard::fillNoteRectangleMap() {
  // Keep everything in float to match Rectangle type
  const float componentWidth = static_cast<float>(getWidth());
  const float componentHeight = static_cast<float>(getHeight());

  const float keyWidth = componentWidth / 7.0f;
  const float blackKeyOffset = BLACK_NOTE_SIZE_RATIO * keyWidth / 2.0f;

  const float notePos[Utils::PitchClass::COUNT] = {0.0f * keyWidth,
                                                   1.0f * keyWidth - blackKeyOffset,
                                                   1.0f * keyWidth,
                                                   2.0f * keyWidth - blackKeyOffset,
                                                   2.0f * keyWidth,
                                                   3.0f * keyWidth,
                                                   4.0f * keyWidth - blackKeyOffset,
                                                   4.0f * keyWidth,
                                                   5.0f * keyWidth - blackKeyOffset,
                                                   5.0f * keyWidth,
                                                   6.0f * keyWidth - blackKeyOffset,
                                                   6.0f * keyWidth};

  const float blackNoteWidth = BLACK_NOTE_SIZE_RATIO * keyWidth;
  const float whiteNoteWidth = keyWidth;
  const float blackNoteHeight = componentHeight / 2.0f;
  const float whiteNoteHeight = componentHeight;

  for (Utils::PitchClass key : Utils::ALL_PITCH_CLASS) {
    const bool blackKey = isBlackKey(key);
    mNoteRectangleMap[key].setBounds(notePos[key], 0, (blackKey) ? blackNoteWidth : whiteNoteWidth,
                                     (blackKey) ? blackNoteHeight : whiteNoteHeight);
  }
}

void RainbowKeyboard::drawKey(juce::Graphics& g, Utils::PitchClass pitchClass) {
  juce::Colour keyColor = Utils::getRainbow12Colour(pitchClass);
  const bool isDown = pitchClass == mCurrentNote.pitch;

  // if down, extra dark
  // if no note is down, lightly darken if mouse is hovering it
  if (isDown) {
    keyColor = keyColor.darker().darker();
  } else if (pitchClass == mHoverNote.pitch) {
    keyColor = keyColor.darker();
  }

  g.setColour(keyColor);
  juce::Colour color1 = (isDown) ? keyColor.brighter() : keyColor;
  juce::Colour color2 = (isDown) ? keyColor : keyColor.brighter();
  g.setFillType(juce::ColourGradient(color1, juce::Point<float>(0.0f, 0.0f), color2, juce::Point<float>(0, getHeight()), false));

  juce::Rectangle<float> area = mNoteRectangleMap[pitchClass];
  g.fillRect(area);
  g.setColour(juce::Colours::black);
  g.drawRect(area, isBlackKey(pitchClass) ? 2 : 1);
}

void RainbowKeyboard::resized() { fillNoteRectangleMap(); }

void RainbowKeyboard::setNoteOn(Utils::PitchClass pitchClass, float velocity) {
  mCurrentNote = RainbowKeyboard::Note(pitchClass, velocity);
}

void RainbowKeyboard::setNoteOff(Utils::PitchClass pitchClass) { mCurrentNote = RainbowKeyboard::Note(); }

void RainbowKeyboard::mouseMove(const juce::MouseEvent& e) { updateMouseState(e, false); }

void RainbowKeyboard::mouseDrag(const juce::MouseEvent& e) { updateMouseState(e, true); }

void RainbowKeyboard::mouseDown(const juce::MouseEvent& e) { updateMouseState(e, true); }

void RainbowKeyboard::mouseUp(const juce::MouseEvent& e) { updateMouseState(e, false); }

void RainbowKeyboard::mouseEnter(const juce::MouseEvent& e) {
  // This is NOT called if mouseDrag() is still happening
  updateMouseState(e, false);
}

void RainbowKeyboard::mouseExit(const juce::MouseEvent& e) {
  // This is NOT called if mouseDrag() is still happening
  updateMouseState(e, false);
}

/**
  @brief Only used to inject a note into the midiKeyboard state. The listener
  will set the rainbowKeyboard state

  Takes input of Component::keyStateChanged from parent component due to lack
  of always having focus on this component
*/
void RainbowKeyboard::updateMouseState(const juce::MouseEvent& e, bool isDown) {
  juce::Point<float> pos = e.getEventRelativeTo(this).position;
  mHoverNote = xyMouseToNote(pos);

  // Will be invalid if mouse is dragged outside of keyboard
  bool isValidNote = mHoverNote.pitch != Utils::PitchClass::NONE;
  if (mHoverNote.pitch != mCurrentNote.pitch) {
    // Hovering over new note, send note off for old note if necessary
    // Will turn off also if mouse exit keyboard
    if (mCurrentNote.pitch != Utils::PitchClass::NONE) {
      mState.noteOff(MIDI_CHANNEL, mCurrentNote.pitch, mCurrentNote.velocity);
    }
    if (isDown && isValidNote) {
      mState.noteOn(MIDI_CHANNEL, mHoverNote.pitch, mHoverNote.velocity);
    }
  } else {
    if (isDown && (mCurrentNote.pitch == Utils::PitchClass::NONE) && isValidNote) {
      // Note on if pressing current note
      mState.noteOn(MIDI_CHANNEL, mHoverNote.pitch, mHoverNote.velocity);
    } else if ((mCurrentNote.pitch != Utils::PitchClass::NONE) && !isDown) {
      // Note off if released current note
      mState.noteOff(MIDI_CHANNEL, mCurrentNote.pitch, mCurrentNote.velocity);
    }
  }
  repaint();
}

RainbowKeyboard::Note RainbowKeyboard::xyMouseToNote(juce::Point<float> pos) {
  if (!reallyContains(pos.toInt(), false)) return RainbowKeyboard::Note();
  // Since the Juce canvas is all in float, keep in float to prevent strange
  // int-vs-float rounding errors selecting the wrong key
  const float componentHeight = static_cast<float>(getHeight());
  const float blackNoteLength = componentHeight / 2.0f;

  // can skip quick checking black notes from y position
  if (pos.getY() < blackNoteLength) {
    for (Utils::PitchClass pitchClass : BLACK_KEYS_PITCH_CLASS) {
      if (mNoteRectangleMap[pitchClass].contains(pos)) {
        return RainbowKeyboard::Note(pitchClass, juce::jmax(0.0f, 1.0f - (pos.y / blackNoteLength)));
      }
    }
  }

  for (Utils::PitchClass pitchClass : WHITE_KEYS_PITCH_CLASS) {
    if (mNoteRectangleMap[pitchClass].contains(pos)) {
      return RainbowKeyboard::Note(pitchClass, juce::jmax(0.0f, 1.0f - (pos.y / componentHeight)));
    }
  }

  // note not found
  return RainbowKeyboard::Note();
}
