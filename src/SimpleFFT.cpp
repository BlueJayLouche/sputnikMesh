#include "SimpleFFT.h"

//==============================================================================
// Simple FFT Implementation (Cooley-Tukey radix-2)
//==============================================================================
void SimpleFFT::compute(std::vector<float>& input, std::vector<float>& output) {
    int N = input.size();
    if (N == 0) return;
    
    // Ensure output is correct size
    output.resize(N / 2 + 1);
    
    // Create complex data array
    std::vector<std::complex<float>> data(N);
    for (int i = 0; i < N; i++) {
        data[i] = std::complex<float>(input[i], 0.0f);
    }
    
    // Perform FFT
    fft(data);
    
    // Compute magnitude spectrum (only first N/2+1 values are unique for real input)
    for (int i = 0; i < N / 2 + 1 && i < output.size(); i++) {
        output[i] = std::abs(data[i]);
    }
}

void SimpleFFT::bitReversalPermutation(std::vector<std::complex<float>>& data) {
    int N = data.size();
    int j = 0;
    for (int i = 1; i < N; i++) {
        int bit = N >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }
}

void SimpleFFT::fft(std::vector<std::complex<float>>& data) {
    int N = data.size();
    if (N <= 1) return;
    
    // Ensure N is power of 2
    if ((N & (N - 1)) != 0) return;
    
    // Bit reversal permutation
    bitReversalPermutation(data);
    
    // FFT computation
    for (int len = 2; len <= N; len <<= 1) {
        float ang = -2.0f * PI / len;
        std::complex<float> wlen(cosf(ang), sinf(ang));
        for (int i = 0; i < N; i += len) {
            std::complex<float> w(1.0f, 0.0f);
            for (int j = 0; j < len / 2; j++) {
                std::complex<float> u = data[i + j];
                std::complex<float> v = data[i + j + len / 2] * w;
                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}
