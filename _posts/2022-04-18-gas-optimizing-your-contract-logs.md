---
layout: post
title: Silly gas optimizations Part 1 - Logging with log0
---

_Warning: this is likely an incredibly stupid idea and not valid for the vast majority of use cases :P.
Proceed at your own risk._

# Intro
Emitting events is an indispensable feature of Solidity and the Ethereum blockchain. Events are used for 
many purposes [1]:
1. Smart contract return values for dapp frontends
2. Asynchronous triggers based on event data
3. A cheaper form of storage

In this article we'll look at how to reduce the gas cost of emitting events. We're going to look at Yul
instructions to help us go as cheap as possible on gas at the expense of pretty much _everything_ else
(hence the disclaimer for why this is a dumb idea).

## The anatomy of an event
Let's say we have a basic wallet contract for depositing/withdrawing ether and tracking balances. When there's
a withdrawal it emits a `Withdrawal` event and when there's a deposit it emits a `Deposit` event.
```solidity
//SPDX-License-Identifier: MIT
pragma solidity ^0.8.4;

contract Wallet {
    /// @notice Emitted upon successful withdrawal
    /// @param _address withdrawer address
    /// @param withdrawan amount sent to withdrawer
    /// @param remaining leftover balance
    event Withdrawal(address indexed _address, uint256 withdrawan, uint256 remaining);

    /// @notice emitted upon successful deposit
    /// @param _address depositor address
    /// @param deposited amount deposited
    /// @param available balance after deposit
    event Deposit(address indexed _address, uint256 deposited, uint256 available);

    mapping(address => uint256) private balances;

    function withdraw(uint256 amount) external {
        require(amount <= balances[msg.sender], "insufficient balance");

        balances[msg.sender] -= amount;
        payable(msg.sender).transfer(amount);

        emit Withdrawal(msg.sender, amount, balances[msg.sender]);
    }

    function deposit() external payable {
        require(msg.value > 0, "0 deposit");
        balances[msg.sender] += msg.value;
        emit Deposit(msg.sender, msg.value, balances[msg.sender]);
    }

    function getBalance(address _address)
        public
        view
        returns (uint256)
    {
        return balances[_address];
    }
}
```

Let's take a look at the raw data if we deposit 1 ether into an account with a balance of 5 ether.
What does the log object for this event look like in the ethersjs transaction receipt [2]?
```
{
    transactionIndex: 0,
    blockNumber: 2,
    transactionHash: '0xa6a74e0e35761d3af4aff69cb062ee600a077134f13fa4ac44290699818fb85e',
    address: '0x5FbDB2315678afecb367f032d93F642f64180aa3',
    topics: [
      '0x90890809c654f11d6e72a28fa60149770a0d11ec6c92319d6ceb2bb0a4ea1a15',
      '0x000000000000000000000000f39fd6e51aad88f6f4ce6ab8827279cfffb92266'
    ],
    data: '0x0000000000000000000000000000000000000000000000000de0b6b3a764000000000000000000000000000000000000000000000000000053444835ec580000',
    logIndex: 0,
    blockHash: '0x5e3b2911c40dd26648fa286ac559dae8823d1c81efe234147cede1dc7579e756'
}
```
Let's dive into the `topics` and `data` fields. What do they mean...?

## Topics
`topics` is a multi-purpose array. The first element in the topics array is the hash
of the event's signature and is used to identify the emitted event [3]:
```
// Outputs 0x90890809c654f11d6e72a28fa60149770a0d11ec6c92319d6ceb2bb0a4ea1a15
// which is the first element in the topics array
ethers.utils.keccak256(ethers.utils.toUtf8Bytes("Deposit(address,uint256,uint256)"));
```

The remaining elements are used for _indexed_ parameters. Since the `Deposit` event only has
one indexed parameter, `_address`, there is only one additional element in `topics`. In our
case `0x000000000000000000000000f39fd6e51aad88f6f4ce6ab8827279cfffb92266` is actually the
address of the depositor (recall addresses are 20 bytes which is why the first 12 bytes of
the data is 0).

