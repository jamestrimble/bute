package bute;

import java.util.*;
import java.util.stream.Collectors;

import tw.exact.Graph;
import tw.exact.XBitSet;

class ButeDecisionProblemSolver {
    final Graph g;
    final int n;
    final Dom dom;
    final int target;
    final ButeOptions options;
    final HashMap<FastBitSet, Integer> setRoot = new HashMap<>();
    FastBitSet[] neighbourSets;
    static final int MIN_LEN_FOR_TRIE = 50;

    ButeDecisionProblemSolver(Graph g, Dom dom, int target, ButeOptions options) {
        this.g = g;
        this.n = g.n;
        this.dom = dom;
        this.target = target;
        this.options = options;

        neighbourSets = new FastBitSet[n];
        for (int i=0; i<n; i++) {
            neighbourSets[i] = new FastBitSet(n, g.neighborSet[i]);
        }
    }

    ArrayList<FastBitSet> getComponentsInInducedSubgraph(FastBitSet vv) {
        XBitSet vvComplement = (XBitSet) g.all.clone();
        for (int v : vv.toArray()) {
            vvComplement.clear(v);
        }
        return g.getComponents(vvComplement)
            .stream()
            .map(set -> new FastBitSet(n, set))
            .collect(Collectors.toCollection(ArrayList::new));
    }

