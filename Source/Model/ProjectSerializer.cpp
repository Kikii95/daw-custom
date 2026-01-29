#include "ProjectSerializer.h"
#include "../Audio/AudioClip.h"
#include "../Audio/DSP/Effects/GainEffect.h"
#include "../Audio/DSP/Effects/ParametricEQ.h"
#include "../Audio/DSP/Effects/CompressorEffect.h"
#include "../Audio/DSP/Effects/ReverbEffect.h"
#include "../Audio/DSP/Effects/DelayEffect.h"
#include "../Audio/DSP/Synthesis/OscillatorEffect.h"
#include "../Audio/DSP/Synthesis/EnvelopeEffect.h"
#include "../Audio/DSP/Synthesis/BasicSynth.h"
#include "../Audio/DSP/Synthesis/KickDesigner.h"
#include "../Audio/Plugins/VST3EffectSlot.h"
#include "../Audio/Plugins/PluginManager.h"

juce::String ProjectSerializer::lastError;

juce::String ProjectSerializer::toJsonString(const Project& project, const AudioMixer& mixer)
{
    auto json = projectToJson(project, mixer);
    return juce::JSON::toString(json, true);
}

bool ProjectSerializer::saveToFile(const juce::File& file,
                                   const Project& project,
                                   const AudioMixer& mixer)
{
    auto json = projectToJson(project, mixer);
    auto jsonString = juce::JSON::toString(json, true);

    if (!file.replaceWithText(jsonString))
    {
        lastError = "Failed to write file: " + file.getFullPathName();
        return false;
    }

    return true;
}

bool ProjectSerializer::loadFromFile(const juce::File& file,
                                     Project& project,
                                     AudioMixer& mixer,
                                     juce::AudioFormatManager& formatManager)
{
    if (!file.existsAsFile())
    {
        lastError = "File not found: " + file.getFullPathName();
        return false;
    }

    auto jsonString = file.loadFileAsString();
    auto json = juce::JSON::parse(jsonString);

    if (!json.isObject())
    {
        lastError = "Invalid JSON format";
        return false;
    }

    return loadFromJson(json, project, mixer, formatManager);
}

juce::var ProjectSerializer::projectToJson(const Project& project, const AudioMixer& mixer)
{
    auto obj = new juce::DynamicObject();
    obj->setProperty("version", FORMAT_VERSION);

    // Project metadata
    auto projObj = new juce::DynamicObject();
    projObj->setProperty("id", project.getId().toString());
    projObj->setProperty("name", project.getName());
    projObj->setProperty("tempo", project.getTempo());
    projObj->setProperty("sampleRate", project.getSampleRate());
    projObj->setProperty("masterVolume", static_cast<double>(project.getMasterVolume()));
    projObj->setProperty("timeSignatureNum", project.getTimeSignatureNumerator());
    projObj->setProperty("timeSignatureDenom", project.getTimeSignatureDenominator());
    obj->setProperty("project", juce::var(projObj));

    // Tracks
    juce::Array<juce::var> tracksArray;
    for (int i = 0; i < project.getNumTracks(); ++i)
    {
        const auto& track = project.getTracks()[static_cast<size_t>(i)];
        auto* audioTrack = const_cast<AudioMixer&>(mixer).getTrack(i);
        tracksArray.add(trackToJson(track, audioTrack));
    }
    obj->setProperty("tracks", tracksArray);

    return juce::var(obj);
}

juce::var ProjectSerializer::trackToJson(const Track& track, const AudioTrack* audioTrack)
{
    auto obj = new juce::DynamicObject();

    obj->setProperty("id", track.id.toString());
    obj->setProperty("name", track.name);
    obj->setProperty("volume", static_cast<double>(track.volume));
    obj->setProperty("pan", static_cast<double>(track.pan));
    obj->setProperty("muted", track.muted);
    obj->setProperty("solo", track.solo);
    obj->setProperty("colour", track.colour.toString());
    obj->setProperty("height", track.height);

    // Clips
    juce::Array<juce::var> clipsArray;
    for (const auto& clip : track.clips)
        clipsArray.add(clipToJson(clip));
    obj->setProperty("clips", clipsArray);

    // Effects (from AudioTrack's EffectChain)
    juce::Array<juce::var> effectsArray;
    if (audioTrack != nullptr)
    {
        const auto& chain = audioTrack->getEffectChain();
        for (int i = 0; i < chain.getNumEffects(); ++i)
        {
            if (auto* effect = chain.getEffect(i))
                effectsArray.add(effectToJson(effect));
        }
    }
    obj->setProperty("effects", effectsArray);

    return juce::var(obj);
}

juce::var ProjectSerializer::clipToJson(const Clip& clip)
{
    auto obj = new juce::DynamicObject();

    obj->setProperty("id", clip.id.toString());
    obj->setProperty("name", clip.name);
    obj->setProperty("startTime", clip.startTime);
    obj->setProperty("duration", clip.duration);
    obj->setProperty("sourceFile", clip.sourceFile.getFullPathName());
    obj->setProperty("sourceSampleRate", clip.sourceSampleRate);
    obj->setProperty("sourceStartOffset", clip.sourceStartOffset);
    obj->setProperty("gain", static_cast<double>(clip.gain));
    obj->setProperty("muted", clip.muted);
    obj->setProperty("colour", clip.colour.toString());

    return juce::var(obj);
}

