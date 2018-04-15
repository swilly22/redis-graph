#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../../src/util/prng.h"
#include "../../src/hexastore/hexastore.h"
#include "../../src/hexastore/triplet.h"

void test_hexastore() {
    Triplet *t;
    TripletIterator it;
    long int id;

    id = get_new_id();
    Node *subject_node = NewNode(id, "actor");
	id = get_new_id();
    Node *object_node = NewNode(id, "movie");
	id = get_new_id();
    Edge *predicate_edge = NewEdge(id, subject_node, object_node, "act");
    Triplet *triplet = NewTriplet(subject_node, predicate_edge, object_node);

	HexaStore *hexastore = _NewHexaStore();
    assert(hexastore);
    
    HexaStore_InsertAllPerm(hexastore, triplet);
    assert(raxSize(hexastore));

    /* Search hexastore.
     * Scan entire hexastore. */
    HexaStore_Search(hexastore, "", &it);

    for(int i = 0; i < 6; i++) {
        assert(TripletIterator_Next(&it, &t));
        assert(t == triplet);
    }
    assert(!TripletIterator_Next(&it, &t));

    /* Searching all possible permutations. */
    HexaStore_Search(hexastore, "SPO", &it);
    assert(TripletIterator_Next(&it, &t));
    assert(!TripletIterator_Next(&it, &t));

    HexaStore_Search(hexastore, "SOP", &it);
    assert(TripletIterator_Next(&it, &t));
    assert(!TripletIterator_Next(&it, &t));

    HexaStore_Search(hexastore, "PSO", &it);
    assert(TripletIterator_Next(&it, &t));
    assert(!TripletIterator_Next(&it, &t));

    HexaStore_Search(hexastore, "POS", &it);
    assert(TripletIterator_Next(&it, &t));
    assert(!TripletIterator_Next(&it, &t));

    HexaStore_Search(hexastore, "OSP", &it);
    assert(TripletIterator_Next(&it, &t));
    assert(!TripletIterator_Next(&it, &t));

    HexaStore_Search(hexastore, "OPS", &it);
    assert(TripletIterator_Next(&it, &t));
    assert(!TripletIterator_Next(&it, &t));

    HexaStore_RemoveAllPerm(hexastore, triplet);
    assert(raxSize(hexastore) == 0);

    /* Searching an empty hexastore */
    HexaStore_Search(hexastore, "", &it);
    assert(!TripletIterator_Next(&it, &t));
}

int main(int argc, char **argv) {
	test_hexastore();
    printf("test_hexastore - PASS!\n");
	return 0;
}