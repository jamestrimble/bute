package bute;

import java.util.ArrayList;
import java.util.Arrays;

import tw.exact.XBitSet;

class TrieNode {
    private TrieNode[] children = new TrieNode[0];
    XBitSet subtrieIntersectionOfSSets;
    XBitSet subtrieIntersectionOfNSets;
    private int key;
    private XBitSet[] SSets = new XBitSet[0];

    TrieNode(int key, XBitSet initialIntersectionOfNSets,
            XBitSet initialIntersectionOfSSets) {
        this.key = key;
        subtrieIntersectionOfNSets = initialIntersectionOfNSets;
        subtrieIntersectionOfSSets = initialIntersectionOfSSets;
    }

    void addSSet(XBitSet SSet) {
        SSets = Arrays.copyOf(SSets, SSets.length + 1);
        SSets[SSets.length - 1] = (XBitSet) SSet.clone();
    }

    TrieNode getOrAddChildNode(int key, XBitSet SSet, XBitSet NSet) {
        for (TrieNode child : children) {
            if (child.key == key) {
                child.subtrieIntersectionOfNSets.and(NSet);
                child.subtrieIntersectionOfSSets.and(SSet);
                return child;
            }
        }
        // Node not found; add and return it
        TrieNode node = new TrieNode(key, (XBitSet) NSet.clone(), (XBitSet) SSet.clone());
        children = Arrays.copyOf(children, children.length + 1);
        children[children.length - 1] = node;
        return node;
    }

    void query(XBitSet querySUnionN, XBitSet queryN, int k,
            int budget, ArrayList<SetAndNd> out_list) {
        if (subtrieIntersectionOfNSets.subtract(queryN).cardinality() > k) {
            return;
        }
        if (querySUnionN.intersects(subtrieIntersectionOfSSets)) {
            return;
        }
        for (XBitSet SSet : SSets) {
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

    void printLatexBitset(XBitSet bitset, String colour) {
        System.out.print("\\\\ [-1ex] \\scriptsize {\\color{" + colour + "} $");
        if (bitset.isEmpty()) {
            System.out.print("\\emptyset");
        } else {
            String comma = "";
            for (int v = bitset.nextSetBit(0); v >= 0; v = bitset.nextSetBit(v+1)) {
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
            for (XBitSet SSet : SSets) {
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
