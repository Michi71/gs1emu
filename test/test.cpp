#include <AudioToolbox/AudioToolbox.h>
#include <portmidi.h>
#include <termios.h>
#include <unistd.h>
#include <cstdio>
#include <cmath>
#include <thread>
#include <atomic>
#include <iostream>
#include "gs1emu.h"
#include <mutex>

#define SAMPLE_RATE 34687.0f
#define MAX_DELAY_SAMPLES 512

std::mutex synthMutex;
std::atomic<char> lastKey = 0;

CGS1Emu* gs1emu = nullptr;
static AudioUnit audioUnit;
static PmStream* midiInStream;


char getch() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    char c = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return c;
}

void inputThreadFunc() {
    while (true) {
        char c = getch();
        lastKey = c;
        if (c == 'q') break; // optional: Thread beenden bei 'q'
    }
}

OSStatus renderCallback(void*, AudioUnitRenderActionFlags*, const AudioTimeStamp*,
                        UInt32, UInt32 inNumberFrames, AudioBufferList* ioData) {
    float* out = reinterpret_cast<float*>(ioData->mBuffers[0].mData);
    std::vector<float> outL(inNumberFrames * 2 + 1);
    std::vector<float> outR(inNumberFrames * 2 + 1);

    std::lock_guard<std::mutex> lock(synthMutex);
    gs1emu->processBlock(outL.data(), outR.data(), inNumberFrames);

    // Interleave L/R in Float-Buffer
    for (UInt32 i = 0; i < inNumberFrames; ++i) {
        out[i * 2]     = outL[i];
        out[i * 2 + 1] = outR[i];
    }
    return noErr;
}

bool initAudio(int sampleRate) {
    AudioComponentDescription desc = {kAudioUnitType_Output,
                                      kAudioUnitSubType_DefaultOutput,
                                      kAudioUnitManufacturer_Apple,
                                      0, 0};
    AudioComponent comp = AudioComponentFindNext(NULL, &desc);
    if (!comp) return false;

    AudioComponentInstanceNew(comp, &audioUnit);
    AudioStreamBasicDescription format = {};
    format.mSampleRate = sampleRate;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
    format.mChannelsPerFrame = 2;
    format.mBitsPerChannel = 32;
    format.mBytesPerFrame = 8;
    format.mFramesPerPacket = 1;
    format.mBytesPerPacket = 8;

    AudioUnitSetProperty(audioUnit, kAudioUnitProperty_StreamFormat,
                         kAudioUnitScope_Input, 0, &format, sizeof(format));
    AURenderCallbackStruct cb = {renderCallback, NULL};
    AudioUnitSetProperty(audioUnit, kAudioUnitProperty_SetRenderCallback,
                         kAudioUnitScope_Input, 0, &cb, sizeof(cb));
    AudioUnitInitialize(audioUnit);
    AudioOutputUnitStart(audioUnit);
    return true;
}

bool initMIDI() {
    Pm_Initialize();
    int in_id = Pm_CreateVirtualInput("gs1emu", NULL, NULL);
    if (in_id < 0) {
        std::cerr << "Failed to create virtual MIDI input!" << std::endl;
        return false;
    }

    if (Pm_OpenInput(&midiInStream, in_id, NULL, 0, NULL, NULL) != pmNoError) {
        std::cerr << "Failed to open MIDI input!" << std::endl;
        return false;
    }

    return true;
}

void runMIDI() {
    std::cout << "Waiting for MIDI..." << std::endl;
    while (true) {
        PmEvent event;
        if (Pm_Read(midiInStream, &event, 1) > 0) {
            std::cout << "MIDI received: "
                      << std::hex << event.message << std::endl;
        }
        usleep(10000); // 10ms Pause
    }
}

void updateMIDI() {
    PmEvent event;
    uint8_t data[3];
    while (Pm_Read(midiInStream, &event, 1)) {
        std::lock_guard<std::mutex> lock(synthMutex);
        data[0] = Pm_MessageStatus(event.message);
        data[1] = Pm_MessageData1(event.message);
        data[2] = Pm_MessageData2(event.message);
        gs1emu->processMidi(&data[0], 3);

        //printf("MIDI: %02X %02X %02X\n", Pm_MessageStatus(event.message),
        //       Pm_MessageData1(event.message), Pm_MessageData2(event.message));
    }
}

int main() {
    // Init GS1Emu
    gs1emu = new CGS1Emu();
    gs1emu->Initialize();

    std::cout << "GS1 Emu initialized\n";
    
    Boolean quit=false;
    Boolean ensemble=false;
    char buf[128];
    char name[128];
    int instrument=0;

    gs1emu->setCurrentProgram(0);

    if (!initAudio(SAMPLE_RATE)) {
        fprintf(stderr, "Audio init failed.\n");
        return 1;
    }
    if (!initMIDI()) {
        fprintf(stderr, "MIDI init failed.\n");
    }

    std::thread inputThread(inputThreadFunc);

    printf("Press '+' or '-' to change patch, 'q' to quit.\n");

    while (!quit) {
        updateMIDI();

        char c = lastKey.exchange(0); // holt und löscht das letzte Zeichen
        if (c == '+') {
            int prog = (gs1emu->getCurrentProgram() + 1) % gs1emu->getNumPrograms();
            std::lock_guard<std::mutex> lock(synthMutex);
            gs1emu->setCurrentProgram(prog);
            //gs1emu->getInstrumentName(buf);
            std::cout << "Patch: " << prog << std::endl;
        } else if (c == '-') {
            int prog = gs1emu->getCurrentProgram() - 1;
            if (prog < 0) prog = gs1emu->getNumPrograms() - 1;
            std::lock_guard<std::mutex> lock(synthMutex);
            gs1emu->setCurrentProgram(prog);
            //gs1emu->getInstrumentName(buf);
            std::cout << "Patch: " << prog << std::endl;
        } else if (c == 'e') {
            gs1emu->setEnsembleOn(!ensemble);
            ensemble = gs1emu->getEnsembleOn();
            printf("Ensemble = %d\n", ensemble);
        } else if (c == 'q') {
            quit = true;
        }

        else if (c == 'q') {
            quit = true;
        }

        usleep(10000); // 10ms Pause
    }

    inputThread.join(); // wartet auf Eingabe-Thread

    AudioOutputUnitStop(audioUnit);
    AudioUnitUninitialize(audioUnit);
    AudioComponentInstanceDispose(audioUnit);
    Pm_Terminate();
    delete gs1emu;
    return 0;
}