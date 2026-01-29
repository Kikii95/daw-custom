#include "EffectRackPanel.h"
#include "Audio/DSP/Effects/GainEffect.h"
#include "Audio/DSP/Effects/ParametricEQ.h"
#include "Audio/DSP/Effects/CompressorEffect.h"
#include "Audio/DSP/Effects/ReverbEffect.h"
#include "Audio/DSP/Effects/DelayEffect.h"
#include "Audio/DSP/Synthesis/OscillatorEffect.h"
#include "Audio/DSP/Synthesis/EnvelopeEffect.h"
#include "Audio/DSP/Synthesis/KickDesigner.h"
#include "Audio/DSP/Synthesis/SimpleFilter.h"
#include "Audio/DSP/Synthesis/BasicSynth.h"
#include "Audio/Plugins/VST3EffectSlot.h"
#include "../Plugins/PluginEditorWindow.h"
#include "UI/Theme/AppTheme.h"

EffectRackPanel::EffectRackPanel()
{
    titleLabel.setColour(juce::Label::textColourId, Theme::Colours::text());
    titleLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    // Build combo with built-in effects and VST3 plugins
    rebuildPluginComboItems();
    effectTypeCombo.setSelectedId(100, juce::dontSendNotification);
    effectTypeCombo.setColour(juce::ComboBox::backgroundColourId, Theme::colour(Theme::bgHover));
    effectTypeCombo.setColour(juce::ComboBox::textColourId, Theme::Colours::text());
    effectTypeCombo.setColour(juce::ComboBox::arrowColourId, Theme::Colours::textMuted());
    effectTypeCombo.setTooltip("Select an effect to add");
    addAndMakeVisible(effectTypeCombo);

    addButton.setColour(juce::TextButton::buttonColourId, Theme::Colours::success());
    addButton.setColour(juce::TextButton::textColourOffId, Theme::colour(Theme::textOnAccent));
    addButton.setTooltip("Add selected effect");
    addButton.onClick = [this]() { addEffectFromCombo(); };
    addAndMakeVisible(addButton);

    // Scan button for VST3 plugins
    scanButton.setColour(juce::TextButton::buttonColourId, Theme::Colours::accent());
    scanButton.setColour(juce::TextButton::textColourOffId, Theme::colour(Theme::textOnAccent));
    scanButton.setTooltip("Scan for VST3 plugins");
    scanButton.onClick = [this]() { scanForPlugins(); };
    addAndMakeVisible(scanButton);

    viewport.setViewedComponent(&effectContainer, false);
    viewport.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport);
}

EffectRackPanel::~EffectRackPanel()
{
    effectSlots.clear();
}

void EffectRackPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Panel background
    g.fillAll(Theme::colour(Theme::bgPanel));

    // Header background
    auto headerBounds = bounds.removeFromTop(headerHeight);
    g.setColour(Theme::colour(Theme::bgHeader));
    g.fillRect(headerBounds);

    // Separator
    g.setColour(Theme::colour(Theme::border));
    g.fillRect(bounds.getX(), headerBounds.getBottom(), bounds.getWidth(), 1);

    // Empty state
    if (effectSlots.empty())
    {
        g.setColour(Theme::Colours::textMuted());
        g.setFont(13.0f);
        auto textArea = getLocalBounds().withTrimmedTop(headerHeight + 20);
        g.drawText("No effects \xe2\x80\x94 Select from dropdown above",
                   textArea, juce::Justification::centredTop);
    }
}

void EffectRackPanel::resized()
{
    auto bounds = getLocalBounds();

    // Header
    auto header = bounds.removeFromTop(headerHeight).reduced(padding, 4);
    titleLabel.setBounds(header.removeFromLeft(60));
    header.removeFromLeft(8);
    scanButton.setBounds(header.removeFromRight(45));
    header.removeFromRight(4);
    addButton.setBounds(header.removeFromRight(45));
    header.removeFromRight(4);
    effectTypeCombo.setBounds(header);

    // Viewport fills the rest
    bounds.removeFromTop(1); // separator
    viewport.setBounds(bounds);

    // Update container size
    int totalHeight = 0;
    for (auto& slot : effectSlots)
        totalHeight += slot->getPreferredHeight() + slotSpacing;

    effectContainer.setSize(bounds.getWidth() - 16, juce::jmax(totalHeight, bounds.getHeight()));

    // Layout effect slots vertically
    int y = slotSpacing;
    for (auto& slot : effectSlots)
    {
        int h = slot->getPreferredHeight();
        slot->setBounds(slotSpacing, y, effectContainer.getWidth() - slotSpacing * 2, h);
        y += h + slotSpacing;
    }
}

