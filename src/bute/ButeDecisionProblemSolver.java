package bute;

import java.util.*;
import java.util.stream.Collectors;

import tw.exact.Graph;
import tw.exact.XBitSet;

class ButeDecisionProblemSolver {
    long tmpCount2;
    final Graph g;
    final int n;
    final Dom dom;
    final int target;
    final HashMap<XBitSet, Integer> setRoot = new HashMap<>();
    static final int MIN_LEN_FOR_TRIE = 50;

    ButeDecisionProblemSolver(Graph g, Dom dom, int target) {
        this.g = g;
        this.n = g.n;
        this.dom = dom;
        this.target = target;
    }

    ArrayList<XBitSet> getComponentsInInducedSubgraph(XBitSet vv) {
        return g.getComponents(g.all.subtract(vv));
    }

    void tryAddingStsRoot(int w, XBitSet unionOfSubtrees,
            XBitSet ndOfUnionOfSubtrees, int rootDepth,
            HashSet<XBitSet> newSTSsHashSet) {
        XBitSet adjVv = ndOfUnionOfSubtrees
                .unionWith(g.neighborSet[w])
                .subtract(unionOfSubtrees);
        adjVv.clear(w);
        if (adjVv.cardinality() < rootDepth) {
            XBitSet STS = (XBitSet) unionOfSubtrees.clone();
            STS.set(w);
            if (!dom.vvDominatedBy[w].intersects(adjVv) &&
                    !dom.vvThatDominate[w].intersects(STS) &&
                    !newSTSsHashSet.contains(STS)) {
                newSTSsHashSet.add(STS);
                if (!setRoot.containsKey(STS)) {
                    setRoot.put(STS, w);
                }
            }
        }
    }

    // TODO figure out why C program prints different parent array of solution
    void makeSTSsHelper(ArrayList<SetAndNd> STSsAndNds,
                        XBitSet possibleSTSRoots,
                        // TODO: rename next two params?
                        XBitSet unionOfSubtrees,
                        XBitSet ndOfUnionOfSubtrees,
                        int rootDepth,
                        HashSet<XBitSet> newSTSsHashSet) {
        for (int w = possibleSTSRoots.nextSetBit(0); w >= 0;
                w = possibleSTSRoots.nextSetBit(w+1)) {
            tryAddingStsRoot(w, unionOfSubtrees, ndOfUnionOfSubtrees, rootDepth,
                    newSTSsHashSet);
        }

        if (!STSsAndNds.isEmpty()) {
            ++tmpCount2;
        }

        TrieDataStructure visitedSTSs = STSsAndNds.size() >= MIN_LEN_FOR_TRIE ?
                new STSCollection(n, rootDepth) :
                new ListOfSetAndNd();

        for (int i=STSsAndNds.size()-1; i>=0; i--) {
            XBitSet unionOfFilteredSets = new XBitSet();
            XBitSet s = STSsAndNds.get(i).set;
            XBitSet nd = STSsAndNds.get(i).nd;
            XBitSet newPossibleSTSRoots = possibleSTSRoots.intersectWith(nd);
            XBitSet newUnionOfSubtrees = unionOfSubtrees.unionWith(s);
            XBitSet ndOfNewUnionOfSubtrees = ndOfUnionOfSubtrees
                    .unionWith(nd)
                    .subtract(newUnionOfSubtrees);

            XBitSet newUnionOfSubtreesAndNd = newUnionOfSubtrees
                    .unionWith(ndOfNewUnionOfSubtrees);
            ArrayList<SetAndNd> filteredSTSsAndNds = new ArrayList<>();

            // TODO: store SetAndNd references in trie data structure
            ArrayList<SetAndNd> queryResults = visitedSTSs.query(
                    newUnionOfSubtreesAndNd, ndOfNewUnionOfSubtrees);
            for (SetAndNd candidate : queryResults) {
                if (!candidate.nd.intersects(newPossibleSTSRoots))
                    continue;
                if (ndOfNewUnionOfSubtrees.unionWith(candidate.nd).cardinality()
                        > rootDepth)
                    continue;
                if (newUnionOfSubtreesAndNd.intersects(candidate.set))
                    continue;
                filteredSTSsAndNds.add(candidate);
                unionOfFilteredSets.or(candidate.set);
            }

            for (int v = newPossibleSTSRoots.nextSetBit(0); v >= 0;
                    v = newPossibleSTSRoots.nextSetBit(v+1)) {
                XBitSet adjVv = ndOfNewUnionOfSubtrees
                        .unionWith(g.neighborSet[v])
                        .subtract(newUnionOfSubtrees)
                        .subtract(unionOfFilteredSets);
                adjVv.clear(v);
                if (adjVv.cardinality() >= rootDepth ||
                        !unionOfFilteredSets.unionWith(newUnionOfSubtrees)
                        .isSuperset(dom.adjVvDominatedBy[v])) {
                    newPossibleSTSRoots.clear(v);
                }
            }

            if (!newPossibleSTSRoots.isEmpty()) {
                filteredSTSsAndNds.sort(new DescendingNdPopcountComparator());
                makeSTSsHelper(filteredSTSsAndNds, newPossibleSTSRoots,
                        newUnionOfSubtrees, ndOfNewUnionOfSubtrees, rootDepth,
                        newSTSsHashSet);
            }
            visitedSTSs.put(STSsAndNds.get(i));
        }
    }

