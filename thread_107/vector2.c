#include "vector2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void VectorNew2(vector2 *v, int elemSize, VectorFreeFunction2 freeFn, int initialAllocation)
{
	assert(v != NULL);
	v->logicalLength = 0;

	if (initialAllocation == 0)
		initialAllocation = 10;

	v->allocatedLength = initialAllocation;
	v->allocationChunk = initialAllocation;
	v->elemSize = elemSize;
	v->elems = malloc(initialAllocation * elemSize);
	v->freeFn = freeFn;
}

void VectorDispose2(vector2 *v)
{
	assert(v != NULL);
	
	if (v->freeFn != NULL) {
		while (v->logicalLength > 0) {
			v->logicalLength--;
			void *target = (char *)v->elems + (v->logicalLength * v->elemSize);
			v->freeFn(target);
		}
	}
	free(v->elems);
}

int VectorLength2(const vector2 *v)
{ 
	assert(v != NULL);
	return v->logicalLength; 
}

void *VectorNth2(const vector2 *v, int position)
{
	assert((v != NULL) && (position >=0 ) && (position < v->logicalLength)); 
	if (v->logicalLength >= 0) {
		void *source = (char *)v->elems + (position * v->elemSize);
		return source;
	} else {
		return NULL; 
	}
}

void VectorReplace2(vector2 *v, const void *elemAddr, int position)
{
	assert((v != NULL) && (position >=0 ) && (position < v->logicalLength));

	void *target = (char *)v->elems + (position * v->elemSize);

	memcpy(target, elemAddr, v->elemSize);
}

void VectorGrow2(vector2 *v) {
	v->allocatedLength += v->allocationChunk;
	v->elems = realloc(v->elems, v->allocatedLength * v->elemSize);
}

void VectorInsert2(vector2 *v, const void *elemAddr, int position)
{
	assert((v != NULL) && (elemAddr != NULL) && (position >=0 ) && (position <= v->logicalLength));

	if (v->logicalLength == v->allocatedLength) VectorGrow2(v);

	void *insertAt = (char *)v->elems + (position * v->elemSize);
	
	// if not at the end, need to move everthing down by 1 element
	if (position != v->logicalLength) {
		void *moveTo = (char *)insertAt + v->elemSize;
		memmove(moveTo, insertAt, (v->logicalLength - position) * v->elemSize);
	}
	// copy the element to the place to insert in the list
	memcpy(insertAt, elemAddr, v->elemSize);
	
	v->logicalLength++;
}

void VectorAppend2(vector2 *v, const void *elemAddr)
{
	assert((v != NULL) && (elemAddr != NULL));

	if (v->logicalLength == v->allocatedLength) VectorGrow2(v);

	void *endList = (char *)v->elems + (v->logicalLength * v->elemSize);
	memcpy(endList, elemAddr, v->elemSize);
	v->logicalLength++;
}

void VectorDelete2(vector2 *v, int position)
{
	assert((v != NULL) && (position >=0 ) && (position < v->logicalLength));

	v->logicalLength--;

	// if not at the end, need to move the list up by one
	if (position != v->logicalLength) {
		void *moveTo = (char *)v->elems + (position * v->elemSize);
		void *moveFrom = (char *)moveTo + v->elemSize;
		memcpy(moveTo, moveFrom, (v->logicalLength - position) * v->elemSize);
	}
}

void VectorSort2(vector2 *v, VectorCompareFunction2 compare)
{
	assert((v != NULL) && (compare != NULL));

	qsort(v->elems, v->logicalLength, v->elemSize, compare);
}

void VectorMap2(vector2 *v, VectorMapFunction2 mapFn, void *auxData)
{
	assert((v != NULL) && (mapFn != NULL));

	for (int i = 0 ; i < v->logicalLength; i++) {
		void *elem = (char *)v->elems + (i * v->elemSize);
		mapFn(elem, auxData);
	}
}

static const int kNotFound = -1;
int VectorSearch2(const vector2 *v, const void *key, VectorCompareFunction2 searchFn, int startIndex, bool isSorted)
{ 
	int result = kNotFound;

	if (isSorted) {
		// use binary search
		void *elemFound = bsearch(key,v->elems, v->logicalLength * v->elemSize, v->elemSize, searchFn);
		if (elemFound != NULL) {
			int position = (int)(((char *)elemFound - (char *)v->elems) / v->elemSize);
			return position;
		}
	} else {
		// use linear search
		for (int i = startIndex; i < v->logicalLength; i++) {
			void *elem = (char *)v->elems + (i * v->elemSize);
			if (searchFn != NULL) {
				result = searchFn(key, elem);
			} else {
				result = memcmp(key,elem,v->elemSize);
			}
			if (result == 0) return i;
		}
	}
	return kNotFound; 
} 
