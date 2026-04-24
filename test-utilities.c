// Application to test unpack utilities
// PackLab - CS213 - Northwestern University

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unpack-utilities.h"


int test_lfsr_step(void) {
  // A properly created LFSR should do two things
  //  1. It should generate specific new state based on a known initial state
  //  2. It should iterate through all 2^16 integers, once each (except 0)

  // Create an array to track if the LFSR hit each integer (except 0)
  // 2^16 (65536) possibilities
  bool* lfsr_states = malloc_and_check(65536);
  memset(lfsr_states, 0, 65536);

  // Initial 16 LFSR states
  uint16_t correct_lfsr_states[16] = {
    0x1337, 0x099B, 0x84CD, 0x4266,
    0x2133, 0x1099, 0x884C, 0xC426,
    0x6213, 0xB109, 0x5884, 0x2C42,
    0x1621, 0x0B10, 0x8588, 0x42C4
  };

  // Step the LFSR until a state repeats
  bool repeat        = false;
  size_t steps       = 0;
  uint16_t new_state = 0x1337; // known initial state
  while (!repeat) {

    // Iterate LFSR
    steps++;
    new_state = lfsr_step(new_state);

    // Check if this state has already been reached
    repeat = lfsr_states[new_state];
    lfsr_states[new_state] = true;

    // Check first 16 LFSR steps
    if (steps < 16) {
      if (new_state != correct_lfsr_states[steps]) {
        printf("ERROR: at step %lu, expected state 0x%04X but received state 0x%04X\n",
            steps, correct_lfsr_states[steps], new_state);
        free(lfsr_states);
        return 1;
      }
    }
  }

  // Check that all integers were hit. Should take 2^16 (65536) steps (2^16-1 integers, plus a repeat)
  if (steps != 65536) {
    printf("ERROR: expected %d iterations before a repeat, but ended after %lu steps\n", 65536, steps);
    free(lfsr_states);
    return 1;
  }

  // Cleanup
  free(lfsr_states);
  return 0;
}

int test_decrypt_even_length(void) {
  uint8_t input_data[] = {0x60, 0x5A, 0xFF, 0xB7};
  uint8_t input_copy[] = {0x60, 0x5A, 0xFF, 0xB7};
  uint8_t output_data[sizeof(input_data)] = {0};
  uint8_t expected_output[] = {0xFB, 0x53, 0x32, 0x33};

  decrypt_data(input_data, sizeof(input_data), output_data, sizeof(output_data), 0x1337);

  if (memcmp(output_data, expected_output, sizeof(expected_output)) != 0) {
    printf("ERROR: even-length decrypt output mismatch\n");
    return 1;
  }

  if (memcmp(input_data, input_copy, sizeof(input_data)) != 0) {
    printf("ERROR: decrypt_data modified the input buffer\n");
    return 1;
  }

  return 0;
}

int test_decrypt_odd_length(void) {
  uint8_t input_data[] = {0x60, 0x5A, 0xFF};
  uint8_t output_data[sizeof(input_data)] = {0};
  uint8_t expected_output[] = {0xFB, 0x53, 0x32};

  decrypt_data(input_data, sizeof(input_data), output_data, sizeof(output_data), 0x1337);

  if (memcmp(output_data, expected_output, sizeof(expected_output)) != 0) {
    printf("ERROR: odd-length decrypt output mismatch\n");
    return 1;
  }

  return 0;
}

int test_decrypt_truncated_output(void) {
  uint8_t input_data[] = {0x60, 0x5A, 0xFF, 0xB7};
  uint8_t output_data[] = {0xAA, 0xAA, 0xAA, 0xAA};
  uint8_t expected_prefix[] = {0xFB, 0x53, 0x32};

  decrypt_data(input_data, sizeof(input_data), output_data, 3, 0x1337);

  if (memcmp(output_data, expected_prefix, sizeof(expected_prefix)) != 0) {
    printf("ERROR: truncated decrypt output mismatch\n");
    return 1;
  }

  if (output_data[3] != 0xAA) {
    printf("ERROR: decrypt_data wrote past output_len\n");
    return 1;
  }

  return 0;
}

int test_decrypt_zero_length(void) {
  uint8_t input_data[] = {0x60, 0x5A};
  uint8_t output_data[] = {0xAA, 0xAA};

  decrypt_data(input_data, 0, output_data, sizeof(output_data), 0x1337);

  if (output_data[0] != 0xAA || output_data[1] != 0xAA) {
    printf("ERROR: zero-length decrypt should not modify output\n");
    return 1;
  }

  return 0;
}


int main(void) {

  // Test the LFSR implementation
  int result = test_lfsr_step();
  if (result != 0) {
    printf("Error when testing LFSR implementation\n");
    return 1;
  }

  result = test_decrypt_even_length();
  if (result != 0) {
    printf("ERROR: test_decrypt_even_length failed\n");
    return 1;
  }

  result = test_decrypt_odd_length();
  if (result != 0) {
    printf("ERROR: test_decrypt_odd_length failed\n");
    return 1;
  }

  result = test_decrypt_truncated_output();
  if (result != 0) {
    printf("ERROR: test_decrypt_truncated_output failed\n");
    return 1;
  }

  result = test_decrypt_zero_length();
  if (result != 0) {
    printf("ERROR: test_decrypt_zero_length failed\n");
    return 1;
  }

  printf("All tests passed successfully!\n");
  return 0;
}
