/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

//==============================================================================
GRainbowAudioProcessorEditor::GRainbowAudioProcessorEditor(
    GRainbowAudioProcessor& p)
    : AudioProcessorEditor(&p),
      mProcessor(p),
      mKeyboard(mProcessor.getKeyboardState()),
      mFft(FFT_SIZE, HOP_SIZE),
      juce::Thread("main fft thread"),
      mProgressBar(mLoadingProgress) {
  mFormatManager.registerBasicFormats();

  mProcessor.onNoteChanged = [this](Utils::PitchClass pitchClass,
    bool isNoteOn) {
      mCurPitchClass = pitchClass;
    if (isNoteOn) {
      mStartedPlayingTrig = true;
    } else {
      mArcSpec.setNoteOff();
    }
  };

  setLookAndFeel(&mRainbowLookAndFeel);

  /* Open file button */
  juce::Image openFileNormal = juce::PNGImageFormat::loadFrom(
      BinaryData::openFileNormal_png, BinaryData::openFileNormal_pngSize);
  juce::Image openFileOver = juce::PNGImageFormat::loadFrom(
      BinaryData::openFileOver_png, BinaryData::openFileOver_pngSize);
  mBtnOpenFile.setImages(false, true, true, openFileNormal, 1.0f,
                         juce::Colours::transparentBlack, openFileOver, 1.0f,
                         juce::Colours::transparentBlack, openFileOver, 1.0f,
                         juce::Colours::transparentBlack);
  mBtnOpenFile.onClick = [this] { openNewFile(); };
  addAndMakeVisible(mBtnOpenFile);

  /* Recording button */
  juce::Image recordIcon = juce::PNGImageFormat::loadFrom(
      BinaryData::microphone_png, BinaryData::microphone_pngSize);
  juce::Image recordOver = juce::PNGImageFormat::loadFrom(
      BinaryData::microphoneOver_png, BinaryData::microphoneOver_pngSize);
  mBtnRecord.setImages(false, true, true, recordIcon, 1.0f,
                       juce::Colours::transparentBlack, recordOver, 1.0f,
                       juce::Colours::transparentBlack, recordOver, 1.0f,
                       juce::Colours::transparentBlack);
  mBtnRecord.onClick = [this] {
    if (mRecorder.isRecording()) {
      stopRecording();
    } else {
      startRecording();
    }
  };
  addAndMakeVisible(mBtnRecord);

  /* Position tabs */
  mPositionTabs.onTabChanged = [this](Utils::GeneratorColour tab,
                                      bool isSelected, bool isEnabled) {
    mPositionBoxes[tab].setVisible(isSelected);
    mPositionBoxes[tab].setActive(isEnabled);
    mProcessor.synth.updateParameter(tab, GranularSynth::ParameterType::ENABLED,
                           isEnabled);
    std::vector<bool> positionStates;
    for (int i = 0; i < Utils::GeneratorColour::NUM_GEN; ++i) {
      bool canPlay =
          mPositionBoxes[i].getState() != PositionBox::BoxState::SOLO_WAIT;
      bool shouldPlay =
          mPositionBoxes[i].getActive() ||
          mPositionBoxes[i].getState() == PositionBox::BoxState::SOLO;
      positionStates.push_back(canPlay && shouldPlay);
    }
    mProcessor.synth.updateGeneratorStates(positionStates);
  };
  addAndMakeVisible(mPositionTabs);

  /* Position boxes */
  for (int i = 0; i < mPositionBoxes.size(); ++i) {
    mPositionBoxes[i].setColour((Utils::GeneratorColour)i);
    mPositionBoxes[i].setActive(i == 0);
    mPositionBoxes[i].setParams(
        mProcessor.synth.getGeneratorParams((Utils::GeneratorColour)i));
    mPositionBoxes[i].onPositionChanged = [this, i](bool isRight) {
      int newPosition = mProcessor.synth.incrementPosition(i, isRight);
      mPositionBoxes[i].setPositionNumber(newPosition);
    };
    mPositionBoxes[i].onParameterChanged =
        [this](Utils::GeneratorColour pos, GranularSynth::ParameterType param,
               float value) {
          if (param == GranularSynth::ParameterType::SOLO) {
            std::vector<bool> positionStates;
            for (int i = 0; i < Utils::GeneratorColour::NUM_GEN; ++i) {
              if (i != pos) {
                mPositionBoxes[i].setState(
                    (value == true) ? PositionBox::BoxState::SOLO_WAIT
                                    : PositionBox::BoxState::READY);
              }
              bool canPlay = mPositionBoxes[i].getState() !=
                             PositionBox::BoxState::SOLO_WAIT;
              bool shouldPlay =
                  mPositionBoxes[i].getActive() ||
                  mPositionBoxes[i].getState() == PositionBox::BoxState::SOLO;
              positionStates.push_back(canPlay && shouldPlay);
            }
            mProcessor.synth.updateGeneratorStates(positionStates);
          }
          mProcessor.synth.updateParameter(pos, param, value);
        };
    addChildComponent(mPositionBoxes[i]);
    if (i == 0) mPositionBoxes[i].setVisible(true);
  }

  /* Arc spectrogram */
  addAndMakeVisible(mArcSpec);

  addChildComponent(mProgressBar);

  mTransientDetector.onTransientsUpdated =
      [this](std::vector<TransientDetector::Transient>& transients) {
        mArcSpec.setTransients(&transients);
      };

  mPitchDetector.onPitchesUpdated =
      [this](std::vector<std::vector<float>>& hpcpBuffer,
             std::vector<std::vector<float>>& notesBuffer) {
        mArcSpec.loadBuffer(&hpcpBuffer, ArcSpectrogram::SpecType::HPCP);
        mArcSpec.loadBuffer(&notesBuffer, ArcSpectrogram::SpecType::NOTES);
        mProcessor.synth.setPitches(&mPitchDetector.getPitches());
        mIsProcessingComplete = true;
      };

  mPitchDetector.onProgressUpdated = [this](float progress) {
    mLoadingProgress = progress;
    juce::MessageManagerLock lock;
    if (progress >= 1.0) {
      mProgressBar.setVisible(false);
    } else if (!mProgressBar.isVisible()) {
      mProgressBar.setVisible(true);
    }
  };

  addAndMakeVisible(mKeyboard);

  mAudioDeviceManager.initialise(1, 2, nullptr, true, {}, nullptr);

  mAudioDeviceManager.addAudioCallback(&mRecorder);

  startTimer(50);  // Keyboard focus timer

  setSize(1200, 565);
}

