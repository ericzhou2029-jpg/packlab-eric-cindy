// Utilities for unpacking files
// PackLab - CS213 - Northwestern University

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unpack-utilities.h"


// --- public functions ---

void error_and_exit(const char* message) {
  fprintf(stderr, "%s", message);
  exit(1);
}

void* malloc_and_check(size_t size) {
  void* pointer = malloc(size);
  if (pointer == NULL) {
    error_and_exit("ERROR: malloc failed\n");
  }
  return pointer;
}

void parse_header(uint8_t* input_data, size_t input_len, packlab_config_t* config) {
  // function takes in raw header data
  // outputs the config struct

  // init config variables
  size_t offset = 0;
  uint8_t flags = 0;
  uint64_t orig_data_size = 0;
  uint64_t data_size = 0;

  // check null
  if (config == NULL) {
    return;
  }

  config->is_valid = false;

  // basic checks
  if (input_data == NULL) {
    return;
  }

  if (input_len < 4) {
    return;
  }

  // check if the magic and version are correct (first 2 magic, second version)
  if (input_data[0] != 0x02 || input_data[1] != 0x13 || input_data[2] != 0x03) {
    return;
  }

  // obtain flags
  flags = input_data[3];

  // check if bits 0 and 1 are unused (little endian)
  if ((flags & 0x03) != 0) {
    return;
  }

  // obtain all flags
  config->is_compressed   = (flags & 0x80) != 0;
  config->is_encrypted    = (flags & 0x40) != 0;
  config->is_checksummed  = (flags & 0x20) != 0;
  config->should_continue = (flags & 0x10) != 0;
  config->should_float    = (flags & 0x08) != 0;
  config->should_float3   = (flags & 0x04) != 0;

  // set offset (think of it like a cursor in memory)
  offset = 4;

  // if offset already wrote over
  if (input_len < offset + 8) {
    return;
  }

  // rebuild original stream length (little endian)
  for (size_t i = 0; i < 8; i++) {
    orig_data_size |= ((uint64_t)input_data[offset + i]) << (8 * i);
  }
  config->orig_data_size = orig_data_size;
  offset += 8;

  if (input_len < offset + 8) {
    return;
  }

  // rebuild compressedsteram length (little endian)
  for (size_t i = 0; i < 8; i++) {
    data_size |= ((uint64_t)input_data[offset + i]) << (8 * i);
  }
  config->data_size = data_size;
  offset += 8;

  if (config->is_compressed) {
    // check if there are 16 more bytes
    if (input_len < offset + DICTIONARY_LENGTH) {
      return;
    }
    // copy byes into config strucut
    memcpy(config->dictionary_data, &input_data[offset], DICTIONARY_LENGTH);
    offset += DICTIONARY_LENGTH;
  }

  if (config->is_checksummed) {
    // check if enough for check sum
    if (input_len < offset + 2) {
      return;
    }
    // check sum is big endian
    config->checksum_value = (((uint16_t)input_data[offset]) << 8)
                             | ((uint16_t)input_data[offset + 1]);
    offset += 2;
  }

  if (offset > MAX_HEADER_SIZE) {
    return;
  }

  config->header_len = offset;
  config->is_valid = true;

}

uint16_t calculate_checksum(uint8_t* input_data, size_t input_len) {

  // TODO
  // Calculate a checksum over input_data
  // Return the checksum value

  uint16_t checksum = 0;

  for (size_t i = 0; i < input_len; i++) {
    checksum += input_data[i];
  }

  return checksum;
}

uint16_t lfsr_step(uint16_t oldstate) {

  // TODO
  // Calculate the new LFSR state given previous state
  // Return the new LFSR state

  return 0;
}

void decrypt_data(uint8_t* input_data, size_t input_len,
                  uint8_t* output_data, size_t output_len,
                  uint16_t encryption_key) {

  // TODO
  // Decrypt input_data and write result to output_data
  // Uses lfsr_step() to calculate psuedorandom numbers, initialized with encryption_key
  // Step the LFSR once before encrypting data
  // Apply psuedorandom number with an XOR in little-endian order
  // Beware: input_data may be an odd number of bytes
  

}

size_t decompress_data(uint8_t* input_data, size_t input_len,
                       uint8_t* output_data, size_t output_len,
                       uint8_t* dictionary_data) {

  // TODO
  // Decompress input_data and write result to output_data
  // Return the length of the decompressed data
  size_t in = 0;
  size_t out = 0;

  while (in < input_len)
  {
    uint8_t tempd = input_data[in];
    if (tempd != ESCAPE_BYTE || in == input_len - 1)
    {
      if (out < output_len)
      {
        output_data[out] = tempd;
      }
      out++;
      in++;
      continue;
    }
  

    uint8_t code = input_data[in + 1];

    if (code == 0x00)
    {
      if (out < output_len)
      {
        output_data[out] = ESCAPE_BYTE;
      }
      out+=2;

    }
    else
    {
      uint8_t rl = code >> 4;
      uint8_t dic_ind = code & 0x0F;
      uint8_t value = dictionary_data[dic_ind];

      for (uint8_t i = 0; i < rl; i++)
      {
        if (out < output_len)
        {
          output_data[out] = value;
        }
        out++;
      }
    }
  }

  return out;
}

void join_float_array(uint8_t* input_signfrac, size_t input_len_bytes_signfrac,
                      uint8_t* input_exp, size_t input_len_bytes_exp,
                      uint8_t* output_data, size_t output_len_bytes) {

  // TODO
  // Combine two streams of bytes, one with signfrac data and one with exp data,
  // into one output stream of floating point data
  // Output bytes are in little-endian order
  size_t num_floats = input_len_bytes_exp;

  for (size_t i = 0; i < num_floats; i++) {
    size_t sf_index = 3 * i;
    size_t out_index = 4 * i;

    uint8_t sf0 = input_signfrac[sf_index];
    uint8_t sf1 = input_signfrac[sf_index + 1];
    uint8_t sf2 = input_signfrac[sf_index + 2];
    uint8_t exp = input_exp[i];

    if (out_index + 3 < output_len_bytes) {
      output_data[out_index] = sf0;
      output_data[out_index + 1] = sf1;
      output_data[out_index + 2] = (sf2 & 0x7F) | ((exp & 0x01) << 7);
      output_data[out_index + 3] = (sf2 & 0x80) | (exp >> 1);
    }
  }


}
/* End of mandatory implementation. */

/* Extra credit */
void join_float_array_three_stream(uint8_t* input_frac,
                                   size_t   input_len_bytes_frac,
                                   uint8_t* input_exp,
                                   size_t   input_len_bytes_exp,
                                   uint8_t* input_sign,
                                   size_t   input_len_bytes_sign,
                                   uint8_t* output_data,
                                   size_t   output_len_bytes) {

  // TODO
  // Combine three streams of bytes, one with frac data, one with exp data,
  // and one with sign data, into one output stream of floating point data
  // Output bytes are in little-endian order

}
