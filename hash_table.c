/*
 * In this file, you will write the structures and functions needed to
 * implement a hash table.  Feel free to implement any helper functions
 * you need in this file to implement a hash table.  Make sure to add your
 * name and @oregonstate.edu email address below:
 *
 * Name: Kaden Fugate
 * Email: fugateka@oregonstate.edu
 */

#include <stdlib.h>

#include "dynarray.h"
#include "list.h"
#include "hash_table.h"

/*
 * This is the structure that represents a hash table.  You must define
 * this struct to contain the data needed to implement a hash table.
 */
struct ht
{

    struct dynarray *array;
    int size;

};

/*
 * this structure will hold the a key and its corresponding value
 */
struct ht_node
{

    void *key;
    void *val;

};

/*
 * this function is meant to convert a node to a key. it is created
 * so you can easily obtain a key from a void * in functions in other files.
 *
 *  Params:
 *      node - node to be converted
 */
void* ntk(void *node)
{
    struct ht_node *ht_node = node;
    return (ht_node->key);

}

/*
 * this function is meant to swap a nodes value easily in different
 * files.
 *
 *  Params:
 *      a - node whose value will be swapped
 *      b - value being input into node
 */
void swap(void *a, void *b)
{

    struct ht_node *ht_node = a;
    ht_node->val = b;

}

/*
 * this function is mean to compare to given void values
 *
 *  Params:
 *      a - first to compare
 *      b - second to compare
 */
int compare(void *a, void *b)
{

    return *((int*) a) - *((int*) b);

}

/*
 * This function should allocate and initialize an empty hash table and
 * return a pointer to it.
 */
struct ht* ht_create()
{
    
    struct ht *ht = malloc ( sizeof( struct ht ) );
    ht->array = dynarray_create();
    ht->size = 0;
    set_null(ht->array); // init array to NULL *'s

    return ht;
}

/*
 * This function should free the memory allocated to a given hash table.
 * Note that this function SHOULD NOT free the individual elements stored in
 * the hash table.  That is the responsibility of the caller.
 *
 * Params:
 *   ht - the hash table to be destroyed.  May not be NULL.
 */
void ht_free(struct ht* ht)
{
    
    for(int i = 0; i < dynarray_cap(ht->array); i++)
    {

        if(dynarray_get(ht->array, i)){list_free( (struct list *) dynarray_get(ht->array, i) );}

    }
    dynarray_free(ht->array);
    free(ht);

}

/*
 * This function should return 1 if the specified hash table is empty and
 * 0 otherwise.
 *
 * Params:
 *   ht - the hash table whose emptiness is to be checked.  May not be
 *     NULL.
 *
 * Return:
 *   Should return 1 if ht is empty and 0 otherwise.
 */
int ht_isempty(struct ht* ht)
{
    
    if( ht->size ){ return 0; }

    return 1;

}


/*
 * This function returns the size of a given hash table (i.e. the number of
 * elements stored in it, not the capacity).
 */
int ht_size(struct ht* ht)
{

    return ht->size;
}


/*
 * This function takes a key, maps it to an integer index value in the hash table,
 * and returns it. The hash algorithm is totally up to you. Make sure to consider
 * Determinism, Uniformity, and Speed when design the hash algorithm
 *
 * Params:
 *   ht - the hash table into which to store the element.  May not be NULL.
 *   key - the key of the element to be stored
 *   convert - pointer to a function that can be passed the void* key from
 *     to convert it to a unique integer hashcode
 */
int ht_hash_func(struct ht* ht, void* key, int (*convert)(void*)) // hash function: f(x) = ((x - 2) / 3) * ((x + 7) / 6)
{
    
    int hash = ( convert(key) - 2 ) / 3;

    hash *= ( ( hash + 7 ) / 6 );

    return hash % dynarray_cap( ht->array );

}

/*
 *  this function is a version of the ht_hash_func that takes an array 
 *  instead of an ht. stupid and only used in one occasion. exact same
 *  as ht_hash_func 
 *
 *  Params:
 *      array - array to get capacity from
 *      key - key to be hashed
 *      convert - function to convert void key to int
 */
int rehash(struct dynarray *array, void *key, int (*convert)(void *a))
{

    int hash = ( convert(key) - 2 ) / 3;

    hash *= ( ( hash + 7 ) / 6 );

    return hash % dynarray_cap( array );

}

/*
 *  this function is meant to resize and rehash a ht when the load factor
 *  exceeds 4. 
 *
 *  Params:
 *      ht - hashtable to be resized and rehashed
 *      convert - function that will convert a key to a hashable value
 */
