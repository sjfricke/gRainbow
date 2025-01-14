/*
  ==============================================================================

    Parameters.cpp
    Created: 10 Aug 2021 6:47:57pm
    Author:  brady

  ==============================================================================
*/

#include "Parameters.h"

void ParamGlobal::addParams(juce::AudioProcessor& p) {
  p.addParameter(common[GAIN] = new juce::AudioParameterFloat({ParamIDs::globalGain, 1}, "Master Gain", ParamRanges::GAIN,
                                                              ParamDefaults::GAIN_DEFAULT));
  p.addParameter(common[ATTACK] = new juce::AudioParameterFloat({ParamIDs::globalAttack, 1}, "Master Attack", ParamRanges::ATTACK,
                                                                ParamDefaults::ATTACK_DEFAULT_SEC));
  p.addParameter(common[DECAY] = new juce::AudioParameterFloat({ParamIDs::globalDecay, 1}, "Master Decay", ParamRanges::DECAY,
                                                               ParamDefaults::DECAY_DEFAULT_SEC));
  p.addParameter(common[SUSTAIN] = new juce::AudioParameterFloat({ParamIDs::globalSustain, 1}, "Master Sustain", ParamRanges::SUSTAIN,
                                                                 ParamDefaults::SUSTAIN_DEFAULT));
  p.addParameter(common[RELEASE] = new juce::AudioParameterFloat({ParamIDs::globalRelease, 1}, "Master Release", ParamRanges::RELEASE,
                                                                 ParamDefaults::RELEASE_DEFAULT_SEC));
  p.addParameter(common[FILT_CUTOFF] =
                     new juce::AudioParameterFloat({ParamIDs::globalFilterCutoff, 1}, "Master Filter Cutoff", ParamRanges::CUTOFF,
                                                   ParamDefaults::FILTER_LP_CUTOFF_DEFAULT_HZ));
  common[FILT_CUTOFF]->addListener(this);
  p.addParameter(common[FILT_RESONANCE] =
                     new juce::AudioParameterFloat({ParamIDs::globalFilterResonance, 1}, "Master Filter Resonance",
                                                   ParamRanges::RESONANCE, ParamDefaults::FILTER_RESONANCE_DEFAULT));
  common[FILT_RESONANCE]->addListener(this);
  p.addParameter(common[FILT_TYPE] =
                     new juce::AudioParameterChoice({ParamIDs::globalFilterType, 1}, "Master Filter Type", FILTER_TYPE_NAMES, 0));
  common[FILT_TYPE]->addListener(this);

  p.addParameter(common[GRAIN_SHAPE] = new juce::AudioParameterFloat({ParamIDs::globalGrainShape, 1}, "Master Grain Shape",
                                                                     ParamRanges::GRAIN_SHAPE, ParamDefaults::GRAIN_SHAPE_DEFAULT));
  p.addParameter(common[GRAIN_TILT] = new juce::AudioParameterFloat({ParamIDs::globalGrainTilt, 1}, "Master Grain Tilt",
                                                                    ParamRanges::GRAIN_TILT, ParamDefaults::GRAIN_TILT_DEFAULT));

  p.addParameter(common[GRAIN_RATE] = new juce::AudioParameterFloat({ParamIDs::globalGrainRate, 1}, "Master Grain Rate",
                                                                    ParamRanges::GRAIN_RATE, ParamDefaults::GRAIN_RATE_DEFAULT));
  p.addParameter(common[GRAIN_DURATION] =
                     new juce::AudioParameterFloat({ParamIDs::globalGrainDuration, 1}, "Master Grain Duration",
                                                   ParamRanges::GRAIN_DURATION, ParamDefaults::GRAIN_DURATION_DEFAULT));
  p.addParameter(common[GRAIN_SYNC] = new juce::AudioParameterBool({ParamIDs::globalGrainSync, 1}, "Master Grain Sync", ParamDefaults::GRAIN_SYNC_DEFAULT));

  p.addParameter(common[PITCH_ADJUST] =
                     new juce::AudioParameterFloat({ParamIDs::globalPitchAdjust, 1}, "Master Pitch Adjust", ParamRanges::PITCH_ADJUST,
                                                   ParamDefaults::PITCH_ADJUST_DEFAULT));
  p.addParameter(common[PITCH_SPRAY] = new juce::AudioParameterFloat({ParamIDs::globalPitchSpray, 1}, "Master Pitch Spray",
                                                                     ParamRanges::PITCH_SPRAY, ParamDefaults::PITCH_SPRAY_DEFAULT));
  p.addParameter(common[POS_ADJUST] =
                     new juce::AudioParameterFloat({ParamIDs::globalPositionAdjust, 1}, "Master Position Adjust",
                                                   ParamRanges::POSITION_ADJUST, ParamDefaults::POSITION_ADJUST_DEFAULT));
  p.addParameter(common[POS_SPRAY] =
                     new juce::AudioParameterFloat({ParamIDs::globalPositionSpray, 1}, "Master Position Spray",
                                                   ParamRanges::POSITION_SPRAY, ParamDefaults::POSITION_SPRAY_DEFAULT));

  p.addParameter(common[PAN_ADJUST] = new juce::AudioParameterFloat({ParamIDs::globalPanAdjust, 1}, "Master Pan Adjust",
                                                                    ParamRanges::PAN_ADJUST, ParamDefaults::PAN_ADJUST_DEFAULT));
  p.addParameter(common[PAN_SPRAY] = new juce::AudioParameterFloat({ParamIDs::globalPanSpray, 1}, "Master Pan Spray",
                                                                   ParamRanges::PAN_SPRAY, ParamDefaults::PAN_SPRAY_DEFAULT));
}