GRainbowAudioProcessorEditor::~GRainbowAudioProcessorEditor() {
  auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
  auto recordFile = parentDir.getChildFile(RECORDING_FILE);
  recordFile.deleteFile();
  mAudioDeviceManager.removeAudioCallback(&mRecorder);
  setLookAndFeel(nullptr);
  stopThread(4000);
}

void GRainbowAudioProcessorEditor::timerCallback() {
  if (mStartedPlayingTrig && mCurPitchClass != Utils::PitchClass::NONE) {
    int numFoundPositions = mProcessor.synth.getNumFoundPositions();
    std::vector<bool> tabStates;
    for (int i = 0; i < mPositionBoxes.size(); ++i) {
      mPositionBoxes[i].setParams(
          mProcessor.synth.getGeneratorParams((Utils::GeneratorColour)i));
      mPositionBoxes[i].setNumPositions(numFoundPositions);
      tabStates.push_back(
          mProcessor.synth.getGeneratorParams((Utils::GeneratorColour)i)
              .isActive);
    }
    mPositionTabs.setTabStates(tabStates);
    std::vector<GrainPositionFinder::GrainPosition> positions =
        mProcessor.synth.getCurrentPositions();
    mArcSpec.setNoteOn(mCurPitchClass, mProcessor.synth.getCurrentPositions());
    mStartedPlayingTrig = false;
    repaint();  // Update note display
  }
}

void GRainbowAudioProcessorEditor::run() {
  mFft.processBuffer(mFileBuffer);
  mArcSpec.loadBuffer(&mFft.getSpectrum(),
                      ArcSpectrogram::SpecType::SPECTROGRAM);
}

