# Tests

Unit tests for DAW Custom using JUCE's built-in UnitTest framework.

## Running Tests

```bash
# Build with tests enabled
cmake -B build -DBUILD_TESTS=ON
cmake --build build --target DAWCustomTests

# Run tests
./build/DAWCustomTests_artefacts/Debug/DAWCustomTests
```

## Test Structure

```
tests/
├── README.md          # This file
Source/Tests/
├── TestRunner.cpp     # Main entry point
├── AudioTests.cpp     # Audio engine tests
└── EffectTests.cpp    # DSP effect tests (future)
```

## Writing Tests

Tests use JUCE's `UnitTest` class:

```cpp
class MyTest : public juce::UnitTest
{
public:
    MyTest() : UnitTest("MyTest") {}

    void runTest() override
    {
        beginTest("Test case name");
        expect(someCondition);
        expectEquals(actual, expected);
    }
};
```

## Coverage

Current test coverage:

- [ ] AudioClip loading/playback
- [ ] AudioTrack mixing
- [ ] Effect chain processing
- [ ] Transport controller
- [ ] Project serialization
