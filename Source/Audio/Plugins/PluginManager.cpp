#include "PluginManager.h"
#include "VST3EffectSlot.h"

PluginManager& PluginManager::getInstance()
{
    static PluginManager instance;
    return instance;
}

PluginManager::PluginManager()
{
    // Initialize search paths
    searchPaths = getDefaultSearchPaths();
}

PluginManager::~PluginManager()
{
    cancelScan();
}

void PluginManager::initialize()
{
    if (initialized)
        return;

    // Add VST3 format
    formatManager.addDefaultFormats();

    // Load cached plugin list
    loadPluginList();

    // Check for crashed plugins from last scan
    auto crashed = getLastCrashedPlugin();
    if (crashed.isNotEmpty())
    {
        DBG("PluginManager: Last scan crashed on: " + crashed);
        pluginList.addToBlacklist(crashed);
        clearCurrentlyScanning();
    }

    initialized = true;
    DBG("PluginManager initialized with " + juce::String(getNumPlugins()) + " cached plugins");
}

juce::StringArray PluginManager::getDefaultSearchPaths() const
{
    juce::StringArray paths;

#if JUCE_LINUX
    // Linux VST3 paths
    paths.add(juce::File::getSpecialLocation(juce::File::userHomeDirectory).getChildFile(".vst3").getFullPathName());
    paths.add("/usr/lib/vst3");
    paths.add("/usr/local/lib/vst3");
#elif JUCE_MAC
    // macOS VST3 paths
    paths.add(juce::File::getSpecialLocation(juce::File::userHomeDirectory)
              .getChildFile("Library/Audio/Plug-Ins/VST3").getFullPathName());
    paths.add("/Library/Audio/Plug-Ins/VST3");
#elif JUCE_WINDOWS
    // Windows VST3 paths
    paths.add("C:\\Program Files\\Common Files\\VST3");
    paths.add("C:\\Program Files (x86)\\Common Files\\VST3");
#endif

    return paths;
}

void PluginManager::addCustomSearchPath(const juce::String& path)
{
    if (!searchPaths.contains(path))
        searchPaths.add(path);
}

void PluginManager::scanForPluginsAsync(std::function<void(float progress)> onProgress,
                                        std::function<void()> onComplete)
{
    if (scanning.load())
    {
        DBG("PluginManager: Scan already in progress");
        return;
    }

    scanning = true;
    scanProgress = 0.0f;

    // Create scan thread if needed
    if (!scanThread)
    {
        scanThread = std::make_unique<juce::TimeSliceThread>("Plugin Scanner");
        scanThread->startThread();
    }

    // Build file list to scan
    juce::FileSearchPath searchPath;
    for (const auto& path : searchPaths)
        searchPath.add(juce::File(path));

    // Get VST3 format
    auto* vst3Format = formatManager.getFormat(0);  // VST3 is typically first after addDefaultFormats
    if (!vst3Format)
    {
        DBG("PluginManager: No VST3 format available");
        scanning = false;
        if (onComplete)
            onComplete();
        return;
    }

    // Create scanner
    scanner = std::make_unique<juce::PluginDirectoryScanner>(
        pluginList,
        *vst3Format,
        searchPath,
        true,  // recursive
        getDeadmanFile(),
        true   // allow plugins requiring async
    );

    // Run scan in background
    juce::Thread::launch([this, onProgress, onComplete]()
    {
        juce::String pluginBeingScanned;
        int total = 0;
        int scanned = 0;

        while (scanner && scanner->scanNextFile(true, pluginBeingScanned))
        {
            setCurrentlyScanning(pluginBeingScanned);
            scanned++;
            total = juce::jmax(total, scanned);

            float progress = total > 0 ? static_cast<float>(scanned) / static_cast<float>(total + 10) : 0.0f;
            scanProgress = progress;

            if (onProgress)
            {
                juce::MessageManager::callAsync([onProgress, progress]()
                {
                    onProgress(progress);
                });
            }
        }

        clearCurrentlyScanning();
        scanning = false;
        scanProgress = 1.0f;

        // Save updated list
        savePluginList();

        DBG("PluginManager: Scan complete, found " + juce::String(getNumPlugins()) + " plugins");

        if (onComplete)
        {
            juce::MessageManager::callAsync([onComplete]()
            {
                onComplete();
            });
        }
    });
}

void PluginManager::cancelScan()
{
    if (scanner)
    {
        scanner.reset();
    }

    if (scanThread)
    {
        scanThread->stopThread(2000);
        scanThread.reset();
    }

    scanning = false;
    clearCurrentlyScanning();
}

bool PluginManager::isScanning() const
{
    return scanning.load();
}

float PluginManager::getScanProgress() const
{
    return scanProgress.load();
}

juce::Array<juce::PluginDescription> PluginManager::getAvailablePlugins() const
{
    juce::Array<juce::PluginDescription> result;

    for (const auto& type : pluginList.getTypes())
    {
        result.add(type);
    }

    return result;
}

void PluginManager::savePluginList()
{
    auto file = getPluginListFile();
    auto xml = pluginList.createXml();

    if (xml)
    {
        file.getParentDirectory().createDirectory();
        xml->writeTo(file);
        DBG("PluginManager: Saved plugin list to " + file.getFullPathName());
    }
}

void PluginManager::loadPluginList()
{
    auto file = getPluginListFile();

    if (file.existsAsFile())
    {
        if (auto xml = juce::parseXML(file))
        {
            pluginList.recreateFromXml(*xml);
            DBG("PluginManager: Loaded " + juce::String(getNumPlugins()) + " plugins from cache");
        }
    }
}

void PluginManager::clearPluginList()
{
    pluginList.clear();
    savePluginList();
}

juce::File PluginManager::getPluginListFile() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
           .getChildFile("DAWCustom")
           .getChildFile("plugin-list.xml");
}

juce::File PluginManager::getDeadmanFile() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
           .getChildFile("DAWCustom")
           .getChildFile("plugin-scan-deadman.txt");
}

void PluginManager::setCurrentlyScanning(const juce::String& pluginPath)
{
    getDeadmanFile().getParentDirectory().createDirectory();
    getDeadmanFile().replaceWithText(pluginPath);
}

void PluginManager::clearCurrentlyScanning()
{
    getDeadmanFile().deleteFile();
}

juce::String PluginManager::getLastCrashedPlugin() const
{
    auto file = getDeadmanFile();
    if (file.existsAsFile())
        return file.loadFileAsString().trim();
    return {};
}

void PluginManager::createPluginAsync(const juce::PluginDescription& desc,
                                      double sampleRate,
                                      int blockSize,
                                      std::function<void(std::unique_ptr<VST3EffectSlot>, juce::String error)> callback)
{
    if (!callback)
        return;

    DBG("PluginManager: Creating plugin async: " + desc.name);

    formatManager.createPluginInstanceAsync(
        desc,
        sampleRate,
        blockSize,
        [callback, sampleRate, blockSize](std::unique_ptr<juce::AudioPluginInstance> instance, const juce::String& error)
        {
            if (instance)
            {
                // Prepare plugin
                instance->prepareToPlay(sampleRate, blockSize);

                // Wrap in VST3EffectSlot
                auto slot = std::make_unique<VST3EffectSlot>(std::move(instance));
                callback(std::move(slot), {});
            }
            else
            {
                DBG("PluginManager: Failed to create plugin: " + error);
                callback(nullptr, error);
            }
        });
}
