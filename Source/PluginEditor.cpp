/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#define GRAINBOW_PRODUCTION 1

#include "PluginEditor.h"
#include "Preset.h"
#include "BinaryData.h"
#include "Utils/Colour.h"
#include "Utils/MidiNote.h"

// Used for getting memory usage
#ifdef __linux__
#include <sys/sysinfo.h>
#include <unistd.h>
#endif
#ifdef _WINDOWS
#include <windows.h>
#include <Psapi.h>
#endif

GRainbowLogo::GRainbowLogo() { mLogoImage = juce::PNGImageFormat::loadFrom(BinaryData::logo_png, BinaryData::logo_pngSize); }

void GRainbowLogo::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::transparentBlack);
  g.drawImage(mLogoImage, getLocalBounds().toFloat(),
              juce::RectanglePlacement(juce::RectanglePlacement::yBottom | juce::RectanglePlacement::yTop), false);
}

//==============================================================================
GRainbowAudioProcessorEditor::GRainbowAudioProcessorEditor(GranularSynth& synth)
    : AudioProcessorEditor(&synth),
      mSynth(synth),
      mParameters(synth.getParams()),
      mArcSpec(synth.getParams()),
      mTrimSelection(synth.getFormatManager(), synth.getParamUI()),
      mProgressBar(mParameters.ui.loadingProgress),
      mKeyboard(synth.getKeyboardState(), synth.getParams()),
      mEnvAdsr(synth.getParams()),
      mEnvGrain(synth.getParams()),
      mGrainControl(synth.getParams(), synth.getMeterSource()),
      mFilterControl(synth.getParams()) {
  setLookAndFeel(&mRainbowLookAndFeel);
  mRainbowLookAndFeel.setColour(juce::PopupMenu::ColourIds::backgroundColourId, Utils::GLOBAL_COLOUR);

  // Search for better fonts
  {
    juce::Array<juce::Font> fonts;
    juce::Font::findFonts(fonts);
    for (size_t i = 0; i < fonts.size(); i++) {
      // Ordred by fonts we want to use first if found
      juce::String fontName = fonts[i].getTypefaceName();
      if (fontName.equalsIgnoreCase("Century Gothic")) {
        juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypefaceName(fontName);
        break;
      } else if (fontName.equalsIgnoreCase("Ubuntu")) {
        juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypefaceName(fontName);
        break;
      }
    }
  }

  mErrorMessage.clear();

  // Open file button
  juce::Image normal = juce::PNGImageFormat::loadFrom(BinaryData::openFileNormal_png, BinaryData::openFileNormal_pngSize);
  juce::Image over = juce::PNGImageFormat::loadFrom(BinaryData::openFileOver_png, BinaryData::openFileOver_pngSize);
  mBtnOpenFile.setImages(false, true, true, normal, 1.0f, juce::Colours::transparentBlack, over, 1.0f,
                         juce::Colours::transparentBlack, over, 1.0f, juce::Colours::transparentBlack);
  mBtnOpenFile.onClick = [this] { openNewFile(); };
  mBtnOpenFile.setTooltip("Load new sample from file or preset");
  addAndMakeVisible(mBtnOpenFile);

  // Recording button
  normal = juce::PNGImageFormat::loadFrom(BinaryData::microphone_png, BinaryData::microphone_pngSize);
  over = juce::PNGImageFormat::loadFrom(BinaryData::microphoneOver_png, BinaryData::microphoneOver_pngSize);
  mBtnRecord.setImages(false, true, true, normal, 1.0f, juce::Colours::transparentBlack, over, 1.0f,
                       juce::Colours::transparentBlack, over, 1.0f, juce::Colours::transparentBlack);
  mBtnRecord.onClick = [this] {
    if (mRecorder.isRecording()) {
      stopRecording();
    } else {
      startRecording();
    }
  };
  mBtnRecord.setTooltip("Record to add new sample");
  addAndMakeVisible(mBtnRecord);

  // Preset button
  normal = juce::PNGImageFormat::loadFrom(BinaryData::presetNormal_png, BinaryData::presetNormal_pngSize);
  over = juce::PNGImageFormat::loadFrom(BinaryData::presetOver_png, BinaryData::presetOver_pngSize);
  mBtnSavePreset.setImages(false, true, true, normal, 1.0f, juce::Colours::transparentBlack, over, 1.0f,
                       juce::Colours::transparentBlack, over, 1.0f, juce::Colours::transparentBlack);
  mBtnSavePreset.onClick = [this] { savePreset(); };
  mBtnSavePreset.setTooltip("Save everything as a preset");
  addAndMakeVisible(mBtnSavePreset);
  // if reloading and images are done, then enable right away
  mBtnSavePreset.setEnabled(mParameters.ui.specComplete);

  // Plugin info button
  normal = juce::PNGImageFormat::loadFrom(BinaryData::infoNormal_png, BinaryData::infoNormal_pngSize);
  over = juce::PNGImageFormat::loadFrom(BinaryData::infoOver_png, BinaryData::infoOver_pngSize);
  mBtnInfo.setImages(false, true, true, normal, 1.0f, juce::Colours::transparentBlack, over, 1.0f,
                           juce::Colours::transparentBlack, over, 1.0f, juce::Colours::transparentBlack);
  mBtnInfo.onClick = [] { juce::URL(MANUAL_URL).launchInDefaultBrowser(); };
  mBtnInfo.setTooltip("Open gRainbow manual");
  addAndMakeVisible(mBtnInfo);

  // File info label
  mLabelFileName.setColour(juce::Label::ColourIds::textColourId, Utils::GLOBAL_COLOUR);
  mLabelFileName.setJustificationType(juce::Justification::centred);
  if (!mParameters.ui.fileName.isEmpty()) {
    // Set if saved from reopening plugin
    mLabelFileName.setText(mParameters.ui.fileName, juce::dontSendNotification);
  }
  addAndMakeVisible(mLabelFileName);

  // Arc spectrogram
  mArcSpec.onImagesComplete = [this]() {
    const juce::MessageManagerLock lock;
    jassert(mParameters.ui.specComplete);
    mArcSpec.setSpecType(ParamUI::SpecType::WAVEFORM);
    mBtnSavePreset.setEnabled(true);
  };

  mTrimSelection.onCancel = [this]() {
    // if nothing was ever loaded, got back to the logo
    updateCenterComponent((mParameters.ui.specComplete) ? ParamUI::CenterComponent::ARC_SPEC : ParamUI::CenterComponent::LOGO);
    mParameters.ui.fileName = mParameters.ui.loadedFileName;
    mLabelFileName.setText(mParameters.ui.fileName, juce::dontSendNotification);
  };

  mTrimSelection.onProcessSelection = [this](juce::Range<double> range) {
    // Convert time to sample range
    const double sampleLength = static_cast<double>(mSynth.getInputBuffer().getNumSamples());
    const double secondLength = sampleLength / mSynth.getSampleRate();
    juce::int64 start = static_cast<juce::int64>(sampleLength * (range.getStart() / secondLength));
    juce::int64 end = static_cast<juce::int64>(sampleLength * (range.getEnd() / secondLength));
    // TODO - if small enough, it will get stuck trying to load
    if (start == end) {
      displayError("Attempted to select an empty range");
    } else {
      mParameters.ui.trimPlaybackOn = false;
      mSynth.resetParameters();
      mSynth.trimAudioBuffer(mSynth.getInputBuffer(), mSynth.getAudioBuffer(), juce::Range<juce::int64>(start, end));
      mSynth.extractSpectrograms();
      mSynth.extractPitches();
      mSynth.getInputBuffer();
      // Reset any UI elements that will need to wait until processing
      mArcSpec.reset();
      mBtnSavePreset.setEnabled(false);
      updateCenterComponent(ParamUI::CenterComponent::ARC_SPEC);
      mArcSpec.loadWaveformBuffer(&mSynth.getAudioBuffer());
      mParameters.ui.loadedFileName = mParameters.ui.fileName;
      mParameters.ui.trimRange = range;
    }
  };

  // Let other components know when the selected note or generator has been updated
  mParameters.onSelectedChange = [this]() {
    mEnvAdsr.updateSelectedParams();
    mEnvGrain.updateSelectedParams();
    mFilterControl.updateSelectedParams();
    mGrainControl.updateSelectedParams();
    mRainbowLookAndFeel.setColour(juce::PopupMenu::ColourIds::backgroundColourId, mParameters.getSelectedParamColour());
  };
  addAndMakeVisible(mKeyboard);

  // These share the same space, but only 1 is seen at a time
  addAndMakeVisible(mLogo);
  addChildComponent(mArcSpec);
  addChildComponent(mProgressBar);
  addChildComponent(mTrimSelection);

  addAndMakeVisible(mEnvAdsr);
  addAndMakeVisible(mEnvGrain);
  addAndMakeVisible(mFilterControl);
  mGrainControl.onRefToneOn =[this](){
    mSynth.startReferenceTone(mParameters.getSelectedPitchClass());
  };
  mGrainControl.onRefToneOff = [this](){
    mSynth.stopReferenceTone();
  };
  addAndMakeVisible(mGrainControl);

  mCloudLeftImage = juce::PNGImageFormat::loadFrom(BinaryData::cloudLeft_png, BinaryData::cloudLeft_pngSize);
  mCloudRightImage = juce::PNGImageFormat::loadFrom(BinaryData::cloudRight_png, BinaryData::cloudRight_pngSize);
  mRainImage = juce::PNGImageFormat::loadFrom(BinaryData::rain_png, BinaryData::rain_pngSize);
  // Use different offsets to start to make look like different images
  mLeftRainDeltY = mRainImage.getHeight() / 2;
  mRightRainDeltY = mRainImage.getHeight() / 3;

  // Only want keyboard input focus for standalone as DAW will have own input
  // mappings
  if (mSynth.wrapperType == GranularSynth::WrapperType::wrapperType_Standalone) {
    setWantsKeyboardFocus(true);
  }

  mTooltipWindow->setMillisecondsBeforeTipAppears(500);  // default is 700ms

  startTimer(50);

#ifndef GRAINBOW_PRODUCTION
  addAndMakeVisible(mSettings);
  Utils::EDITOR_HEIGHT += mSettings.getHeight();
#endif

  setSize(Utils::EDITOR_WIDTH, Utils::EDITOR_HEIGHT);

  // Will update to be what it was when editor was last closed
  updateCenterComponent(mParameters.ui.centerComponent);
}

