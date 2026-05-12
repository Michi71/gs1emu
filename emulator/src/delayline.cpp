#include "delayline.h"

void DelayLine::setDelay(float newDelayInSamples)
{
    delay = newDelayInSamples;
    if (delay < 0.0f) delay = 0.0f;
    if (delay > DELAY_LINE_SIZE - 2) delay = (float)(DELAY_LINE_SIZE - 2);
    delayInt = (int)delay;
    delayFrac = delay - (float)delayInt;
}

float DelayLine::getDelay() const
{
    return delay;
}

void DelayLine::reset()
{
    for (int i = 0; i < DELAY_LINE_SIZE; i++)
        buffer[i] = 0.0f;
    writePos = 0;
    readPos = 0;
}

void DelayLine::pushSample(float sample)
{
    buffer[writePos] = sample;
    writePos = (writePos - 1) & MASK;
}

float DelayLine::popSample()
{
    float result = interpolateSample();
    readPos = (readPos - 1) & MASK;
    return result;
}

float DelayLine::interpolateSample() const
{
    int readIndex1 = (readPos + delayInt) & MASK;
    int readIndex2 = (readIndex1 + 1) & MASK;

    float s1 = buffer[readIndex1];
    float s2 = buffer[readIndex2];

    return s1 + (s2 - s1) * delayFrac;
}