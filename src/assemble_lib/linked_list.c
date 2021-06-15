#include "linked_list.h"
#include "../global_helpers/definitions.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

linked_list *init_linked_list(void) {
  linked_list *list = malloc(sizeof(linked_list));
  PTR_CHECK(list, "Memory allocation failure\n");
  list->head = NULL;
  return list;
}

node *init_node(uint32_t address, void *val) {
  node *new_node = malloc(sizeof(node));
  PTR_CHECK(new_node, "Memory allocation failure\n");
  new_node->address = address;
  new_node->value = val;
  new_node->next = NULL;
  return new_node;
}

uint32_t append_via_node(node *entry, void *val) {

  assert(entry != NULL);
  assert(val);

  node *curr = entry;
  while (curr->next != NULL) {
    curr = curr->next;
  }

  uint32_t new_address = curr->address + WORD_SIZE_IN_BYTES;
  curr->next = init_node(new_address, val);

  return new_address;
}

uint32_t append_to_linked_list(linked_list *list, void *val) {
  if (list->head == NULL) {
    list->head = init_node(0, val);
    return 0;
  }

  return append_via_node(list->head, val);
}

node *traverse_linked_list(linked_list *list) {
  assert(list);

  node *curr;
  curr = list->head;

  while (curr != NULL) {
    curr = curr->next;
  }
  return curr;
}

void change_node(linked_list *list, uint32_t address, void *val) {
  assert(list);
  assert(address % WORD_SIZE_IN_BYTES == 0);

  node *node_to_change = traverse_linked_list(list);
  if (node_to_change != NULL) {
    node_to_change->value = val;
  }
}

void free_linked_list(linked_list *list, void (*value_free)(void *)) {
  assert(list);

  node *curr = list->head;
  while (curr) {
    node *temp = curr;
    curr = curr->next;
    value_free(temp->value);
    free(temp->value);
    free(temp);
  }
  free(list);
}