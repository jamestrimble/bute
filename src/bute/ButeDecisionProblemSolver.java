package bute;

import java.util.*;
import java.util.stream.Collectors;

import tw.exact.Graph;
import tw.exact.XBitSet;

class ButeDecisionProblemSolver {
    Graph g;
    int n;
    Dom dom;
    int target;
    HashMap<XBitSet, Integer> setRoot = new HashMap<XBitSet, Integer>();
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

    boolean isStsAcceptable(SetAndNd candidate, XBitSet newPossibleSTSRoots,
            XBitSet ndOfNewUnionOfSubtrees, XBitSet newUnionOfSubtreesAndNd,
            int rootDepth) {
        if (!candidate.nd.intersects(newPossibleSTSRoots))
            return false;
        if (ndOfNewUnionOfSubtrees.unionWith(candidate.nd).cardinality()
                > rootDepth)
            return false;
        if (newUnionOfSubtreesAndNd.intersects(candidate.set))
            return false;
        return true;
    }

    // TODO figure out why C program prints different parent array of solution
    void makeSTSsHelper(ArrayList<SetAndNd> STSsAndNds,
                        XBitSet possibleSTSRoots,
                        // TODO: rename next two params?
                        XBitSet unionOfSubtrees,
                        XBitSet ndOfUnionOfSubtrees,
                        int rootDepth,
                        HashSet<XBitSet> newSTSsHashSet) {
        if (STSsAndNds.isEmpty()) {
            return;
        }

        TrieDataStructure visitedSTSs = STSsAndNds.size() >= MIN_LEN_FOR_TRIE ?
                new NewTrie(n, rootDepth) :
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

            for (int w = newPossibleSTSRoots.nextSetBit(0); w >= 0;
                    w = newPossibleSTSRoots.nextSetBit(w+1)) {
                XBitSet adjVv = ndOfNewUnionOfSubtrees
                        .unionWith(g.neighborSet[w])
                        .subtract(newUnionOfSubtrees);
                if (adjVv.cardinality() <= rootDepth) {
                    XBitSet STS = (XBitSet) newUnionOfSubtrees.clone();
                    STS.set(w);
                    if (!dom.vvDominatedBy[w].intersects(adjVv) &&
                            !dom.vvThatDominate[w].intersects(STS) &&
                            !setRoot.containsKey(STS)) {
                        newSTSsHashSet.add(STS);
                        setRoot.put(STS, w);
                    }
                }
            }

            XBitSet newUnionOfSubtreesAndNd = newUnionOfSubtrees
                    .unionWith(ndOfNewUnionOfSubtrees);
            ArrayList<SetAndNd> filteredSTSsAndNds = new ArrayList<>();

            if (nd.cardinality() == rootDepth) {
                for (int j=i+1; j<STSsAndNds.size(); j++) {
                    SetAndNd candidate = STSsAndNds.get(j);
                    if (!nd.equals(candidate.nd)) {
                        break;
                    }
                    if (isStsAcceptable(candidate, newPossibleSTSRoots,
                            ndOfNewUnionOfSubtrees, newUnionOfSubtreesAndNd,
                            rootDepth)) {
                        filteredSTSsAndNds.add(candidate);
                        unionOfFilteredSets.or(candidate.set);
                    }
                }
            }

            // TODO: store SetAndNd references in trie data structure
            ArrayList<SetAndNd> queryResults = visitedSTSs.query(
                    newUnionOfSubtreesAndNd, nd);
            for (SetAndNd candidate : queryResults) {
                if (isStsAcceptable(candidate, newPossibleSTSRoots,
                        ndOfNewUnionOfSubtrees, newUnionOfSubtreesAndNd,
                        rootDepth)) {
                    filteredSTSsAndNds.add(candidate);
                    unionOfFilteredSets.or(candidate.set);
                }
            }

            filteredSTSsAndNds.sort(new DescendingNdPopcountComparator());

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
                makeSTSsHelper(filteredSTSsAndNds, newPossibleSTSRoots,
                        newUnionOfSubtrees, ndOfNewUnionOfSubtrees, rootDepth,
                        newSTSsHashSet);
            }

            if (rootDepth > STSsAndNds.get(i).nd.cardinality()) {
                visitedSTSs.put(STSsAndNds.get(i));
            }
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

        for (int v=0; v<n; v++) {
            XBitSet singleVtxSTS = new XBitSet();
            singleVtxSTS.set(v);
            if (g.neighborSet[v].cardinality() < rootDepth &&
                    dom.adjVvDominatedBy[v].isEmpty() &&
                    !setRoot.containsKey(singleVtxSTS)) {
                newSTSsHashSet.add(singleVtxSTS);
                setRoot.put(singleVtxSTS, v);
            }
        }

        XBitSet emptySet = new XBitSet();
        XBitSet fullSet = new XBitSet(n);
        fullSet.set(0, n);
        makeSTSsHelper(STSsAndNds, fullSet, emptySet, emptySet, rootDepth,
                newSTSsHashSet);

        return newSTSsHashSet.stream()
                .collect(Collectors.toCollection(ArrayList::new));
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
            ArrayList<XBitSet> newSTSs = makeSTSs(STSs, i);

            if (newSTSs.size() == 0) {
                break;
            }

            STSs.addAll(newSTSs);

            int k = 0;
            for (int j=0; j<STSs.size(); j++) {
                XBitSet adjacentVv = findAdjacentVv(STSs.get(j));
                if (adjacentVv.cardinality() < i) {
                    if (STSs.get(j).cardinality() + adjacentVv.cardinality() == n) {
                        // TODO double-check this against C code
                        int[] parent = new int[n];
                        int parentVertex = -1;
                        for (int w = adjacentVv.nextSetBit(0); w >= 0;
                                w = adjacentVv.nextSetBit(w+1)) {
                            parent[w] = parentVertex;
                            parentVertex = w;
                        }
                        addParents(parent, STSs.get(j), parentVertex);
                        return new TreedepthResult(target, parent);
                    }
                    STSs.set(k++, STSs.get(j));
                }
            }
            STSs.subList(k, STSs.size()).clear();
        }
        return null;
    }

    class DescendingNdPopcountComparator implements Comparator<SetAndNd> {
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