    void tryAddingStsRoot(int w, FastBitSet unionOfSubtrees,
            FastBitSet ndOfUnionOfSubtrees, int rootDepth,
            HashSet<FastBitSet> newSTSsHashSet) {
        FastBitSet adjVv = ndOfUnionOfSubtrees
                .unionWith(neighbourSets[w])
                .subtract(unionOfSubtrees);
        adjVv.clear(w);
        if (adjVv.cardinality() < rootDepth) {
            FastBitSet STS = new FastBitSet(unionOfSubtrees);
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

    void filterRoots(FastBitSet newPossibleSTSRoots,
            ArrayList<SetAndNd> filteredSTSsAndNds,
            FastBitSet newUnionOfSubtrees,
            FastBitSet ndOfNewUnionOfSubtrees,
            int rootDepth) {
        FastBitSet unionOfFilteredSets = new FastBitSet(n);
        for (SetAndNd setAndNd : filteredSTSsAndNds) {
            unionOfFilteredSets.or(setAndNd.set);
        }
        for (int v : newPossibleSTSRoots.toArray()) {
            FastBitSet adjVv = ndOfNewUnionOfSubtrees
                    .unionWith(neighbourSets[v])
                    .subtract(newUnionOfSubtrees)
                    .subtract(unionOfFilteredSets);
            adjVv.clear(v);
            if (adjVv.cardinality() >= rootDepth ||
                    !unionOfFilteredSets.unionWith(newUnionOfSubtrees)
                    .isSuperset(dom.adjVvDominatedBy[v])) {
                newPossibleSTSRoots.clear(v);
            }
        }
    }

    // TODO figure out why C program prints different parent array of solution
    void makeSTSsHelper(ArrayList<SetAndNd> STSsAndNds,
                        FastBitSet possibleSTSRoots,
                        // TODO: rename next two params?
                        FastBitSet unionOfSubtrees,
                        FastBitSet ndOfUnionOfSubtrees,
                        int rootDepth,
                        HashSet<FastBitSet> newSTSsHashSet) {
        ++Stats.helperCalls;
        for (int w : possibleSTSRoots.toArray()) {
            tryAddingStsRoot(w, unionOfSubtrees, ndOfUnionOfSubtrees, rootDepth,
                    newSTSsHashSet);
        }

        TrieDataStructure visitedSTSs =
                options.useTrie && STSsAndNds.size() >= MIN_LEN_FOR_TRIE ?
                new STSCollection(n, rootDepth) :
                new ListOfSetAndNd();

        for (int i=STSsAndNds.size()-1; i>=0; i--) {
            FastBitSet s = STSsAndNds.get(i).set;
            FastBitSet nd = STSsAndNds.get(i).nd;
            FastBitSet newPossibleSTSRoots = possibleSTSRoots.intersectWith(nd);
            FastBitSet newUnionOfSubtrees = unionOfSubtrees.unionWith(s);
            FastBitSet ndOfNewUnionOfSubtrees = ndOfUnionOfSubtrees
                    .unionWith(nd)
                    .subtract(newUnionOfSubtrees);

            FastBitSet newUnionOfSubtreesAndNd = newUnionOfSubtrees
                    .unionWith(ndOfNewUnionOfSubtrees);

            // TODO: store SetAndNd references in trie data structure
            ArrayList<SetAndNd> filteredSTSsAndNds = visitedSTSs
                    .query(newUnionOfSubtreesAndNd, ndOfNewUnionOfSubtrees)
                    .stream()
                    .filter(item ->
                        item.nd.intersects(newPossibleSTSRoots) &&
                        ndOfNewUnionOfSubtrees.unionWith(item.nd).cardinality()
                                <= rootDepth &&
                        !newUnionOfSubtreesAndNd.intersects(item.set))
                    .collect(Collectors.toCollection(ArrayList::new));
            ++Stats.queries;

            filterRoots(newPossibleSTSRoots, filteredSTSsAndNds,
                    newUnionOfSubtrees, ndOfNewUnionOfSubtrees, rootDepth);

            if (!newPossibleSTSRoots.isEmpty()) {
                filteredSTSsAndNds.sort(new DescendingNdPopcountComparator());
                makeSTSsHelper(filteredSTSsAndNds, newPossibleSTSRoots,
                        newUnionOfSubtrees, ndOfNewUnionOfSubtrees, rootDepth,
                        newSTSsHashSet);
            }
            visitedSTSs.put(STSsAndNds.get(i));
        }
    }

    ArrayList<FastBitSet> makeSTSs(ArrayList<SetAndNd> STSsAndNds, int rootDepth) {
        HashSet<FastBitSet> newSTSsHashSet = new HashSet<>();
        FastBitSet emptySet = new FastBitSet(n);
        FastBitSet fullSet = new FastBitSet(n);
        fullSet.set(0, n);
        makeSTSsHelper(STSsAndNds, fullSet, emptySet, emptySet, rootDepth,
                newSTSsHashSet);
        return new ArrayList<>(newSTSsHashSet);
    }

    ArrayList<FastBitSet> makeSTSsVertexDriven(ArrayList<SetAndNd> STSsAndNds, int rootDepth) {
        HashSet<FastBitSet> newSTSsHashSet = new HashSet<>();
        for (int i=0; i<n; i++) {
            FastBitSet emptySet = new FastBitSet(n);
            final int i_ = i;
            ArrayList<SetAndNd> filteredSets = STSsAndNds
                .stream()
                .filter(item -> item.nd.get(i_) &&
                        !item.nd.intersects(dom.adjVvDominatedBy[i_]))
                .collect(Collectors.toCollection(ArrayList::new));
            FastBitSet possibleRoot = new FastBitSet(n);
            possibleRoot.set(i);
            makeSTSsHelper(filteredSets, possibleRoot, emptySet, emptySet, rootDepth,
                    newSTSsHashSet);
        }

        return new ArrayList<>(newSTSsHashSet);
    }

    FastBitSet findAdjacentVv(FastBitSet s) {
        FastBitSet unionOfNds = new FastBitSet(n);
        for (int v : s.toArray()) {
            unionOfNds.or(neighbourSets[v]);
        }
        return unionOfNds.subtract(s);
    }

    void addParents(int[] parent, FastBitSet s, int parentVertex) {
        int v = setRoot.get(s);
        parent[v] = parentVertex;
        FastBitSet descendants = new FastBitSet(s);
        descendants.clear(v);
        for (FastBitSet component : getComponentsInInducedSubgraph(descendants)) {
            addParents(parent, component, v);
        }
    }

    TreedepthResult solve() {
        ArrayList<FastBitSet> STSs = new ArrayList<>();

        for (int i=target; i>=1; i--) {
            int prevSetRootSize = setRoot.size();

            // TODO: always keep sets together with their neighbourhoods,
            // avoiding the need to package them up here?
            ArrayList<SetAndNd> STSsAndNds = STSs
                    .stream()
                    .map(STS -> new SetAndNd(STS, findAdjacentVv(STS)))
                    .sorted(new DescendingNdPopcountComparator())
                    .collect(Collectors.toCollection(ArrayList::new));
            STSs = options.vertexDriven ?
                    makeSTSsVertexDriven(STSsAndNds, i) :
                    makeSTSs(STSsAndNds, i);

            if (setRoot.size() == prevSetRootSize) {
                break;
            }

            Stats.setCount += setRoot.size() - prevSetRootSize;

            if (i == 1) {
                // This case is necessary if the graph is disconnected
                int totalSize = 0;
                for (FastBitSet STS : STSs) {
                    totalSize += STS.cardinality();
                }
                if (totalSize < n) {
                    return null;
                }
                int[] parent = new int[n];
                for (FastBitSet STS : STSs) {
                    addParents(parent, STS, -1);
                }
                return new TreedepthResult(target, parent);
            } else if (options.lookForTopChain) {
                // TODO: perform this check each time an STS is found, in order
                // to return the result more quickly
                for (FastBitSet STS : STSs) {
                    FastBitSet adjacentVv = findAdjacentVv(STS);
                    if (STS.cardinality() + adjacentVv.cardinality() == n) {
                        int[] parent = new int[n];
                        int parentVertex = -1;
                        for (int w : adjacentVv.toArray()) {
                            parent[w] = parentVertex;
                            parentVertex = w;
                        }
                        addParents(parent, STS, parentVertex);
                        return new TreedepthResult(target, parent);
                    }
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