GRainbowAudioProcessorEditor::~GRainbowAudioProcessorEditor() {
  // can't wait for the message manager to eventually delete this
  if (mDialogWindow != nullptr) {
    mDialogWindow->exitModalState(0);
    delete mDialogWindow;
  }

  auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
  auto recordFile = parentDir.getChildFile(FILE_RECORDING);
  recordFile.deleteFile();
  mAudioDeviceManager.removeAudioCallback(&mRecorder);
  setLookAndFeel(nullptr);

  mSynth.stopReferenceTone();
}

void GRainbowAudioProcessorEditor::updateCenterComponent(ParamUI::CenterComponent component) {
  mParameters.ui.centerComponent = component;
  mLogo.setVisible(component == ParamUI::CenterComponent::LOGO);
  mArcSpec.setVisible(component == ParamUI::CenterComponent::ARC_SPEC);
  mTrimSelection.setVisible(component == ParamUI::CenterComponent::TRIM_SELECTION);
}

void GRainbowAudioProcessorEditor::timerCallback() {
  // Update progress bar when loading audio clip
  // Will overlay on the other center components
  if (mParameters.ui.loadingProgress < 1.0 && mParameters.ui.loadingProgress > 0.0) {
    mProgressBar.setVisible(true);
  } else {
    mProgressBar.setVisible(false);
  }

  // Check for buffers needing to be updated
  if (!mParameters.ui.specComplete) {
    std::vector<Utils::SpecBuffer*> specs = mSynth.getProcessedSpecs();
    for (size_t i = 0; i < specs.size(); ++i) {
      if (specs[i] != nullptr && mArcSpec.shouldLoadImage((ParamUI::SpecType)i))
        mArcSpec.loadSpecBuffer(specs[i], (ParamUI::SpecType)i);
    }
  }

  // Get notes being played, send off to each children and then redraw.
  // Grab the notes from the Synth instead of MidiKeyboardState::Listener to not block the thread to draw.
  // There is a chance notes are pressed and released inbetween timer callback if they are super short, but can always increase the
  // callback timer
  const juce::Array<Utils::MidiNote>& midiNotes = mSynth.getMidiNotes();
  // Each component has has a different use for the midi notes, so just give them the notes and have them do what logic they want
  // with it
  mKeyboard.setMidiNotes(midiNotes);
  mArcSpec.setMidiNotes(midiNotes);

  /* if (PowerUserSettings::get().getResourceUsage()) {
    const double cpuPerc = mAudioDeviceManager.getCpuUsage() * 100;
    size_t virtual_memory = 0;
    size_t resident_memory = 0;
#if defined(__linux__)
    FILE* file = fopen("/proc/self/statm", "r");
    if (file) {
      unsigned long VmSize = 0;
      unsigned long VmRSS = 0;
      fscanf(file, "%lu %lu", &VmSize, &VmRSS);
      fclose(file);
      virtual_memory = static_cast<size_t>(VmSize) * getpagesize();
      resident_memory = static_cast<size_t>(VmRSS) * getpagesize();
    }
#elif defined(_WINDOWS)
    // According to MSDN
    PROCESS_MEMORY_COUNTERS counters;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters))) {
      virtual_memory = counters.PagefileUsage;
      resident_memory = counters.WorkingSetSize;
    }
#endif

    mResourceUsage.setText(juce::String(cpuPerc, 1) + "% CPU | Virtual " + juce::String(virtual_memory >> 20) + " MB" +
                               " | Resident " +
                               juce::String(resident_memory >> 20) + " MB",
                           juce::dontSendNotification);
  }*/

  repaint();
}

