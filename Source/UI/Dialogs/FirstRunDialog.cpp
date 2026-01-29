#include "FirstRunDialog.h"

//=============================================================================
// FirstRunDialog
//=============================================================================
FirstRunDialog::FirstRunDialog()
{
    // Title
    titleLabel.setText("Welcome to DAW Custom", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, Theme::Colours::text());
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Subtitle
    subtitleLabel.setText("Your personal audio workstation for mashups and sound design",
                          juce::dontSendNotification);
    subtitleLabel.setFont(juce::Font(14.0f));
    subtitleLabel.setColour(juce::Label::textColourId, Theme::Colours::textMuted());
    subtitleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(subtitleLabel);

    // Scan button
    scanButton.setColour(juce::TextButton::buttonColourId, Theme::Colours::accent());
    scanButton.setColour(juce::TextButton::textColourOffId, Theme::colour(Theme::textOnAccent));
    scanButton.onClick = [this]()
    {
        showScanning();
        if (onScanPlugins)
            onScanPlugins();
    };
    addAndMakeVisible(scanButton);

    // Skip button
    skipButton.setColour(juce::TextButton::buttonColourId, Theme::colour(Theme::bgSlot));
    skipButton.setColour(juce::TextButton::textColourOffId, Theme::Colours::textMuted());
    skipButton.onClick = [this]()
    {
        markFirstRunComplete();
        if (onSkip)
            onSkip();
    };
    addAndMakeVisible(skipButton);

    // Progress bar
    progressBar.setColour(juce::ProgressBar::foregroundColourId, Theme::Colours::accent());
    progressBar.setColour(juce::ProgressBar::backgroundColourId, Theme::colour(Theme::bgSlot));

    // Scanning label
    scanningLabel.setFont(juce::Font(14.0f));
    scanningLabel.setColour(juce::Label::textColourId, Theme::Colours::textMuted());
    scanningLabel.setJustificationType(juce::Justification::centred);

    // Complete label
    completeLabel.setFont(juce::Font(16.0f));
    completeLabel.setColour(juce::Label::textColourId, Theme::Colours::text());
    completeLabel.setJustificationType(juce::Justification::centred);

    // Start button
    startButton.setColour(juce::TextButton::buttonColourId, Theme::Colours::success());
    startButton.setColour(juce::TextButton::textColourOffId, Theme::colour(Theme::textOnAccent));
    startButton.onClick = [this]()
    {
        markFirstRunComplete();
        if (onComplete)
            onComplete();
    };

    showWelcome();
}

void FirstRunDialog::paint(juce::Graphics& g)
{
    g.fillAll(Theme::colour(Theme::bgPanel));

    // Decorative top bar
    g.setColour(Theme::Colours::accent());
    g.fillRect(0, 0, getWidth(), 4);
}

void FirstRunDialog::resized()
{
    auto bounds = getLocalBounds().reduced(40);

    switch (currentState)
    {
        case State::Welcome:
        {
            auto centre = bounds.getCentre();

            titleLabel.setBounds(bounds.getX(), centre.y - 80, bounds.getWidth(), 40);
            subtitleLabel.setBounds(bounds.getX(), centre.y - 35, bounds.getWidth(), 30);

            auto buttonArea = bounds.withY(centre.y + 20).withHeight(40);
            auto buttonWidth = 150;
            auto gap = 20;

            scanButton.setBounds(centre.x - buttonWidth - gap / 2, buttonArea.getY(),
                                 buttonWidth, 40);
            skipButton.setBounds(centre.x + gap / 2, buttonArea.getY(),
                                 buttonWidth, 40);
            break;
        }

        case State::Scanning:
        {
            auto centre = bounds.getCentre();

            titleLabel.setBounds(bounds.getX(), centre.y - 60, bounds.getWidth(), 40);
            progressBar.setBounds(centre.x - 150, centre.y, 300, 20);
            scanningLabel.setBounds(bounds.getX(), centre.y + 30, bounds.getWidth(), 30);
            break;
        }

        case State::Complete:
        {
            auto centre = bounds.getCentre();

            titleLabel.setBounds(bounds.getX(), centre.y - 60, bounds.getWidth(), 40);
            completeLabel.setBounds(bounds.getX(), centre.y - 10, bounds.getWidth(), 30);
            startButton.setBounds(centre.x - 100, centre.y + 40, 200, 40);
            break;
        }
    }
}