## Data
You might have guessed it already, but non indexed event parameters go in the `data` field. In our case
the `deposited` and `available` amounts are concatenated together. Let's decode the first 32 bytes
```
0x0000000000000000000000000000000000000000000000000de0b6b3a7640000
```
This is a `uint256` value in hexadecimal notation. Converting `0xde0b6b3a7640000` to decimal we get
`1000000000000000000 wei` or `1 ether` for the deposited amount. Similarly, converting `0x53444835ec580000` to decimal yields
`6 ether` available balance.

## Topics cost more gas than data
Like every other EVM operation, logging costs gas, and logging topics costs more gas than logging data.
According to the yellow paper, 1 byte of log data costs 8 gas and a log topic costs 375 gas [4].
![image](https://i.imgur.com/NZoW9jr.png)
Since a topic is 32 bytes of data, storing an equivalent amount of bytes in the log `data` would cost
`32 bytes * 8 gas/byte = 256 gas`. I naturally asked myself, could we ignore the topics array to save
gas on our events? The answer is yes! We can eliminate the use of topics completely with the
`log0` Yul instruction [5]. But before we try that, a quick note on why topics are useful in the first
place so we understand what we're giving up...

### What's the point of topics?
Topic data is added to a bloom filter in the block header for efficient lookup of events in that block.
The actual event logs aren't included in a block because data storage on chain is expensive. An interested
party can use the filter to quickly check if the block contains the logs they're looking for. If it does
then they can regenerate the full logs by re-running the transactions. Without the bloom filter there's
no way to check if a block contains the logs they're looking for without actually re-running the transactions
(this is a paraphrased explanation from a well-written stack overflow post [6]). As a consequence, if we're
going to eliminate our log topics to save on gas then our logs will _no longer be searchable_.

## Manually logging Withdrawal and Deposit with log0
The EVM offers several logging instructions. Log0, Log1, ..., Log4 which log an event with 0 topics, 1 topic,
..., 4 topics respectively. We're going to use Log0 to avoid topics altogether and save gas. Here's what the
new `deposit` function looks like:

```solidity
function deposit() external payable {
    require(msg.value > 0, "0 deposit");
    balances[msg.sender] += msg.value;

    // emit event
    uint256 _balance = balances[msg.sender];
    assembly {
        // Store the identifier 0x1 immediately to the left
        // of the 20 bytes used for address
        mstore(0, xor(shl(160, 0x1), caller())) 
        // In the next 32 bytes store the amount deposited
        mstore(32, callvalue()) 
        // In the next 32 bytes store the balance after deposit
        mstore(64, _balance) 
        log0(0, 96) // log all 128 bytes of data
    }
}
```

The first instruction stores the log identifier **and** the caller's address in the first 32 bytes.
The caller's address is only 20 bytes, so we shouldn't let those extra 12 go to waste. We'll use the
the identifier `0x1` distinguish a withdrawal event.
```
mstore(0, xor(shl(160, 0x1), caller())) 
```

The next two instructions load the data we need for our event into memory.
```
mstore(32, callvalue()) 
mstore(64, _balance) 
```

The final instruction actually logs the data.
```
log0(0, 96) // log all 128 bytes of data
```

We update the `withdraw` function similarly. Instead of `0x1` we'll use `0x2` for the identifier.
```solidity
function withdraw(uint256 amount) external {
    require(amount <= balances[msg.sender], "insufficient balance");

    balances[msg.sender] -= amount;
    payable(msg.sender).transfer(amount);

    // emit event
    uint256 remaining = balances[msg.sender];
    assembly {
        // Store the identifier 0x2 immediately to the left
        // of the 20 bytes used for the address
        mstore(0, xor(shl(160, 0x2), caller())) 
        // next 32 bytes stores the amount withdrawan
        mstore(32, amount) 
        // next 32 bytes stores the remaining balance
        mstore(64, remaining) 
        log0(0, 96) // log all 128 bytes of data
    }
}
```

With those changes, we can write a (very crude) function in js to interpret our logs.
We know the structure of the data so we can slice it up and interpret accordingly:
```javascript
const getWalletEvent = (rawLog) => {
    /**
     * Parses the log data from a transaction receipt into a wallet event
     * Sample input:
     * {
        transactionIndex: 0,
        blockNumber: 2,
        transactionHash: '0x3505400ac96d7165ba95252f54c3dcbe46436efd9d25ef031dd6fb5c40968506',
        address: '0x5FbDB2315678afecb367f032d93F642f64180aa3',
        topics: [],
        data: '0x000000000000000000000001f39fd6e51aad88f6f4ce6ab8827279cfffb922660000000000000000000000000000000000000000000000004563918244f400000000000000000000000000000000000000000000000000004563918244f40000',
        logIndex: 0,
        blockHash: '0x18447452df8df7611b9c6fae418adbf414fbd1b97e3dd6f6f74a79e3514ae308'
     * }
     * Sample output:
     * {
        name: 'Deposit',
        address: 'f39fd6e51aad88f6f4ce6ab8827279cfffb92266',
        deposited: '0000000000000000000000000000000000000000000000004563918244f40000',
        available: '0000000000000000000000000000000000000000000000004563918244f40000'
     * }
     */

    // Slice the data
    const identifier = rawLog.data.slice(25, 26);
    const address = rawLog.data.slice(26, 66);
    const amount = rawLog.data.slice(66, 130);
    const balance = rawLog.data.slice(130, 194);
    if (identifier === "1") {
        return {
            name: "Deposit",
            address,
            deposited: amount,
            available: balance
        };
    } else if (identifier === "2") {
        return {
            name: "Withdrawal",
            address,
            withdrawan: amount,
            remaining: balance
        };
    } else {
        throw "Invalid identifier";
    }
}
```

## Gas comparison
In both cases we confirm the `log0` implementations use less gas [7], but only by a small amount of 733 gas.
This is a minuscule gain, but a gain nonetheless. Here is the output of my gas tests (see GitHub repo for all code)
```
WALLET1 DEPOSIT GAS USED: 28636
WALLET2 DEPOSIT GAS USED: 27903
GAS DIFFERENCE: 733
      ✔ should be cheaper to deposit with wallet2 than wallet1
WALLET1 WITHDRAW GAS USED: 36192
WALLET2 WITHDRAW GAS USED: 35459
GAS DIFFERENCE: 733
      ✔ should be cheaper to withdraw with wallet2 than wallet1
```

## Is this actually good for anything?
As you saw in the gas comparison section, the gains aren't good. But this might be useful
1. As a last resort and you're out of all other ideas
2. If you don't care about obscuring your logs and making them more difficult to search 

As I said in the introduction this is a _hyper_ optimization and is probably a bad idea in 99.99% of cases, but
at least now you know it's an option.

If you like Solidity content connect with [me](https://twitter.com/DaltonSweeney9) on twitter. That's also where I am active and receptive to feedback!

# Resources
1. Consensys article on logging and events: https://consensys.net/blog/developers/guide-to-events-and-logs-in-ethereum-smart-contracts/
2. Ethersjs TransactionReceipt type: https://docs.ethers.io/v5/api/providers/types/#providers-TransactionReceipt
3. Medium article on low level log data: https://medium.com/mycrypto/understanding-event-logs-on-the-ethereum-blockchain-f4ae7ba50378
4. Ethereum Yellow Paper: https://ethereum.github.io/yellowpaper/paper.pdf
5. The Yul intermediate language: https://docs.soliditylang.org/en/v0.8.13/yul.html
6. Bloom filters in Ethereum SO Post: https://ethereum.stackexchange.com/questions/3418/how-does-ethereum-make-use-of-bloom-filters
7. My Github repo with unit tests and gas comparison: https://github.com/daltyboy11/gas-efficient-events