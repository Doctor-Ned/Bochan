#include "pch.h"
#include "AudioFileProvider.h"
#include "CodecUtil.h"

using namespace bochan;

constexpr double EXPECTED_DURATION{ 120.451 };
constexpr int SAMPLE_RATE{ 48000 };
const size_t BUFFER_SIZE{ CodecUtil::getBytesPerSecond(SAMPLE_RATE) };

struct TestData {
    const char* filename{ nullptr };
    double epsilon{ 0.0 };
};

const std::vector<TestData> TEST_DATA{
    {"TestSounds/spanish_flea.mp3", 0.1},
    {"TestSounds/spanish_flea.flac", 0.25},
    {"TestSounds/spanish_flea.ogg", 0.75},
    {"TestSounds/spanish_flea.wav", 0.1}
};

TEST(AudioFileProviderTest, InitializationTest) {
    for (const TestData& data : TEST_DATA) {
        AudioFileProvider provider;
        ASSERT_TRUE(provider.initialize(data.filename, SAMPLE_RATE, BUFFER_SIZE));
    }
}

TEST(AudioFileProviderTest, DurationTest) {
    for (const TestData& data : TEST_DATA) {
        AudioFileProvider provider;
        ASSERT_TRUE(provider.initialize(data.filename, SAMPLE_RATE, BUFFER_SIZE));
        ASSERT_LE(abs(provider.getDuration() - EXPECTED_DURATION), data.epsilon);
    }
}

TEST(AudioFileProviderTest, PositionAndRewindingTest) {
    for (const TestData& data : TEST_DATA) {
        AudioFileProvider provider;
        ASSERT_TRUE(provider.initialize(data.filename, SAMPLE_RATE, BUFFER_SIZE));
        ASSERT_LE(provider.getPositionSeconds(), data.epsilon);
        ASSERT_TRUE(provider.setPositionSeconds(150.0));
        ASSERT_LE(abs(provider.getPositionSeconds() - provider.getDuration()), data.epsilon);
        ASSERT_TRUE(provider.setPositionSeconds(80.0));
        ASSERT_LE(abs(provider.getPositionSeconds() - 80.0), data.epsilon);
        ASSERT_TRUE(provider.rewindForward(10.0));
        ASSERT_LE(abs(provider.getPositionSeconds() - 90.0), data.epsilon);
        ASSERT_TRUE(provider.rewindBackward(20.0));
        ASSERT_LE(abs(provider.getPositionSeconds() - 70.0), data.epsilon);
        ASSERT_TRUE(provider.rewindToStart());
        ASSERT_LE(provider.getPositionSeconds(), data.epsilon);
    }
}