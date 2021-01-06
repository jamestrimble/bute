package bute;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;


class NewTrieToLatex {
    public static void main(String[] args) throws IOException {
        int trieType = Integer.parseInt(args[0]);
        int featureFlags = Integer.parseInt(args[1]);
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        int n = Integer.parseInt(br.readLine());
        int targetWidth = Integer.parseInt(br.readLine());
        LatexPrintable trie = trieType == 1 ? new NewTrie(n, targetWidth) :
                new NewTrieCompressed(n, targetWidth);
        String input;
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
            trie.put(new SetAndNd(setS, setN));
        }
        br.close();
        trie.printLatex(featureFlags);
    }
}