//==============================================================================
void GRainbowAudioProcessorEditor::paint(juce::Graphics& g) {
  // Set gradient
  g.setFillType(Utils::getBgGradient(getBounds(), mParameters.ui.loadingProgress));
  g.fillRect(getLocalBounds());
  /*
  // Draw background for open file button
  g.setColour(juce::Colours::darkgrey);
  g.fillRoundedRectangle(mBtnOpenFile.getBounds().toFloat(), 14);

  // Draw background for record button
  g.setColour(mRecorder.isRecording() ? juce::Colours::red : juce::Colours::darkgrey);
  g.fillRoundedRectangle(mBtnRecord.getBounds().toFloat(), 14);

  // Draw background for preset button
  g.setColour(juce::Colours::darkgrey);
  g.fillRoundedRectangle(mBtnSavePreset.getBounds().toFloat(), 14);

  // Draw background for info button
  g.setColour(juce::Colours::darkgrey);
  g.fillRoundedRectangle(mBtnInfo.getBounds().toFloat(), 14);*/
}

/**
  @brief Draw note display (the small section between the keyboard and arc spectrogram)
*/
void GRainbowAudioProcessorEditor::paintOverChildren(juce::Graphics& g) {
  // Right now just give last note played, not truely polyphony yet
  // TODO: new note displaying
  /*
  const juce::Array<Utils::MidiNote>& midiNotes = mSynth.getMidiNotes();
  if (!midiNotes.isEmpty()) {
    // If there are not candidates, will just not draw any arrows/lines
    std::vector<ParamCandidate*> candidates = mSynth.getActiveCandidates();
    if (!candidates.empty()) {
      // Draw position arrows
      juce::Colour pitchColour = Utils::getRainbow12Colour(mSynth.getLastPitchClass());
      for (int i = 0; i < candidates.size(); ++i) {
        if (candidates[i] == nullptr) continue;
        // TODO: fix this coloring below
        g.setColour(pitchColour);
        //g.setColour((i == mGeneratorsBox.getSelectedGenerator()) ? pitchColour.brighter() : pitchColour.darker().darker());
        auto middlePos = candidates[i]->posRatio + (candidates[i]->duration / 2.0f);
        float angleRad = (juce::MathConstants<float>::pi * middlePos) - (juce::MathConstants<float>::pi / 2.0f);
        juce::Point<float> startPoint = juce::Point<float>(mNoteDisplayRect.getCentreX(), mNoteDisplayRect.getY());
        juce::Point<float> endPoint = startPoint.getPointOnCircumference(mArcSpec.getHeight() / 4.5f, angleRad);
        g.drawArrow(juce::Line<float>(startPoint, endPoint), 4.0f, 10.0f, 6.0f);
      }

      // Draw path from rainbow key to the arrow's base
      for (const Utils::MidiNote& midiNote : midiNotes) {
        const Utils::PitchClass pitchClass = midiNote.pitch;
        // if more than 1 note played, the last note will be empathized
        const bool empathized = (midiNote == midiNotes.getLast());

        float noteX = mKeyboard.getBounds().getX() + (mKeyboard.getWidth() * mKeyboard.getPitchXRatio(pitchClass));
        juce::Path displayPath;
        displayPath.startNewSubPath(noteX, mNoteDisplayRect.getBottom());
        displayPath.lineTo(noteX, mNoteDisplayRect.getBottom() - (NOTE_DISPLAY_HEIGHT / 2.0f));
        displayPath.lineTo(mNoteDisplayRect.getCentre());
        displayPath.lineTo(mNoteDisplayRect.getCentreX(), mNoteDisplayRect.getY());
        const float alpha = (empathized) ? 1.0f : 0.3f;
        g.setColour(Utils::getRainbow12Colour(pitchClass).withAlpha(alpha));
        g.strokePath(displayPath, juce::PathStrokeType(4.0f));
        if (empathized) {
          g.fillEllipse(mNoteDisplayRect.getCentreX() - (NOTE_BULB_SIZE / 2.0f), mNoteDisplayRect.getY() - (NOTE_BULB_SIZE / 2.0f),
                        NOTE_BULB_SIZE, NOTE_BULB_SIZE);
        }
      }
    }
  }*/

  // When dragging a file over, give feedback it will be accepted when released
  if (mIsFileHovering) {
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.fillRect(getLocalBounds());
  }

  // Border path
  g.setColour(Utils::GLOBAL_COLOUR);
  g.strokePath(mBorderPath, juce::PathStrokeType(2));

  // Clouds
  if (mParameters.ui.centerComponent == ParamUI::CenterComponent::ARC_SPEC) {
    g.drawImage(mCloudLeftImage, mCloudLeftTargetArea, juce::RectanglePlacement::fillDestination);
    g.drawImage(mCloudRightImage, mCloudRightTargetArea, juce::RectanglePlacement::fillDestination);

    // Make it rain girl (while loading)
    if (mProgressBar.isVisible()) {
      g.setColour(juce::Colours::blue);
      // Exploit fact left and right are same dimension
      const int rainHeight = mLeftRain.getHeight();
      const int rainWidth = mLeftRain.getWidth();
      g.drawImage(mRainImage, mLeftRain.getX(), mLeftRain.getY(), rainWidth, rainHeight, 0, mLeftRainDeltY, rainWidth, rainHeight);
      g.drawImage(mRainImage, mRightRain.getX(), mRightRain.getY(), rainWidth, rainHeight, 0, mRightRainDeltY, rainWidth,
                  rainHeight);

      // make rain slow up as closer to full progress (which is the value 1.0)
      const int speed = 20 - static_cast<int>(18.0 * mParameters.ui.loadingProgress);
      mLeftRainDeltY -= speed;
      mRightRainDeltY -= speed;
      if (mLeftRainDeltY < rainHeight) {
        mLeftRainDeltY = mRainImage.getHeight() - rainHeight;
      }
      if (mRightRainDeltY < rainHeight) {
        mRightRainDeltY = mRainImage.getHeight() - rainHeight;
      }
    }
  }
}

