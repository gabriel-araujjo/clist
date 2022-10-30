#include "clist.h"

#include <stdalign.h>
#include <stdlib.h>

/**
 * @brief clist_s is a ring.
 *
 * An empty list has item equals to CLIST_NO_ITEM
 *
 */
struct clist_s {
  void *item;
  clist_p prev;
  clist_p next;
};

clist_p clist_singleton(void *item) {
  clist_p list = aligned_alloc(alignof(struct clist_s), sizeof(struct clist_s));
  list->prev = list;
  list->next = list;
  list->item = item;
  return list;
}

/**
 * @brief Check whether a non empty list is a singleton
 *
 * @param non_empty
 * @return true if it has only one element
 * @return false otherwise
 */
bool clist_is_singleton(const clist_p non_empty) {
  return non_empty->next == non_empty;
}

/**
 * @brief Clear sublist from start (inclusive) to end (exclusively).
 *
 * if `start == end`, it will not affect the list
 *
 * @param start First item to exclude (inclusive)
 * @param end No item before end will be excluded
 * @param cb
 */
void clist_clear_sublist(const clist_p start, const clist_p end, void *u_data,
                         clist_release_item_cb cb) {
  clist_p prev = start->prev;
  clist_p node = start;

  while (node != end) {
    clist_p next = node->next;
    if (cb)
      cb(u_data, node->item);
    free(node);

    node = next;
  }

  prev->next = end;
  end->prev = prev;
}

clist_p clist_clear_cb(clist_p list, void *u_data, clist_release_item_cb cb) {
  if (clist_is_empty(list))
    return CLIST_EMPTY;

  const clist_p last = list->prev;
  clist_clear_sublist(list, last, u_data, cb);

  if (cb)
    cb(u_data, last->item);

  free(last);

  return CLIST_EMPTY;
}

void *clist_item(const clist_p node) {
  if (clist_is_empty(node))
    return CLIST_NO_ITEM;
  return node->item;
}

clist_p clist_concat(clist_p node, clist_p list) {
  if (clist_is_empty(list)) {
    return node;
  }

  if (clist_is_empty(node)) {
    return list->prev;
  }

  clist_p next = node->next;
  clist_p last = list->prev;

  node->next = list;
  list->prev = node;

  last->next = next;
  next->prev = last;

  return last;
}

clist_p clist_unlink(clist_p start, clist_p end) {
  clist_p prev = start->prev;
  clist_p last = end->prev;

  prev->next = end;
  end->prev = prev;

  start->prev = last;
  last->next = start;

  return start;
}

clist_p clist_delete_cb(clist_p node, void *u_data, clist_release_item_cb cb) {
  if (clist_is_empty(node))
    return CLIST_EMPTY;

  if (cb)
    cb(u_data, node->item);

  if (clist_is_singleton(node)) {
    free(node);
    return CLIST_EMPTY;
  }

  clist_p prev = node->prev;
  clist_p next = node->next;

  prev->next = next;
  next->prev = prev;

  free(node);

  return prev;
}

clist_p clist_find(const clist_p list, void *u_data, clist_find_cb cb) {
  if (clist_is_empty(list) || !cb)
    return CLIST_EMPTY;

  clist_p node = list;
  do {
    if (cb(u_data, node->item))
      return node;
    node = node->next;
  } while (node != list);

  return CLIST_EMPTY;
}

const clist_p clist_next(const clist_p node) { return node->next; }
