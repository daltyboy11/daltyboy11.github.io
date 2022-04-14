---
layout: post
title: Challenge: write a basic Patreon clone in Solidity
---

Inspired by various challenges and tutorials like [ethernauts](https://ethernaut.openzeppelin.com/)
and [speedrunethereum](https://speedrunethereum.com/), I have created my own.

The [Solidity Patreon Challenge](https://github.com/daltyboy11/solidity-patreon-challenge) is a challenge
to create a (very basic) clone of Patreon in Solidity. The specification allows users to:
- Create a new Patreon with a subscription fee and subscription period
- Charge the fee to their subscribers 
- Subscribe/unsubscribe to a Patreon

I have provided two interfaces with full NatSpec descriptions: `IPatreon.sol` and `IPatreonRegistry.sol`.
The goal is to implement these interfaces and pass all the unit tests. Best of luck!