//==============================================================================
void GRainbowAudioProcessorEditor::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  // Draw background for open file button
  g.setColour(juce::Colours::darkgrey);
  g.fillRoundedRectangle(mBtnOpenFile.getBounds().toFloat(), 14);

  // Draw background for record button
  g.setColour(mRecorder.isRecording() ? juce::Colours::red
                                      : juce::Colours::darkgrey);
  g.fillRoundedRectangle(mBtnRecord.getBounds().toFloat(), 14);
}

void GRainbowAudioProcessorEditor::paintOverChildren(juce::Graphics& g) {
  // Draw note display
  if (mCurPitchClass != Utils::PitchClass::NONE) {
    std::vector<GrainPositionFinder::GrainPosition> positions =
        mProcessor.synth.getCurrentPositions();
    // Draw position arrows
    for (int i = 0; i < positions.size(); ++i) {
      if (positions[i].pitch.pitchClass == Utils::PitchClass::NONE) continue;
      g.setColour(juce::Colour(Utils::POSITION_COLOURS[i]));
      auto middlePos = positions[i].pitch.posRatio + (positions[i].pitch.duration / 2.0f);
      float angleRad = (juce::MathConstants<float>::pi * middlePos) -
                       (juce::MathConstants<float>::pi / 2.0f);
      juce::Point<float> startPoint = juce::Point<float>(
          mNoteDisplayRect.getCentreX(), mNoteDisplayRect.getY());
      juce::Point<float> endPoint = startPoint.getPointOnCircumference(
          mArcSpec.getHeight() / 4.5f, angleRad);
      g.drawArrow(juce::Line<float>(startPoint, endPoint), 4.0f, 10.0f, 6.0f);
    }
    // Draw path to positions
    float noteX =
        mKeyboard.getBounds().getX() +
        (mKeyboard.getWidth() * mKeyboard.getPitchXRatio(mCurPitchClass));
    juce::Path displayPath;
    displayPath.startNewSubPath(noteX, mNoteDisplayRect.getBottom());
    displayPath.lineTo(
        noteX, mNoteDisplayRect.getBottom() - (NOTE_DISPLAY_HEIGHT / 2.0f));
    displayPath.lineTo(mNoteDisplayRect.getCentre());
    displayPath.lineTo(mNoteDisplayRect.getCentreX(), mNoteDisplayRect.getY());
    g.setColour(Utils::getRainbow12Colour(mCurPitchClass));
    g.strokePath(displayPath, juce::PathStrokeType(4.0f));
    g.fillEllipse(mNoteDisplayRect.getCentreX() - (NOTE_BULB_SIZE / 2.0f),
                  mNoteDisplayRect.getY() - (NOTE_BULB_SIZE / 2.0f),
                  NOTE_BULB_SIZE, NOTE_BULB_SIZE);
  }
}

void GRainbowAudioProcessorEditor::resized() {
  auto r = getLocalBounds();

  // Position tabs
  auto leftPanel = r.removeFromLeft(PANEL_WIDTH);
  mPositionTabs.setBounds(leftPanel.removeFromTop(TABS_HEIGHT));
  // Position boxes
  for (int i = 0; i < mPositionBoxes.size(); ++i) {
    mPositionBoxes[i].setBounds(leftPanel);
  }

  auto rightPanel = r.removeFromRight(PANEL_WIDTH);
  // TODO: global param box

  // Open and record buttons
  auto filePanel = r.removeFromTop(BTN_PANEL_HEIGHT + BTN_PADDING);
  filePanel.removeFromLeft(BTN_PADDING);
  filePanel.removeFromTop(BTN_PADDING);
  mBtnOpenFile.setBounds(filePanel.removeFromLeft(OPEN_FILE_WIDTH));
  filePanel.removeFromLeft(BTN_PADDING);
  mBtnRecord.setBounds(filePanel.removeFromLeft(OPEN_FILE_WIDTH));

  r.removeFromTop(NOTE_DISPLAY_HEIGHT);  // Just padding

  // Arc spectrogram
  mArcSpec.setBounds(r.removeFromTop(r.getWidth() / 2.0f));
  mProgressBar.setBounds(
      mArcSpec.getBounds().withSizeKeepingCentre(PROGRESS_SIZE, PROGRESS_SIZE));

  // Space for note display
  mNoteDisplayRect = r.removeFromTop(NOTE_DISPLAY_HEIGHT).toFloat();

  // Keyboard
  juce::Rectangle<int> keyboardRect =
      r.removeFromTop(r.getHeight() - NOTE_DISPLAY_HEIGHT)
          .reduced(NOTE_DISPLAY_HEIGHT, 0.0f);
  mKeyboard.setBounds(keyboardRect);
}

