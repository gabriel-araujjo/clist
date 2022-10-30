#pragma once

#include <stdbool.h>
#include <stddef.h>

#define CLIST_VERSION_MAJOR 0
#define CLIST_VERSION_MINOR 0
#define CLIST_VERSION_PATCH 0

#define CLIST_EMPTY NULL
#define CLIST_NO_ITEM ((void *)-1)

struct clist_s;

typedef struct clist_s *clist_p;

typedef void clist_release_item_cb(void *u_data, void *item);
typedef bool clist_find_cb(void *u_data, void *item);

/**
 * @brief Return a new heap allocated singleton list, or nullptr if it cannot.
 *
 * @param item The item to be placed on the new list
 *
 * @return A list containing
 */
clist_p clist_singleton(void *item);

/**
 * @brief Clear the list and returns CLIST_EMPTY.
 *
 * @param list Any node of a list
 * @param u_data User data that will be passed for each cb
 * @param cb If not NULL, it will be called for each node item, before the node
 * is released;
 *
 * @return CLIST_EMPTY
 */
clist_p clist_clear_cb(clist_p list, void *u_data, clist_release_item_cb cb);

#define clist_clear(list) clist_clear_cb((list), NULL, NULL)

/**
 * @brief Returns whether a list is empty.
 *
 * @param list Some list
 */
#define clist_is_empty(list) ((list) == CLIST_EMPTY)

/**
 * @brief Retrieves the value stored inside `node`.
 *
 * If node is empty, returns `CLIST_NO_ITEM`
 *
 * @param node
 */
void *clist_item(const clist_p node);

/**
 * @brief Iterate over the list and returns the first item where clist_find_cb
 * returns true, or CLIST_EMPTY;
 *
 *
 * @param item Any Heap allocated value or nullptr
 * @return clist_p
 */
clist_p clist_find(const clist_p list, void *u_data, clist_find_cb);

/**
 * @brief Place `list` just after `node`.
 *
 * If `list` is inside `node`, it's an UB.
 *
 * If `node` is an empty list, returns `list` last node.
 * If `list` is empty, returns `node`.
 * If both `node` and `list` are empty, returns CLIST_EMPTY;
 *
 * The caller must call `clist_clear_cb()` on the returned list.
 *
 * It's an error to clear `node` or `list` after call this function.
 *
 * @param node Node just before the place where `list` will be inserted
 * @param list A detached list to be inserted just after node;
 * @return `list` last node. If `list` is empty, returns `node`
 */
clist_p clist_concat(clist_p node, clist_p list);

/**
 * @brief Unlinks a list of nodes from a list
 *
 * It's an error if either start or end are empty, or do not belong to the same
 * list
 *
 * @param start The first node to unlink (inclusive)
 * @param end The node just after the last one that should be unlinked
 * @return The removed nodes as a list, pointing to the first removed node
 */
clist_p clist_unlink(clist_p start, clist_p end);

/**
 * @brief Insert `item` just after `node` and returns a pointer to the just
 * inserted node.
 *
 *
 * @param item Any heap allocated value or `nullptr`
 * @return clist_p
 */
#define clist_insert(node, item) clist_concat((node), clist_singleton((item)))

/**
 * @brief Deletes `node` from list and returns the previous one
 *
 * If `node` is singleton, an empty list will be returned
 *
 * @param node
 * @return clist_p
 */
clist_p clist_delete_cb(clist_p node, void *u_data, clist_release_item_cb cb);
#define clist_delete(node) clist_delete_cb((node), NULL, NULL)

/**
 * @brief Get the next node
 *
 * It's only valid if node is not empty
 *
 * # Example
 * assert(!clist_is_empty(list));
 *
 * clist_p node = list;
 * do {
 *  printf("item = %s", clist_item(node));
 *  node = clist_next(node);
 * } while(node != list);
 *
 * @param list
 * @param node
 * @return true
 * @return false
 */
const clist_p clist_next(const clist_p node);
