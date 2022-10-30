#include "clist.h"
#include <assert.h>
#include <stddef.h>
#include <unity.h>

void setUp() {}

void tearDown() {}

struct calls {
  size_t len;
  size_t cap;
  void **items;
};

/* callbacks declarations ---------------------------------------------------*/
bool find_ptr(void *expected, void *item);
void register_call(void *calls, void *item);

/* test cases ---------------------------------------------------------------*/
void test_clist_sanity() {
  TEST_ASSERT(CLIST_EMPTY == NULL);
  TEST_ASSERT(CLIST_NO_ITEM == (void *)-1);
}

void test_clist_singleton() {
  int item = 42;
  clist_p list = clist_singleton(&item);

  TEST_ASSERT_MESSAGE(clist_item(list) == &item, "singleton item not matches");
  clist_clear(list);
}

void test_clist_is_empty() {
  int item = 42;
  clist_p list = clist_singleton(&item);

  TEST_ASSERT_MESSAGE(!clist_is_empty(list), "singleton should be empty");
  TEST_ASSERT_MESSAGE(clist_is_empty(CLIST_EMPTY),
                      "CLIST_EMPTY should be empty");
  clist_clear(list);
}

void test_clist_insert() {
  int items[] = {1, 2, 3, 4, 5};
  clist_p list = CLIST_EMPTY;
  for (int i = 0; i < 5; ++i) {
    list = clist_insert(list, &items[i]);
  }

  list = clist_next(list);

  clist_p node = list;
  int expected_index = 0;

  do {
    TEST_ASSERT_EQUAL_INT(&items[expected_index], clist_item(node));
    ++expected_index;
    node = clist_next(node);
  } while (node != list);

  clist_clear(list);
}

void test_clist_find() {
  int items[] = {0, 1, 2, 3, 4};
  clist_p list = CLIST_EMPTY;
  for (int i = 0; i < 5; ++i) {
    list = clist_insert(list, &items[i]);
  }

  TEST_ASSERT_EQUAL_PTR(&items[3],
                        clist_item(clist_find(list, &items[3], find_ptr)));
  TEST_ASSERT_EQUAL_PTR(CLIST_EMPTY, clist_find(list, NULL, find_ptr));
  TEST_ASSERT_EQUAL_PTR(CLIST_EMPTY, clist_find(list, NULL, NULL));
  TEST_ASSERT_EQUAL_PTR(CLIST_EMPTY, clist_find(CLIST_EMPTY, NULL, find_ptr));
  TEST_ASSERT_EQUAL_PTR(CLIST_EMPTY, clist_find(CLIST_EMPTY, NULL, NULL));

  clist_clear(list);
}