void GRainbowAudioProcessorEditor::resized() {
  auto r = getLocalBounds();

#ifndef GRAINBOW_PRODUCTION
  mSettings.setBounds(r.removeFromBottom(mSettings.getHeight()));
#endif

  // Rainbow keyboard
  juce::Rectangle<int> keyboardRect = r.removeFromBottom(Utils::KEYBOARD_HEIGHT);
  mKeyboard.setBounds(keyboardRect);

  // Left and right panels
  auto leftPanel = r.removeFromLeft(Utils::PANEL_WIDTH);
  mEnvGrain.setBounds(leftPanel.removeFromTop(leftPanel.getHeight() / 2.0f));
  mEnvAdsr.setBounds(leftPanel);

  auto rightPanel = r.removeFromRight(Utils::PANEL_WIDTH);
  // TODO: add back in resource usage
  // TODO: add modulators
  mGrainControl.setBounds(rightPanel.removeFromTop(rightPanel.getHeight() / 2.0f));
  mFilterControl.setBounds(rightPanel);

  // Open and record buttons
  auto filePanel = r.removeFromTop(BTN_PANEL_HEIGHT).reduced(Utils::PADDING);
  const int btnWidth = filePanel.getHeight();
  mBtnOpenFile.setBounds(filePanel.removeFromLeft(btnWidth));
  filePanel.removeFromLeft(Utils::PADDING);
  mBtnRecord.setBounds(filePanel.removeFromLeft(btnWidth));
  // preset button
  mBtnSavePreset.setBounds(filePanel.removeFromRight(btnWidth));
  filePanel.removeFromLeft(Utils::PADDING);
  // info button
  mBtnInfo.setBounds(filePanel.removeFromRight(btnWidth));
  // remaining space on sides remaing is for file information
  mLabelFileName.setBounds(filePanel);

  // Center middle space
  mLogo.setBounds(r.reduced(r.getHeight() / 4.0f));
  const juce::Rectangle<int> centerRect = r.removeFromTop(r.getWidth() / 2.0f);
  mArcSpec.setBounds(centerRect);
  mTrimSelection.setBounds(centerRect);
  mProgressBar.setBounds(centerRect.withSizeKeepingCentre(PROGRESS_SIZE, PROGRESS_SIZE));

  // Border path around children
  const float halfRound = Utils::ROUNDED_AMOUNT / 2.0f;
  mBorderPath.clear();
  auto mArcBounds = mArcSpec.getBounds().toFloat();
  mBorderPath.startNewSubPath(mArcBounds.getTopLeft().translated(Utils::ROUNDED_AMOUNT, 0));
  auto point = mArcBounds.getTopRight().translated(-Utils::ROUNDED_AMOUNT, 0);
  mBorderPath.lineTo(point);
  mBorderPath.cubicTo(point.translated(halfRound, 0), point.translated(Utils::ROUNDED_AMOUNT, halfRound),
                      point.translated(Utils::ROUNDED_AMOUNT, Utils::ROUNDED_AMOUNT));
  point = mArcBounds.getBottomRight().withY(mKeyboard.getBounds().getY() - Utils::ROUNDED_AMOUNT);
  mBorderPath.lineTo(point);
  mBorderPath.cubicTo(point.translated(0, halfRound), point.translated(halfRound, Utils::ROUNDED_AMOUNT),
                      point.translated(Utils::ROUNDED_AMOUNT, Utils::ROUNDED_AMOUNT));
  point = mKeyboard.getBounds().getTopRight().toFloat().translated(-Utils::ROUNDED_AMOUNT, 0);
  mBorderPath.lineTo(point);
  mBorderPath.cubicTo(point.translated(halfRound, 0), point.translated(Utils::ROUNDED_AMOUNT, halfRound),
                      point.translated(Utils::ROUNDED_AMOUNT, Utils::ROUNDED_AMOUNT));
  mBorderPath.lineTo(getBounds().getBottomRight().toFloat());
  mBorderPath.lineTo(getBounds().getBottomLeft().toFloat());
  point = mKeyboard.getBounds().getTopLeft().toFloat().translated(0, Utils::ROUNDED_AMOUNT);
  mBorderPath.lineTo(point);
  mBorderPath.cubicTo(point.translated(0, -halfRound), point.translated(halfRound, -Utils::ROUNDED_AMOUNT),
                      point.translated(Utils::ROUNDED_AMOUNT, -Utils::ROUNDED_AMOUNT));
  point = mArcBounds.getBottomLeft().withY(mKeyboard.getBounds().getY()).translated(-Utils::ROUNDED_AMOUNT, 0);
  mBorderPath.lineTo(point);
  mBorderPath.cubicTo(point.translated(halfRound, 0), point.translated(Utils::ROUNDED_AMOUNT, -halfRound),
                      point.translated(Utils::ROUNDED_AMOUNT, -Utils::ROUNDED_AMOUNT));
  point = mArcBounds.getTopLeft().translated(0, Utils::ROUNDED_AMOUNT);
  mBorderPath.lineTo(point);
  mBorderPath.cubicTo(point.translated(0, -halfRound), point.translated(halfRound, -Utils::ROUNDED_AMOUNT),
                      point.translated(Utils::ROUNDED_AMOUNT, -Utils::ROUNDED_AMOUNT));

  // Cloud centers
  {
    const int expansion = mCloudLeftImage.getWidth() / 4.0f;
    const int translation = expansion * 2;
    const auto leftCenter = mArcSpec.getBounds().getBottomLeft().translated(translation, 0);
    const auto rightCenter = mArcSpec.getBounds().getBottomRight().translated(-translation, 0);
    mCloudLeftTargetArea = mCloudLeftImage.getBounds().expanded(expansion).withCentre(leftCenter).toFloat();
    mCloudRightTargetArea = mCloudRightImage.getBounds().expanded(expansion).withCentre(rightCenter).toFloat();

    // This was sfigured out by using drawRect() until saw the area it should be
    const float leftCloudWidth = mCloudLeftTargetArea.getWidth();
    const float leftCloudHeight = mCloudLeftTargetArea.getHeight();
    mLeftRain = mCloudLeftTargetArea.translated(leftCloudWidth / 3.8f, leftCloudHeight / 1.7f)
                    .withWidth(leftCloudWidth / 2.0f)
                    .withHeight(leftCloudHeight / 1.6f);
    const float rightCloudWidth = mCloudRightTargetArea.getWidth();
    const float rightCloudHeight = mCloudRightTargetArea.getHeight();
    mRightRain = mCloudRightTargetArea.translated(rightCloudWidth / 4.2f, rightCloudHeight / 1.7f)
                     .withWidth(rightCloudWidth / 2.0f)
                     .withHeight(rightCloudHeight / 1.6f);
  }
}

