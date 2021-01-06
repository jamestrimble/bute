package bute;

import java.util.ArrayList;


class NewTrie implements TrieDataStructure, LatexPrintable {
    private final int target;
    private final TrieNode root;
    private int size;

    public NewTrie(int n, int target) {
        this.target = target;
        FastBitSet initialIntersectionOfNSets = new FastBitSet(n);
        initialIntersectionOfNSets.set(0, n);
        FastBitSet initialIntersectionOfSSets = new FastBitSet(n);
        initialIntersectionOfSSets.set(0, n);
        root = new TrieNode(-1, initialIntersectionOfNSets,
                initialIntersectionOfSSets);
    }

    public void put(SetAndNd setAndNd) {
        FastBitSet SSet = setAndNd.set;
        FastBitSet NSet = setAndNd.nd;
        root.subtrieIntersectionOfNSets.and(NSet);
        root.subtrieIntersectionOfSSets.and(SSet);
        TrieNode node = root;
        // iterate over elements of NSet
        for (int i : NSet.toArray()) {
            node = node.getOrAddChildNode(i, SSet, NSet);
        }
        node.addSSet(SSet);
        ++size;
    }

    // for debugging
    void showList(ArrayList<FastBitSet> bitsets) {
        for (FastBitSet bs : bitsets) {
            System.out.println(bs);
        }
    }

    public ArrayList<SetAndNd> query(FastBitSet querySUnionN, FastBitSet neighbours) {
        ArrayList<SetAndNd> list = new ArrayList<>();
        int k = target - neighbours.cardinality();
        if (k >= 0) {
            root.query(querySUnionN, neighbours, k, k, list);
        }
        return list;
    }

    public void printLatex(int featureFlags) {
        System.out.println("\\documentclass{standalone}");
        System.out.println("\\usepackage{forest}");
        System.out.println("\\forestset{  default preamble={  for tree={draw,rounded corners}  }}");
        System.out.println("\\begin{document}");
        System.out.println("\\begin{forest}");
        root.printLatex(new ArrayList<>(), featureFlags);
        System.out.println();
        System.out.println("\\end{forest}");
        System.out.println("\\end{document}");
    }
}
