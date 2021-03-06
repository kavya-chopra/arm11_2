#include "../global_helpers/definitions.h"
#include "../global_helpers/types.h"
#include "alu.h"
#include "emulate_utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <stdbool.h>

/*------------------------------------------------------------
 * Arithmetic and Logical Data Processing Functions.
 * */

int32_t and_tst(int32_t op1, int32_t op2, int *carry_out) {
  return op1 & op2;
}

int32_t eor_teq(int32_t op1, int32_t op2, int *carry_out) {
  return op1 ^ op2; 
}

int32_t sub(int32_t op1, int32_t op2, int *carry_out) {
  *carry_out = !overflow(op1, twos_comp(op2));
  return op1 - op2;
}

int32_t rsb(int32_t op1, int32_t op2, int *carry_out) {
  *carry_out = !overflow(op2, twos_comp(op1));
  return op2 - op1;
}

int32_t add(int32_t op1, int32_t op2, int *carry_out) {
  *carry_out = overflow(op1, op2);
  return op1 + op2;
}

int32_t cmp(int32_t op1, int32_t op2, int *carry_out) {
  *carry_out = !(op1 < op2);
  return op1 - op2;
}

int32_t orr(int32_t op1, int32_t op2, int *carry_out) {
  return op1 | op2; 
}

int32_t mov(int32_t op1, int32_t op2, int *carry_out) {
  return op2; 
}

/*------------------------------------------------------------
 * Shift and Rotate Functions
 * */

uint32_t lsl(uint32_t to_shift, uint8_t shift_amt, int *carry_out) {
  *carry_out = EXTRACT_BIT(to_shift, (INSTR_SIZE - shift_amt + 1));
  return to_shift << shift_amt;
}

uint32_t lsr(uint32_t to_shift, uint8_t shift_amt, int *carry_out) {
  *carry_out = EXTRACT_BIT(to_shift, (shift_amt - 1));
  return to_shift >> shift_amt;
}

uint32_t asr(uint32_t to_shift, uint8_t shift_amt, int *carry_out) {
  *carry_out = EXTRACT_BIT(to_shift, (shift_amt - 1));
  uint32_t result = to_shift >> shift_amt;
  uint8_t mask = (int32_t)result < 0 ? 1 << (shift_amt - 1) : 0;
  return result |= (mask << (INSTR_SIZE - shift_amt));
}

uint32_t ror(uint32_t to_rotate, uint8_t rotate_amt, int *carry_out) {
  *carry_out = EXTRACT_BIT(to_rotate, (rotate_amt - 1));
  return (to_rotate >> rotate_amt) | (to_rotate << (INSTR_SIZE - rotate_amt));
}


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

int32_t signed_24_to_32(uint32_t num) {
  if (num >> TWENTY_FOUR_MSB) {
    return (int32_t) (TOP_EIGHT_BITMASK | num);
  }
  return (int32_t) num;
}

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

bool discards_result(int val) { return val == TST || val == TEQ || val == CMP; }

uint32_t barrel_shifter(bool is_immediate, uint16_t offset,
                        uint32_t *register_file, int *carry_out) {

  uint32_t to_shift = 0;
  uint8_t shift_amt = 0;
  uint8_t shift_type = 0;
  if (is_immediate) {
    to_shift = EXTRACT_BITS(offset, OP2_POS, IMM_VALUE_SIZE);
    shift_amt =
        SHIFT_MULTIPLIER * EXTRACT_BITS(offset, IMM_VALUE_SIZE, REG_SIZE);

    shift_type = ROR;
  } else {
    to_shift = register_file[EXTRACT_BITS(offset, OP2_POS, REG_SIZE)];
    uint8_t shift = EXTRACT_BITS(offset, REG_SIZE, IMM_VALUE_SIZE);

    if ((shift & LSR) == 0) {
      shift_amt = EXTRACT_BITS(offset, SHIFT_VAL, SHIFT_VAL_SIZE);
    } else {
      // Shift amount is the bottom byte of the register specified
      shift_amt =
          register_file[EXTRACT_BITS(offset, RS_POS, REG_SIZE)] & MASK(8);
    }

    shift_type = EXTRACT_BITS(offset, SHIFT_TYPE_POS, SHIFT_TYPE_SIZE);
  }

  if (shift_amt == 0) {
    return to_shift;
  }

  assert(shift_amt > 0);
  const shift_operation shifts[] = {lsl, lsr, asr, ror};

  assert(shift_type >= LSL && shift_type <= ROR);
  return shifts[shift_type](to_shift, shift_amt, carry_out);
}