bool GRainbowAudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray& files) {
  // Only accept 1 file of wav/mp3/gbow at a time
  if (files.size() == 1) {
    juce::String extension = files[0].fromLastOccurrenceOf(".", false, false);
    if (extension == "wav" || extension == "mp3" || extension == "gbow") {
      return true;
    }
  }
  return false;
}
void GRainbowAudioProcessorEditor::fileDragEnter(const juce::StringArray&, int, int) {
  mIsFileHovering = true;
  repaint();
}
void GRainbowAudioProcessorEditor::fileDragExit(const juce::StringArray&) {
  mIsFileHovering = false;
  repaint();
}
void GRainbowAudioProcessorEditor::filesDropped(const juce::StringArray& files, int, int) {
  jassert(files.size() == 1);
  mIsFileHovering = false;
  repaint();
  loadFile(juce::File(files[0]));
}

/** Pauses audio to open file
    @param path optional path to load, otherwise will prompt user for file
   location
*/
void GRainbowAudioProcessorEditor::openNewFile(const char* path) {
  if (path == nullptr) {
    mFileChooser = std::make_unique<juce::FileChooser>("Select a file to granulize...", juce::File::getCurrentWorkingDirectory(),
                                                       "*.wav;*.mp3;*.gbow", true);

    int openFlags =
        juce::FileBrowserComponent::FileChooserFlags::openMode | juce::FileBrowserComponent::canSelectFiles;

  mFileChooser->launchAsync(openFlags, [this](const juce::FileChooser& fc) {
      auto file = fc.getResult();
      if (file.existsAsFile()) loadFile(file);
    });
  } else {
    auto file = juce::File(juce::String(path));
  if (file.existsAsFile()) loadFile(file);
  }
}

