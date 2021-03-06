#include "../global_helpers/definitions.h"
#include "../global_helpers/types.h"
#include "emulate_utils.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

instruction_t *decode_dataproc(instruction_t *instr) {
  // the instruction to be decoded must be raw
  assert(instr->tag == RAW);

  uint32_t raw_data = instr->data.raw_data;

  instr->data.dataproc.cond = EXTRACT_BITS(raw_data, COND_POS, COND_SIZE);
  instr->data.dataproc.is_immediate = EXTRACT_BIT(raw_data, I_POS);
  instr->data.dataproc.opcode = EXTRACT_BITS(raw_data, OPCODE_POS, OPCODE_SIZE);
  instr->data.dataproc.set_cond = EXTRACT_BIT(raw_data, S_POS);
  instr->data.dataproc.rn = EXTRACT_BITS(raw_data, RN_POS, REG_SIZE);
  instr->data.dataproc.rd = EXTRACT_BITS(raw_data, RD_POS, REG_SIZE);
  instr->data.dataproc.op2 = EXTRACT_BITS(raw_data, OP2_POS, OP2_SIZE);

  instr->tag = DATAPROC;
  return instr;
}

instruction_t *decode_multiply(instruction_t *instr) {
  // the instruction to be decoded must be raw
  assert(instr->tag == RAW);

  uint32_t raw_data = instr->data.raw_data;

  instr->data.multiply.cond = EXTRACT_BITS(raw_data, COND_POS, COND_SIZE);
  instr->data.multiply.accumulate = EXTRACT_BIT(raw_data, A_POS);
  instr->data.multiply.set_cond = EXTRACT_BIT(raw_data, S_POS);
  instr->data.multiply.rd = EXTRACT_BITS(raw_data, MULTIPLY_RD_POS, REG_SIZE);
  instr->data.multiply.rn = EXTRACT_BITS(raw_data, MULTIPLY_RN_POS, REG_SIZE);
  instr->data.multiply.rs = EXTRACT_BITS(raw_data, RS_POS, REG_SIZE);
  instr->data.multiply.rm = EXTRACT_BITS(raw_data, RM_POS, REG_SIZE);

  instr->tag = MULTIPLY;
  return instr;
}

instruction_t *decode_sdt(instruction_t *instr) {
  // the instruction to be decoded must be raw
  assert(instr->tag == RAW);

  uint32_t raw_data = instr->data.raw_data;

  instr->data.sdt.cond = EXTRACT_BITS(raw_data, COND_POS, COND_SIZE);
  instr->data.sdt.is_shift_R = EXTRACT_BIT(raw_data, I_POS);
  instr->data.sdt.is_preindexed = EXTRACT_BIT(raw_data, P_POS);
  instr->data.sdt.up_bit = EXTRACT_BIT(raw_data, U_POS);
  instr->data.sdt.load = EXTRACT_BIT(raw_data, L_POS);
  instr->data.sdt.rn = EXTRACT_BITS(raw_data, RN_POS, REG_SIZE);
  instr->data.sdt.rd = EXTRACT_BITS(raw_data, RD_POS, REG_SIZE);
  instr->data.sdt.offset = EXTRACT_BITS(raw_data, OFFSET_POS, SDT_OFFSET_SIZE);

  instr->tag = SDT;
  return instr;
}

instruction_t *decode_branch(instruction_t *instr) {
  // the instruction to be decoded must be raw
  assert(instr->tag == RAW);

  uint32_t raw_data = instr->data.raw_data;

  instr->data.branch.cond = EXTRACT_BITS(raw_data, COND_POS, COND_SIZE);
  instr->data.branch.offset = EXTRACT_BITS(raw_data, OFFSET_POS, THREE_B);

  instr->tag = BRANCH;
  return instr;
}

instruction_t *decode(instruction_t *instr) {
  // the instruction to be decoded must be raw
  assert(instr->tag == RAW);

  uint32_t raw = instr->data.raw_data;

  // Halt
  if (raw == 0) {
    instr->tag = HALT;
    return instr;
  }

  // Multiply
  if (EXTRACT_BITS(raw, ZERO_POS, 6) == 0 && 
      EXTRACT_BITS(raw, NINE_POS, REG_SIZE) == 0x9) {
    return decode_multiply(instr);
  }

  switch (EXTRACT_BITS(raw, INSTR_FLAG, 2)) {
    case DP_FLAG: // Data Process
      return decode_dataproc(instr);
    case SDT_FLAG: // Single Data Transfer
      return decode_sdt(instr);
    case BRANCH_FLAG: // Branch
      return decode_branch(instr);
  }

  // Cannot decode the instruction
  fprintf(stderr, "0x%08x is a bad instruction. Halting emulator.", raw);
  instr->tag = HALT;
  return instr;
}
