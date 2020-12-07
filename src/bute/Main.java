package bute;

import java.util.*;

import tw.exact.Graph;

class Main {
    public static void main(String[] args) {
        Graph g = Graph.readGraph(System.in);
        ButeSolver solver = new ButeSolver(g);
        TreedepthResult result = solver.solve();
        System.out.println(result.getDepth());
        for (int v : result.getParent()) {
            System.out.println(v + 1);
        }
    }
}
