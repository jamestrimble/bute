package bute;

import java.io.*;

class ButeGraph {
    int n;
    FastBitSet[] neighbourSets;

    ButeGraph(int n) {
        this.n = n;
        neighbourSets = new FastBitSet[n];
        for (int i=0; i<n; i++) {
            neighbourSets[i] = new FastBitSet(n);
        }
    }

    void addEdge(int v, int w) {
        neighbourSets[v].set(w);
        neighbourSets[w].set(v);
    }

    static ButeGraph readGraph(InputStream stream) throws IOException {
        BufferedReader reader = new BufferedReader(new InputStreamReader(stream));
        String s;
        int n = 0;
        int edgeCount = 0;
        ButeGraph g = null;
        while ((s = reader.readLine()) != null) {
            String[] tokens = s.split("\\s+");
            if (tokens.length < 2 || tokens[0].equals("c")) {
                continue;
            }
            if (tokens[0].equals("p")) {
                n = Integer.parseInt(tokens[2]);
                edgeCount = Integer.parseInt(tokens[3]);
                g = new ButeGraph(n);
            } else {
                int v = Integer.parseInt(tokens[0]);
                int w = Integer.parseInt(tokens[1]);
                g.addEdge(v-1, w-1);
            }
        }
        return g;
    }
}