juce::var ProjectSerializer::effectToJson(const EffectSlot* effect)
{
    auto obj = new juce::DynamicObject();

    obj->setProperty("type", effect->getName());
    obj->setProperty("bypassed", effect->isBypassed());
    obj->setProperty("mix", static_cast<double>(effect->getMix()));

    // Check if VST3
    if (auto* vst = dynamic_cast<const VST3EffectSlot*>(effect))
    {
        obj->setProperty("type", "VST3");

        const auto& desc = vst->getPluginDescription();
        obj->setProperty("pluginId", desc.createIdentifierString());
        obj->setProperty("pluginName", desc.name);

        // Serialize VST3 state as base64
        juce::MemoryBlock state;
        const_cast<VST3EffectSlot*>(vst)->getStateInformation(state);
        obj->setProperty("state", state.toBase64Encoding());
    }
    else
    {
        // Native effect - serialize parameters
        auto params = new juce::DynamicObject();
        for (int i = 0; i < effect->getNumParameters(); ++i)
        {
            auto paramName = effect->getParameterName(i);
            params->setProperty(paramName, static_cast<double>(effect->getParameter(i)));
        }
        obj->setProperty("parameters", juce::var(params));
    }

    return juce::var(obj);
}

bool ProjectSerializer::loadFromJson(const juce::var& json,
                                     Project& project,
                                     AudioMixer& mixer,
                                     juce::AudioFormatManager& formatManager)
{
    // Check version
    auto version = json.getProperty("version", "").toString();
    if (version.isEmpty())
    {
        lastError = "Missing version in project file";
        return false;
    }

    // Load project metadata
    auto projJson = json.getProperty("project", juce::var());
    if (!projJson.isObject())
    {
        lastError = "Invalid project metadata";
        return false;
    }

    project.setName(projJson.getProperty("name", "Untitled").toString());
    project.setTempo(projJson.getProperty("tempo", 120.0));
    project.setSampleRate(projJson.getProperty("sampleRate", 44100.0));
    project.setMasterVolume(static_cast<float>(projJson.getProperty("masterVolume", 1.0)));
    project.setTimeSignature(
        static_cast<int>(projJson.getProperty("timeSignatureNum", 4)),
        static_cast<int>(projJson.getProperty("timeSignatureDenom", 4))
    );

    // Load tracks
    auto tracksJson = json.getProperty("tracks", juce::var());
    if (!tracksJson.isArray())
    {
        lastError = "Invalid tracks data";
        return false;
    }

    mixer.clearTracks();

    for (const auto& trackJson : *tracksJson.getArray())
    {
        Track track;
        auto* audioTrack = mixer.addTrack();

        if (!jsonToTrack(trackJson, track, audioTrack, formatManager))
            return false;

        project.getTracks().push_back(track);

        if (audioTrack != nullptr)
            audioTrack->setTrackData(track);
    }

    project.setModified(false);
    return true;
}

