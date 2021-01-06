package bute;

import java.util.ArrayList;


interface TrieDataStructure {
    void put(SetAndNd setAndNd);

    ArrayList<SetAndNd> query(FastBitSet querySUnionN, FastBitSet neighbours);
}