    ArrayList<XBitSet> makeSTSs(ArrayList<XBitSet> STSs, int rootDepth) {
        HashSet<XBitSet> newSTSsHashSet = new HashSet<>();
        // TODO: always keep sets together with their neighbourhoods,
        // avoiding the need to package them up here?
        ArrayList<SetAndNd> STSsAndNds = STSs
                .stream()
                .map(STS -> new SetAndNd(STS, findAdjacentVv(STS)))
                .collect(Collectors.toCollection(ArrayList::new));

        STSsAndNds.sort(new DescendingNdPopcountComparator());

        XBitSet emptySet = new XBitSet();
        XBitSet fullSet = new XBitSet(n);
        fullSet.set(0, n);
        makeSTSsHelper(STSsAndNds, fullSet, emptySet, emptySet, rootDepth,
                newSTSsHashSet);

        return new ArrayList<>(newSTSsHashSet);
    }

    XBitSet findAdjacentVv(XBitSet s) {
        XBitSet unionOfNds = new XBitSet();
        for (int v = s.nextSetBit(0); v >= 0; v = s.nextSetBit(v+1)) {
            unionOfNds.or(g.neighborSet[v]);
        }
        return unionOfNds.subtract(s);
    }

    void addParents(int[] parent, XBitSet s, int parentVertex) {
        int v = setRoot.get(s);
        parent[v] = parentVertex;
        XBitSet descendants = (XBitSet) s.clone();
        descendants.clear(v);
        for (XBitSet component : getComponentsInInducedSubgraph(descendants)) {
            addParents(parent, component, v);
        }
    }

    TreedepthResult solve() {
        System.err.println("Solving decision problem " + target);

        ArrayList<XBitSet> STSs = new ArrayList<>();

        for (int i=target; i>=1; i--) {
            int prevSetRootSize = setRoot.size();
            STSs = makeSTSs(STSs, i);

            if (setRoot.size() == prevSetRootSize) {
                break;
            }

            System.err.println("# " + (setRoot.size() - prevSetRootSize));
            System.err.println("## " + tmpCount2);
            tmpCount2 = 0;

            for (XBitSet STS : STSs) {
                XBitSet adjacentVv = findAdjacentVv(STS);
                if (STS.cardinality() + adjacentVv.cardinality() == n) {
                    int[] parent = new int[n];
                    int parentVertex = -1;
                    for (int w = adjacentVv.nextSetBit(0); w >= 0;
                            w = adjacentVv.nextSetBit(w+1)) {
                        parent[w] = parentVertex;
                        parentVertex = w;
                    }
                    addParents(parent, STS, parentVertex);
                    return new TreedepthResult(target, parent);
                }
            }
        }
        return null;
    }

    static class DescendingNdPopcountComparator implements Comparator<SetAndNd> {
        @Override
        public int compare(SetAndNd a, SetAndNd b) {
            int aPopcount = a.nd.cardinality();
            int bPopcount = b.nd.cardinality();
            if (aPopcount != bPopcount) {
                return aPopcount > bPopcount ? -1 : 1;
            }
            int cmp = a.nd.compareTo(b.nd);
            if (cmp != 0) {
                return cmp;
            }
            aPopcount = a.set.cardinality();
            bPopcount = b.set.cardinality();
            if (aPopcount != bPopcount) {
                return aPopcount > bPopcount ? -1 : 1;
            }
            return a.set.compareTo(b.set);
        }
    }
}