void FirstRunDialog::setProgress(float progress, const juce::String& currentPlugin)
{
    progressValue = static_cast<double>(progress);
    progressBar.setTextToDisplay(juce::String(static_cast<int>(progress * 100)) + "%");
    scanningLabel.setText("Scanning: " + currentPlugin, juce::dontSendNotification);
    repaint();
}

void FirstRunDialog::setScanComplete(int pluginCount)
{
    showComplete(pluginCount);
}

bool FirstRunDialog::isFirstRun()
{
    auto settingsFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("DAWCustom")
        .getChildFile("settings.xml");

    if (!settingsFile.existsAsFile())
        return true;

    auto xml = juce::parseXML(settingsFile);
    if (!xml)
        return true;

    return !xml->getBoolAttribute("firstRunComplete", false);
}

void FirstRunDialog::markFirstRunComplete()
{
    auto settingsDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("DAWCustom");
    settingsDir.createDirectory();

    auto settingsFile = settingsDir.getChildFile("settings.xml");

    auto xml = settingsFile.existsAsFile() ? juce::parseXML(settingsFile) : nullptr;
    if (!xml)
        xml = std::make_unique<juce::XmlElement>("DAWCustomSettings");

    xml->setAttribute("firstRunComplete", true);

    settingsFile.replaceWithText(xml->toString());
}

void FirstRunDialog::showWelcome()
{
    currentState = State::Welcome;

    titleLabel.setText("Welcome to DAW Custom", juce::dontSendNotification);
    titleLabel.setVisible(true);
    subtitleLabel.setVisible(true);
    scanButton.setVisible(true);
    skipButton.setVisible(true);

    progressBar.setVisible(false);
    scanningLabel.setVisible(false);
    completeLabel.setVisible(false);
    startButton.setVisible(false);

    resized();
}

void FirstRunDialog::showScanning()
{
    currentState = State::Scanning;

    titleLabel.setText("Scanning Plugins...", juce::dontSendNotification);
    titleLabel.setVisible(true);
    subtitleLabel.setVisible(false);
    scanButton.setVisible(false);
    skipButton.setVisible(false);

    progressBar.setVisible(true);
    addAndMakeVisible(progressBar);
    scanningLabel.setVisible(true);
    addAndMakeVisible(scanningLabel);
    completeLabel.setVisible(false);
    startButton.setVisible(false);

    resized();
}

void FirstRunDialog::showComplete(int pluginCount)
{
    currentState = State::Complete;

    titleLabel.setText("Setup Complete!", juce::dontSendNotification);
    titleLabel.setVisible(true);
    subtitleLabel.setVisible(false);
    scanButton.setVisible(false);
    skipButton.setVisible(false);
    progressBar.setVisible(false);
    scanningLabel.setVisible(false);

    completeLabel.setText(juce::String(pluginCount) + " plugins found",
                          juce::dontSendNotification);
    completeLabel.setVisible(true);
    addAndMakeVisible(completeLabel);
    startButton.setVisible(true);
    addAndMakeVisible(startButton);

    resized();
}

//=============================================================================
// FirstRunWindow
//=============================================================================
FirstRunWindow::FirstRunWindow()
    : juce::DocumentWindow("Welcome",
                           Theme::colour(Theme::bgPanel),
                           juce::DocumentWindow::closeButton)
{
    setContentNonOwned(&dialog, true);
    setResizable(false, false);
    centreWithSize(500, 300);
    setVisible(true);
}

void FirstRunWindow::closeButtonPressed()
{
    FirstRunDialog::markFirstRunComplete();
    setVisible(false);
}

void FirstRunWindow::showIfFirstRun(std::function<void()> onScanPlugins)
{
    if (!FirstRunDialog::isFirstRun())
        return;

    auto* window = new FirstRunWindow();

    window->getDialog().onScanPlugins = onScanPlugins;
    window->getDialog().onSkip = [window]() { delete window; };
    window->getDialog().onComplete = [window]() { delete window; };
}