void EffectRackPanel::setTrack(AudioTrack* track)
{
    currentTrack = track;

    if (track)
        titleLabel.setText("FX: " + track->getTrackData().name, juce::dontSendNotification);
    else
        titleLabel.setText("Effects", juce::dontSendNotification);

    refreshEffects();
}

void EffectRackPanel::refreshEffects()
{
    updateEffectSlots();
    resized();
}

void EffectRackPanel::updateEffectSlots()
{
    // Clear existing
    for (auto& slot : effectSlots)
        effectContainer.removeChildComponent(slot.get());
    effectSlots.clear();

    if (!currentTrack)
        return;

    auto& chain = currentTrack->getEffectChain();
    int numEffects = chain.getNumEffects();

    for (int i = 0; i < numEffects; ++i)
    {
        auto slot = std::make_unique<EffectSlotComponent>();
        slot->setEffect(chain.getEffect(i), i);

        // Wire callbacks
        slot->onBypassChanged = [this](int index, bool bypassed)
        {
            if (currentTrack && onEffectBypassChanged)
                onEffectBypassChanged(currentTrack->getId(), index, bypassed);
        };

        slot->onParameterChanged = [this](int effectIdx, int paramIdx, float value)
        {
            if (currentTrack && onEffectParameterChanged)
                onEffectParameterChanged(currentTrack->getId(), effectIdx, paramIdx, value);
        };

        slot->onRemoveClicked = [this](int index)
        {
            removeEffect(index);
        };

        effectContainer.addAndMakeVisible(slot.get());
        effectSlots.push_back(std::move(slot));
    }
}

void EffectRackPanel::addEffectFromCombo()
{
    if (!currentTrack)
        return;

    int selectedId = effectTypeCombo.getSelectedId();
    if (selectedId == 100 || selectedId == EffectType::None)
        return;

    // Check if it's a VST3 plugin
    if (selectedId >= EffectType::VST3Base)
    {
        int pluginIndex = selectedId - EffectType::VST3Base;
        if (pluginIndex >= 0 && pluginIndex < pluginDescriptions.size())
        {
            addVST3Plugin(pluginDescriptions[pluginIndex]);
        }
        return;
    }

    std::unique_ptr<EffectSlot> newEffect;

    switch (selectedId)
    {
        case EffectType::Gain:
            newEffect = std::make_unique<GainEffect>();
            break;
        case EffectType::EQ:
            newEffect = std::make_unique<ParametricEQ>();
            break;
        case EffectType::Compressor:
            newEffect = std::make_unique<CompressorEffect>();
            break;
        case EffectType::Reverb:
            newEffect = std::make_unique<ReverbEffect>();
            break;
        case EffectType::Delay:
            newEffect = std::make_unique<DelayEffect>();
            break;
        case EffectType::Oscillator:
            newEffect = std::make_unique<OscillatorEffect>();
            break;
        case EffectType::Envelope:
            newEffect = std::make_unique<EnvelopeEffect>();
            break;
        case EffectType::Kick:
            newEffect = std::make_unique<KickDesigner>();
            break;
        case EffectType::Filter:
            newEffect = std::make_unique<SimpleFilter>();
            break;
        case EffectType::Synth:
            newEffect = std::make_unique<BasicSynth>();
            break;
        default:
            return;
    }

    if (newEffect)
    {
        auto& chain = currentTrack->getEffectChain();
        int newIndex = chain.getNumEffects();
        chain.addEffect(std::move(newEffect));

        if (onEffectAdded)
            onEffectAdded(currentTrack->getId(), newIndex);

        refreshEffects();
        effectTypeCombo.setSelectedId(100, juce::dontSendNotification);
    }
}

void EffectRackPanel::removeEffect(int index)
{
    if (!currentTrack)
        return;

    auto& chain = currentTrack->getEffectChain();
    if (index >= 0 && index < chain.getNumEffects())
    {
        // Close any plugin editor windows for VST3 effects
        auto* effect = chain.getEffect(index);
        if (auto* vstSlot = dynamic_cast<VST3EffectSlot*>(effect))
        {
            PluginEditorWindow::closeWindowFor(vstSlot);
        }

        chain.removeEffect(index);

        if (onEffectRemoved)
            onEffectRemoved(currentTrack->getId(), index);

        refreshEffects();
    }
}

