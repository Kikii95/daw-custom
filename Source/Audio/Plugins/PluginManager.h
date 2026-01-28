#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>
#include <memory>

class VST3EffectSlot;

/**
 * Singleton managing VST3 plugin discovery and instantiation.
 *
 * Key responsibilities:
 * - Plugin format management (VST3)
 * - Background scanning of plugin directories
 * - Async plugin instantiation (required for stability)
 * - Plugin list persistence
 */
class PluginManager
{
public:
    static PluginManager& getInstance();


    // Initialization
    void initialize();
    bool isInitialized() const { return initialized; }

    // Scanning
    void scanForPluginsAsync(std::function<void(float progress)> onProgress = nullptr,
                            std::function<void()> onComplete = nullptr);
    void cancelScan();
    bool isScanning() const;
    float getScanProgress() const;

    // Plugin list
    const juce::KnownPluginList& getPluginList() const { return pluginList; }
    int getNumPlugins() const { return pluginList.getNumTypes(); }
    juce::Array<juce::PluginDescription> getAvailablePlugins() const;

    // Plugin list persistence
    void savePluginList();
    void loadPluginList();
    void clearPluginList();

    // Plugin creation (MUST be async for stability)
    void createPluginAsync(const juce::PluginDescription& desc,
                          double sampleRate,
                          int blockSize,
                          std::function<void(std::unique_ptr<VST3EffectSlot>, juce::String error)> callback);

    // Direct access for advanced use
    juce::AudioPluginFormatManager& getFormatManager() { return formatManager; }

    // Search paths
    juce::StringArray getDefaultSearchPaths() const;
    void addCustomSearchPath(const juce::String& path);
    juce::StringArray getSearchPaths() const { return searchPaths; }

private:
    PluginManager();
    ~PluginManager();

    // Format manager
    juce::AudioPluginFormatManager formatManager;

    // Plugin list
    juce::KnownPluginList pluginList;

    // Scanning state
    std::unique_ptr<juce::PluginDirectoryScanner> scanner;
    std::unique_ptr<juce::TimeSliceThread> scanThread;
    std::atomic<bool> scanning { false };
    std::atomic<float> scanProgress { 0.0f };

    // Search paths
    juce::StringArray searchPaths;

    // Persistence
    juce::File getPluginListFile() const;

    // State
    bool initialized = false;

    // Deadman's pedal for crash detection
    juce::File getDeadmanFile() const;
    void setCurrentlyScanning(const juce::String& pluginPath);
    void clearCurrentlyScanning();
    juce::String getLastCrashedPlugin() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginManager)
};
