/*
 * Copyright (c) 2017-2021 Arm Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef ARM_COMPUTE_NEEARLYEXITLAYER_H
#define ARM_COMPUTE_NEEARLYEXITLAYER_H

#include "arm_compute/runtime/IFunction.h"
#include "arm_compute/runtime/MemoryGroup.h"
#include <memory>

namespace arm_compute
{
class ITensor;
class ITensorInfo;

/** Basic function to compute a SoftmaxLayer and a Log SoftmaxLayer. */
class NEEarlyExitLayer : public IFunction
{
public:
    /** Constructor */
	NEEarlyExitLayer();
    /** Prevent instances of this class from being copied (As this class contains pointers) */
	NEEarlyExitLayer(const NEEarlyExitLayer &) = delete;
    /** Default move constructor */
	NEEarlyExitLayer(NEEarlyExitLayer &&);
    /** Prevent instances of this class from being copied (As this class contains pointers) */
	NEEarlyExitLayer &operator=(const NEEarlyExitLayer &) = delete;
    /** Default move assignment operator */
	NEEarlyExitLayer &operator=(NEEarlyExitLayer &&);
    /** Default destructor */
    ~NEEarlyExitLayer();
    /** Set the input and output tensors.
     *
     * @param[in,out] input  Source tensor. Data types supported: QASYMM8/QASYMM8_SIGNED/F16/F32. If the width is not a
     *                       multiple of the internal processing block size, @ref NEFillBorder replicates the
     *                       last value of each row to the nearest multiple.
     * @param[out]    output Destination tensor. Data types supported: same as @p input.
     * @param[in]     beta   (Optional) A scaling factor for the exponent.
     * @param[in]     axis   (Optional) The dimension in which to apply the function. E.g. for input of shape 4x5x6 and
     *                       axis=1, softmax will be applied to 4x6=24 vectors of size 5. Defaults to 0
     */
    void configure(ITensor *input);

    /** Static function to check if given info will lead to a valid configuration of @ref NESoftmaxLayer
     *
     * @param[in] input  Source tensor info. Data types supported: QASYMM8/QASYMM8_SIGNED/F16/F32.
     * @param[in] output Destination tensor info. Data types supported: same as @p input
     * @param[in] beta   (Optional) A scaling factor for the exponent.
     * @param[in] axis   (Optional) The dimension in which to apply the function. E.g. for input of shape 4x5x6 and
     *                       axis=1, softmax will be applied to 4x6=24 vectors of size 5. Defaults to 0
     *
     * @return a status
     */
    static Status validate(const ITensorInfo *input);

    // Inherited methods overridden:
    void run() override;

private:
    const ITensor    *_input;
};


} // namespace arm_compute
#endif /* ARM_COMPUTE_NESOFTMAXLAYER_H */
