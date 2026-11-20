/**
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 * (c) Daniel Lemire
 */
#ifndef SIMDBITPACKING_H_
#define SIMDBITPACKING_H_

#include "common.h"

namespace FastPForLib {

void simdpack(const uint32_t *__restrict__ in, __m128i *__restrict__ out,
              uint32_t bit);
void simdpackwithoutmask(const uint32_t *__restrict__ in,
                         __m128i *__restrict__ out, uint32_t bit);
void simdunpack(const __m128i *__restrict__ in, uint32_t *__restrict__ out,
                uint32_t bit);

void SIMD_fastunpack_32(const __m128i *__restrict__ in,
                        uint32_t *__restrict__ out, const uint32_t bit);
void SIMD_fastpackwithoutmask_32(const uint32_t *__restrict__ in,
                                 __m128i *__restrict__ out, const uint32_t bit);
void SIMD_fastpack_32(const uint32_t *__restrict__ in,
                      __m128i *__restrict__ out, const uint32_t bit);

} // namespace FastPFor

#endif /* SIMDBITPACKING_H_ */
