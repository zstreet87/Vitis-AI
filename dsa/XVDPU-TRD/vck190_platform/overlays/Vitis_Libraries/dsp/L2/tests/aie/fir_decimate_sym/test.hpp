/*
 * Copyright 2021 Xilinx, Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _DSPLIB_TEST_HPP_
#define _DSPLIB_TEST_HPP_

#include <adf.h>
#include <vector>
#include "utils.hpp"

#include "uut_config.h"
#include "test_stim.hpp"

#define Q(x) #x
#define QUOTE(x) Q(x)

#ifndef UUT_GRAPH
#define UUT_GRAPH fir_decimate_sym_graph
#endif

#include QUOTE(UUT_GRAPH.hpp)

using namespace adf;

namespace xf {
namespace dsp {
namespace aie {
namespace testcase {

class test_graph : public graph {
   private:
    static constexpr unsigned int kNumTaps = (FIR_LEN + 1) / 2;
    COEFF_TYPE taps[kNumTaps];

   public:
    port<input> in;
    port<output> out;
#if (NUM_OUTPUTS == 2)
    port<output> out2;
#endif
#if (USE_COEFF_RELOAD == 1) // Reloadable coefficients
    port<input> coeff;
#endif

    COEFF_TYPE m_taps[2][kNumTaps];
    std::vector<COEFF_TYPE> m_taps_v;

    // Constructor
    test_graph() {
        printf("========================\n");
        printf("== UUT Graph Class: ");
        printf(QUOTE(UUT_GRAPH));
        printf("\n");
        printf("========================\n");
        printf("Input samples     = %d \n", INPUT_SAMPLES);
        printf("Input window [B]  = %lu \n", INPUT_SAMPLES * sizeof(DATA_TYPE));
        printf("Input margin      = %lu \n", INPUT_MARGIN(FIR_LEN, DATA_TYPE));
        printf("Output samples    = %d \n", OUTPUT_SAMPLES);
        printf("FIR Length        = %d \n", FIR_LEN);
        printf("Decimation factor = %d \n", DECIMATE_FACTOR);
        printf("Shift             = %d \n", SHIFT);
        printf("Round_mode        = %d \n", ROUND_MODE);
        printf("Dual inputs       = %d \n", DUAL_IP);
        printf("Coeff reload      = %d \n", USE_COEFF_RELOAD);
        printf("Number of outputs = %d \n", NUM_OUTPUTS);
        printf("Data type         = ");
        printf(QUOTE(DATA_TYPE));
        printf("\n");
        printf("Coeff type        = ");
        printf(QUOTE(COEFF_TYPE));
        printf("\n");
        namespace dsplib = xf::dsp::aie;
        // Generate random taps
        // STIM_GEN_INCONES, STIM_GEN_ALLONES, STIM_GEN_IMPULSE, STIM_GEN_RANDOM
        test_stim<COEFF_TYPE, kNumTaps, 0> taps_gen(QUOTE(COEFF_FILE));
        srand(115552);
        int error_tap =
            rand() %
            kNumTaps; // Randomly selects a single coefficient to be changed in second coefficient array to test reload
#ifdef _DSPLIB_FIR_DEBUG_ADL_
        error_tap = kNumTaps - 1; // Always overwrite the last coeff only.
#endif                            // _DSPLIB_FIR_DEBUG_ADL_
        for (int j = 0; j < 2; j++) {
            taps_gen.prepSeed(COEFF_SEED);
            taps_gen.gen(STIM_TYPE, taps);
            for (int i = 0; i < kNumTaps; i++) {
                m_taps[j][i] = taps[i];
                if (i == error_tap && j == 1) {
                    m_taps[j][i] = addError(m_taps[j][i]);
                }
            }
        }
        // Copy taps from C++ array into std::vector
        for (int i = 0; i < kNumTaps; i++) {
            m_taps_v.push_back(m_taps[0][i]);
        }

// FIR sub-graph
#if (USE_COEFF_RELOAD == 1) // Reloadable coefficients
        dsplib::fir::decimate_sym::UUT_GRAPH<DATA_TYPE, COEFF_TYPE, FIR_LEN, DECIMATE_FACTOR, SHIFT, ROUND_MODE,
                                             INPUT_SAMPLES, CASC_LEN, DUAL_IP, USE_COEFF_RELOAD_TRUE, NUM_OUTPUTS>
            firGraph;
#else // Static coefficients
        dsplib::fir::decimate_sym::UUT_GRAPH<DATA_TYPE, COEFF_TYPE, FIR_LEN, DECIMATE_FACTOR, SHIFT, ROUND_MODE,
                                             INPUT_SAMPLES, CASC_LEN, DUAL_IP, USE_COEFF_RELOAD_FALSE, NUM_OUTPUTS>
            firGraph(m_taps_v);
#endif

        // Make connections
        // Size of window in Bytes.
        connect<>(in, firGraph.in);
#if (DUAL_IP == 1)
        connect<>(in, firGraph.in2);
#endif
#if (USE_CHAIN == 1 && NUM_OUTPUTS == 1)
        // Chained connections mutually explusive with multiple outputs.
        dsplib::fir::decimate_sym::UUT_GRAPH<DATA_TYPE, COEFF_TYPE, FIR_LEN, DECIMATE_FACTOR, SHIFT, ROUND_MODE,
                                             INPUT_SAMPLES / DECIMATE_FACTOR, CASC_LEN>
            firGraph2(m_taps_v);
        connect<>(firGraph.out, firGraph2.in);
        connect<>(firGraph2.out, out);
#else
        connect<>(firGraph.out, out);
#if (NUM_OUTPUTS == 2)
        connect<>(firGraph.out2, out2);
#endif
#endif
#if (USE_COEFF_RELOAD == 1)
        connect<>(coeff, firGraph.coeff);
#endif

#ifdef USING_UUT
        // Report out for AIE Synthesizer QoR harvest
        kernel* myKernel;
        dsplib::fir::decimate_sym::fir_decimate_sym<DATA_TYPE, COEFF_TYPE, FIR_LEN, DECIMATE_FACTOR, SHIFT, ROUND_MODE,
                                                    INPUT_SAMPLES, false, false, FIR_LEN, 0, CASC_LEN, USE_COEFF_RELOAD,
                                                    NUM_OUTPUTS>* myDecSym;
        if (&firGraph.getKernels()[0] != NULL) {
            printf("KERNEL_ARCHS: [");
            for (int i = 0; i < CASC_LEN; i++) {
                myKernel = &firGraph.getKernels()[i];
                myDecSym =
                    (dsplib::fir::decimate_sym::fir_decimate_sym<DATA_TYPE, COEFF_TYPE, FIR_LEN, DECIMATE_FACTOR, SHIFT,
                                                                 ROUND_MODE, INPUT_SAMPLES, false, false, FIR_LEN, 0,
                                                                 CASC_LEN, USE_COEFF_RELOAD, NUM_OUTPUTS>*)myKernel;
                printf("%d", myDecSym->get_m_kArch());
                if (i == CASC_LEN - 1) {
                    printf("]\n");
                } else {
                    printf(",");
                }
            }
        }
#endif
        printf("========================\n");
    };
};
}
}
}
};

#endif // _DSPLIB_TEST_HPP_
