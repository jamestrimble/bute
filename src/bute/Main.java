package bute;

import java.io.*;

class Main {
    static void printHelp() {
        System.err.println("  --no-trie         Don't use trie");
        System.err.println("  --no-domination   Don't use domination");
        System.err.println("  --no-top-chain    Don't look for top chain");
        System.err.println("  --vertex-driven   Use vertex-driven variant");
        System.err.println("  --print-stats     Print stats");
    }

    public static void main(String[] args) throws IOException {
        boolean useTrie = true;
        boolean useDomination = true;
        boolean lookForTopChain = true;
        boolean vertexDriven = false;
        boolean printStats = false;
        for (String arg : args) {
            if (arg.equals("--no-trie")) {
                useTrie = false;
            } else if (arg.equals("--no-domination")) {
                useDomination = false;
            } else if (arg.equals("--no-top-chain")) {
                lookForTopChain = false;
            } else if (arg.equals("--vertex-driven")) {
                vertexDriven = true;
            } else if (arg.equals("--print-stats")) {
                printStats = true;
            } else {
                printHelp();
                System.exit(0);
            }
        }
        ButeOptions options = new ButeOptions(useTrie, useDomination,
                lookForTopChain, vertexDriven);
        ButeGraph g = ButeGraph.readGraph(System.in);
//        Graph g = Graph.readGraph(System.in);
        ButeSolver solver = new ButeSolver(g, options);
        TreedepthResult result = solver.solve();
        if (printStats) {
            System.out.println("# queries " + Stats.queries);
            System.out.println("# helperCalls " + Stats.helperCalls);
            System.out.println("# lastDecisionProblemHelperCalls " + Stats.lastDecisionProblemHelperCalls);
            System.out.println("# setCount " + Stats.setCount);
        }
        System.out.println(result.getDepth());
        for (int v : result.getParent()) {
            System.out.println(v + 1);
        }
    }
}
