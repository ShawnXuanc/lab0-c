#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));

    if (head) {
        INIT_LIST_HEAD(head);
    }


    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    element_t *free_node;
    element_t *next_node;
    list_for_each_entry_safe (free_node, next_node, l, list) {
        free(free_node->value);
        free(free_node);
    }

    free(l);
    return;
}

bool new_element(element_t **node, char *s)
{
    int len = strlen(s) + 1;
    (*node) = malloc(sizeof(element_t));
    char *tmp_s = malloc(len);
    if (!(*node) || !tmp_s) {
        free(*node);
        free(tmp_s);
        return true;
    }
    strncpy(tmp_s, s, len);
    (*node)->value = tmp_s;

    INIT_LIST_HEAD(&(*node)->list);
    return false;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *node;
    if (new_element(&node, s))
        return false;

    list_add(&node->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *node;
    if (new_element(&node, s))
        return false;

    list_add_tail(&node->list, head);
    return true;
}

element_t *q_remove_list(struct list_head *head,
                         struct list_head *list_dir,
                         char *sp,
                         size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    struct list_head *remove_list = list_dir;
    element_t *tmp = list_entry(remove_list, element_t, list);
    if (sp) {
        strncpy(sp, tmp->value, bufsize);
        sp[bufsize - 1] = '\0';
    }
    list_del(remove_list);
    return tmp;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    return q_remove_list(head, head->next, sp, bufsize);
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    return q_remove_list(head, head->prev, sp, bufsize);
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    struct list_head *slow = head->next, *fast = head->next;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }
    element_t *del_node = list_entry(slow, element_t, list);
    list_del(slow);
    free(del_node->value);
    free(del_node);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return false;
    element_t *node, *safe, *tmp = NULL;
    list_for_each_entry_safe (node, safe, head, list) {
        if (&safe->list != head && !strcmp(node->value, safe->value)) {
            list_del(&node->list);
            q_release_element(node);
            tmp = safe;
        } else {
            if (tmp) {
                list_del(&tmp->list);
                q_release_element(tmp);
            }
            tmp = NULL;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    for (struct list_head *n1 = head->next, *pre = head;
         n1 != head && n1->next != head; n1 = pre->next) {
        struct list_head *n2 = n1->next;
        n2->next->prev = n1;
        n1->next = n2->next;
        n2->next = n1;
        n2->prev = n1->prev;
        n1->prev = n2;
        pre->next = n2;
        pre = n1;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *cur, *safe;
    list_for_each_safe (cur, safe, head) {
        list_move(cur, head);
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    int count = 0;
    struct list_head *node, *safe, *move = head, *record = head->next;
    list_for_each_safe (node, safe, head) {
        count += 1;
        list_move(node, move);
        if (count == k) {
            move = record;
            record = record->next;
            count = 0;
        }
    }
}

int cmp(const char *a, const char *b, bool descend)
{
    if (descend)
        return strcmp(b, a);
    else
        return strcmp(a, b);
}

struct list_head *merge(struct list_head *l1,
                        struct list_head *l2,
                        bool descend)
{
    LIST_HEAD(list);
    INIT_LIST_HEAD(&list);
    struct list_head *tmp = &list;
    while (l1 && l2) {
        element_t *e1 = list_entry(l1, element_t, list),
                  *e2 = list_entry(l2, element_t, list);
        if (cmp(e1->value, e2->value, descend) < 0) {
            tmp->next = l1;
            l1 = l1->next;
        } else {
            tmp->next = l2;
            l2 = l2->next;
        }
        tmp = tmp->next;
    }
    tmp->next = l1 ? l1 : l2;
    return list.next;
}

struct list_head *q_mergeSort(struct list_head *head, bool descend)
{
    if (!head || !head->next)
        return head;


    struct list_head *slow = head, *fast = head->next, *right;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    right = slow->next;
    slow->next = NULL;
    head = q_mergeSort(head, descend);
    right = q_mergeSort(right, descend);
    return merge(head, right, descend);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return;

    struct list_head *list = head->next, *pre, *node;
    head->prev->next = NULL;
    head->next = q_mergeSort(list, descend);

    for (pre = head, node = head->next; node->next != NULL;
         pre = node, node = node->next) {
        node->prev = pre;
    }
    node->next = head;
    head->prev = node;
}


int q_remove(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return q_size(head);
    int count = 1;
    struct list_head *cur = head->prev, *move = cur->prev, *safe;
    for (safe = move->prev; move != head; move = safe, safe = move->prev) {
        element_t *cur_ele = list_entry(cur, element_t, list);
        element_t *move_ele = list_entry(move, element_t, list);
        if (cmp(cur_ele->value, move_ele->value, descend) < 0) {
            list_del(move);
            q_release_element(move_ele);
        } else {
            cur = move;
            count += 1;
        }
    }
    return count;
}
/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    return q_remove(head, false);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    return q_remove(head, true);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;
    int size = 0;
    struct list_head *pre, *node, *mrg_q = NULL, *move = head->next;
    while (move != head) {
        queue_contex_t *tmp = list_entry(move, queue_contex_t, chain);
        size += tmp->size;
        tmp->q->prev->next = NULL;
        mrg_q = merge(mrg_q, tmp->q->next, descend);
        move = move->next;
        INIT_LIST_HEAD(tmp->q);
    }
    LIST_HEAD(list);
    list.next = mrg_q;
    for (pre = &list, node = list.next; node->next != NULL;
         pre = node, node = node->next) {
        node->prev = pre;
    }
    node->next = &list;
    list.prev = node;
    list_splice(&list, list_first_entry(head, queue_contex_t, chain)->q);
    return size;
}

void swap(struct list_head *n1, struct list_head *n2)
{
    struct list_head *pre, *pn;
    pre = n1->prev, pn = n2->next;
    list_move(n2, pre);
    list_move_tail(n1, pn);
}

void q_shuffle(struct list_head *head)
{
    if (!head || list_is_singular(head))
        return;

    int len = q_size(head), i, j;
    struct list_head *move = head->prev, *tmp = NULL;
    for (i = 0; i < len; ++i, move = move->prev) {
        int rand_n = rand() % (len - i);
        if (rand_n == len - i - 1)
            continue;
        for (j = 0, tmp = head->next; j < rand_n; ++j) {
            tmp = tmp->next;
        }
        swap(tmp, move);
        move = tmp;
    }
}