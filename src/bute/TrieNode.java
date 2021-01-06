package bute;

import java.util.ArrayList;
import java.util.Arrays;


class TrieNode {
    private TrieNode[] children = new TrieNode[0];
    FastBitSet subtrieIntersectionOfSSets;
    FastBitSet subtrieIntersectionOfNSets;
    private final int key;
    private FastBitSet[] SSets = new FastBitSet[0];

    TrieNode(int key, FastBitSet initialIntersectionOfNSets,
            FastBitSet initialIntersectionOfSSets) {
        this.key = key;
        subtrieIntersectionOfNSets = initialIntersectionOfNSets;
        subtrieIntersectionOfSSets = initialIntersectionOfSSets;
    }

    void addSSet(FastBitSet SSet) {
        SSets = Arrays.copyOf(SSets, SSets.length + 1);
        SSets[SSets.length - 1] = new FastBitSet(SSet);
    }

    TrieNode getOrAddChildNode(int key, FastBitSet SSet, FastBitSet NSet) {
        for (TrieNode child : children) {
            if (child.key == key) {
                child.subtrieIntersectionOfNSets.and(NSet);
                child.subtrieIntersectionOfSSets.and(SSet);
                return child;
            }
        }
        // Node not found; add and return it
        TrieNode node = new TrieNode(key, new FastBitSet(NSet), new FastBitSet(SSet));
        children = Arrays.copyOf(children, children.length + 1);
        children[children.length - 1] = node;
        return node;
    }

    void query(FastBitSet querySUnionN, FastBitSet queryN, int k,
            int budget, ArrayList<SetAndNd> out_list) {
        if (subtrieIntersectionOfNSets.cardinalityOfDifference(queryN) > k) {
            return;
        }
        if (querySUnionN.intersects(subtrieIntersectionOfSSets)) {
            return;
        }
        for (FastBitSet SSet : SSets) {
            if (!querySUnionN.intersects(SSet)) {
                SetAndNd elem = new SetAndNd(SSet, subtrieIntersectionOfNSets);
                out_list.add(elem);
            }
        }
        for (TrieNode child : children) {
            int newBudget = queryN.get(child.key) ? budget : budget - 1;
            if (newBudget >= 0) {
                child.query(querySUnionN, queryN, k, newBudget, out_list);
            }
        }
    }

    void printLatexBitset(FastBitSet bitset, String colour) {
        System.out.print("\\\\ [-1ex] \\scriptsize {\\color{" + colour + "} $");
        if (bitset.isEmpty()) {
            System.out.print("\\emptyset");
        } else {
            String comma = "";
            for (int v : bitset.toArray()) {
                System.out.print(comma + v);
                comma = " ";
            }
        }
        System.out.print("$} ");
    }

    void printLatex(ArrayList<Integer> currentNodeN, int featureFlags) {
        System.out.print("[{$");
        if (currentNodeN.isEmpty()) {
            System.out.print("\\emptyset");
        } else {
            for (int v : currentNodeN) {
                if (v == currentNodeN.get(currentNodeN.size() - 1)) {
                    System.out.print("\\mathbf{\\underline{" + v + "}}");
                } else {
                    System.out.print(v);
                }
            }
        }
        System.out.print("$ ");

        if (0 != (featureFlags & 1)) {
            printLatexBitset(subtrieIntersectionOfNSets, "black!50");
        }

        if (0 != (featureFlags & 2)) {
            printLatexBitset(subtrieIntersectionOfSSets, "blue");
        }

        if (0 != (featureFlags & 4)) {
            for (FastBitSet SSet : SSets) {
                printLatexBitset(SSet, "blue!50");
            }
        }
        System.out.print("},align=center");
        if (SSets.length > 0) {
            System.out.print(",line width=.7mm");
        }
        for (TrieNode child : children) {
            currentNodeN.add(child.key);
            child.printLatex(currentNodeN, featureFlags);
            currentNodeN.remove(currentNodeN.size() - 1);
        }
        System.out.print("]");
    }
}
