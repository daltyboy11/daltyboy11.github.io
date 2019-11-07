---
layout: post
title: Instant runoff voting and my question submission to leetcode.
---

# Introduction
I recently read about the City of New York's plans to implement instant runoff
voting (IRV) for certain elections. If you don't know what IRV is
you can read about it
[here](https://en.wikipedia.org/wiki/Instant-runoff_voting), but the gist of it
is that candidates are **ranked** on the ballot in order of preference. Vote
allocation occurs in rounds. A voter's highest preference candidate is given the
vote. If a candidate has a simple majority of votes they win. If they don't
then the candidate with the least number of votes is eliminated. If a voter's
highest preference candidate is eliminated in the next round of vote allocation
then the vote is given to the voter's highest preference candidate that has **NOT**
already been eliminated.

Interestingly a lot of countries have employed IRV in some way shape or form,
including Canada, my country of birth (although in Canada they call it an
*alternative vote*).

I thought that writing a piece of code to determine the results of an IRV
election would be an entertaining easy/medium difficulty interview question
for entry level software engineers (like the type of problem you would see on
[leetcode](https://www.leetcode.com])). I wrote up an implementation
and some test cases for it and submitted the question to leetcode as a
contribution. If my contribution is accepted then it will be available for leetcode users to try.

# Submission
My full submission, broken down into its various parts, is presented here.

## Problem Title
Winner of an instant-runoff voting election

## Description
The citizens of Foolandia are electing their next president. Foolandia
determines the result of their 
elections using an
[instant-runoff voting](https://en.wikipedia.org/wiki/Instant-runoff_voting)
system.

Each voter uses a ballot to rank the candidates in order of preference. A rank
of 1 indicates highest preference. Initially the ballots are counted for each
voter's top choice. For example, if ballot `b` ranked candidate `c` with a 1,
then `c` receives one vote. If a candidate has a majority of votes based on each
voter's top choice then that candidate wins. If there is no candidate with a
majority of votes then the candidate(s) with the least number of votes are
eliminated from the ballot. The voters who selected the eliminated candidate as
their first choice have their votes added to the total for the candidate who is
their next highest choice. This process repeats until there is a winner or
there is a tie among the remaining candidates.

Help the citizens of Foolandia elect their next president by implementing a
function that takes a list of candidates and a list of ballots and returns the
winner(s) of the election. Don't worry if there is a tie. Foolandia uses a
different method to resolve ties in their elections.

### Input
- A list `candidates`, where `candidates[i]` is the name of the `i`th
candidate.
- A list `ballots`, where `ballots[j][k]` is voter `j`'s preference for
  candidate `k`.

### Example 1
Given `candidates = ["Chesterfield Appleyard", "Winston Wimplesnatch"]` and
`ballots = [[1, 2], [1, 2], [2, 1]]`, `winners = ["Chesterfield Appleyard"]`.
#### Explanation
In the first round of voting the first and second ballot had Appleyard for
their top choice, so Appleyard received two votes. Winston received only one
vote from the third ballot. Appleyard wins because he received a majority of
votes.

### Example 2
Given `candidates = ["Tony Jasper", "Hiram Douglas", "Monte Ashley", "Harcourt
Nichols"]` and `ballots = [[1, 2, 3, 4], [2, 3, 4, 1], [2, 4, 1, 3],
[2, 1, 4, 3], [2, 1, 3, 4], [1, 2, 3, 4], [4, 2, 1, 3], [4, 3, 2, 1],
[1, 3, 4, 2], [4, 1, 3, 2], [3, 2, 1, 4]]`, `winners = ["Tony Jasper"]`.
#### Explanation
- Round 1: Jasper, Douglas, and Ashley received three votes each. Nichols
  received only two votes. No candidate received a majority of votes. Nichols
  was the candidate with the least number of votes so he is eliminated and we
  proceed to the next round.
- Round 2: Jasper and Ashley received four votes each. Douglas received three
  votes. Again, no candidate received a majority of the votes. Douglas was the
  candidate with the least number of votes so he is eliminated and we proceed
  to the next round.
- Round 3: Jasper received six votes and Ashley received five votes. Jasper wins
  the election.

## Solution
```cpp
struct Comparator {
  inline bool operator()(const std::pair<unsigned int, unsigned int>& lhs,
                         const std::pair<unsigned int, unsigned int>& rhs) const
  {
    return lhs.second < rhs.second;
  }
};

/*
Computes the winner(s) of the runoff-voting election.
*/
std::vector<std::string> election(const std::vector<std::string>& candidates,
                                  const std::vector<std::vector<unsigned int>>& ballots)
{
  std::set<unsigned int> eliminatedCandidates;
  std::vector<std::vector<std::pair<unsigned int, unsigned int>>> ballotsSortedByRank;

  // Build ballotsSortedByRank
  for ( const std::vector<unsigned int>& ballot: ballots ) {
    std::vector<std::pair<unsigned int, unsigned int>> ballotSortedByRank;
    for ( unsigned int i = 0; i < ballot.size(); ++i ) {
      ballotSortedByRank.push_back( std::make_pair( i, ballot[i] ) );
    }
    std::sort( ballotSortedByRank.begin(), ballotSortedByRank.end(), Comparator() );
    ballotsSortedByRank.push_back( ballotSortedByRank );
  }

  while ( true ) {
    // Tally the votes for this round
    std::vector<unsigned int> votes( candidates.size(), 0 );
    for ( const std::vector<std::pair<unsigned int, unsigned int>>& ballotSortedByRank: ballotsSortedByRank ) {
      for ( const std::pair<unsigned int, unsigned int>& candidateRankPair: ballotSortedByRank ) {
        const unsigned int candidate = candidateRankPair.first;
        if ( eliminatedCandidates.find( candidate ) != eliminatedCandidates.end() )
          continue;

        votes[candidate] += 1;
        break;
      }
    }

    // Check for a winner.
    const unsigned int majorityVote = std::accumulate(votes.begin(), votes.end(), 0) / 2 + 1;
    std::vector<unsigned int> leastVotedForCandidates = {0};
    for ( unsigned int i = 0; i < votes.size(); ++i ) {
      // Skip over candidates that have already been eliminated.
      if ( eliminatedCandidates.find( i ) != eliminatedCandidates.end() )
        continue;

      // Found a winner
      if ( votes[i] >= majorityVote ) {
        return {candidates[i]};
      }

      // Found a new candidate with a lower number of votes than the least voted for candidate so far.
      // This candidate is now the least voted for candidate.
      if ( votes[i] < votes[leastVotedForCandidates[0]] ) {
        leastVotedForCandidates.clear();
        leastVotedForCandidates.push_back( i );
      } else if ( votes[i] == votes[leastVotedForCandidates[0]] && i != 0 ) { // i != 0 to avoid adding the first candidate to the list twice
        // Found another candidate with the same number of votes as the least voted for candidate so far.
        // This candidate is added to the list.
        leastVotedForCandidates.push_back( i );
      }
    }

    // Check for a tie among all remaining candidates. If all candidates received the same number
    // of votes they would all techincally be in the leastVotedForCandidates vector.
    if ( leastVotedForCandidates.size() == (candidates.size() - eliminatedCandidates.size()) ) {
      std::vector<std::string> electionResults;
      for (unsigned int i: leastVotedForCandidates)
        electionResults.push_back( candidates[i] );
      return electionResults;
    }

    // There was no winner in this round. Add the least voted for candidates to the set of
    // eliminated candidates and proceed to the next round.
    for ( const unsigned int candidate: leastVotedForCandidates ) {
      eliminatedCandidates.insert( candidate );
    }
  }

  // We should never be here.
  return {""};
}
```

## Test Cases
Input: `candidates = ["John", "Jeff"]`, `ballots = [[1,2], [1,2]]`

Output: `winners = ["John"]`

<br />
Input: `candidates = ["John", "Jeff"]`, `ballots = [[1,2], [2,1]]`

Output: `winners = ["John", "Jeff"]`

<br />
Input: `candidates = ["John", "Jeff", "Jerry", "Jim"]`,
`ballots = [[1, 2, 3, 4], [3, 2, 4, 1], [1, 3, 2, 4],
[3, 2, 1, 4], [2, 3, 4, 1], [2, 1, 4, 3], [3, 1, 2, 4]]`

Output: `winners = ["Jeff"]`

<br />
Input: `candidates = ["John", "Jeff", "Jerry", "Jim"]`,
`ballots = [[1, 2, 3, 4], [2, 3, 4, 1], [2, 4, 1, 3],
[2, 1, 4, 3], [2, 1, 3, 4], [1, 2, 3, 4], [4, 2, 1, 3],
[4, 3, 2, 1], [1, 3, 4, 2], [4, 1, 3, 2], [3, 2, 1, 4]]`

Output: `winners = ["John"]`

## Tags
None

## Contributor
ElvisTheKing

# Reflections On My Submission 
As of writing this the status of the submission is pending, and my contribution
was submitted two days ago.

## Testing My Solution
I wanted to have some confidence in my code before submitting. I came up with
four test cases:

1. An election that ends in a tie after one round of voting.
2. An election with a single winner after one round of voting.
3. An election with a single winner after two rounds of voting.
4. An election with a single winner after three rounds of voting.

I learnt that it is quite tedious to write a test case for a desired number of
rounds. Even with the three-round test case it took 2-3 iterations before I had
the ballots right. Manually writing more test cases with 5+ rounds of voting
would be a nightmare given the large minimum number of ballots required.

### Candidate Names
For my submission to leetcode I used generic male names like "John" and "Jeff".
In my [source code](../src_code/instant-runoff-vote/instant-runoff-votes.cc) I used the names of real politicians ;).

## Likelihood Of Acceptance
Although my hope will never die, I expect this question will be rejected for the
following reasons:
- The problem description is long and requires more background knowledge than is
  typical for a problem on leetcode.
- The solution itself is longer than a typical problem on leetcode.
- The primary test is thoroughness and edge case coverage, rather than
  cleverness and knowledge of algorithms. This is reflected in the fact that I
  could not find any suitable tags for the question like "dynamic programming",
  "trees", etc. The typical leetcode question targets
  cleverness and algorithmic knowledge.
- As mentioned, coming up with sufficient test cases *if* the problem were to be
  accepted is arduous.

## But How Does Foolandia Resolve Ties???
The candidates vying for the presidency participate in a group fight to the
death.

# Conclusion
I now know how IRV elections work.
I got to practice my C++ skills just a little bit.
It will be nice if my contribution is accepted by leetcode.