void GRainbowAudioProcessorEditor::startRecording() {
  if (!juce::RuntimePermissions::isGranted(juce::RuntimePermissions::writeExternalStorage)) {
    SafePointer<GRainbowAudioProcessorEditor> safeThis(this);

    juce::RuntimePermissions::request(juce::RuntimePermissions::writeExternalStorage, [safeThis](bool granted) mutable {
      if (granted) safeThis->startRecording();
    });
    return;
  }

  mAudioDeviceManager.initialiseWithDefaultDevices(1, 0);
  mAudioDeviceManager.addAudioCallback(&mRecorder);

  auto parentDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
  parentDir.getChildFile(FILE_RECORDING).deleteFile();
  mRecordedFile = parentDir.getChildFile(FILE_RECORDING);

  mRecorder.startRecording(mRecordedFile);

  juce::Image recordIcon = juce::PNGImageFormat::loadFrom(BinaryData::microphone_png, BinaryData::microphone_pngSize);
  mBtnRecord.setImages(false, true, true, recordIcon, 1.0f, juce::Colours::red, recordIcon, 1.0f, juce::Colours::red, recordIcon,
                       1.0f, juce::Colours::red);
  repaint();
}

void GRainbowAudioProcessorEditor::stopRecording() {
  mRecorder.stop();

  loadFile(mRecordedFile);

  mAudioDeviceManager.removeAudioCallback(&mRecorder);
  mAudioDeviceManager.closeAudioDevice();

  mRecordedFile = juce::File();

  juce::Image recordIcon = juce::PNGImageFormat::loadFrom(BinaryData::microphone_png, BinaryData::microphone_pngSize);
  juce::Image recordOver = juce::PNGImageFormat::loadFrom(BinaryData::microphoneOver_png, BinaryData::microphoneOver_pngSize);
  mBtnRecord.setImages(false, true, true, recordIcon, 1.0f, juce::Colours::transparentBlack, recordOver, 1.0f,
                       juce::Colours::transparentBlack, recordOver, 1.0f, juce::Colours::transparentBlack);
  repaint();
}

