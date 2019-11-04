#include <string>
#include <vector>
#include <set>
#include <numeric>

#include "instant-runoff-votes.hpp"

struct Comparator {
  inline bool operator()(const std::pair<unsigned int, unsigned int>& lhs, const std::pair<unsigned int, unsigned int>& rhs) const {
    return lhs.second < rhs.second;
  }
};

std::vector<std::string> election(const std::vector<std::string>& candidates, const std::vector<std::vector<unsigned int>>& ballots) {
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
