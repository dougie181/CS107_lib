/** 
 * File: vector2.h
 * ---------------
 * Defines the interface for the vector2.
 *
 * The vector2 allows the client to store any number of elements of any desired
 * primitive type and is appropriate for a wide variety of storage problems.  It 
 * supports efficient element access and appending/inserting/deleting elements
 * as well as optional sorting and searching.  In all cases, the vector2 imposes 
 * no upper bound on the number of elements and deals with all its own memory 
 * management. The client specifies the size (in bytes) of the elements that 
 * will be stored in the vector2 when it is created.  Thereafter the client and 
 * the vector2 can refer to elements via (void *) ptrs.
 */

#ifndef _vector2_
#define _vector2_

#include "bool.h"

/**
 * Type: VectorCompareFunction2
 * ----------------------------
 * VectorCompareFunction2 is a pointer to a client-supplied function which the
 * vector2 uses to sort or search for elements.  The comparator takes two 
 * (const void *) pointers (these will point to elements) and returns an int.
 * The comparator should indicate the ordering of the two elements
 * using the same convention as the strcmp library function:
 * 
 *   If elemAddr1 is less than elemAddr2, return a negative number.
 *   If elemAddr1 is greater than elemAddr2, return a positive number.
 *   If the two elements are equal, return 0.
 */

typedef int (*VectorCompareFunction2)(const void *elemAddr1, const void *elemAddr2);

/** 
 * Type: VectorMapFunction2
 * ------------------------
 * VectorMapFunction2 defines the space of functions that can be used to map over
 * the elements in a vector2.  A map function is called with a pointer to
 * the element and a client data pointer passed in from the original
 * caller.
 */

typedef void (*VectorMapFunction2)(void *elemAddr, void *auxData);

/** 
 * Type: VectorFreeFunction2
 * ---------------------------------
 * VectorFreeFunction2 defines the space of functions that can be used as the
 * clean-up function for each element as it is deleted from the vector2
 * or when the entire vector2 is destroyed.  The cleanup function is 
 * called with a pointer to an element about to be deleted.
 */

typedef void (*VectorFreeFunction2)(void *elemAddr);

/**
 * Type: vector2
 * ------------
 * Defines the concrete representation of
 * the vector2.  Even though everything is
 * exposed, the client should respect the 
 * the privacy of the representation and initialize,
 * dispose of, and otherwise interact with a
 * vector using those functions defined in this file.
 */

typedef struct {
	void *elems;
	int logicalLength;
	int allocatedLength;
	int allocationChunk;
	int elemSize;
	VectorFreeFunction2 freeFn;
} vector2;

/** 
 * Function: VectorNew2
 * Usage: vector2 myFriends;
 *        VectorNew2(&myFriends, sizeof(char *), StringFree, 10);
 * -------------------
 * Constructs a raw or previously destroyed vector2 to be the
 * empty vector2.
 * 
 * The elemSize parameter specifies the number of bytes that a single 
 * element of the vector2 should take up.  For example, if you want to store 
 * elements of type char *, you would pass sizeof(char *) as this parameter.
 * An assert is raised if the size is not greater than zero.
 *
 * The ArrayFreeFunction is the function that will be called on an element that
 * is about to be deleted (using VectorDelete2) or on each element in the
 * vector2 when the entire vector2 is being freed (using VectorDispose2).  This function
 * is your chance to do any deallocation/cleanup required for the element
 * (such as freeing/deleting any pointers contained in the element). The client can pass 
 * NULL for the ArrayFreeFunction if the elements don't require any special handling.
 *
 * The initialAllocation parameter specifies the initial allocated length 
 * of the vector2, as well as the dynamic reallocation increment for those times when the 
 * vector2 needs to grow.  Rather than growing the vector2 one element at a time as 
 * elements are added (inefficient), you will grow the vector2 
 * in chunks of initialAllocation size.  The allocated length is the number
 * of elements for which space has been allocated: the logical length 
 * is the number of those slots currently being used.
 * 
 * A new vector2 pre-allocates space for initialAllocation elements, but the
 * logical length is zero.  As elements are added, those allocated slots fill
 * up, and when the initial allocation is all used, grow the vector2 by another 
 * initialAllocation elements.  You will continue growing the vector2 in chunks
 * like this as needed.  Thus the allocated length will always be a multiple
 * of initialAllocation.  Don't worry about using realloc to shrink the vector2's 
 * allocation if a bunch of elements get deleted.  It turns out that 
 * many implementations of realloc don't even pay attention to such a request, 
 * so there is little point in asking.  Just leave the vector2 over-allocated and no
 * one will care.
 *
 * The initialAllocation is the client's opportunity to tune the resizing
 * behavior for his/her particular needs.  Clients who expect their vectors2 to
 * become large should probably choose a large initial allocation size, whereas
 * clients who expect the vector2 to be relatively small should choose a smaller
 * initialAllocation size.  You want to minimize the number of reallocations, but
 * you don't want to pre-allocate all that much memory if you don't expect to use very
 * much of it.  If the client passes 0 for initialAllocation, the implementation
 * will use the default value of its own choosing.  As assert is raised is 
 * the initialAllocation value is less than 0.
 */

void VectorNew2(vector2 *v, int elemSize, VectorFreeFunction2 freefn, int initialAllocation);

