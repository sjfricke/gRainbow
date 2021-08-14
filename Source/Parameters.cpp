/*
  ==============================================================================

    Parameters.cpp
    Created: 10 Aug 2021 6:47:57pm
    Author:  brady

  ==============================================================================
*/

#include "Parameters.h"

void GlobalParams::addParams(juce::AudioProcessor& p) {
  p.addParameter(attack = new juce::AudioParameterFloat(
                     ParamIDs::globalAttack, "Master Attack",
                     ParamRanges::ATTACK, ParamDefaults::ATTACK_DEFAULT_SEC));
  p.addParameter(decay = new juce::AudioParameterFloat(
                     ParamIDs::globalDecay, "Master Decay", ParamRanges::DECAY,
                     ParamDefaults::DECAY_DEFAULT_SEC));
  p.addParameter(sustain = new juce::AudioParameterFloat(
                     ParamIDs::globalSustain, "Master Sustain",
                     juce::NormalisableRange<float>(0.0f, 1.0f),
                     ParamDefaults::SUSTAIN_DEFAULT));
  p.addParameter(release = new juce::AudioParameterFloat(
                     ParamIDs::globalRelease, "Master Release",
                     ParamRanges::RELEASE, ParamDefaults::RELEASE_DEFAULT_SEC));
}

void GlobalParams::resetParams() {
  ParamHelper::setParam(attack, ParamDefaults::ATTACK_DEFAULT_SEC);
  ParamHelper::setParam(decay, ParamDefaults::DECAY_DEFAULT_SEC);
  ParamHelper::setParam(sustain, ParamDefaults::SUSTAIN_DEFAULT);
  ParamHelper::setParam(release, ParamDefaults::RELEASE_DEFAULT_SEC);
}

void CandidateParams::addParams(juce::AudioProcessor& p) {
  p.addParameter(valid = new juce::AudioParameterBool(
                     ParamIDs::candidateValid + juce::String(noteIdx) +
                         juce::String("_") + juce::String(candidateIdx),
                     "Valid", false));
  p.addParameter(posRatio = new juce::AudioParameterFloat(
                     ParamIDs::candidatePosRatio + juce::String(noteIdx) +
                         juce::String("_") + juce::String(candidateIdx),
                     "Pos Ratio", juce::NormalisableRange<float>(0.0f, 1.0f),
                     0.0f));
  p.addParameter(pbRate = new juce::AudioParameterFloat(
                     ParamIDs::candidatePbRate + juce::String(noteIdx) +
                         juce::String("_") + juce::String(candidateIdx),
                     "Playback Rate",
                     juce::NormalisableRange<float>(0.01f, 2.0f), 1.0f));
  p.addParameter(duration = new juce::AudioParameterFloat(
                     ParamIDs::candidateDuration + juce::String(noteIdx) +
                         juce::String("_") + juce::String(candidateIdx),
                     "Duration", juce::NormalisableRange<float>(0.0f, 1.0f),
                     0.0f));
  p.addParameter(salience = new juce::AudioParameterFloat(
                     ParamIDs::candidateSalience + juce::String(noteIdx) +
                         juce::String("_") + juce::String(candidateIdx),
                     "Salience", juce::NormalisableRange<float>(0.0f, 1.0f),
                     0.0f));
}

