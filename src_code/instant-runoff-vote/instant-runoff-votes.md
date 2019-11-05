# Contributing a leetcode question

## Background
The citizens of Foolandia are voting to elect their next president. Foolandia implements
an instant runoff vote for their presidential election. On the ballot, each voter ranks
their candidates in order of preference. Top preference is indicated witha  rank of 1.

The ballots are initially counted for each voter's top choice. If a candidate has at least a majority of votes, that candidate wins. If not, the candidate with the fewest number of votes is eliminated and the voters who chose the eliminated candidate as their top choice have their votes added to the total for the candidate who is their next top choice.

If there is a tie for candidates with the fewest number of votes then all the candidates with the fewest number of votes are eliminated.

This process repeates until a candidate wins.

## Problem Statement
Given a list of candidates `candidates` and a list of ballots `ballots`, return the name of the candidate that wins the election.

## Implementation
My slightly tested implementation is very well documented in the comments. Give it a read if you're interested. I wrote four test cases. This is definitely not enough for me to state with confidence my implementation is correct. I will only say it is somewhat correct ;).

## Leetcode Submission

### Title
Winner of an instant-runoff voting election

### Description
The citizens of Foolandia are electing their next president. Foolandia runs elections using an instant-runoff voting system.

Each voter uses a ballot to rank the candidates in order of preference. A rank of 1 indicates highest preference. Initially, the ballots are counted for each voter's top choice. For example, if ballot `b` ranked candidate `c` with a 1, then `c` receives one vote. If a candidate has a majority of votes based on each voter's top choice then that candidate wins. If there is no candidate with a majority of votes, then the candidate(s) with the least number of votes are eliminated from the ballot. The voters who selected the eliminated candiate as their first choice have their votes added to the total for the candidate who is their next highest choice. This process repeats until there is a winner or there is a tie among the remaining candidates.

Help the citizens of Foolandia elect their next president by implementing a function that takes a list of candidates and a list of ballots and returns the winner(s) of the election. (Don't worry if there is a tie. Foolandia uses a different method to resolve ties in their elections).

Input:

- A list of candidates, `candidates`, where `candidates[i]` is the name of the `i`th candidate.

- A list of ballots, `ballots`, where `ballots[j]` is the ballot for the `j`th voter. A `ballot` is a list of ranks such that `ballots[j][k]` is voter `j`'s rank for `candidates[k]`.

Example 1:

Given `candidates = ["Chesterfield Appleyard", "Winston Wimplesnatch"]` and `ballots = [[1, 2], [1, 2], [2, 1]]`, `winners = ["Chesterfield Appleyard"]`.

Explanation:

In the first round of voting the first and second ballot have Appleyard as their top choice, so Appleyard received two votes. Winston received one vote from the third ballot. Appleyard wins because he received a majority of votes.

Example 2:

Given `candidates = ["Tony Jasper", "Hiram Douglas", "Monte Ashley", "Harcourt Nichols"]` and  `ballots = [[1, 2, 3, 4], [2, 3, 4, 1], [2, 4, 1, 3], [2, 1, 4, 3], [2, 1, 3, 4], [1, 2, 3, 4], [4, 2, 1, 3], [4, 3, 2, 1], [1, 3, 4, 2], [4, 1, 3, 2], [3, 2, 1, 4]]`, `winners = ["Tony Jasper"]`.

Explanation:

Round 1: Jasper, Douglas, and Ashley received three votes each. Nichols received only two votes. No candidate received a majority of votes. Nichols was the candidate with the least number of votes so they are eliminated and we proceed to the next round.

Round 2: Jasper and Ashley received four votes each. Douglas received three votes. Again, no candidate received a majority of votes. Douglas was the candidate with the least number of votes so they are eliminated and we proceed to the next round.

Round 3: Jasper received six votes and Ashley received five votes. Jasper wins the election.

### Solution

To setup, declare a set to keep track of the eliminated candidates and sort the ballots by preference. We need the ballots sorted by preference to determine which candidate to allocate votes to for each voter.

For each round of voting we
1. Tally the votes. When we count votes we exclude the candidates that are in the set of eliminated candidates.
2. Check for winners.
3. If there are winners, return the winners.
4. Otherwise add the least voted for candidates to the set of eliminated candidates and repeat.

### Testcases
Note that these are the exact text cases in `main.cc` but with sanitized candidate names ;).

Input: `candidates = ["John", "Jeff"]`, `ballots = [[1,2], [1,2]]`
Output: `winners = ["John"]`

Input: `candidates = ["John", "Jeff"]`, `ballots = [[1,2], [2,1]]`
Output: `winners = ["John", "Jeff"]`

Input: `candidates = ["John", "Jeff", "Jerry", "Jim"]`, `ballots = [[1, 2, 3, 4], [3, 2, 4, 1], [1, 3, 2, 4], [3, 2, 1, 4], [2, 3, 4, 1], [2, 1, 4, 3], [3, 1, 2, 4]]`
Output: `winners = ["Jeff"]

Input: `candidates = ["John", "Jeff", "Jerry", "Jim"]`, ` `ballots = [[1, 2, 3, 4], [2, 3, 4, 1], [2, 4, 1, 3], [2, 1, 4, 3], [2, 1, 3, 4], [1, 2, 3, 4], [4, 2, 1, 3], [4, 3, 2, 1], [1, 3, 4, 2], [4, 1, 3, 2], [3, 2, 1, 4]]`
Output: `winners = ["John"]`