bool ProjectSerializer::jsonToTrack(const juce::var& json,
                                    Track& track,
                                    AudioTrack* audioTrack,
                                    juce::AudioFormatManager& formatManager)
{
    track.id = juce::Uuid(json.getProperty("id", "").toString());
    track.name = json.getProperty("name", "Track").toString();
    track.volume = static_cast<float>(json.getProperty("volume", 1.0));
    track.pan = static_cast<float>(json.getProperty("pan", 0.0));
    track.muted = json.getProperty("muted", false);
    track.solo = json.getProperty("solo", false);
    track.colour = juce::Colour::fromString(json.getProperty("colour", "ff808080").toString());
    track.height = static_cast<int>(json.getProperty("height", 100));

    // Apply to audio track
    if (audioTrack != nullptr)
    {
        audioTrack->setVolume(track.volume);
        audioTrack->setPan(track.pan);
        audioTrack->setMuted(track.muted);
        audioTrack->setSolo(track.solo);
    }

    // Load clips
    auto clipsJson = json.getProperty("clips", juce::var());
    if (clipsJson.isArray())
    {
        for (const auto& clipJson : *clipsJson.getArray())
        {
            Clip clip;
            if (!jsonToClip(clipJson, clip))
                continue;

            track.addClip(clip);

            // Load audio data for AudioTrack
            if (audioTrack != nullptr)
            {
                juce::File sourceFile(clip.sourceFile);
                if (sourceFile.existsAsFile())
                {
                    std::unique_ptr<juce::AudioFormatReader> reader(
                        formatManager.createReaderFor(sourceFile));

                    if (reader != nullptr)
                    {
                        auto buffer = std::make_unique<juce::AudioBuffer<float>>(
                            static_cast<int>(reader->numChannels),
                            static_cast<int>(reader->lengthInSamples));
                        reader->read(buffer.get(), 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

                        auto audioClip = std::make_unique<AudioClip>();
                        audioClip->loadFromBuffer(std::move(buffer), reader->sampleRate, clip);
                        audioTrack->addClip(std::move(audioClip));
                    }
                }
            }
        }
    }

    // Load effects
    auto effectsJson = json.getProperty("effects", juce::var());
    if (effectsJson.isArray() && audioTrack != nullptr)
    {
        auto& chain = audioTrack->getEffectChain();
        for (const auto& effectJson : *effectsJson.getArray())
        {
            if (!jsonToEffect(effectJson, chain))
            {
                DBG("Failed to load effect");
            }
        }
    }

    return true;
}

bool ProjectSerializer::jsonToClip(const juce::var& json, Clip& clip)
{
    clip.id = juce::Uuid(json.getProperty("id", "").toString());
    clip.name = json.getProperty("name", "Clip").toString();
    clip.startTime = json.getProperty("startTime", 0.0);
    clip.duration = json.getProperty("duration", 0.0);
    clip.sourceFile = juce::File(json.getProperty("sourceFile", "").toString());
    clip.sourceSampleRate = json.getProperty("sourceSampleRate", 44100.0);
    clip.sourceStartOffset = json.getProperty("sourceStartOffset", 0.0);
    clip.gain = static_cast<float>(json.getProperty("gain", 1.0));
    clip.muted = json.getProperty("muted", false);
    clip.colour = juce::Colour::fromString(json.getProperty("colour", "ff1e90ff").toString());

    return true;
}

bool ProjectSerializer::jsonToEffect(const juce::var& json, EffectChain& chain)
{
    auto type = json.getProperty("type", "").toString();
    bool bypassed = json.getProperty("bypassed", false);
    float mix = static_cast<float>(json.getProperty("mix", 1.0));

    if (type == "VST3")
    {
        auto pluginId = json.getProperty("pluginId", "").toString();
        auto pluginName = json.getProperty("pluginName", "").toString();
        auto stateBase64 = json.getProperty("state", "").toString();

        // Find plugin description by identifier
        auto descriptions = PluginManager::getInstance().getAvailablePlugins();
        juce::PluginDescription matchedDesc;
        bool found = false;

        for (const auto& desc : descriptions)
        {
            if (desc.createIdentifierString() == pluginId)
            {
                matchedDesc = desc;
                found = true;
                break;
            }
        }

        if (!found)
        {
            // Plugin not available - log warning but continue loading
            DBG("VST3 plugin not found, skipping: " + pluginName + " (" + pluginId + ")");
            return true; // Not a fatal error
        }

        // Create plugin synchronously using format manager directly
        juce::String errorMessage;
        auto& formatManager = PluginManager::getInstance().getFormatManager();

        auto instance = formatManager.createPluginInstance(
            matchedDesc, 44100.0, 512, errorMessage);

        if (instance == nullptr)
        {
            DBG("Failed to create VST3 plugin: " + errorMessage);
            return true; // Not a fatal error, continue loading
        }

        auto vst = std::make_unique<VST3EffectSlot>(std::move(instance));

        // Restore state
        if (stateBase64.isNotEmpty())
        {
            juce::MemoryBlock state;
            state.fromBase64Encoding(stateBase64);
            vst->setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        }

        vst->setBypass(bypassed);
        vst->setMix(mix);
        chain.addEffect(std::move(vst));
    }
    else
    {
        // Native effect
        auto effect = createEffectByType(type);
        if (effect == nullptr)
        {
            lastError = "Unknown effect type: " + type;
            return false;
        }

        // Load parameters
        auto paramsJson = json.getProperty("parameters", juce::var());
        if (paramsJson.isObject())
        {
            auto* paramsObj = paramsJson.getDynamicObject();
            if (paramsObj != nullptr)
            {
                for (int i = 0; i < effect->getNumParameters(); ++i)
                {
                    auto paramName = effect->getParameterName(i);
                    if (paramsObj->hasProperty(paramName))
                    {
                        auto value = static_cast<float>(paramsObj->getProperty(paramName));
                        effect->setParameter(i, value);
                    }
                }
            }
        }

        effect->setBypass(bypassed);
        effect->setMix(mix);
        chain.addEffect(std::move(effect));
    }

    return true;
}

std::unique_ptr<EffectSlot> ProjectSerializer::createEffectByType(const juce::String& type)
{
    if (type == "Gain")
        return std::make_unique<GainEffect>();
    if (type == "EQ" || type == "Parametric EQ")
        return std::make_unique<ParametricEQ>();
    if (type == "Compressor")
        return std::make_unique<CompressorEffect>();
    if (type == "Reverb")
        return std::make_unique<ReverbEffect>();
    if (type == "Delay")
        return std::make_unique<DelayEffect>();
    if (type == "Oscillator")
        return std::make_unique<OscillatorEffect>();
    if (type == "Envelope")
        return std::make_unique<EnvelopeEffect>();
    if (type == "Basic Synth")
        return std::make_unique<BasicSynth>();
    if (type == "Kick Designer")
        return std::make_unique<KickDesigner>();

    return nullptr;
}