void GRainbowAudioProcessorEditor::loadFile(juce::File file) {

  if (file.getFileExtension() == ".gbow") {
    Utils::Result r = mSynth.loadPreset(file);
    if (r.success) {
      mBtnSavePreset.setEnabled(true);
      mArcSpec.loadPreset();
      updateCenterComponent(ParamUI::CenterComponent::ARC_SPEC);
      mParameters.ui.loadedFileName = file.getFullPathName();
      mParameters.ui.fileName = mParameters.ui.loadedFileName;
      mLabelFileName.setText(mParameters.ui.fileName, juce::dontSendNotification);
      resized();
    } else {
      displayError(r.message);
    }
  } else {
    Utils::Result r = mSynth.loadAudioFile(file, true);
    if (r.success) {
      // Show users which file is being loaded/processed
      mParameters.ui.fileName = file.getFullPathName();
      mLabelFileName.setText(mParameters.ui.fileName, juce::dontSendNotification);

      mTrimSelection.parse(mSynth.getInputBuffer(), mSynth.getSampleRate(), mErrorMessage);
      if (mErrorMessage.isEmpty()) {
        // display screen to trim sample
        updateCenterComponent(ParamUI::CenterComponent::TRIM_SELECTION);
      } else {
        displayError(mErrorMessage);
        mErrorMessage.clear();
        return;
      }
    } else {
      displayError(r.message);
    }
  }
}