/** Pauses audio to open file
    @param path optional path to load, otherwise will prompt user for file
   location
*/
void GRainbowAudioProcessorEditor::openNewFile(const char* path) {
  if (path == nullptr) {
    juce::FileChooser chooser("Select a file to granulize...",
                              juce::File::getCurrentWorkingDirectory(),
                              "*.wav;*.mp3", true);

    if (chooser.browseForFileToOpen()) {
      auto file = chooser.getResult();
      processFile(file);
    }
  } else {
    auto file = juce::File(juce::String(path));
    processFile(file);
  }
}

void GRainbowAudioProcessorEditor::processFile(juce::File file) {
  std::unique_ptr<juce::AudioFormatReader> reader(
      mFormatManager.createReaderFor(file));

  if (reader.get() != nullptr) {
    mFileBuffer.setSize(reader->numChannels, (int)reader->lengthInSamples);

    juce::AudioBuffer<float> tempBuffer = juce::AudioBuffer<float>(
        reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&tempBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
    std::unique_ptr<juce::LagrangeInterpolator> resampler =
        std::make_unique<juce::LagrangeInterpolator>();
    double ratio = reader->sampleRate / mProcessor.getSampleRate();
    const float** inputs = tempBuffer.getArrayOfReadPointers();
    float** outputs = mFileBuffer.getArrayOfWritePointers();
    for (int c = 0; c < mFileBuffer.getNumChannels(); c++) {
      resampler->reset();
      resampler->process(ratio, inputs[c], outputs[c],
                         mFileBuffer.getNumSamples());
    }
    mProcessor.synth.resetParameters();
    mArcSpec.resetBuffers();
    stopThread(4000);
    startThread();  // process fft and pass to arc spec
    // mTransientDetector.processBuffer(&mFileBuffer);
    mPitchDetector.processBuffer(&mFileBuffer, mProcessor.getSampleRate());
    mProcessor.synth.setFileBuffer(&mFileBuffer, mProcessor.getSampleRate());
    mIsProcessingComplete = false;  // Reset processing flag
  }
}

void GRainbowAudioProcessorEditor::startRecording() {
  if (!juce::RuntimePermissions::isGranted(
          juce::RuntimePermissions::writeExternalStorage)) {
    SafePointer<GRainbowAudioProcessorEditor> safeThis(this);

    juce::RuntimePermissions::request(
        juce::RuntimePermissions::writeExternalStorage,
        [safeThis](bool granted) mutable {
          if (granted) safeThis->startRecording();
        });
    return;
  }

  auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
  parentDir.getChildFile(RECORDING_FILE).deleteFile();
  mRecordedFile = parentDir.getChildFile(RECORDING_FILE);

  mRecorder.startRecording(mRecordedFile);

  juce::Image recordIcon = juce::PNGImageFormat::loadFrom(
      BinaryData::microphone_png, BinaryData::microphone_pngSize);
  mBtnRecord.setImages(false, true, true, recordIcon, 1.0f,
                       juce::Colours::transparentBlack, recordIcon, 1.0f,
                       juce::Colours::transparentBlack, recordIcon, 1.0f,
                       juce::Colours::transparentBlack);
  repaint();
}

void GRainbowAudioProcessorEditor::stopRecording() {
  mRecorder.stop();

  processFile(mRecordedFile);

  mRecordedFile = juce::File();

  juce::Image recordIcon = juce::PNGImageFormat::loadFrom(
      BinaryData::microphone_png, BinaryData::microphone_pngSize);
  juce::Image recordOver = juce::PNGImageFormat::loadFrom(
      BinaryData::microphoneOver_png, BinaryData::microphoneOver_pngSize);
  mBtnRecord.setImages(false, true, true, recordIcon, 1.0f,
                       juce::Colours::transparentBlack, recordOver, 1.0f,
                       juce::Colours::transparentBlack, recordOver, 1.0f,
                       juce::Colours::transparentBlack);
  repaint();
}