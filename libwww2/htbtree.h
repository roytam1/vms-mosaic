/*
                BALANCED BINARY TREE FOR SORTING THINGS
                                             
   Tree creation, traversal and freeing.  User-supplied comparison routine.
   
   Author: Arthur Secret, CERN.  Public domain.
   
   part of libWWW
   
 */
#ifndef HTBTREE_H
#define HTBTREE_H

/*
 * Data structures
 */
typedef struct _HTBTree_element {
    void                        *object;        /* User object */
    struct _HTBTree_element     *up;
    struct _HTBTree_element     *left;
    int                          left_depth;
    struct _HTBTree_element     *right;
    int                          right_depth;
} HTBTElement;

typedef int (*HTComparer) (void *a, void *b);

typedef struct _HTBTree_top {
    HTComparer                  compare;
    struct _HTBTree_element    *top;
    int				count;
} HTBTree;

/*
 * Create a binary tree given its discrimination routine
 */
extern HTBTree *HTBTree_new(HTComparer comp);

/*
 * Free storage of the tree but not of the objects
 */
extern void HTBTree_free(HTBTree *tree);

/*
 * Free storage of the tree and of the objects
 */
extern void HTBTreeAndObject_free(HTBTree *tree);

/*
 * Return number of tree elements
 */
extern int HTBTree_count(HTBTree *tree);

/*
 * Add an object to a binary tree
 */
extern void HTBTree_add(HTBTree *tree, void *object);
/* Add only if not already in list */
extern void HTBTree_add_new(HTBTree *tree, void *object);

/*
 * Search for an object
 *
 *   returns          Pointer to equivalent object in a tree or NULL if none.
 */
extern void *HTBTree_search(HTBTree *tree, void *object);

/*
 * Delete the element of an object
 */
extern void HTBTree_delete(HTBTree *tree, void *object);

/*
 * Delete an element
 */
extern void HTBTree_ele_delete(HTBTree *tree, HTBTElement *ele);

/*
 * Get object of element
 */
#define HTBTree_object(element)  ((element)->object)

/*
 * Replace object of element
 */
#define HTBTree_set_object(element, obj)  ((element)->object = (void *)obj)

/*
 * Free object of element
 */
#define HTBTree_free_object(element)  free((char *)(element)->object)

/*
 * Find next element in depth-first order
 *
 *  ON ENTRY,
 *   ele                  if NULL, start with leftmost element.  If != 0 give
 *                        next object to the right.
 *                        
 * returns                Pointer to element or NULL if none left.
 *                       
 */
extern HTBTElement *HTBTree_next(HTBTree *tree, HTBTElement *ele);

#endif