void ParamGenerator::addParams(juce::AudioProcessor& p) {
  juce::String enableId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genEnable + juce::String(genIdx);
  p.addParameter(enable = new juce::AudioParameterBool({enableId, 1}, enableId, genIdx == 0));
  juce::String candidateId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genCandidate + juce::String(genIdx);
  p.addParameter(candidate = new juce::AudioParameterInt({candidateId, 1}, candidateId, 0, MAX_CANDIDATES - 1, 0));

  juce::String gainId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGain + juce::String(genIdx);
  p.addParameter(common[GAIN] = new juce::AudioParameterFloat({gainId, 1}, gainId, ParamRanges::GAIN, ParamDefaults::GAIN_DEFAULT));
  juce::String attackId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genAttack + juce::String(genIdx);
  p.addParameter(common[ATTACK] =
                     new juce::AudioParameterFloat({attackId, 1}, attackId, ParamRanges::ATTACK, ParamDefaults::ATTACK_DEFAULT_SEC));
  juce::String decayId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genDecay + juce::String(genIdx);
  p.addParameter(common[DECAY] =
                     new juce::AudioParameterFloat({decayId, 1}, decayId, ParamRanges::DECAY, ParamDefaults::DECAY_DEFAULT_SEC));
  juce::String sustainId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genSustain + juce::String(genIdx);
  p.addParameter(common[SUSTAIN] =
                     new juce::AudioParameterFloat({sustainId, 1}, sustainId, ParamRanges::SUSTAIN, ParamDefaults::SUSTAIN_DEFAULT));
  juce::String releaseId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genRelease + juce::String(genIdx);
  p.addParameter(common[RELEASE] =
                     new juce::AudioParameterFloat({releaseId, 1}, releaseId, ParamRanges::RELEASE, ParamDefaults::RELEASE_DEFAULT_SEC));
  // Filter params have listeners to set juce::dsp::StateVariableFilter parameters when changed
  juce::String cutoffId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genFilterCutoff + juce::String(genIdx);
  p.addParameter(common[FILT_CUTOFF] = new juce::AudioParameterFloat({cutoffId, 1}, cutoffId, ParamRanges::CUTOFF,
                                                                     ParamDefaults::FILTER_LP_CUTOFF_DEFAULT_HZ));
  common[FILT_CUTOFF]->addListener(this);
  juce::String resonanceId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genFilterResonance + juce::String(genIdx);
  p.addParameter(common[FILT_RESONANCE] = new juce::AudioParameterFloat({resonanceId, 1}, resonanceId, ParamRanges::RESONANCE,
                                                                        ParamDefaults::FILTER_RESONANCE_DEFAULT));
  common[FILT_RESONANCE]->addListener(this);
  juce::String filterTypeId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genFilterType + juce::String(genIdx);
  p.addParameter(common[FILT_TYPE] = new juce::AudioParameterChoice({filterTypeId, 1}, filterTypeId, FILTER_TYPE_NAMES, 0));
  common[FILT_TYPE]->addListener(this);
  juce::String pitchAdjustId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genPitchAdjust + juce::String(genIdx);
  p.addParameter(common[PITCH_ADJUST] = new juce::AudioParameterFloat({pitchAdjustId, 1}, pitchAdjustId, ParamRanges::PITCH_ADJUST,
                                                                      ParamDefaults::PITCH_ADJUST_DEFAULT));
  juce::String pitchSprayId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genPitchSpray + juce::String(genIdx);
  p.addParameter(common[PITCH_SPRAY] = new juce::AudioParameterFloat({pitchSprayId, 1}, pitchSprayId, ParamRanges::PITCH_SPRAY,
                                                                     ParamDefaults::PITCH_SPRAY_DEFAULT));
  juce::String posAdjustId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genPositionAdjust + juce::String(genIdx);
  p.addParameter(common[POS_ADJUST] = new juce::AudioParameterFloat({posAdjustId, 1}, posAdjustId, ParamRanges::POSITION_ADJUST,
                                                                    ParamDefaults::POSITION_ADJUST_DEFAULT));
  juce::String posSprayId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genPositionSpray + juce::String(genIdx);
  p.addParameter(common[POS_SPRAY] = new juce::AudioParameterFloat({posSprayId, 1}, posSprayId, ParamRanges::POSITION_SPRAY,
                                                                   ParamDefaults::POSITION_SPRAY_DEFAULT));
  juce::String panAdjustId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genPanAdjust + juce::String(genIdx);
  p.addParameter(common[PAN_ADJUST] = new juce::AudioParameterFloat({panAdjustId, 1}, panAdjustId, ParamRanges::PAN_ADJUST,
                                                                    ParamDefaults::PAN_ADJUST_DEFAULT));
  juce::String panSprayId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genPanSpray + juce::String(genIdx);
  p.addParameter(common[PAN_SPRAY] = new juce::AudioParameterFloat({panSprayId, 1}, panSprayId, ParamRanges::PAN_SPRAY,
                                                                   ParamDefaults::PAN_SPRAY_DEFAULT));

  juce::String shapeId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGrainShape + juce::String(genIdx);
  p.addParameter(common[GRAIN_SHAPE] =
                     new juce::AudioParameterFloat({shapeId, 1}, shapeId, ParamRanges::GRAIN_SHAPE, ParamDefaults::GRAIN_SHAPE_DEFAULT));
  juce::String tiltId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGrainTilt + juce::String(genIdx);
  p.addParameter(common[GRAIN_TILT] =
                     new juce::AudioParameterFloat({tiltId, 1}, tiltId, ParamRanges::GRAIN_TILT, ParamDefaults::GRAIN_TILT_DEFAULT));

  juce::String rateId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGrainRate + juce::String(genIdx);
  p.addParameter(common[GRAIN_RATE] =
                     new juce::AudioParameterFloat({rateId, 1}, rateId, ParamRanges::GRAIN_RATE, ParamDefaults::GRAIN_RATE_DEFAULT));
  juce::String durationId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGrainDuration + juce::String(genIdx);
  p.addParameter(common[GRAIN_DURATION] = new juce::AudioParameterFloat({durationId, 1}, durationId, ParamRanges::GRAIN_DURATION,
                                                                        ParamDefaults::GRAIN_DURATION_DEFAULT));
  juce::String syncId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genGrainSync + juce::String(genIdx);
  p.addParameter(common[GRAIN_SYNC] = new juce::AudioParameterBool({syncId, 1}, syncId, ParamDefaults::GRAIN_SYNC_DEFAULT));
}

