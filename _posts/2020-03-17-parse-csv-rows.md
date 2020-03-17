---
layout: post
title: Parse a CSV row allowing commas as values
---

I recently started working on my [vocab](https://github.com/daltyboy11/vocab)
side project after a three month long hiatus. The project uses basic text files
in csv format to persist data. I realized I had a bug where I was unable to
store and retrieve rows where the values themselves had commas in them. My
program was interpreting these commas not as values but as column delimiters.

One of the data records stored in *vocab* includes the definition of an English
word. An example value could be *a privilege, gain, or profit incidental to regular salary or wages* (can you guess what the word is?). Notice the multiple commas in the definition. This needs to be stored in csv format and properly retrieved. Before fixing this my program would retrieve three separate column values for *privilege*, *gain* and *or profit incidental to regular salary or wages*.

My fix wasn't difficult, nor was it trivial. The fix is mildly interesting, so
I'm sharing it here. I also submitted it for a question proposal on
[leetcode](leetcode.com). This is my fifth question submission so far. I have
yet to see any of my problems make it on to the site.

## Approach
I augmented the data format so that any values with commas are wrapped in double
quotes. For example "\"Cook, Tim\",Apple" would be the string representation of
a row where the first column value is "Cook, Tim" and the second column value is
"Apple".

If we didn't have to treat certain commas differently the solution would be
trivial: simply call a string splitting library function using the comma as the
delimiter. We can't do that here. What we can do is slightly modify the string
before calling split to avoid splitting at the commas that appear in values. I
decided to map each value comma to a replacement character. We know which commas are value commas based on if they appear wrapped inside double quotes (double quotes are not allowed to appear as values). I arbitrarily chose *$* as the replacement character.
After modifying the string we can call split on it, replace the replacement
characters with the original commas, and remove the double quotes.

Special care must be taken if the string has instances of the replacement
character BEFORE modification. We don't want to turn these into commas after the
split. For the *vocab* project, the *$* is ok because word definitions are not
allowed to have special characters in them. On the other hand, for my leetcode
question submission and provided C++ solution, I account for this case.

Here is the scala code that appears in the vocab project. It is simpler because
it doesn't need to consider if the *$* character already appears in the string.
Code is heavily documented for your enjoyment.

```scala
def parseCSVRow(s: String): Seq[String] = {
  /*
   * Returns the indices in the string s at which char occurs. Indices are in
   * sorted order
   */
  def indicesOfChar(char: Char, s: String): List[Int] =
    s.zipWithIndex.foldLeft(List.empty[Int]) { case (indices, (c, i)) =>
      if (char == c) indices :+ i else indices
    }

  /*
   * Groups adjacent elements in a list, producing a list of pairs. Throws an
   * exception if there is an odd number of elements in the supplied list.
   */
  def groupAdjacent[T](l: List[T]): List[List[T]] = l match {
    case Nil => Nil
    case x :: Nil => throw new IllegalArgumentException("odd number of elements")
    case x :: y :: lt => groupAdjacent(lt) :+ List(x, y)
  }

  val indicesOfCommas = indicesOfChar(',', s)
  val indicesOfQuotes = indicesOfChar('"', s)
  val quoteIndexPairs = groupAdjacent(indicesOfQuotes)

  val indicesOfCommasWrappedInQuotes = (indicesOfCommas filter { case index =>
    quoteIndexPairs.exists { case p =>
        index >= p(0) && index <= p(1)
    }
  }).toSet

  val replacementChar = '$'
  val sWithWrappedCommasReplaced = (s.zipWithIndex.map { case (c, i) =>
    if (indicesOfCommasWrappedInQuotes contains i)
      replacementChar
    else
      c
  }).mkString

  sWithWrappedCommasReplaced.split(",").map { case col =>
    col.map(c => if (c == replacementChar) ',' else c).filter(_ != '"')
  }
}
```

Here is the full c++ solution in my leetcode problem solution. It's a bit longer
than the scala solution because of the extra edge case and also because it's
c++.

```cpp
#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <unordered_set>

std::vector<std::string> parseCSVRow( const std::string row ) {
  if ( row.length() == 0 )
    return std::vector<std::string>();

  // Find positions of commas and double quotes in the strings
  std::vector<int> indicesOfCommas;
  std::vector<int> indicesOfDoubleQuotes;
  for ( int i = 0; i < row.size(); ++i ) {
     if ( row[i] == ',' ) {
       indicesOfCommas.push_back( i );
     } else if ( row[i] == '"' ) {
       indicesOfDoubleQuotes.push_back( i );
     }
  }

  // A quote index pair identifies a range in the string that is wrapped by
  // double quotes. The first element in the pair is the start index of the
  // range and the second element in the pair is the end index of the range.
  std::vector<std::pair<int, int>> quoteIndexPairs;
  for ( int i = 0; i < indicesOfDoubleQuotes.size(); i += 2 ) {
    quoteIndexPairs.push_back( std::make_pair( indicesOfDoubleQuotes[i], indicesOfDoubleQuotes[i+1] ) );
  }

  // Now that we have the quote wrap ranges we can determine which commas are
  // value delimiters and which commas are in the values themselves. If the
  // index of a comma is in one of the double quote ranges then it is a value,
  // otherwise it is a delimiter. Binary search on the ranges to find this
  // range.
  std::vector<int> indicesOfCommasWrappedInQuotes;
  for ( const int& index: indicesOfCommas ) {
    int lo = 0;
    int hi = quoteIndexPairs.size() - 1;
    while (lo <= hi) {
      const int mid = lo + (hi - lo)  / 2;
      if ( index >= quoteIndexPairs[mid].first && index <= quoteIndexPairs[mid].second ) {
        indicesOfCommasWrappedInQuotes.push_back( index );
        break;
      } else if ( index < quoteIndexPairs[mid].first ) {
        hi = mid - 1;
      } else {
        lo = mid + 1;
      }
    }
  }

  // Replace all value commas with a special character. We need to remember
  // which values in the string were ALREADY the replacement char to avoid
  // incorrectly overwriting them with commas after splitting the string
  const char replacementChar = '$';
  std::unordered_set<int> indicesOfPreexistingReplacements;
  for ( int i = 0; i < row.size(); ++i )
    if ( row[i] == replacementChar )
      indicesOfPreexistingReplacements.insert( i );

  std::string rowWithQuotedCommasReplaced = row;
  for ( const int commaIndex: indicesOfCommasWrappedInQuotes ) {
    rowWithQuotedCommasReplaced[commaIndex] = replacementChar;
  }

  // Manual tokenization with the comma delimiter. After we find the the token
  // we need to a) substitute commas for the replacement characters ONLY in the
  // positions where they used to be commas and b) remove any double quote
  // wrappers since these should not be part of the value itself.
  std::vector<std::string> result;
  int left_pos = 0;
  int right_pos = 0;
  for ( ; right_pos <= rowWithQuotedCommasReplaced.size(); ++right_pos ) {
    if ( right_pos == rowWithQuotedCommasReplaced.size() || rowWithQuotedCommasReplaced[right_pos] == ',' ) {
      std::string token = rowWithQuotedCommasReplaced.substr( left_pos, right_pos - left_pos );
      // Change special character back to comma
      for ( int i = 0; i < token.size(); ++i ) {
        if ( token[i] == replacementChar &&
            indicesOfPreexistingReplacements.find( i + left_pos ) == indicesOfPreexistingReplacements.end() ) {
          token[i] = ',';
        }
      }
      // Remove wrapped double quotes
      if ( token[0] == '"' && token[token.size() - 1] == '"' ) {
        token = token.substr( 1, token.size() - 2 );
      }
      result.push_back( token );
      left_pos = right_pos + 1;
    }
  }

  return result;
}
```

[Scala source code with tests](https://leetcode.com/playground/uXqrsraS)
[C++ source code with tests](https://leetcode.com/playground/39nTHFTR)

## Thanks for reading!
Have any suggestions on improving the implementations? Send me an email :).
