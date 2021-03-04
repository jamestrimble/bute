package bute;

class FastBitSet {
    // The following two constants are from the Java BitSet source
    private final static int ADDRESS_BITS_PER_WORD = 6;
    private final static int BITS_PER_WORD = 1 << ADDRESS_BITS_PER_WORD;

    private final static long LONG_MSB = 1L << (BITS_PER_WORD - 1);

    private long[] words;

    FastBitSet(int nbits) {
        words = new long[(nbits + BITS_PER_WORD - 1) >> ADDRESS_BITS_PER_WORD];
    }

    FastBitSet(FastBitSet other) {
        words = other.words.clone();
    }

    FastBitSet unionWith(FastBitSet other) {
        FastBitSet result = new FastBitSet(other);
        for (int i=0; i<words.length; i++) {
            result.words[i] |= words[i];
        }
        return result;
    }

    FastBitSet intersectWith(FastBitSet other) {
        FastBitSet result = new FastBitSet(other);
        for (int i=0; i<words.length; i++) {
            result.words[i] &= words[i];
        }
        return result;
    }

    FastBitSet subtract(FastBitSet other) {
        FastBitSet result = new FastBitSet(this);
        for (int i=0; i<words.length; i++) {
            result.words[i] &= ~other.words[i];
        }
        return result;
    }

    boolean get(int bit) {
        return 0 != (words[bit >> ADDRESS_BITS_PER_WORD] & (1L << bit));
    }

    void clear(int bit) {
        words[bit >> ADDRESS_BITS_PER_WORD] &= ~(1L << bit);
    }

    void set(int bit) {
        words[bit >> ADDRESS_BITS_PER_WORD] |= 1L << bit;
    }

    void set(int from, int to) {
        // naive implementation; could be optimised if necessary
        for (int i=from; i<to; i++) {
            set(i);
        }
    }

    int cardinality() {
        int result = 0;
        for (long word : words) {
            result += Long.bitCount(word);
        }
        return result;
    }

    int cardinalityOfDifference(FastBitSet other) {
        int result = 0;
        for (int i=0; i<words.length; i++) {
            result += Long.bitCount(words[i] & ~other.words[i]);
        }
        return result;
    }

    boolean isEmpty() {
        for (long word : words) {
            if (0 != word) {
                return false;
            }
        }
        return true;
    }

    boolean intersects(FastBitSet other) {
        for (int i=0; i<words.length; i++) {
            if (0 != (words[i] & other.words[i])) {
                return true;
            }
        }
        return false;
    }

    boolean isSuperset(FastBitSet other) {
        for (int i=0; i<words.length; i++) {
            if (0 != (~words[i] & other.words[i])) {
                return false;
            }
        }
        return true;
    }

    void and(FastBitSet other) {
        for (int i=0; i<words.length; i++) {
            words[i] &= other.words[i];
        }
    }

    void or(FastBitSet other) {
        for (int i=0; i<words.length; i++) {
            words[i] |= other.words[i];
        }
    }

    int[] toArray() {
        int[] result = new int[cardinality()];
        int j = 0;
        for (int i=0; i<words.length; i++) {
            long word = words[i];
            while (0 != word) {
                int ntz = Long.numberOfTrailingZeros(word);
                result[j++] = (i << ADDRESS_BITS_PER_WORD) + ntz;
                word ^= (1L << ntz);
            }
        }
        return result;
    }

    int compareTo(FastBitSet other) {
        for (int i=words.length; --i>=0; ) {
            long word = words[i];
            long otherWord = other.words[i];
            if (word != otherWord) {
                return Long.compareUnsigned(word, otherWord);
            }
        }
        return 0;
    }

    public boolean equals(Object other) {
        if (!(other instanceof FastBitSet)) {
            return false;
        }
        if (this == other) {
            return true;
        }
        FastBitSet o = (FastBitSet) other;
        for (int i=0; i<words.length; i++) {
            if (words[i] != o.words[i]) {
                return false;
            }
        }
        return true;
    }

    public int hashCode() {
        // this is the algorithm used by Java's BitSet
        long h = 1234;
        for (int i=words.length; --i>=0; ) {
            h ^= words[i] * (i+1);
        }
        return (int)((h >>32) ^ h);
    }
}