void GRainbowAudioProcessorEditor::savePreset() {
  mFileChooser = std::make_unique<juce::FileChooser>("Save gRainbow presets to a file", juce::File::getCurrentWorkingDirectory(),
                                                     "*.gbow", true);

  int saveFlags =
      juce::FileBrowserComponent::FileChooserFlags::saveMode | juce::FileBrowserComponent::FileChooserFlags::warnAboutOverwriting;

  mFileChooser->launchAsync(saveFlags, [this](const juce::FileChooser& fc) {
    juce::File file = fc.getResult().withFileExtension("gbow");
    file.deleteFile();  // clear file if replacing
    juce::FileOutputStream outputStream(file);

    if (file.hasWriteAccess() && outputStream.openedOk()) {
      Preset::Header header;
      header.magic = Preset::MAGIC;
      header.versionMajor = Preset::VERSION_MAJOR;
      header.versionMinor = Preset::VERSION_MINOR;
      // Audio buffer data is grabbed from current synth
      const juce::AudioBuffer<float>& audioBuffer = mSynth.getAudioBuffer();
      header.audioBufferSamplerRate = mSynth.getSampleRate();
      header.audioBufferNumberOfSamples = audioBuffer.getNumSamples();
      header.audioBufferChannel = audioBuffer.getNumChannels();
      header.audioBufferSize = header.audioBufferNumberOfSamples * header.audioBufferChannel * sizeof(float);

      // There is no way in JUCE to be able to know the size of the
      // png/imageFormat blob until after it is written into the outstream which
      // is too late. To keep things working, just do a double copy to a
      // internal memory object so the size is know prior to writing the image
      // data to the stream.
      juce::MemoryOutputStream spectrogramStaging;
      if (!mParameters.ui.saveSpecImage(spectrogramStaging, ParamUI::SpecType::SPECTROGRAM)) {
        displayError("Unable to write spectrogram image out the file");
        return;
      }
      header.specImageSpectrogramSize = spectrogramStaging.getDataSize();

      juce::MemoryOutputStream hpcpStaging;
      if (!mParameters.ui.saveSpecImage(hpcpStaging, ParamUI::SpecType::HPCP)) {
        displayError("Unable to write HPCP image out the file");
        return;
      }
      header.specImageHpcpSize = hpcpStaging.getDataSize();

      juce::MemoryOutputStream detectedStaging;
      if (!mParameters.ui.saveSpecImage(detectedStaging, ParamUI::SpecType::DETECTED)) {
        displayError("Unable to write Detected image out the file");
        return;
      }
      header.specImageDetectedSize = detectedStaging.getDataSize();

      // XML structure of preset contains all audio related information
      // These include not just AudioParams but also other params not exposes to
      // the DAW or UI directly
      juce::MemoryBlock xmlMemoryBlock;
      mSynth.getPresetParamsXml(xmlMemoryBlock);

      // Write data out section by section
      outputStream.write(&header, sizeof(header));
      outputStream.write(reinterpret_cast<const void*>(audioBuffer.getReadPointer(0)), header.audioBufferSize);
      outputStream.write(spectrogramStaging.getData(), header.specImageSpectrogramSize);
      outputStream.write(hpcpStaging.getData(), header.specImageHpcpSize);
      outputStream.write(detectedStaging.getData(), header.specImageDetectedSize);
      outputStream.write(xmlMemoryBlock.getData(), xmlMemoryBlock.getSize());
    } else {
      displayError(juce::String::formatted("Unable to open %s to write", file.getFullPathName().toRawUTF8()));
      return;
    }
  });
}

void GRainbowAudioProcessorEditor::displayError(juce::String message) {
  juce::DialogWindow::LaunchOptions options;
  juce::Label* label = new juce::Label();
  label->setText(message, juce::dontSendNotification);
  label->setColour(juce::Label::textColourId, juce::Colours::whitesmoke);
  options.content.setOwned(label);

  juce::Rectangle<int> area(0, 0, 400, 300);
  options.content->setSize(area.getWidth(), area.getHeight());
  options.dialogTitle = "gRainbow Error";
  options.dialogBackgroundColour = juce::Colour(0xff0e345a);
  options.escapeKeyTriggersCloseButton = true;
  options.useNativeTitleBar = false;
  options.resizable = true;

  mDialogWindow = options.launchAsync();
  if (mDialogWindow != nullptr) {
    mDialogWindow->centreWithSize(300, 200);
  }
}

/** Fast Debug Mode is used to speed up iterations of testing
    This method should be called only once and no-op if not being used
*/
void GRainbowAudioProcessorEditor::fastDebugMode() {
#ifdef FDB_LOAD_FILE
  // Loads a file right away - make sure macro is in quotes in Projucer
  DBG("Fast Debug Mode - Loading file " << FDB_LOAD_FILE);
  openNewFile(FDB_LOAD_FILE);
#endif  // FDB_LOAD_FILE
}