void ParamNote::addParams(juce::AudioProcessor& p) {
  juce::String notePrefix = PITCH_CLASS_NAMES[noteIdx];

  // First the note's common parameters
  p.addParameter(common[GAIN] = new juce::AudioParameterFloat({notePrefix + ParamIDs::noteGain, 1}, notePrefix + ParamIDs::noteGain,
                                                              ParamRanges::GAIN, ParamDefaults::GAIN_DEFAULT));
  p.addParameter(common[ATTACK] =
                     new juce::AudioParameterFloat({notePrefix + ParamIDs::noteAttack, 1}, notePrefix + ParamIDs::noteAttack,
                                                   ParamRanges::ATTACK, ParamDefaults::ATTACK_DEFAULT_SEC));
  p.addParameter(common[DECAY] = new juce::AudioParameterFloat({notePrefix + ParamIDs::noteDecay, 1}, notePrefix + ParamIDs::noteDecay,
                                                               ParamRanges::DECAY, ParamDefaults::DECAY_DEFAULT_SEC));
  p.addParameter(common[SUSTAIN] =
                     new juce::AudioParameterFloat({notePrefix + ParamIDs::noteSustain, 1}, notePrefix + ParamIDs::noteSustain,
                                                   ParamRanges::SUSTAIN, ParamDefaults::SUSTAIN_DEFAULT));
  p.addParameter(common[RELEASE] =
                     new juce::AudioParameterFloat({notePrefix + ParamIDs::noteRelease, 1}, notePrefix + ParamIDs::noteRelease,
                                                   ParamRanges::RELEASE, ParamDefaults::RELEASE_DEFAULT_SEC));
  p.addParameter(common[FILT_CUTOFF] =
                     new juce::AudioParameterFloat({notePrefix + ParamIDs::noteFilterCutoff, 1}, notePrefix + ParamIDs::noteFilterCutoff,
                                                   ParamRanges::CUTOFF, ParamDefaults::FILTER_LP_CUTOFF_DEFAULT_HZ));
  common[FILT_CUTOFF]->addListener(this);
  p.addParameter(common[FILT_RESONANCE] = new juce::AudioParameterFloat({
                     notePrefix + ParamIDs::noteFilterResonance, 1}, notePrefix + ParamIDs::noteFilterResonance, ParamRanges::RESONANCE,
                     ParamDefaults::FILTER_RESONANCE_DEFAULT));
  common[FILT_RESONANCE]->addListener(this);
  p.addParameter(common[FILT_TYPE] = new juce::AudioParameterChoice({notePrefix + ParamIDs::noteFilterType, 1},
                                                                    notePrefix + ParamIDs::noteFilterType, FILTER_TYPE_NAMES, 0));
  common[FILT_TYPE]->addListener(this);

  p.addParameter(common[GRAIN_SHAPE] = new juce::AudioParameterFloat({notePrefix + ParamIDs::noteGrainShape, 1}, notePrefix + ParamIDs::noteGrainShape,
                                                   ParamRanges::GRAIN_SHAPE, ParamDefaults::GRAIN_SHAPE_DEFAULT));
  p.addParameter(common[GRAIN_TILT] = new juce::AudioParameterFloat({notePrefix + ParamIDs::noteGrainTilt, 1}, notePrefix + ParamIDs::noteGrainTilt,
                                                   ParamRanges::GRAIN_TILT, ParamDefaults::GRAIN_TILT_DEFAULT));

  p.addParameter(common[GRAIN_RATE] =
                     new juce::AudioParameterFloat({notePrefix + ParamIDs::noteGrainRate, 1}, notePrefix + ParamIDs::noteGrainRate,
                                                   ParamRanges::GRAIN_RATE, ParamDefaults::GRAIN_RATE_DEFAULT));
  p.addParameter(common[GRAIN_DURATION] = new juce::AudioParameterFloat({
                     notePrefix + ParamIDs::noteGrainDuration, 1}, notePrefix + ParamIDs::noteGrainDuration,
                     ParamRanges::GRAIN_DURATION, ParamDefaults::GRAIN_DURATION_DEFAULT));
  p.addParameter(common[GRAIN_SYNC] = new juce::AudioParameterBool({notePrefix + ParamIDs::noteGrainSync, 1},
                                                                   notePrefix + ParamIDs::noteGrainSync, ParamDefaults::GRAIN_SYNC_DEFAULT));

  p.addParameter(common[PITCH_ADJUST] =
                     new juce::AudioParameterFloat({notePrefix + ParamIDs::notePitchAdjust, 1}, notePrefix + ParamIDs::notePitchAdjust,
                                                   ParamRanges::PITCH_ADJUST, ParamDefaults::PITCH_ADJUST_DEFAULT));
  p.addParameter(common[PITCH_SPRAY] =
                     new juce::AudioParameterFloat({notePrefix + ParamIDs::notePitchSpray, 1}, notePrefix + ParamIDs::notePitchSpray,
                                                   ParamRanges::PITCH_SPRAY, ParamDefaults::PITCH_SPRAY_DEFAULT));
  p.addParameter(common[POS_ADJUST] = new juce::AudioParameterFloat({
                     notePrefix + ParamIDs::notePositionAdjust, 1}, notePrefix + ParamIDs::notePositionAdjust,
                     ParamRanges::POSITION_ADJUST, ParamDefaults::POSITION_ADJUST_DEFAULT));
  p.addParameter(common[POS_SPRAY] = new juce::AudioParameterFloat({
                     notePrefix + ParamIDs::notePositionSpray, 1}, notePrefix + ParamIDs::notePositionSpray,
                     ParamRanges::POSITION_SPRAY, ParamDefaults::POSITION_SPRAY_DEFAULT));
  p.addParameter(common[PAN_ADJUST] =
                     new juce::AudioParameterFloat({notePrefix + ParamIDs::notePanAdjust, 1}, notePrefix + ParamIDs::notePanAdjust,
                                                   ParamRanges::PAN_ADJUST, ParamDefaults::PAN_ADJUST_DEFAULT));
  p.addParameter(common[PAN_SPRAY] =
                     new juce::AudioParameterFloat({notePrefix + ParamIDs::notePanSpray, 1}, notePrefix + ParamIDs::notePanSpray,
                                                   ParamRanges::PAN_SPRAY, ParamDefaults::PAN_SPRAY_DEFAULT));

  // Then make each of its generators
  for (auto& generator : generators) {
    generator->addParams(p);
  }
  juce::String soloId = PITCH_CLASS_NAMES[noteIdx] + ParamIDs::genSolo;
  p.addParameter(soloIdx = new juce::AudioParameterInt({soloId, 1}, soloId, SOLO_NONE, NUM_GENERATORS - 1, SOLO_NONE));
}

ParamCandidate* ParamNote::getCandidate(int genIdx) {
  if (candidates.empty()) return nullptr;
  return &candidates[generators[genIdx]->candidate->get()];
}

void ParamNote::setStartingCandidatePosition() {
  // We start each candidate position at zero and here update it to start each generator at a unique position.
  // If there are only 2 candidate and 4 generators, we want genIdx 3 and 4 to also get candidate[1]
  const int maxPosition = candidates.size() - 1;
  for (int i = 0; i < NUM_GENERATORS; ++i) {
    ParamHelper::setParam(generators[i].get()->candidate, (i > maxPosition) ? maxPosition : i);
  }
}

bool ParamNote::shouldPlayGenerator(int genIdx) {
  return (soloIdx->get() == genIdx) || (generators[genIdx]->enable->get() && soloIdx->get() == SOLO_NONE);
}

void ParamsNote::addParams(juce::AudioProcessor& p) {
  for (auto& note : notes) {
    note->addParams(p);
  }
}
