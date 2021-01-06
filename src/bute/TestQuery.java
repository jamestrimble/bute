package bute;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;


class TestQuery {
    public static void main(String[] args) throws IOException {
        int trieType = Integer.parseInt(args[0]);
        int featureFlags = Integer.parseInt(args[1]);
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        int n = Integer.parseInt(br.readLine());
        int target = Integer.parseInt(br.readLine());
        TrieDataStructure trie = trieType == 1 ? new NewTrie(n, target) :
                new NewTrieCompressed(n, target);
        int numSets = Integer.parseInt(br.readLine());
        String input;
        for (int i=0; i<numSets; i++) {
            input = br.readLine();
            String[] sets = input.split(" ");
            String[] setSStrings = sets[0].split(",");
            String[] setNStrings = sets[1].split(",");
            FastBitSet setS = new FastBitSet(n);
            FastBitSet setN = new FastBitSet(n);
            for (String s : setSStrings) {
                setS.set(Integer.parseInt(s));
            }
            for (String s : setNStrings) {
                setN.set(Integer.parseInt(s));
            }
            trie.put(new SetAndNd(setS, setN));
        }
        while ((input = br.readLine()) != null) {
            String[] sets = input.split(" ");
            String[] setSStrings = sets[0].split(",");
            String[] setNStrings = sets[1].split(",");
            FastBitSet setS = new FastBitSet(n);
            FastBitSet setN = new FastBitSet(n);
            for (String s : setSStrings) {
                setS.set(Integer.parseInt(s));
            }
            for (String s : setNStrings) {
                setN.set(Integer.parseInt(s));
            }
            ArrayList<SetAndNd> out = trie.query(setS.unionWith(setN), setN);
            System.out.println(setS + " " + setN);
            System.out.println("----------");
            for (SetAndNd setAndNd : out) {
                System.out.println(setAndNd.set + " " + setAndNd.nd);
            }
            System.out.println();
        }
        br.close();
    }
}
