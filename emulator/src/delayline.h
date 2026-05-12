#pragma once

#include <cstdint>
#include <cstddef>

#ifndef DELAY_LINE_SIZE
#define DELAY_LINE_SIZE 512
#endif

static_assert(DELAY_LINE_SIZE > 0 && (DELAY_LINE_SIZE & (DELAY_LINE_SIZE - 1)) == 0,
              "DELAY_LINE_SIZE must be a power of 2");

class DelayLine
{
public:
    DelayLine() = default;

    void setDelay(float newDelayInSamples);
    float getDelay() const;

    void pushSample(float sample);
    float popSample();

    void reset();

private:
    float interpolateSample() const;

    float buffer[DELAY_LINE_SIZE] = {};
    int writePos = 0;
    int readPos = 0;
    static constexpr int MASK = DELAY_LINE_SIZE - 1;

    float delay = 0.0f;
    int delayInt = 0;
    float delayFrac = 0.0f;
};