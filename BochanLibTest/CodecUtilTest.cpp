#include "pch.h"
#include "CodecUtil.h"

#include <cmath>

#define BIT_PRECISION 2
const int IEPSILON{ BIT_PRECISION };
const float EPSILON{ static_cast<float>(BIT_PRECISION) / pow(2.0f, 16.0f) };

using namespace bochan;
TEST(CodecUtil, Int16ToFloat) {
    int16_t ints[4]{ 0, -32768, 32767, 16384 };
    float floats[4];
    CodecUtil::int16ToFloat(gsl::make_span<int16_t>(ints, 4), gsl::make_span<float>(floats, 4));
    ASSERT_EQ(0.0f, floats[0]);
    ASSERT_LE(abs(floats[1] + 1.0f), EPSILON);
    ASSERT_LE(abs(floats[2] - 1.0f), EPSILON);
    ASSERT_LE(abs(floats[3] - 0.5f), EPSILON);
}
TEST(CodecUtil, FloatToInt16) {
    float floats[4]{ 0.0f, -1.0f, 1.0f, 0.5f };
    int16_t ints[4];
    CodecUtil::floatToInt16(gsl::make_span<float>(floats, 4), gsl::make_span<int16_t>(ints, 4));
    ASSERT_EQ(0, ints[0]);
    ASSERT_LE(abs(static_cast<int>(ints[1]) + 32768), IEPSILON);
    ASSERT_LE(abs(static_cast<int>(ints[2]) - 32767), IEPSILON);
    ASSERT_LE(abs(static_cast<int>(ints[3]) - 16384), IEPSILON);
}
TEST(CodecUtil, Int16ToFloatAndBack) {
    int16_t ints[4]{ 0, -32768, 32767, 16384 };
    float floats[4];
    CodecUtil::int16ToFloat(gsl::make_span<int16_t>(ints, 4), gsl::make_span<float>(floats, 4));
    CodecUtil::floatToInt16(gsl::make_span<float>(floats, 4), gsl::make_span<int16_t>(ints, 4));
    ASSERT_EQ(ints[0], 0);
    ASSERT_LE(abs(static_cast<int>(ints[1]) + 32768), IEPSILON);
    ASSERT_LE(abs(static_cast<int>(ints[2]) - 32767), IEPSILON);
    ASSERT_LE(abs(static_cast<int>(ints[3]) - 16384), IEPSILON);
}
TEST(CodecUtil, FloatToInt16AndBack) {
    float floats[4]{ 0.0f, -1.0f, 1.0f, 0.5f };
    int16_t ints[4];
    CodecUtil::floatToInt16(gsl::make_span<float>(floats, 4), gsl::make_span<int16_t>(ints, 4));
    CodecUtil::int16ToFloat(gsl::make_span<int16_t>(ints, 4), gsl::make_span<float>(floats, 4));
    ASSERT_EQ(0.0f, floats[0]);
    ASSERT_LE(abs(floats[1] + 1.0f), EPSILON);
    ASSERT_LE(abs(floats[2] - 1.0f), EPSILON);
    ASSERT_LE(abs(floats[3] - 0.5f), EPSILON);
}