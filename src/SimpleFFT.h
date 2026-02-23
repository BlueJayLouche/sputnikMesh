#pragma once

#include "ofMain.h"
#include <complex>
#include <vector>

//==============================================================================
// Simple FFT Implementation (Cooley-Tukey radix-2)
// Self-contained - no external dependencies
//==============================================================================
class SimpleFFT {
public:
    // Compute FFT on real input, return magnitude spectrum
    // Output size will be N/2 + 1 (only positive frequencies)
    static void compute(std::vector<float>& input, std::vector<float>& output);
    
private:
    // Bit-reversal permutation for FFT
    static void bitReversalPermutation(std::vector<std::complex<float>>& data);
    
    // Core FFT computation (in-place)
    static void fft(std::vector<std::complex<float>>& data);
};
