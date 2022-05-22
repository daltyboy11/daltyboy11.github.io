---
layout: post
title: Silly gas optimizations Part 2 - Rearranging your contract's function dispatcher
---

**Note 1**: This post requires an understanding of the bytecode structure of a solc compiled
contract. Please read [Deconstructing a Solidity Contract — Part III: The Function Selector](https://blog.openzeppelin.com/deconstructing-a-solidity-contract-part-iii-the-function-selector-6a9b6886ea49/) to understand how a contract's bytecode dispatches an incoming transaction to the correct function. In this post I will refer to it as the contract's _function dispatcher_.

**Note 2**: If you want to read about an even sillier and less
practical optimization then check out [Part 1](https://daltyboy11.github.io/gas-optimizing-your-contract-logs/) of this series.


# Motivation
A contract's function dispatcher checks the first four bytes of calldata against each function selector in your contract. If it finds a match it jumps to the bytecode that executes that function. If it doesn't find a match it checks the next selector (image from _Deconstructing a Solidity Contract_):

![image1](https://i0.wp.com/miro.medium.com/max/700/1*IgrF4NZNL4UNpnTKn33S1A.png?resize=700%2C299&ssl=1)

Consequently, functions whose selectors appear later cost more gas to execute because there are more failed checks before reaching the right selector. We can verify this ourselves with a toy contract (compiled on Remix):

```
// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.7.0 <0.9.0;

contract Storage {
    uint256 public x;
    function setX1(uint256 _x) external { x = _x; }
    function setX2(uint256 _x) external { x = _x; }
}
```

The implementations of `setX1` and `setX2` are identical, but executing `setX1` costs 26438 gas whereas `setX2` costs 26416 gas. This 22 gas difference stems from the fact that `setX2`'s function selector comes _before_ `setX1`'s function selector in the dispatcher:

![image2](https://i.imgur.com/LWTCRes.png)

When we call the contract to execute `setX1`, we still spend 22 gas on the 5 instructions that check the calldata against `setX2`.

# Analysis
Now is this a problem? In 99.99% of cases, I would say no! But imagine a DEX like Curve which processes 1000s of txns's per day...

A quick analysis on the most recent 100 txns on Curve's [3Pool](https://etherscan.io/address/0xbebc44782c7db0a1a60cb6fe97d0b483032ff1c7#code) shows the following function calls:
* Remove_liquidity - 56 calls (56%)
* Exchange - 40 calls (40%)
* add_liquidity - 4 calls (4%)

For the sake of simplicity let's assume these are the only three functions on the contract, and the worst case dispatch ordering: from least frequently called to most frequently called. Also suppose the implementations of Remove_liquidity, Exchange, and add_liquidity costs `X`, `Y`, and `Z` gas respectively.

Then across 1000 txns we would have the following gas costs:
```
560 * (22 + 22 + X) + 400 * (22 + Y) + 4 * Z =
560X + 400Y + 4Z + 33440
```

But if the jump table was reordered so that Remove_liquidity came first, Exchange came second, and add_liquidity came last, then the gas costs become:
```
560X + 400(22 + Y) + 4 * (22 + 22 + Z) = 
560X + 400Y + 4Z + 8976
```

A difference of 24,464 gas.

Returning to our toy contract, what if it processed thousands of txns per day, and 90% of those txns were calls to `setX1`? We could save gas by reordering the jump table: swap `setX1`'s selector and jump destination with `setX2`'s selector and jump destination:

![image3](https://i.imgur.com/ax6YtPo.png)

And with that we save 22 gas on 90% of txns.

# Drawbacks
There are a couple of huge drawbacks that make this optimization impractical for most
1. Manual bytecode maintenance - this costs time and labor. It's easy to make mistakes.
2. Inability to verify contracts - your contract's bytecode now deviates from what the compiler can produce. As a result, you can't verify your contracts on Etherscan.

# Discussion
1. I wonder if this has even been tried in the wild, e.g. a mainnet deployed contract that serves a non-trivial level of traffic
2. Does `solc` employ such optimization techniques already? There is a long list of Yul optimizations in the [Optimizer documentation](https://docs.soliditylang.org/en/v0.8.14/internals/optimizer.html#block-flattener), but I couldn't find anything specifically related to this.

# Conclusion
If you enjoy this content then consider following me on [twitter](https://twitter.com/DaltonSweeney9).