void GeneratorParams::addParams(juce::AudioProcessor& p) {
  p.addParameter(enable = new juce::AudioParameterBool(
                     ParamIDs::genEnable + juce::String(genIdx) +
                         juce::String("_") + juce::String(noteIdx),
                     "Gen Enable", genIdx == 0));
  p.addParameter(candidate = new juce::AudioParameterInt(
                     ParamIDs::genCandidate + juce::String(genIdx) +
                         juce::String("_") + juce::String(noteIdx),
                     "Gen Candidate", 0, MAX_CANDIDATES - 1, genIdx));
  p.addParameter(pitchAdjust = new juce::AudioParameterFloat(
                     ParamIDs::genPitchAdjust + juce::String(genIdx) +
                         juce::String("_") + juce::String(noteIdx),
                     "Gen Pitch Adjust", ParamRanges::PITCH_ADJUST, 0.0f));
  p.addParameter(positionAdjust = new juce::AudioParameterFloat(
                     ParamIDs::genPositionAdjust + juce::String(genIdx) +
                         juce::String("_") + juce::String(noteIdx),
                     "Gen Position Adjust", ParamRanges::POSITION_ADJUST,
                     0.0f));
  p.addParameter(grainShape = new juce::AudioParameterFloat(
                     ParamIDs::genGrainShape + juce::String(genIdx) +
                         juce::String("_") + juce::String(noteIdx),
                     "Gen Grain Shape",
                     juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
  grainShape->addListener(this);
  p.addParameter(grainTilt = new juce::AudioParameterFloat(
                     ParamIDs::genGrainTilt + juce::String(genIdx) +
                         juce::String("_") + juce::String(noteIdx),
                     "Gen Grain Tilt",
                     juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
  grainTilt->addListener(this);
  p.addParameter(grainRate = new juce::AudioParameterFloat(
                     ParamIDs::genGrainRate + juce::String(genIdx) +
                         juce::String("_") + juce::String(noteIdx),
                     "Gen Grain Rate", ParamRanges::GRAIN_RATE,
                     ParamDefaults::GRAIN_RATE_DEFAULT));
  p.addParameter(grainDuration = new juce::AudioParameterFloat(
                     ParamIDs::genGrainDuration + juce::String(genIdx) +
                         juce::String("_") + juce::String(noteIdx),
                     "Gen Grain Duration", ParamRanges::GRAIN_DURATION,
                     ParamDefaults::GRAIN_DURATION_DEFAULT));
  p.addParameter(grainGain = new juce::AudioParameterFloat(
                     ParamIDs::genGrainGain + juce::String(genIdx) +
                         juce::String("_") + juce::String(noteIdx),
                     "Gen Grain Gain",
                     juce::NormalisableRange<float>(0.0f, 1.0f),
                     ParamDefaults::GRAIN_GAIN_DEFAULT));
  p.addParameter(attack = new juce::AudioParameterFloat(
                     ParamIDs::genAttack + juce::String(genIdx) +
                         juce::String("_") + juce::String(noteIdx),
                     "Gen Attack", ParamRanges::ATTACK,
                     ParamDefaults::ATTACK_DEFAULT_SEC));
  p.addParameter(decay = new juce::AudioParameterFloat(
                     ParamIDs::genDecay + juce::String(genIdx) +
                         juce::String("_") + juce::String(noteIdx),
                     "Gen Decay", ParamRanges::DECAY,
                     ParamDefaults::DECAY_DEFAULT_SEC));
  p.addParameter(sustain = new juce::AudioParameterFloat(
                     ParamIDs::genSustain + juce::String(genIdx) +
                         juce::String("_") + juce::String(noteIdx),
                     "Gen Sustain", juce::NormalisableRange<float>(0.0f, 1.0f),
                     ParamDefaults::SUSTAIN_DEFAULT));
  p.addParameter(release = new juce::AudioParameterFloat(
                     ParamIDs::genRelease + juce::String(genIdx) +
                         juce::String("_") + juce::String(noteIdx),
                     "Gen Release", ParamRanges::RELEASE,
                     ParamDefaults::RELEASE_DEFAULT_SEC));
  updateGrainEnvelope();
}

void GeneratorParams::addListener(
    juce::AudioProcessorParameter::Listener* listener) {
  enable->addListener(listener);
  candidate->addListener(listener);
  pitchAdjust->addListener(listener);
  positionAdjust->addListener(listener);
  grainShape->addListener(listener);
  grainTilt->addListener(listener);
  grainRate->addListener(listener);
  grainDuration->addListener(listener);
  grainGain->addListener(listener);
  attack->addListener(listener);
  decay->addListener(listener);
  sustain->addListener(listener);
  release->addListener(listener);
}

void GeneratorParams::removeListener(
    juce::AudioProcessorParameter::Listener* listener) {
  enable->removeListener(listener);
  candidate->removeListener(listener);
  pitchAdjust->removeListener(listener);
  positionAdjust->removeListener(listener);
  grainShape->removeListener(listener);
  grainTilt->removeListener(listener);
  grainRate->removeListener(listener);
  grainDuration->removeListener(listener);
  grainGain->removeListener(listener);
  attack->removeListener(listener);
  decay->removeListener(listener);
  sustain->removeListener(listener);
  release->removeListener(listener);
}

void GeneratorParams::updateGrainEnvelope() {
  grainEnv.clear();
  float scaledShape = (grainShape->get() * ENV_LUT_SIZE) / 2.0f;
  float scaledTilt = grainTilt->get() * ENV_LUT_SIZE;
  int rampUpEndSample = juce::jmax(0.0f, scaledTilt - scaledShape);
  int rampDownStartSample =
      juce::jmin((float)ENV_LUT_SIZE, scaledTilt + scaledShape);
  for (int i = 0; i < ENV_LUT_SIZE; i++) {
    if (i < rampUpEndSample) {
      grainEnv.push_back((float)i / rampUpEndSample);
    } else if (i > rampDownStartSample) {
      grainEnv.push_back(1.0f - (float)(i - rampDownStartSample) /
                                    (ENV_LUT_SIZE - rampDownStartSample));
    } else {
      grainEnv.push_back(1.0f);
    }
  }
  juce::FloatVectorOperations::clip(grainEnv.data(), grainEnv.data(), 0.0f,
                                    1.0f, grainEnv.size());
}

void NoteParam::addParams(juce::AudioProcessor& p) {
  for (auto& generator : generators) {
    generator->addParams(p);
  }
  for (auto& candidate : candidates) {
    candidate->addParams(p);
  }
  p.addParameter(soloIdx = new juce::AudioParameterInt(
                     ParamIDs::genSolo + juce::String(noteIdx),
                     "Gen Solo", SOLO_NONE, NUM_GENERATORS - 1, SOLO_NONE));
}

bool NoteParam::shouldPlayGenerator(int genIdx) {
  return (soloIdx->get() == genIdx) || (generators[genIdx]->enable->get() && soloIdx->get() == SOLO_NONE);
}

void NoteParams::addParams(juce::AudioProcessor& p) {
  for (auto& note : notes) {
    note->addParams(p);
  }
}

void NoteParams::resetParams() {
  for (auto& note : notes) {
    for (auto& generator : note->generators) {
      ParamHelper::setParam(generator->enable, generator->genIdx == 0);
      ParamHelper::setParam(generator->candidate, generator->genIdx);
      ParamHelper::setParam(generator->pitchAdjust, 0.0f);
      ParamHelper::setParam(generator->positionAdjust, 0.0f);
      ParamHelper::setParam(generator->grainShape, 0.5f);
      ParamHelper::setParam(generator->grainTilt, 0.5f);
      ParamHelper::setParam(generator->grainRate, ParamDefaults::GRAIN_RATE_DEFAULT);
      ParamHelper::setParam(generator->grainDuration, ParamDefaults::GRAIN_DURATION_DEFAULT);
      ParamHelper::setParam(generator->grainGain, ParamDefaults::GRAIN_GAIN_DEFAULT);
      ParamHelper::setParam(generator->attack, ParamDefaults::ATTACK_DEFAULT_SEC);
      ParamHelper::setParam(generator->decay, ParamDefaults::DECAY_DEFAULT_SEC);
      ParamHelper::setParam(generator->sustain, ParamDefaults::SUSTAIN_DEFAULT);
      ParamHelper::setParam(generator->release, ParamDefaults::RELEASE_DEFAULT_SEC);
    }
    for (auto& candidate : note->candidates) {
      ParamHelper::setParam(candidate->valid, false);
      ParamHelper::setParam(candidate->posRatio, 0.0f);
      ParamHelper::setParam(candidate->pbRate, 0.0f);
      ParamHelper::setParam(candidate->duration, 0.0f);
      ParamHelper::setParam(candidate->salience, 0.0f);
    }
    ParamHelper::setParam(note->soloIdx, SOLO_NONE);
  }
}