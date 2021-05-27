#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include "definitions.h"
#include "emulate.h"
#include "emulate_utils.h"
#include "exec_utils.h"
#include "decoder.h"

bool satisfies_cpsr(uint8_t cond, uint32_t regs[NUM_REGS]) {
  uint32_t cpsr_register = regs[CPSR];
  bool n = EXTRACT_BIT(cpsr_register, N_FLAG);
  bool z = EXTRACT_BIT(cpsr_register, Z_FLAG);
  bool v = EXTRACT_BIT(cpsr_register, V_FLAG);

  switch (cond) {
    case EQ :
      return z;
    case NE :
      return !z;
    case GE :
      return n == v;
    case LT :
      return n != v;
    case GT :
      return !z && (n == v);
    case LE :
      return z | (n != v);
    case AL :
      return true;
    default:
      return false;
  }
}

bool overflow(int32_t a, int32_t b) {
  if ((a > 0) ^ (b > 0)) {
    // if the signs are different
    return ((a > 0) && (a > (INT_MAX + b))) || ((a < 0) && (a < (INT_MIN + b)));
  } else {
    // if the signs are the same
    return ((a > 0) && (a > (INT_MAX - b))) || ((a < 0) && (a < (INT_MIN - b)));
  }
}
int32_t twos_comp(int32_t x) {
  return ~x + 1;
}

/**
 * @brief Sign-extends a signed number to 32 bits
 *
 * @param num Input
 *
 * @return 32 bit extended version of num
 */

int32_t signed_24_to_32(uint32_t num) {
  if (num >> 23) {
    return - (~(0xFF000000 + num));
  }
  return (int32_t) num;
}

/**
 * @brief Sets corresponding flag in the CPSR register.
 *
 * @param reg_file The current register state of the ARM11 system.
 * @param set Either 0 or 1
 * @param flag The flag the user wants to change.
 */

void set_flag(uint32_t *reg_file, bool set, int flag) {
  if (set) {
    reg_file[CPSR] |= 1 << flag;
  } else {
    reg_file[CPSR] &= ~(1 << flag);
  }
}

bool is_logic(int val) {
  return
    (val == AND
    || val == EOR
    || val == TST
    || val == TEQ
    || val == ORR
    || val == MOV);
}

bool is_arithmetic(int val) {
  return
    (val == SUB
    || val == RSB
    || val == ADD
    || val == CMP);
}

uint32_t rotate_right(uint32_t to_rotate, uint8_t rotate_amt) {
  return (to_rotate >> rotate_amt) | (to_rotate << (INSTR_SIZE - rotate_amt));
}

uint32_t barrel_shifter(bool is_immediate, uint16_t offset,
                          uint32_t *register_file, int *carry_out) {

  uint32_t to_shift = 0;
  uint32_t result = 0;
  uint8_t shift_type = 0;
  uint8_t shift_amt = 0;

  if (is_immediate) {
    to_shift = EXTRACT_BITS(offset, 0, 8);
    shift_amt = 2 * EXTRACT_BITS(offset, 8, 4);
    result = rotate_right(to_shift, shift_amt);
  } else {

    uint32_t to_shift = register_file[EXTRACT_BITS(offset, 0, 4)];
    uint8_t shift = EXTRACT_BITS(offset, 4, 8);

    if ((shift & 0x1) == 0) {
      shift_amt = EXTRACT_BITS(shift, 3, 5);
    } else {
      // Shift amount is the bottom byte of the register specified
      shift_amt = register_file[EXTRACT_BITS(shift, 4, 4)] & 0xf;
    }

    shift_type = EXTRACT_BITS(shift, 1, 2);

    switch (shift_type) {
    case LSL:
      result = to_shift << shift_amt;
      break;
    case ASR:
      result = to_shift >> shift_amt;
      bool top_bit = EXTRACT_BIT(to_shift, (INSTR_SIZE - 1));
      uint8_t mask =  top_bit == 1 ? top_bit << (shift_amt - 1) : 0;
      result |= (mask << (INSTR_SIZE - shift_amt));
      break;
    case LSR:
      result = to_shift >> shift_amt;
      break;
    case ROR:
      result = rotate_right(to_shift, shift_amt);
      break;
    }
  }

  // find carry
  if (shift_amt > 0) {
    if (shift_type == LSL) {
      *carry_out = EXTRACT_BIT(to_shift, (INSTR_SIZE - shift_amt + 1));
    } else {
      *carry_out = EXTRACT_BIT(to_shift, (shift_amt - 1));
    }
  }

  return result;
}
