---
layout: post
title: How many strings are there lexicographically between two strings?
---

This post was inspired by [this
problem](https://leetcode.com/problems/find-all-good-strings/).

## Problem statement
Given an alphabet _A_ and strings _S1_ and _S2_ of length _n_ such that _S1 < S2_
in [lexicographical](https://en.wikipedia.org/wiki/Lexicographical_order) order,
how many strings are there between _S1_ and _S2_?

## Reasoning with a basic example
Let's take a simple example where _A = {a, b, c}, (a < b < c)_,
_S1 = aba_, and _S2 = ccb_. We need to find all strings _S_ of
length _n_ such that _S1 < S_ and _S < S2_. We will analyze _S_ character by
character.

Pick `a` for _S[0]_. Since _S2[0]_ = `c` and `a` < `c`, any permutation possible
suffix for _S[1...n]_ will be lexicographically smaller than _S2[1...n]_.
We just have to make sure that the characters we choose don't make _S_ smaller
than _S1_ too! How do we ensure this? _S1[1]_ = `b`, so we could pick `b` or `c`
for _S[1]_. _S1[2]_ = `a`, so we could pick `a` or `b` or `c` for _S[2]_. That's
two options for the second character and three options for the third character
for a total of six possibible strings (_aba, abb, abc, aca, acb, and acc_).

If we let _S_ start with `b` we apply the same reasoning and have another
six possibilities (_bba, bbb, bba, bca, bcb, and bcc_).

What about _S[0]_ = `c`? We can't apply the same rules we used for the `a` and `b` cases
because some of the resultings strings may be larger
than _S2_. We must now fix _S[0]_ = `c` and analyze _S[1]_ similarly to how we
analyzed _S[0]_ in the `a` and `b` cases.

Pick `b` for _S[1]_ and we can choose any character for _S[2]_ greater than or equal
to `a` because _S1[2]_ = `a`. We can pick `a` or `b` or `c`, so three
possibilities (_cba, cbb, and cbc_).

Pick `c` for _S[1]_, and just like when we picked `c` for _S[0]_, we must now
fix _S[1]_ = `c` and analyze _S[2]_.

We have two possiblities for _S[2]_, `a` or `b` (_cca and ccb_).

### The grand total
Adding it all up we get `17` strings. Notice we counted
_S1_ and _S2_. If we want the strings **strictly** between _S1_ and _S2_,
that's `17 - 2 = 15` strings.

## A general formula
Let `c1` and `c2` be elements of _A_ and define the subtraction operation
`c1 - c2` as the numerical difference between `c1` and `c2`'s positions in _A_'s
ordering (this is standard character subtraction). For example, if 
_A = [a, b, c] (a < b < c)_, then _c - a = 2_, _a - b = -1_, etc.

**Case 1**: If _S[i]_ = _x_ such that _S1[i]_ <= _x_ and _x_ < _S2[i]_, then we have
S2[i+1] - S1[i+1] + 1 choices for S[i+1], S2[i+2] - S1[i+2] + 1 choices for S[i+2],
S2[i+3] - S1[i+3] + 1 choices for S[i+3], and so forth. There are _S2[i]_ - _S1[i]_ such
choices for _x_ for a total of:

_(S2[i] - S1[i]) * (1 + S2[i+1] - S1[i+1]) * ... * (1 + S2[n] - S1[n])_ strings.

**Case 2**: If _S[i]_ = _x_ such that x == _S2[i]_ then revert to case 1 for i+1

## A Scala implementation
```scala
/*
 * Returns the number of strings s such that s1 < s and
 * s < s2 lexicographically for the alphabet A. The ordering
 * is assumed to be A(i) < A(j) for all i < j.
 */
def countStrings(A: List[Char])(s1: String, s2: String): Int = {
  val ord = A.zipWithIndex.map(p => p._1 -> p._2).toMap

  // Difference in ordering between each character in s1 and the
  // last character in A
  val s1Diffs = s1 map (A.length - ord(_))

  // Keen observer will notice that this counter includes s1 but
  // not s2. Therefore we must subtract 1.
  -1 + s1.zipWithIndex.foldLeft(0) { case (count, (c, i)) =>
    val numFirstChars = ord(s2(i)) - ord(c)
    val numSuffixes = s1Diffs.drop(i+1).foldLeft(1)(_ * _)
    count + numFirstChars * numSuffixes
  }
}
```

## A Cpp implementation
For those unfamiliar with Scala (and/or functional programming) the first
implementation will look like a foreign language. Here is an imperative
solution.
```cpp
#include <vector>
#include <map>

int count_strings(const std::vector<char>& A, const std::string& s1, const std::string& s2) {
  int count = 0;
  std::map<char, int> ord;
  std::vector<int> s1Diffs( s1.length() );

  for ( int i = 0; i < A.size(); ++i ) ord[A[i]] = i;
  for ( int i = 0; i < s1.length(); ++i ) s1Diffs[i] = A.size() - ord[s1[i]];

  for ( int i = 0; i < s1.length(); ++i ) {
    const int num_first_chars = ord[s2[i]] - ord[s1[i]];
    int num_suffixes = 1;
    for ( int j = i+1; j < s1.length(); ++j ) {
      num_suffixes *= s1Diffs[j];
    }
    count += num_first_chars * num_suffixes;
  }

  return count - 1;
}
```

## Follow up questions

1. What if we allowed the lengths of s1 and s2 to differ? How do we define the
   ordering for strings of varying lengths? The [wikipedia
   page](https://en.wikipedia.org/wiki/Lexicographical_order) demonstrates two
   common definitions: pad the end of the smaller string with "blank"
   characters, where a blank character is less than any character in A.
   Alternatively we could say that a shorter string is always less than a longer
   string.

2. What if instead of counting the strings we wanted to generate all of them? We
   could accomplish this with some slight modifications to one of the
   implementations above (I think the Cpp solution would be easier to adapt).
