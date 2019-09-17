---
layout: post
title: on the longest increasing subsequence of an array
---

Earlier this week I came across [this](https://leetcode.com/problems/increasing-triplet-subsequence/) question on leetcode.
> Given an unsorted array determine if an increasing subsequence of length 3 exists in the array.

If you're clever and familiar with leetcode style problems you may notice that this is just a special case of finding the longest increasing subsequence in an array.

Given an algorithm to find the longest increasing subsequence of an array, we can terminate early if we find one of length 3. Here is my O(n²) time, O(n) memory solution.

```cpp
int longestIncreasingSubsequence(vector<int>& nums) {
  // let lens[i] be the length of the longest increasing subsequence from nums[0]...nums[i]
  vector<int> lens(nums.size(), 1);

  /*
  We may find the longest increasing subsequence up to nums[i] by checking the length of the longest increasing subsequences up to nums[j], for j in 0...i-1, and then adding nums[i] to that subsequence.
  If nums[i] > nums[j] then we may add nums[i] to the longest subsequence ending at nums[j], because whatever the last element in that subsequence is, it is bounded above by nums[j]. We do this and check if the resulting subsequence is longer than the longest subequence up to nums[i] so far.
  */
  for (int i = 1; i < nums.size(); ++i)
    for (int j = 0; j < i; ++j)
      if (nums[i] > nums[j])
        lens[i] = max(lens[i], lens[j]+1);

  int ans = 0;
  for (auto& len: lens)
    ans = max(ans, len);

  return ans;
```

Here is the same solution modified for the triplet increasing subsequence problem on leetcode.
```cpp
bool increasingTriplet(vector<int>& nums) {
  vector<int> len(nums.size(), 1);
  for (int i = 1; i < nums.size(); ++i) {
    for (int j = 0; j < i; ++j) {
      if (nums[i] > nums[j]) {
        lens[i] = max(lens[i], lens[j]+1);
        if (lens[i] >= 3) {
          return true;
        }
      }
    }
  }
  return false;
}
```

But wait, we can do it even faster. There is an O(nlog(n)) time, O(n) solution to finding the longest increasing subsequence. Admittedly, I was unable to discover it on my own. But I spent a long time reading about it an effort to understand it thoroughly. Here it is explained in my own words.
```cpp
int longestIncreasingSubsequence(vector<int>& nums) {
  /*
  indexOfLongest[i] is the index k such that there exists a smallest 
  nums[k] that ends a longest increasing subsequence of length i.
  */
  vector<int> indexOfLongest(nums.size()+1);

  // The length of the longest increasing subsequence seen so far
  int longestSoFar = 0;
  
  /*
  Note that nums[indexOfLongest[1]], ..., nums[indexOfLongest[longestSoFar]] is an increasing subsequence.
  Thus, suppose we are going through the array from left to right, currently examining nums[i] for some i.
  We can perform a binary search to find the largest j <= longestSoFar such that nums[indexOfLongest[j]] < nums[i]
  This is the largest increasing subsequence so far to which we could add nums[i].
  */
  for (int i = 0; i < nums.size(); ++i) {
    int lo = 1; // the lower bound is the smallest possible length of an increasing subsequence (assuming we made it into the for loop, i.e. nums is nonempty)
    int hi = longestSoFar;

    while (lo <= hi) {
      int mid = lo + (hi - lo) / 2; // avoid integer overflow
      if (nums[indexOfLongest[mid]] < nums[i]) { // there is a subsequence longer than mid to which we can add nums[i].
        lo = mid+1;
      } else { // the longest subsequence to which we can add nums[i] is less than mid.
        hi = mid-1;
      }
    }
    
    /*
    The binary search terminates when lo > hi. Thus, lo is the size of the longest increasing
    subsequence we could add nums[i] to PLUS one, which is exactly the subsequence length we want.
    */
    int newLongestSoFar = lo;
    
    longestSoFar = max(longestSoFar, newLongestSoFar); // updated the longest increasing subsequence seen so far

    indexOfLongest[newLongestSoFar] = i; // nums[i] is the smallest element we could have added to a subsequence of that length so far, so we update indexOfLongest
  }

  return longestSoFar;
}
```

And with that we can generate an O(n) solution to the triplet increasing subsequence problem. Here is the modified longest increasing subsequence solution with the comments removed.
```cpp
bool increasingTriplet(vector<int>& nums) {
  vector<int> indexOfLongest(nums.size()+1);
  int longestSoFar = 0;
  
  for (int i = 0; i < nums.size(); ++i) {
    int lo = 1;
    int hi = longestSoFar;

    while (lo <= hi) {
      int mid = lo + (hi - lo) / 2;
      if (nums[indexOfLongest[mid]] < nums[i]) {
        lo = mid+1;
      } else {
        hi = mid-1;
      }
    }
    
    int newLongestSoFar = lo;
    
    longestSoFar = max(longestSoFar, newLongestSoFar);

    if (longestSoFar >= 3)
      return true;

    indexOfLongest[newLongestSoFar] = i;
  }

  return false;
}
```

## Finding a k-increasing subsequence
The above solution can trivially be generalized to finding a k increasing subsequence for some constant k. Return true if the longest subsequence seen so far reaches length k! 

### Analysis
Given an array of size n what is the time and space complexity of an optimal solution to finding a k-increasing subsequence?

#### Time Complexity
This is simple. We have n as an upper bound on iterating through the array and k as an upper bound on the number of elements on which we perform the binary search. Hence it is O(nlog(k)).

The runtime for the increasing triplet subsequence solution would be O(nlog(3)), which is really just O(n).

#### Space Complexity
This is also simple. We still need `indexOfLongest`, which uses O(n) space.

## Conclusion
If you are new to these types of problems I hope my article provided insight and understanding. There are many intersting leetcode-style problems that are variations on the longest increasing subsequence and I encourage you to check them out.
