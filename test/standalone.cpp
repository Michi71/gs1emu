#include "SDL.h"
#include <portmidi.h>
#include <termios.h>
#include <unistd.h>
#include <algorithm>
#include <cmath>
#include <thread>
#include <atomic>
#include <iostream>
#include "gs1emu.h"

#define SAMPLE_RATE 34687
//#define SAMPLE_RATE 48000
#define MAX_DELAY_SAMPLES 512
#define AUDIO_CHANNELS 2

static int audio_buffer_size;
static int audio_page_size;

static SDL_AudioDeviceID sdl_audio;

CGS1Emu* gs1emu = nullptr;
float sampleBufferL[512];
float sampleBufferR[512];

std::atomic<char> lastKey = 0;

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

// Wandelt float [-1..1] nach int16_t [-32767..32767]
inline int16_t floatToInt16(float val) {
    return static_cast<int16_t>(std::fmax(-32767.0f, std::fmin(32767.0f, val * 32767.0f)));
}

void audio_callback(void * /*userdata*/, Uint8 *stream, int len) {
    int numSamples = len / 4;  // 2 x 16bit

    gs1emu->processBlock(sampleBufferL, sampleBufferR, numSamples);

    int16_t* out = (int16_t*)stream;
    for (int i = 0; i < numSamples; ++i) {
        out[i * 2]     = floatToInt16(sampleBufferL[i]);
        out[i * 2 + 1] = floatToInt16(sampleBufferR[i]);
    }
}

static const char *audio_format_to_str(int format) {
  switch (format) {
  case AUDIO_S8:
    return "S8";
  case AUDIO_U8:
    return "U8";
  case AUDIO_S16MSB:
    return "S16MSB";
  case AUDIO_S16LSB:
    return "S16LSB";
  case AUDIO_U16MSB:
    return "U16MSB";
  case AUDIO_U16LSB:
    return "U16LSB";
  case AUDIO_S32MSB:
    return "S32MSB";
  case AUDIO_S32LSB:
    return "S32LSB";
  case AUDIO_F32MSB:
    return "F32MSB";
  case AUDIO_F32LSB:
    return "F32LSB";
  }
  return "UNK";
}

int MCU_OpenAudio(int deviceIndex, int pageSize, int pageNum) {
  SDL_AudioSpec spec = {};
  SDL_AudioSpec spec_actual = {};

  audio_page_size = (pageSize / 2) * 2; // must be even
  audio_buffer_size = audio_page_size * pageNum;

  spec.format = AUDIO_S16SYS;
  spec.freq = SAMPLE_RATE;
  spec.channels = 2;
  spec.callback = audio_callback;
  spec.samples = audio_page_size / 4;

  int num = SDL_GetNumAudioDevices(0);
  if (num == 0) {
    printf("No audio output device found.\n");
    return 0;
  }

  if (deviceIndex < -1 || deviceIndex >= num) {
    printf("Out of range audio device index is requested. Default audio output "
           "device is selected.\n");
    deviceIndex = -1;
  }

  const char *audioDevicename = deviceIndex == -1
                                    ? "Default device"
                                    : SDL_GetAudioDeviceName(deviceIndex, 0);

  sdl_audio = SDL_OpenAudioDevice(deviceIndex == -1 ? NULL : audioDevicename, 0,
                                  &spec, &spec_actual, 0);
  if (!sdl_audio) {
    return 0;
  }

  printf("Audio device: %s\n", audioDevicename);

  printf("Audio Requested: F=%s, C=%d, R=%d, B=%d\n",
         audio_format_to_str(spec.format), spec.channels, spec.freq,
         spec.samples);

  printf("Audio Actual: F=%s, C=%d, R=%d, B=%d\n",
         audio_format_to_str(spec_actual.format), spec_actual.channels,
         spec_actual.freq, spec_actual.samples);
  fflush(stdout);

  SDL_PauseAudioDevice(sdl_audio, 0);

  return 1;
}

void MCU_CloseAudio(void) { SDL_CloseAudio(); }

