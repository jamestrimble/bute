package bute;

import java.util.ArrayList;


class ListOfSetAndNd implements TrieDataStructure {
    ArrayList<SetAndNd> list = new ArrayList<>();

    public void put(SetAndNd setAndNd) {
        list.add(setAndNd);
    }

    public ArrayList<SetAndNd> query(FastBitSet querySUnionN, FastBitSet neighbours) {
        return list;
    }
}
