#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

// Include test files
#include "AudioTests.cpp"

int main()
{
    juce::UnitTestRunner runner;
    runner.setAssertOnFailure(false);

    // Run all registered tests
    runner.runAllTests();

    // Print results
    int numTests = runner.getNumResults();
    int numPassed = 0;
    int numFailed = 0;

    for (int i = 0; i < numTests; ++i)
    {
        auto* result = runner.getResult(i);
        if (result->failures > 0)
            numFailed++;
        else
            numPassed++;
    }

    std::cout << "\n========================================\n";
    std::cout << "Test Results: " << numPassed << " passed, " << numFailed << " failed\n";
    std::cout << "========================================\n";

    return numFailed > 0 ? 1 : 0;
}
