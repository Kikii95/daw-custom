#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "UI/Theme/AppTheme.h"
#include <functional>

/**
 * First-run onboarding dialog.
 * Welcomes user and offers to scan for VST3 plugins.
 */
class FirstRunDialog : public juce::Component
{
public:
    FirstRunDialog();
    ~FirstRunDialog() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Callbacks
    std::function<void()> onScanPlugins;
    std::function<void()> onSkip;
    std::function<void()> onComplete;

    // Progress updates
    void setProgress(float progress, const juce::String& currentPlugin);
    void setScanComplete(int pluginCount);

    // Check if first run (call before showing)
    static bool isFirstRun();
    static void markFirstRunComplete();

private:
    enum class State { Welcome, Scanning, Complete };
    State currentState = State::Welcome;

    // Welcome state
    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::TextButton scanButton { "Scan Plugins" };
    juce::TextButton skipButton { "Skip" };

    // Scanning state
    double progressValue = 0.0;
    juce::ProgressBar progressBar { progressValue };
    juce::Label scanningLabel;

    // Complete state
    juce::Label completeLabel;
    juce::TextButton startButton { "Start Creating" };

    void showWelcome();
    void showScanning();
    void showComplete(int pluginCount);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FirstRunDialog)
};

/**
 * Window wrapper for FirstRunDialog.
 */
class FirstRunWindow : public juce::DocumentWindow
{
public:
    FirstRunWindow();
    ~FirstRunWindow() override = default;

    void closeButtonPressed() override;

    FirstRunDialog& getDialog() { return dialog; }

    static void showIfFirstRun(std::function<void()> onScanPlugins);

private:
    FirstRunDialog dialog;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FirstRunWindow)
};