void ht_resize(struct ht *ht, int (*convert)(void* a))
{ 
	
    struct dynarray *new_arr = dynarray_create(); // init new dynarray mem
    set_arr_params( new_arr, dynarray_cap(ht->array) * 2 ); // update new dynarray data size and capacity 

    for(int i = 0; i < dynarray_cap(ht->array); i++)
    {

        if( dynarray_get( ht->array, i ) ) // check if arr at idx is null
        { 
            struct ht_node *ht_node = list_rehash( (struct list *) dynarray_get( ht->array, i ) ); // remove head of list and get val

            while( ht_node ) 
            {

                int hash = rehash( new_arr, ht_node->key, convert ); // new hashed value

                if( !dynarray_get( new_arr, hash ) ){ dynarray_set( new_arr, hash, (void *) list_create() ); } // if list empty, add list at idx
                list_insert( (struct list *) dynarray_get( new_arr, hash), (void *) ht_node ); // add node at head of linked list

                ht_node = list_rehash( (struct list *) dynarray_get( ht->array, i ) ); // get next node

                if(!ht_node){ list_free( (struct list *) dynarray_get(ht->array, i) ); } // if no more nodes, free list
            }

        }

    }

    dynarray_free(ht->array); // free old array

    ht->array = new_arr; // replace old array

}


/*
 * This function should insert a given element into a hash table with a
 * specified key.  Note that you cannot have two same keys in one hash table.
 * If the key already exists, update the value associated with the key.  
 * This function is passed a function pointer that is used to convert the key (void*) 
 * to a unique hashcode (int). 
 * Resolution of collisions is requried, use either chaining or open addressing.
 * If using chaining, double the number of buckets when the load factor is >= 4
 * If using open addressing, double the array capacity when the load factor is >= 0.75
 * load factor = (number of elements) / (hash table capacity)
 *
 * Params:
 *   ht - the hash table into which to insert an element.  May not be NULL.
 *   key - the key of the element
 *   value - the value to be inserted into ht.
 *   convert - pointer to a function that can be passed the void* key from
 *     to convert it to a unique integer hashcode
 */

void ht_insert(struct ht* ht, void* key, void* value, int (*convert)(void*))
{
    if( (ht->size) - dynarray_cap(ht->array) == 4 ){ ht_resize(ht, convert); } // if resize needed, resize

    int hash = ht_hash_func( ht, key, convert );

    if( !dynarray_get( ht->array, hash ) ) // if map empty at idx, add list at idx
    { 
        struct list *list = list_create(); 
        dynarray_set (ht->array, hash, (void *) list ); 
    }
    else if( update_val( (struct list *) dynarray_get( ht->array, hash ) , key, value, convert, &ntk, &swap) == 1 ){ return; } // if not empty, try to update val

    // if no node to be updated, add node to hashtable

    struct ht_node *ht_node = malloc( sizeof( struct ht_node ) ); 
    ht_node->key = key, ht_node->val = value;

    list_insert( (struct list *) dynarray_get( ht->array, hash ), (void *) ht_node );

    (ht->size)++;
}


/*
 * This function should search for a given element in a hash table with a
 * specified key provided.   
 * This function is passed a function pointer that is used to convert the key (void*) 
 * to a unique hashcode (int). 
 * If the key is found, return the corresponding value (void*) of the element,
 * otherwise, return NULL
 *
 * Params:
 *   ht - the hash table into which to loop up for an element.  May not be NULL.
 *   key - the key of the element to search for
 *   convert - pointer to a function that can be passed the void* key from
 *     to convert it to a unique integer hashcode
 */
void* ht_lookup(struct ht* ht, void* key, int (*convert)(void*)){

    int hash = ht_hash_func( ht, key, convert );
    
    struct ht_node *ht_node = find_val( (struct list *) dynarray_get(ht->array, hash), key, &compare, &ntk); // returns node from map in O(1) avg.
    
    if(!ht_node){return NULL;} // if no node, return null
    
    return ht_node->val;
}


/*
 * This function should remove a given element in a hash table with a
 * specified key provided.   
 * This function is passed a function pointer that is used to convert the key (void*) 
 * to a unique hashcode (int). 
 * If the key is found, remove the element and return, otherwise, do nothing and return 
 *
 * Params:
 *   ht - the hash table into which to remove an element.  May not be NULL.
 *   key - the key of the element to remove
 *   convert - pointer to a function that can be passed the void* key from
 *     to convert it to a unique integer hashcode
 */
void ht_remove(struct ht* ht, void* key, int (*convert)(void*))
{
    int hash = ht_hash_func( ht, key, convert );

    list_remove( (struct list*) dynarray_get( ht->array, hash ), key, &compare, &ntk );
    
    ht->size--;
} 