void test_clist_concat() {
  int items[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  clist_p list1 = CLIST_EMPTY;
  for (int i = 0; i < 5; ++i)
    list1 = clist_insert(list1, &items[i]);

  clist_p list2 = CLIST_EMPTY;
  for (int i = 5; i < 10; i++)
    list2 = clist_insert(list2, &items[i]);

  clist_p list = clist_concat(list1, clist_next(list2));

  list = clist_next(list);

  clist_p node = list;
  int expected_index = 0;

  do {
    TEST_ASSERT_EQUAL_INT(&items[expected_index], clist_item(node));
    ++expected_index;
    node = clist_next(node);
  } while (node != list);

  clist_clear(list);
}

void test_clist_unlink() {
  int items[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  clist_p list = CLIST_EMPTY;
  for (int i = 0; i < 10; ++i) {
    list = clist_insert(list, &items[i]);
  }

  list = clist_next(list);

  // unlink 2-3-4-5
  clist_p start = list;
  for (int i = 0; i < 2; ++i) {
    start = clist_next(start);
  }

  clist_p end = start;
  for (int i = 0; i < 4; ++i) {
    end = clist_next(end);
  }

  clist_p unlinked = clist_unlink(start, end);

  for (int i = 2; i < 6; ++i) {
    TEST_ASSERT_EQUAL(i, *(int *)clist_item(unlinked));
    unlinked = clist_next(unlinked);
  }

  TEST_ASSERT_EQUAL(2, *(int *)clist_item(unlinked));

  clist_clear(unlinked);

  for (int i = 0; i < 2; ++i) {
    TEST_ASSERT_EQUAL(i, *(int *)clist_item(list));
    list = clist_next(list);
  }

  for (int i = 6; i < 10; ++i) {
    TEST_ASSERT_EQUAL(i, *(int *)clist_item(list));
    list = clist_next(list);
  }

  TEST_ASSERT_EQUAL(0, *(int *)clist_item(list));

  clist_clear(list);
}

void test_clist_delete() {
  int items[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  clist_p list = CLIST_EMPTY;
  for (int i = 0; i < 10; ++i) {
    list = clist_insert(list, &items[i]);
  }

  list = clist_next(list);

  // delete the 2
  clist_p to_delete = list;
  for (int i = 0; i < 2; ++i) {
    to_delete = clist_next(to_delete);
  }

  void *deleted_items[1] = {NULL};
  struct calls calls = {
      .cap = sizeof(deleted_items) / sizeof(void *),
      .len = 0,
      .items = deleted_items,
  };

  clist_p prev = clist_delete_cb(to_delete, &calls, register_call);

  TEST_ASSERT(calls.len == 1);
  TEST_ASSERT_EQUAL_PTR(&items[2], deleted_items[0]);
  TEST_ASSERT_EQUAL_PTR(&items[1], clist_item(prev));
  calls.len = 0;

  clist_clear(list);

  // singleton
  list = clist_singleton(&items[0]);
  prev = clist_delete_cb(list, &calls, register_call);
  TEST_ASSERT(calls.len == 1);
  TEST_ASSERT_EQUAL_PTR(&items[0], deleted_items[0]);
  TEST_ASSERT_EQUAL_PTR(CLIST_EMPTY, prev);
  calls.len = 0;

  // empty
  list = CLIST_EMPTY;
  prev = clist_delete_cb(list, &calls, register_call);
  TEST_ASSERT(calls.len == 0);
  TEST_ASSERT_EQUAL_PTR(CLIST_EMPTY, prev);
}

void test_clist_clear() {
  int items[] = {0, 1};
  clist_p list = CLIST_EMPTY;
  for (int i = 0; i < 10; ++i) {
    list = clist_insert(list, &items[i]);
  }

  list = clist_next(list);

  void *deleted_items[2] = {NULL};
  struct calls calls = {
      .cap = sizeof(deleted_items) / sizeof(void *),
      .len = 0,
      .items = deleted_items,
  };

  list = clist_clear_cb(list, &calls, register_call);

  TEST_ASSERT(calls.len == 2);
  TEST_ASSERT_EQUAL_PTR(&items[0], deleted_items[0]);
  TEST_ASSERT_EQUAL_PTR(&items[1], deleted_items[1]);
  TEST_ASSERT_EQUAL_PTR(CLIST_EMPTY, list);
  calls.len = 0;

  list = clist_singleton(&items[0]);
  list = clist_clear_cb(list, &calls, register_call);
  TEST_ASSERT(calls.len == 1);
  TEST_ASSERT_EQUAL_PTR(&items[0], deleted_items[0]);
  TEST_ASSERT_EQUAL_PTR(CLIST_EMPTY, list);
  calls.len = 0;

  list = clist_clear_cb(list, &calls, register_call);
  TEST_ASSERT(calls.len == 0);
  TEST_ASSERT_EQUAL_PTR(CLIST_EMPTY, list);
  calls.len = 0;
}

/* callbacks definitions ----------------------------------------------------*/
bool find_ptr(void *expected, void *item) { return expected == item; }

void register_call(void *opaque, void *item) {
  struct calls *calls = (struct calls *)opaque;
  assert(calls->len < calls->cap);
  calls->items[calls->len] = item;
  calls->len++;
}

/* Main function ------------------------------------------------------------*/
int main() {
  UNITY_BEGIN();

  RUN_TEST(test_clist_sanity);
  RUN_TEST(test_clist_singleton);
  RUN_TEST(test_clist_is_empty);
  RUN_TEST(test_clist_insert);
  RUN_TEST(test_clist_find);
  RUN_TEST(test_clist_concat);
  RUN_TEST(test_clist_unlink);
  RUN_TEST(test_clist_delete);

  return UNITY_END();
}
