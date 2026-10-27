#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>

class ScarlettWarmthAudioProcessor : public juce::AudioProcessor
{
public:
    ScarlettWarmthAudioProcessor();
    ~ScarlettWarmthAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override;
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Parametros (el editor se conecta a esto)
    juce::AudioProcessorValueTreeState apvts;

    // Nivel de salida normalizado 0..1 para el medidor LED
    std::atomic<float> outputLevel { 0.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateFilters();

    //==========================================================================
    // Filtro biquad (formulas RBJ), sin asignaciones de memoria en el hilo de audio
    struct Biquad
    {
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
        float z1 = 0.0f, z2 = 0.0f;

        void reset() { z1 = z2 = 0.0f; }

        float process (float x)
        {
            const float y = b0 * x + z1;
            z1 = b1 * x - a1 * y + z2;
            z2 = b2 * x - a2 * y;
            return y;
        }

        void set (double nb0, double nb1, double nb2, double na0, double na1, double na2)
        {
            b0 = (float) (nb0 / na0);
            b1 = (float) (nb1 / na0);
            b2 = (float) (nb2 / na0);
            a1 = (float) (na1 / na0);
            a2 = (float) (na2 / na0);
        }

        void makeHighpass (double fs, double f, double q)
        {
            f = juce::jmin (f, 0.45 * fs);
            const double w0 = 2.0 * juce::MathConstants<double>::pi * f / fs;
            const double c = std::cos (w0), s = std::sin (w0);
            const double alpha = s / (2.0 * q);
            set ((1.0 + c) * 0.5, -(1.0 + c), (1.0 + c) * 0.5,
                 1.0 + alpha, -2.0 * c, 1.0 - alpha);
        }

        void makePeak (double fs, double f, double q, double gainDb)
        {
            f = juce::jmin (f, 0.45 * fs);
            const double A = std::pow (10.0, gainDb / 40.0);
            const double w0 = 2.0 * juce::MathConstants<double>::pi * f / fs;
            const double c = std::cos (w0), s = std::sin (w0);
            const double alpha = s / (2.0 * q);
            set (1.0 + alpha * A, -2.0 * c, 1.0 - alpha * A,
                 1.0 + alpha / A, -2.0 * c, 1.0 - alpha / A);
        }

        void makeHighShelf (double fs, double f, double gainDb)
        {
            f = juce::jmin (f, 0.45 * fs);
            const double A = std::pow (10.0, gainDb / 40.0);
            const double w0 = 2.0 * juce::MathConstants<double>::pi * f / fs;
            const double c = std::cos (w0), s = std::sin (w0);
            const double alpha = s * 0.5 * std::sqrt (2.0);          // S = 1
            const double tsa = 2.0 * std::sqrt (A) * alpha;
            set (A * ((A + 1.0) + (A - 1.0) * c + tsa),
                 -2.0 * A * ((A - 1.0) + (A + 1.0) * c),
                 A * ((A + 1.0) + (A - 1.0) * c - tsa),
                 (A + 1.0) - (A - 1.0) * c + tsa,
                 2.0 * ((A - 1.0) - (A + 1.0) * c),
                 (A + 1.0) - (A - 1.0) * c - tsa);
        }
    };

    Biquad hpf[2], cm25[2], warm[2], silk[2];
    float dcX[2] { 0.0f, 0.0f }, dcY[2] { 0.0f, 0.0f };
    float dcR = 0.999f;

    double currentSampleRate = 44100.0;
    int  lastHpf = -1;
    bool hpfOn = true;

    juce::SmoothedValue<float> driveSm, ironSm, trimSm, bypassSm;

    std::atomic<float> *pDrive = nullptr, *pIron = nullptr, *pHpf = nullptr,
                       *pChest = nullptr, *pAnti = nullptr, *pSilk = nullptr,
                       *pTopology = nullptr, *pTrim = nullptr, *pBypass = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScarlettWarmthAudioProcessor)
};
