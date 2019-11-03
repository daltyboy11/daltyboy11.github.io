## Background
The citizens of Foolandia are voting to elect their next president. Foolandia implements
an instant runoff vote for their presidential election. On the ballot, each voter ranks
their candidates in order of preference. Top preference is indicated witha  rank of 1.

The ballots are initially counted for each voter's top choice. If a candidate has at least a majority of votes, that candidate wins. If not, the candidate with the fewest number of votes is eliminated and the voters who chose the eliminated candidate as their top choice have their votes added to the total for the candidate who is their next top choice.

If there is a tie for candidates with the fewest number of votes then all the candidates with the fewest number of votes are eliminated.

This process repeates until a candidate wins.

## Problem Statement
Given a list of candidates `candidates` and a list of ballots `ballots`, return the name of the candidate that wins the election.
