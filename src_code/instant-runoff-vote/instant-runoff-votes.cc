#include <string>
#include <vector>
#include <set>
#include <numeric>

struct Comparator {
  inline bool operator()(const std::pair<int, int>& lhs, const std::pair<int, int>& rhs) const {
    return lhs.second < rhs.second;
  }
};

std::vector<std::string> election(const std::vector<std::string>& candidates, const std::vector<std::vector<int>>& ballots) {
  std::set<int> eliminatedCandidates;

  while (true) {
    std::vector<int> votes(candidates.size(), 0);
    for (const std::vector<int>& ballot: ballots) {
      // sort the ballot by candidate preference
      std::vector<std::pair<int, int>> candidateRankPairs;
      for (unsigned int i = 0; i < ballot.size(); ++i) {
        candidateRankPairs.push_back(std::make_pair(i, ballot[i]));
      }
      std::sort(candidateRankPairs.begin(), candidateRankPairs.end(), Comparator());

      // find the highest ranked candidate that has not been eliminated and add vote to the count.
      for (std::pair<int, int>& candidateRankPair: candidateRankPairs) {
        const int candidate = candidateRankPair.first;
        if (eliminatedCandidates.find(candidate) != eliminatedCandidates.end())
          continue;

        votes[candidate]++;
        break;
      }
    }

    // Do we have a winner?
    const int majorityVote = std::accumulate(votes.begin(), votes.end(), 0) / 2 + 1;
    std::vector<int> leastVotedForCandidates = {0};
    for (int i = 0; i < votes.size(); ++i) {
      if (votes[i] >= majorityVote) {
        return {candidates[i]};
      }
      if (votes[i] < votes[leastVotedForCandidates[0]]) {
        leastVotedForCandidates.clear();
        leastVotedForCandidates.push_back(i);
      } else if (votes[i] == votes[leastVotedForCandidates[0]] && i != 0) { // i != 0 so we don't add the first candidate twice
        leastVotedForCandidates.push_back(i);
      }
    }

    // Check for a tie.
    if (leastVotedForCandidates.size() == (candidates.size() - eliminatedCandidates.size())) {
      std::vector<std::string> electionResults;
      for (int i: leastVotedForCandidates)
        electionResults.push_back(candidates[i]);
      return electionResults;
    }

    // If we're here then we don't have a winner.
    // Eliminate all the least voted for candidates.
    for (const int candidate: leastVotedForCandidates) {
      eliminatedCandidates.insert(candidate);
    }
  }
}
