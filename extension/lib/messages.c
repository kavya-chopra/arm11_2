#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <binn.h>
#include <time.h>
#include <pthread.h>

#include "linked_list.h"
#include "transaction.h"
#include "block.h"
#include "blockchain.h"
#include "messages.h"
#include "utils.h"

binn *serialize_bc_msg(blockchain_msg *bc_msg) {
  binn *obj = binn_object();
  binn *bc = serialize_blockchain(bc_msg->bc);
  binn_object_set_object(obj, "blockchain", bc);
  binn_free(bc);
  binn_object_set_str(obj, "username", bc_msg->username);
  binn_object_set_str(obj, "type", bc_msg->type);

  return obj;
}

blockchain_msg *deserialize_bc_msg(binn *input) {
  blockchain_msg *bc_msg = calloc(1, sizeof(blockchain_msg));
  bc_msg->bc = deserialize_blockchain(binn_object_object(input, "blockchain"));
  strncpy(bc_msg->username, binn_object_str(input, "username"), UID_LENGTH);
  strncpy(bc_msg->type, binn_object_str(input, "type"), MESSAGE_TYPE_SIZE);
  return bc_msg;
}

transaction_msg *init_transaction_msg(uint64 amount,
const char *username, char *to) {
  transaction_msg *new = calloc(1, sizeof(transaction_msg));
  new->amount = amount;
  new->timestamp = time(NULL);
  strncpy(new->username, username, UID_LENGTH);
  strncpy(new->to, to, UID_LENGTH);
  strncpy(new->type, "trans", MESSAGE_TYPE_SIZE);

  return new;
}

binn *serialize_t_msg(transaction_msg *t_msg) {
  binn *obj = binn_object();

  binn_object_set_uint64(obj, "amount", t_msg->amount);
  binn_object_set_uint64(obj, "timestamp", t_msg->timestamp);
  binn_object_set_str(obj, "username", t_msg->username);
  binn_object_set_str(obj, "type", t_msg->type);
  binn_object_set_str(obj, "to", t_msg->to);

  return obj;
}

transaction_msg *deserialize_t_msg(binn *input) {
  transaction_msg *t_msg = calloc(1, sizeof(transaction_msg));
  strncpy(t_msg->username, binn_object_str(input, "username"), UID_LENGTH);
  strncpy(t_msg->type, binn_object_str(input, "type"), MESSAGE_TYPE_SIZE);
  strncpy(t_msg->to, binn_object_str(input, "to"), UID_LENGTH);
  t_msg->amount = binn_object_uint64(input, "amount");
  t_msg->timestamp = binn_object_uint64(input, "timestamp");

  return t_msg;
}

transaction *to_transaction(transaction_msg *tc_msg) {
  transaction *tc = init_transaction(tc_msg->username, tc_msg->to,
                                     tc_msg->amount, tc_msg->timestamp);
  return tc;
}
