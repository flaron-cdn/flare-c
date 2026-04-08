#include "test_runner.h"

#include "flare/runtime.h"

#include <string.h>

FLARE_TEST(action_encode_packs_into_high_32_bits) {
    int64_t respond = flare_action_encode(FLARE_ACTION_RESPOND);
    FLARE_ASSERT_EQ_INT(respond, ((int64_t)1) << 32);
    int64_t transform = flare_action_encode(FLARE_ACTION_TRANSFORM);
    FLARE_ASSERT_EQ_INT(transform, ((int64_t)2) << 32);
    int64_t passthrough = flare_action_encode(FLARE_ACTION_PASSTHROUGH);
    FLARE_ASSERT_EQ_INT(passthrough, ((int64_t)3) << 32);
}

FLARE_TEST(arena_alloc_advances_offset) {
    flare_reset_arena();
    int32_t a = flare_alloc(16);
    int32_t b = flare_alloc(16);
    FLARE_ASSERT(a != 0);
    FLARE_ASSERT(b != 0);
    FLARE_ASSERT(b != a);
}

FLARE_TEST(arena_alloc_zero_returns_zero) {
    flare_reset_arena();
    int32_t handle = flare_alloc(0);
    FLARE_ASSERT_EQ_INT(handle, 0);
}

FLARE_TEST(arena_alloc_negative_returns_zero) {
    flare_reset_arena();
    int32_t handle = flare_alloc(-1);
    FLARE_ASSERT_EQ_INT(handle, 0);
}

FLARE_TEST(arena_reset_recycles_space) {
    flare_reset_arena();
    int32_t a = flare_alloc(64);
    flare_reset_arena();
    int32_t b = flare_alloc(64);
    FLARE_ASSERT_EQ_INT(a, b);
}

FLARE_TEST(arena_alloc_exhausted_returns_zero) {
    flare_reset_arena();
    int32_t huge = flare_alloc(1024 * 1024);
    FLARE_ASSERT_EQ_INT(huge, 0);
}

FLARE_TEST(copy_to_arena_round_trips) {
    flare_reset_arena();
    const uint8_t src[] = "round-trip";
    int32_t handle = flare_copy_to_arena(src, 10);
    FLARE_ASSERT(handle != 0);
    uint8_t *got = flare_arena_addr(handle);
    FLARE_ASSERT(got != NULL);
    FLARE_ASSERT(memcmp(got, src, 10) == 0);
}

FLARE_TEST(decode_ptr_len_unpacks_high_low_halves) {
    int64_t packed = ((int64_t)0x12345678 << 32) | (int64_t)0x000000ffu;
    int32_t handle = 0;
    int32_t len = 0;
    flare_decode_ptr_len(packed, &handle, &len);
    FLARE_ASSERT_EQ_INT(handle, 0x12345678);
    FLARE_ASSERT_EQ_INT(len, 0xff);
}

void register_runtime_tests(void) {
    FLARE_RUN(action_encode_packs_into_high_32_bits);
    FLARE_RUN(arena_alloc_advances_offset);
    FLARE_RUN(arena_alloc_zero_returns_zero);
    FLARE_RUN(arena_alloc_negative_returns_zero);
    FLARE_RUN(arena_reset_recycles_space);
    FLARE_RUN(arena_alloc_exhausted_returns_zero);
    FLARE_RUN(copy_to_arena_round_trips);
    FLARE_RUN(decode_ptr_len_unpacks_high_low_halves);
}