static PmStream *midiInStream;

int MIDI_Init() {
  Pm_Initialize();

  int in_id = Pm_CreateVirtualInput("PicoGS1", NULL, NULL);

  Pm_OpenInput(&midiInStream, in_id, NULL, 0, NULL, NULL);
  // Week 4: Remove PM_FILT_ACTIVE - it filters out Note-Off events!
  // Only filter out clock and system exclusive messages
  //Pm_SetFilter(midiInStream, PM_FILT_CLOCK | PM_FILT_SYSEX);
  Pm_SetFilter(midiInStream, PM_FILT_ACTIVE | PM_FILT_CLOCK | PM_FILT_SYSEX);

  // Empty the buffer, just in case anything got through
  PmEvent receiveBuffer[1];
  while (Pm_Poll(midiInStream)) {
    Pm_Read(midiInStream, receiveBuffer, 1);
  }

  return 1;
}

void MIDI_Quit() { Pm_Terminate(); }

void MIDI_Update() {
  PmEvent event;
  uint8_t data[3];
  while (Pm_Read(midiInStream, &event, 1)) {
    data[0] = Pm_MessageStatus(event.message);
    data[1] = Pm_MessageData1(event.message);
    data[2] = Pm_MessageData2(event.message);

    // CRITICAL: SDL ruft den Audio-Callback in einem eigenen Thread.
    // processMidi() modifiziert dexedVoices[].init() Felder, die der
    // Audio-Thread in processBlock() liest. Ohne Lock → Race Condition,
    // Bus Error möglich beim Voice-Wraparound (15 → 0).
    SDL_LockAudioDevice(sdl_audio);
    gs1emu->processMidi(&data[0], 3);
    SDL_UnlockAudioDevice(sdl_audio);

    //printf("MIDI: %02X %02X %02X\n", Pm_MessageStatus(event.message),
    //       Pm_MessageData1(event.message), Pm_MessageData2(event.message));
  }
}

