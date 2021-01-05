package bute;

import tw.exact.Graph;

class Main {
    public static void main(String[] args) {
        Graph g = Graph.readGraph(System.in);
        ButeOptions options = new ButeOptions(true, true, false);
        ButeSolver solver = new ButeSolver(g, options);
        TreedepthResult result = solver.solve();
        System.out.println(result.getDepth());
        for (int v : result.getParent()) {
            System.out.println(v + 1);
        }
    }
}