/**
 * Function: VectorDispose2
 *           VectorDispose2(&studentsDroppingTheCourse);
 * -----------------------
 * Frees up all the memory of the specified vector2 and its elements.  It does *not* 
 * automatically free memory owned by pointers embedded in the elements. 
 * This would require knowledge of the structure of the elements, which the 
 * vector2 does not have.  However, it *will* iterate over the elements calling
 * the VectorFreeFunction2 previously supplied to VectorNew2.
 */

void VectorDispose2(vector2 *v);

/**
 * Function: VectorLength2
 * -----------------------
 * Returns the logical length of the vector2, i.e. the number of elements
 * currently in the vector2.  Must run in constant time.
 */

int VectorLength2(const vector2 *v);
	   
/**
 * Method: VectorNth2
 * -----------------
 * Returns a pointer to the element numbered position in the vector2.  
 * Numbering begins with 0.  An assert is raised if n is less than 0 or greater 
 * than the logical length minus 1.  Note this function returns a pointer into 
 * the vector2's storage, so the pointer should be used with care.
 * This method must operate in constant time.
 *
 * We could have written the vector2 without this sort of access, but it
 * is useful and efficient to offer it, although the client needs to be 
 * careful when using it.  In particular, a pointer returned by VectorNth2
 * becomes invalid after any calls which involve insertion into, deletion from or 
 * sorting of the vector2, as all of these may rearrange the elements to some extent.
 */ 

void *VectorNth2(const vector2 *v, int position);
					  
/**
 * Function: VectorInsert2
 * -----------------------
 * Inserts a new element into the specified vector2, placing it at the specified position.
 * An assert is raised if n is less than 0 or greater than the logical length.
 * The vector2 elements after the supplied position will be shifted over to make room. 
 * The element is passed by address: The new element's contents are copied from 
 * the memory pointed to by elemAddr.  This method runs in linear time.
 */

void VectorInsert2(vector2 *v, const void *elemAddr, int position);

/**
 * Function: VectorAppend2
 * ----------------------
 * Appends a new element to the end of the specified vector2.  The element is 
 * passed by address, and the element contents are copied from the memory pointed 
 * to by elemAddr.  Note that right after this call, the new element will be 
 * the last in the vector2; i.e. its element number will be the logical length 
 * minus 1.  This method must run in constant time (neglecting the memory reallocation 
 * time which may be required occasionally).
 */

void VectorAppend2(vector2 *v, const void *elemAddr);
  
/**
 * Function: VectorReplace2
 * -----------------------
 * Overwrites the element at the specified position with a new value.  Before 
 * being overwritten, the VectorFreeFunction2 that was supplied to VectorNew2 is levied
 * against the old element.  Then that position in the vector2 will get a new value by
 * copying the new element's contents from the memory pointed to by elemAddr.
 * An assert is raised if n is less than 0 or greater than the logical length 
 * minus one.  None of the other elements are affected or rearranged by this
 * operation, and the size of the vector2 remains constant. This method must
 * operate in constant time.
 */

void VectorReplace2(vector2 *v, const void *elemAddr, int position);

/**
 * Function: VectorDelete2
 * -----------------------
 * Deletes the element at the specified position from the vector2. Before the 
 * element is removed,  the ArrayFreeFunction that was supplied to VectorNew2
 * will be called on the element.
 *
 * An assert is raised if position is less than 0 or greater than the logical length 
 * minus one.  All the elements after the specified position will be shifted over to fill 
 * the gap.  This method runs in linear time.  It does not shrink the 
 * allocated size of the vector2 when an element is deleted; the vector2 just 
 * stays over-allocated.
 */

void VectorDelete2(vector2 *v, int position);
  
/* 
 * Function: VectorSearch2
 * ----------------------
 * Searches the specified vector2 for an element whose contents match
 * the element passed as the key.  Uses the comparator argument to test
 * for equality.  The startIndex parameter controls where the search
 * starts.  If the client desires to search the entire vector2,
 * they should pass 0 as the startIndex.  The method will search from
 * there to the end of the vector2.  The isSorted parameter allows the client 
 * to specify that the vector2 is already in sorted order, in which case VectorSearch2
 * uses a faster binary search.  If isSorted is false, a simple linear search is 
 * used.  If a match is found, the position of the matching element is returned;
 * else the function returns -1.  Calling this function does not 
 * re-arrange or change contents of the vector2 or modify the key in any way.
 * 
 * An assert is raised if startIndex is less than 0 or greater than
 * the logical length (although searching from logical length will never
 * find anything, allowing this case means you can search an entirely empty
 * vector2 from 0 without getting an assert).  An assert is raised if the
 * comparator or the key is NULL.
 */  

int VectorSearch2(const vector2 *v, const void *key, VectorCompareFunction2 searchfn, int startIndex, bool isSorted);

/**
 * Function: VectorSort2
 * --------------------
 * Sorts the vector2 into ascending order according to the supplied
 * comparator.  The numbering of the elements will change to reflect the 
 * new ordering.  An assert is raised if the comparator is NULL.
 */

void VectorSort2(vector2 *v, VectorCompareFunction2 comparefn);

/**
 * Method: VectorMap2
 * -----------------
 * Iterates over the elements of the vector2 in order (from element 0 to
 * element n-1, inclusive) and calls the function mapfn on each element.  The function is 
 * called with the address of the vector2 element and the auxData pointer.  
 * The auxData value allows the client to pass extra state information to 
 * the client-supplied function, if necessary.  If no client data is required, 
 * this argument should be NULL.  An assert is raised if the mapfn function is NULL.
 */

void VectorMap2(vector2 *v, VectorMapFunction2 mapfn, void *auxData);

#endif