int main() {

  char buf[100];

  gs1emu = new CGS1Emu();
  gs1emu->Initialize();


  
  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
    fprintf(stderr, "FATAL ERROR: Failed to initialize the SDL2: %s.\n",
            SDL_GetError());
    fflush(stderr);
    return 2;
  }

  if (!MCU_OpenAudio(-1, 512, 32)) {
    fprintf(stderr, "FATAL ERROR: Failed to open the audio stream.\n");
    fflush(stderr);
    return 2;
  }

  if (!MIDI_Init()) {
    fprintf(stderr, "ERROR: Failed to initialize the MIDI Input.\nWARNING: "
                    "Continuing without MIDI Input...\n");
    fflush(stderr);
  }

  std::thread inputThread(inputThreadFunc);

  printf("'+'/'-' patch | 'e' ensemble | 'd' detune | 'x' dbl-stack\n");
  printf("'t' tremolo  | '['/']' speed | '{'/'}' depth\n");
  printf("'v' vibrato  | 'n'/'m' speed | ','/'.' depth\n");
  printf("EQ: 'b'/'B' bass | 'f'/'F' mid | 'h'/'H' treble  (step: 1dB) | 'q' quit\n\n");

  gs1emu->setCurrentProgram(0);
  printf("Patch %2d: %s\n", gs1emu->getCurrentProgram() + 1,
         gs1emu->getProgramName(gs1emu->getCurrentProgram()));

  bool quit_requested = false;
  bool ensemble = false;
  bool doubleStacks = false;

  // Detune-Reihenfolge wie auf dem originalen GS1-Schalter
  const DetuneMode detuneCycle[] = {
      DetuneMode::RANDOM2, DetuneMode::RANDOM1, DetuneMode::OFF,
      DetuneMode::STATIC1, DetuneMode::STATIC2
  };
  const char* detuneNames[] = { "RANDOM 2", "RANDOM 1", "OFF", "STATIC 1", "STATIC 2" };
  int detuneIndex = 2; // startet auf OFF

  while (!quit_requested) {
    MIDI_Update();

    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event)) {
      switch (sdl_event.type) {
      case SDL_QUIT:
        quit_requested = true;
        break;
      }
    }

    char c = lastKey.exchange(0); // holt und löscht das letzte Zeichen
    if (c == '+') {
        int prog = (gs1emu->getCurrentProgram() + 1) % gs1emu->getNumPrograms();
        SDL_LockAudioDevice(sdl_audio);
        gs1emu->setCurrentProgram(prog);
        SDL_UnlockAudioDevice(sdl_audio);
        printf("Patch %2d: %s\n", prog + 1, gs1emu->getProgramName(prog));
    } else if (c == '-') {
        int prog = gs1emu->getCurrentProgram() - 1;
        if (prog < 0) prog = gs1emu->getNumPrograms() - 1;
        SDL_LockAudioDevice(sdl_audio);
        gs1emu->setCurrentProgram(prog);
        SDL_UnlockAudioDevice(sdl_audio);
        printf("Patch %2d: %s\n", prog + 1, gs1emu->getProgramName(prog));
    } else if (c == 'e') {
        gs1emu->setEnsembleOn(!ensemble);
        ensemble = gs1emu->getEnsembleOn();
        printf("Ensemble = %s\n", ensemble ? "ON" : "OFF");
    } else if (c == 'd') {
        detuneIndex = (detuneIndex + 1) % 5;
        SDL_LockAudioDevice(sdl_audio);
        gs1emu->setDetuneMode(detuneCycle[detuneIndex]);
        SDL_UnlockAudioDevice(sdl_audio);
        printf("Detune    = %s\n", detuneNames[detuneIndex]);
    } else if (c == 'x') {
        SDL_LockAudioDevice(sdl_audio);
        gs1emu->setDoubleStacksOn(!doubleStacks);
        SDL_UnlockAudioDevice(sdl_audio);
        doubleStacks = gs1emu->getDoubleStacksOn();
        printf("DblStack  = %s\n", doubleStacks ? "ON" : "OFF");
    } else if (c == 't') {
        SDL_LockAudioDevice(sdl_audio);
        gs1emu->setTremoloOn(!gs1emu->getTremoloOn());
        SDL_UnlockAudioDevice(sdl_audio);
        printf("Tremolo   = %s  Speed=%.1fHz  Depth=%.2f\n",
               gs1emu->getTremoloOn() ? "ON" : "OFF",
               gs1emu->getTremoloSpeed(), gs1emu->getTremoloDepth());
    } else if (c == '[') {
        float spd = std::max(1.0f, gs1emu->getTremoloSpeed() - 0.5f);
        SDL_LockAudioDevice(sdl_audio);
        gs1emu->setTremoloSpeed(spd);
        SDL_UnlockAudioDevice(sdl_audio);
        printf("Tremolo   Speed=%.1fHz\n", spd);
    } else if (c == ']') {
        float spd = std::min(6.0f, gs1emu->getTremoloSpeed() + 0.5f);
        SDL_LockAudioDevice(sdl_audio);
        gs1emu->setTremoloSpeed(spd);
        SDL_UnlockAudioDevice(sdl_audio);
        printf("Tremolo   Speed=%.1fHz\n", spd);
    } else if (c == '{') {
        float dep = std::max(0.0f, gs1emu->getTremoloDepth() - 0.1f);
        SDL_LockAudioDevice(sdl_audio);
        gs1emu->setTremoloDepth(dep);
        SDL_UnlockAudioDevice(sdl_audio);
        printf("Tremolo   Depth=%.2f\n", dep);
    } else if (c == '}') {
        float dep = std::min(1.0f, gs1emu->getTremoloDepth() + 0.1f);
        SDL_LockAudioDevice(sdl_audio);
        gs1emu->setTremoloDepth(dep);
        SDL_UnlockAudioDevice(sdl_audio);
        printf("Tremolo   Depth=%.2f\n", dep);
    } else if (c == 'v') {
        SDL_LockAudioDevice(sdl_audio);
        gs1emu->setVibratoOn(!gs1emu->getVibratoOn());
        SDL_UnlockAudioDevice(sdl_audio);
        printf("Vibrato   = %s  Speed=%.1fHz  Depth=%.2f\n",
               gs1emu->getVibratoOn() ? "ON" : "OFF",
               gs1emu->getVibratoSpeed(), gs1emu->getVibratoDepth());
    } else if (c == 'n') {
        float spd = std::max(4.0f, gs1emu->getVibratoSpeed() - 0.5f);
        SDL_LockAudioDevice(sdl_audio);
        gs1emu->setVibratoSpeed(spd);
        SDL_UnlockAudioDevice(sdl_audio);
        printf("Vibrato   Speed=%.1fHz\n", spd);
    } else if (c == 'm') {
        float spd = std::min(10.0f, gs1emu->getVibratoSpeed() + 0.5f);
        SDL_LockAudioDevice(sdl_audio);
        gs1emu->setVibratoSpeed(spd);
        SDL_UnlockAudioDevice(sdl_audio);
        printf("Vibrato   Speed=%.1fHz\n", spd);
    } else if (c == ',') {
        float dep = std::max(0.0f, gs1emu->getVibratoDepth() - 0.1f);
        SDL_LockAudioDevice(sdl_audio);
        gs1emu->setVibratoDepth(dep);
        SDL_UnlockAudioDevice(sdl_audio);
        printf("Vibrato   Depth=%.2f\n", dep);
    } else if (c == '.') {
        float dep = std::min(1.0f, gs1emu->getVibratoDepth() + 0.1f);
        SDL_LockAudioDevice(sdl_audio);
        gs1emu->setVibratoDepth(dep);
        SDL_UnlockAudioDevice(sdl_audio);
        printf("Vibrato   Depth=%.2f\n", dep);
    } else if (c == 'b') {
        float g = std::max(-12.0f, gs1emu->getEqBass() - 1.0f);
        SDL_LockAudioDevice(sdl_audio); gs1emu->setEqBass(g); SDL_UnlockAudioDevice(sdl_audio);
        printf("EQ Bass   = %+.0f dB\n", g);
    } else if (c == 'B') {
        float g = std::min(12.0f, gs1emu->getEqBass() + 1.0f);
        SDL_LockAudioDevice(sdl_audio); gs1emu->setEqBass(g); SDL_UnlockAudioDevice(sdl_audio);
        printf("EQ Bass   = %+.0f dB\n", g);
    } else if (c == 'f') {
        float g = std::max(-12.0f, gs1emu->getEqMid() - 1.0f);
        SDL_LockAudioDevice(sdl_audio); gs1emu->setEqMid(g); SDL_UnlockAudioDevice(sdl_audio);
        printf("EQ Mid    = %+.0f dB\n", g);
    } else if (c == 'F') {
        float g = std::min(12.0f, gs1emu->getEqMid() + 1.0f);
        SDL_LockAudioDevice(sdl_audio); gs1emu->setEqMid(g); SDL_UnlockAudioDevice(sdl_audio);
        printf("EQ Mid    = %+.0f dB\n", g);
    } else if (c == 'h') {
        float g = std::max(-12.0f, gs1emu->getEqTreble() - 1.0f);
        SDL_LockAudioDevice(sdl_audio); gs1emu->setEqTreble(g); SDL_UnlockAudioDevice(sdl_audio);
        printf("EQ Treble = %+.0f dB\n", g);
    } else if (c == 'H') {
        float g = std::min(12.0f, gs1emu->getEqTreble() + 1.0f);
        SDL_LockAudioDevice(sdl_audio); gs1emu->setEqTreble(g); SDL_UnlockAudioDevice(sdl_audio);
        printf("EQ Treble = %+.0f dB\n", g);
    } else if (c == 'q') {
        quit_requested = true;
        break;
    }
    
  }

  inputThread.join(); // wartet auf Eingabe-Thread

  MCU_CloseAudio();
  MIDI_Quit();
  SDL_Quit();

  delete gs1emu;

  return 0;
}
