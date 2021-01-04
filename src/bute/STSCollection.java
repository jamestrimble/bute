package bute;

import java.util.ArrayList;

import tw.exact.XBitSet;

// This class wraps a NewTrie, but has the following optimisation in order to save
// time and memory.  If a SetAndNd added using put() has a neighbourhood containing
// `target` vertices, it is inserted into the `itemsWithLargeNd` list instead of
// the trie.  If an element whose neighbourhood is different from those already in
// `itemsWithLargeNd` is to be inserted, `itemsWithLargeNd` is first cleared.  Therefore,
// to use this collection it is important that all put() calls with the same neighbourhood
// are contiguous.
class STSCollection implements TrieDataStructure {
    int target;
    private NewTrie trie;
    ArrayList<SetAndNd> itemsWithLargeNd = new ArrayList<>();

    public STSCollection(int n, int target) {
        this.target = target;
        trie = new NewTrie(n, target);
    }

    public void put(SetAndNd setAndNd) {
        if (setAndNd.nd.cardinality() < target) {
            trie.put(setAndNd);
        } else {
            if (!itemsWithLargeNd.isEmpty() && !itemsWithLargeNd.get(0).nd.equals(setAndNd.nd)) {
                itemsWithLargeNd.clear();
            }
            itemsWithLargeNd.add(setAndNd);
        }
    }

    public ArrayList<SetAndNd> query(XBitSet querySUnionN, XBitSet neighbours) {
        ArrayList<SetAndNd> list = trie.query(querySUnionN, neighbours);
        if (!itemsWithLargeNd.isEmpty() && itemsWithLargeNd.get(0).nd.equals(neighbours)) {
            list.addAll(itemsWithLargeNd);
        }
        return list;
    }
}
