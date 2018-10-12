#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
	assert(elemSize > 0);
	assert(numBuckets > 0);
	assert(hashfn != NULL);
	assert(comparefn != NULL);

	h->elemCount = 0;
	h->numBuckets = numBuckets;
        h->elemSize = elemSize;
	
	// this will essentially be an array (numBuckets size) of vectors
	h->buckets = malloc(numBuckets * sizeof(vector));

	// initialise a vector for each 'bucket'
	for (int i = 0; i < numBuckets; i++) {		
		void *theVector = (char *)h->buckets + (i * sizeof(vector));
		VectorNew(theVector, elemSize, freefn, 4);
	}
	h->hashfn = hashfn;
	h->comparefn = comparefn;
}

void HashSetDispose(hashset *h)
{
	assert(h != NULL);

	for (int i = 0; i < h->numBuckets; i++) {
		void *theVector = (char *)h->buckets + (i * sizeof(vector));
		VectorDispose(theVector);
	}
	free(h->buckets);
	h->elemCount = 0;
}

int HashSetCount(const hashset *h)
{ 
	assert(h != NULL);
	
	return h->elemCount;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
	assert(mapfn != NULL && mapfn != NULL);
	for (int i = 0; i < h->numBuckets; i++) {
		void *theVector = (char *)h->buckets + (i * sizeof(vector));
		VectorMap(theVector, mapfn, auxData);
	}
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
	assert(h != NULL);

	int bucket = h->hashfn(elemAddr, h->numBuckets);

	assert(bucket >= 0 && bucket < h->numBuckets);
		
	void *theVector = (char *)h->buckets + (bucket * sizeof(vector));

	int findElem = VectorSearch(theVector, elemAddr, h->comparefn, 0, false);

	// if element is found then replace it, else add it
	if (findElem == -1) {
		VectorAppend(theVector, elemAddr);
		h->elemCount++;
	} else {
		VectorReplace(theVector, elemAddr, findElem);
	}
}

void *HashSetLookup(hashset *h, const void *elemAddr)
{ 
	assert(h != NULL);	
	
	int bucket = h->hashfn(elemAddr, h->numBuckets);

	assert(bucket >= 0 && bucket < h->numBuckets);
	
	void *theVector = (char *)h->buckets + (bucket * sizeof(vector));

	int elemLocation = VectorSearch(theVector, elemAddr, h->comparefn, 0, false);

	if (elemLocation == -1)
		return NULL;
	else 
		return VectorNth(theVector, elemLocation);
}