void EffectRackPanel::rebuildPluginComboItems()
{
    effectTypeCombo.clear();

    // Built-in effects
    effectTypeCombo.addItem("Select Effect...", 100);
    effectTypeCombo.addSeparator();
    effectTypeCombo.addSectionHeading("Built-in Effects");
    effectTypeCombo.addItem("Gain", EffectType::Gain);
    effectTypeCombo.addItem("EQ", EffectType::EQ);
    effectTypeCombo.addItem("Compressor", EffectType::Compressor);
    effectTypeCombo.addItem("Reverb", EffectType::Reverb);
    effectTypeCombo.addItem("Delay", EffectType::Delay);
    effectTypeCombo.addSeparator();
    effectTypeCombo.addSectionHeading("Synthesis");
    effectTypeCombo.addItem("Oscillator", EffectType::Oscillator);
    effectTypeCombo.addItem("Envelope", EffectType::Envelope);
    effectTypeCombo.addItem("Kick", EffectType::Kick);
    effectTypeCombo.addItem("Filter", EffectType::Filter);
    effectTypeCombo.addItem("Synth", EffectType::Synth);

    // VST3 plugins
    auto& pluginManager = PluginManager::getInstance();
    if (pluginManager.isInitialized())
    {
        pluginDescriptions = pluginManager.getAvailablePlugins();

        if (!pluginDescriptions.isEmpty())
        {
            effectTypeCombo.addSeparator();
            effectTypeCombo.addSectionHeading("VST3 Plugins");

            for (int i = 0; i < pluginDescriptions.size(); ++i)
            {
                const auto& desc = pluginDescriptions[i];
                juce::String name = desc.name;
                if (desc.manufacturerName.isNotEmpty())
                    name += " (" + desc.manufacturerName + ")";

                effectTypeCombo.addItem(name, EffectType::VST3Base + i);
            }
        }
    }
}

void EffectRackPanel::addVST3Plugin(const juce::PluginDescription& desc)
{
    if (!currentTrack)
        return;

    auto& pluginManager = PluginManager::getInstance();

    // Get audio settings from track (default to reasonable values)
    double sampleRate = 44100.0;
    int blockSize = 512;

    // Disable add button during async creation
    addButton.setEnabled(false);
    addButton.setButtonText("...");

    pluginManager.createPluginAsync(desc, sampleRate, blockSize,
        [this](std::unique_ptr<VST3EffectSlot> slot, juce::String error)
        {
            // Re-enable button
            addButton.setEnabled(true);
            addButton.setButtonText("Add");

            if (slot && currentTrack)
            {
                auto& chain = currentTrack->getEffectChain();
                int newIndex = chain.getNumEffects();
                chain.addEffect(std::move(slot));

                if (onEffectAdded)
                    onEffectAdded(currentTrack->getId(), newIndex);

                refreshEffects();
                effectTypeCombo.setSelectedId(100, juce::dontSendNotification);
            }
            else if (error.isNotEmpty())
            {
                DBG("EffectRackPanel: Failed to load plugin: " + error);
                juce::AlertWindow::showMessageBoxAsync(
                    juce::MessageBoxIconType::WarningIcon,
                    "Plugin Error",
                    "Failed to load plugin:\n" + error);
            }
        });
}

void EffectRackPanel::scanForPlugins()
{
    auto& pluginManager = PluginManager::getInstance();

    if (!pluginManager.isInitialized())
        pluginManager.initialize();

    if (pluginManager.isScanning())
    {
        DBG("EffectRackPanel: Scan already in progress");
        return;
    }

    // Disable scan button during scan
    scanButton.setEnabled(false);
    scanButton.setButtonText("...");

    pluginManager.scanForPluginsAsync(
        // Progress callback
        [this](float progress)
        {
            int percent = static_cast<int>(progress * 100);
            scanButton.setButtonText(juce::String(percent) + "%");
        },
        // Complete callback
        [this]()
        {
            scanButton.setEnabled(true);
            scanButton.setButtonText("Scan");
            rebuildPluginComboItems();
        });
}
