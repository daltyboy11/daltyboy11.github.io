---
layout: post
title: Batching queries in Solidity with MultiCall
---
# Intro
[MultiCall](https://solidity-by-example.org/app/multi-call) is a contract featured in
Solidity By Example that uses the low level `staticcall` function to batch contract calls in
a single transaction. Batching is useful for speed and gas efficiency.

The contract can
be intimidating if you're not familiar with `staticcall` and function selectors, so I've
created a concrete example that shows how to call this contract in Solidity and with ethers.js.

# Modified MultiCall contract
First we'll generalize our MultiCall contract by replacing `staticcall` with `call`.
This allows us to pass ether to the contract calls. We also add a [NatSpec](https://docs.soliditylang.org/en/develop/natspec-format.html) description
to explain what the function does.

```solidity
// SPDX-License-Identifier: MIT
pragma solidity ^0.8.13;

contract MultiCall {

    error MultiCallFailed(address target, bytes payload, uint etherAmount);

    /// @notice Perform multiple calls one after another. Call `i` is sent
    /// to address `targets[i]` with calldata `payloads[i]` and ether amount
    /// `etherAmounts[i]`. The transaction fails if any call reverts.
    /// 
    /// @param targets addresses to call
    /// @param payloads calldata for each call
    /// @param etherAmounts amount of ether to send with each call
    /// @return results array where `results[i]` is the result of call `i`
    function multiCall(
        address payable[] memory targets,
        bytes[] memory payloads,
        uint[] memory etherAmounts
    )
        public
        payable
        returns (bytes[] memory results)
    {
        uint n = targets.length;
        require(payloads.length == n, "Input arrays must be the same length");
        require(etherAmounts.length == n, "Input arrays must be the same length");

        results = new bytes[](payloads.length);

        for (uint i; i < n; i++) {
            (bool ok, bytes memory res) = targets[i].call{value: etherAmounts[i]}(payloads[i]);
            if (!ok) {
                revert MultiCallFailed(targets[i], payloads[i], etherAmounts[i]);
            }
            results[i] = res;
        }
    }
}
```

# Calling MultiSigWallet
[MultiSigWallet](https://solidity-by-example.org/app/multi-sig-wallet) is another contract from
Solidity By Example. We'll use our Multicall contract to submit and confirm a multisig transaction in a single
ethereum transaction.

**Note** when I say _multisig transaction_ I'm NOT referring to a transaction on the ethereum network. I'm
referring to the `Transaction` struct in the MultiSigWallet contract.

But before we do that we need some background on Solidity calldata and function selectors...

## How does the EVM identify which function to call on a contract?
A function selector is the first four bytes of the hash of a function's prototype.
The function selector is mapped to the area of the contract's code that implements
that function. Whoa, that was a mouthful! Let's look at a specific example:
```solidity
// Compute the function selector for the MultiSigWallet function to submit a transaction
// function submitTransaction(address _to, uint _value, bytes memory _data) { ... }
string prototype = "submitTransaction(address,uint256,bytes)";
bytes4 selector = bytes4(keccak256(prototype);
```

`abi.encodeWithSelector` will combine the function selector with input parameters and return
the full payload for calling the function:
```
address to = ...;
uint value = ...;
bytes memory data = ...;

// Payload for submitTransaction
bytes payload = abi.encodeWithSelector(selector, to, value, data);
```

When you pass `payload` to `call`, `staticcall`, or `delegatecall`, the EVM will use the selector
to select the appropriate function and load the parameters into memory for function execution.

## Multicalling from Solidity
Let's add a function to our `MultiCall` contract called `submitAndConfirm`, which will
submit and confirm a multisig transaction. Here's the updated contract:
```
// SPDX-License-Identifier: MIT
pragma solidity ^0.8.13;

import "@openzeppelin/contracts/access/Ownable.sol";
import "./MultiSigWallet.sol";

contract MultiCall is Ownable {

    error MultiCallFailed(address target, bytes payload, uint etherAmount);
    
    constructor() Ownable() {}

    /// @notice Perform multiple calls one after another. Call `i` is sent
    /// to address `targets[i]` with calldata `payloads[i]` and ether amount
    /// `etherAmounts[i]`. The transaction fails if any call reverts.
    /// 
    /// @param targets addresses to call
    /// @param payloads calldata for each call
    /// @param etherAmounts amount of ether to send with each call
    /// @return results array where `results[i]` is the result of call `i`
    function multiCall(
        address payable[] memory targets,
        bytes[] memory payloads,
        uint[] memory etherAmounts
    )
        public
        payable
        returns (bytes[] memory results)
    {
        uint n = targets.length;
        require(payloads.length == n, "Input arrays must be the same length");
        require(etherAmounts.length == n, "Input arrays must be the same length");

        results = new bytes[](payloads.length);

        for (uint i; i < n; i++) {
            (bool ok, bytes memory res) = targets[i].call{value: etherAmounts[i]}(payloads[i]);
            if (!ok) {
                revert MultiCallFailed(targets[i], payloads[i], etherAmounts[i]);
            }
            results[i] = res;
        }
    }
    
    function submitAndConfirm(
        address payable multiSigAddress,
        address txRecipient,
        uint txValue,
        bytes memory txData
    ) external onlyOwner {
        // If we have access to the contract code we can
        // get the selector directly from the contract object
        bytes4 submitSelector = MultiSigWallet.submitTransaction.selector;
        bytes memory submitPayload = abi.encodeWithSelector(submitSelector, txRecipient, txValue, txData);

        // If we don't have access to the contract we can
        // derive the selector from the function prototype
        bytes4 confirmSelector = bytes4(keccak256("confirmTransaction(uint256)"));
        bytes memory confirmPayload = abi.encodeWithSelector(confirmSelector, 0);

        address payable[] memory targets = new address payable[](2);
        targets[0] = multiSigAddress;
        targets[1] = multiSigAddress;

        bytes[] memory payloads = new bytes[](2);
        payloads[0] = submitPayload;
        payloads[1] = confirmPayload;

        uint[] memory ethAmounts = new uint[](2);

        multiCall(targets, payloads, ethAmounts);
    } 
}
```

Notice we have two ways to compute the function selector. We can derive it from the
function prototype as we saw earlier. But if we have access to the MutliSigWallet source
code we can import it and access the `selector` property of the function:
```solidity
bytes4 submitSelector = MultiSigWallet.submitTransaction.selector;
```

Now we can use `submitAndConfirm` to perform a submit and confirm action in one ethereum transaction!

## Multicalling from ethers.js
We can also call `multiCall` directly from a js/ts library. We'll use [ethers.js](https://docs.ethers.io/v5/),
the most popular library.

The first step is to create an [Interface](https://docs.ethers.io/v5/api/utils/abi/interface/) instance for
the MultiSigWallet contract
```typescript
const iface = new Interface([
    "function submitTransaction(address _to, uint _value, bytes memory _data)",
    "function confirmTransaction(uint _txIndex)",
    "function executeTransaction(uint _txIndex)",
]);
```

The Interface class has a handy function called `encodeFunctionData`. We pass in the
function name and the arguments in an array, and it will return the full payload.

### Submit and Confirm
Call `encodeFunctionData` for the submit and confirm functions:
```typescript
// Submit and confirm calls for a 5 ether multisig transaction
const encodeSubmitData = iface.encodeFunctionData("submitTransaction", [
    recipientAddress, // _to
    ethers.utils.parseEther("5"), // _value
    "0x00" // _data
]);
const encodeConfirmData = iface.encodeFunctionData("confirmTransaction", [0]);
```

Then we can call our MultiCall contract with the data
```typescript
const tx = await multiCallContract.multiCall(
    // The submit and confirm calls are going to the same multiSig
    // contract so we supply the address twice
    [multiSigAddress, multiSigAddress],
    // We call submit and then confirm in that order
    [encodeSubmitData, encodeConfirmData],
    // We're assuming the multisig contract already has sufficient
    // funds for this transaction, so we don't need to send any
    // ether
    [0, 0]
);
```

And that's how we do it with ethers.js!

# Conclusion
I hope this article gave you a better understanding of how low level calling works in Solidity. If you like this content
then consider following [me](https://twitter.com/DaltonSweeney9) on Twitter, where I tweet (mostly) about Ethereum related stuff.

# Source Code
The full source code is on [GitHub](https://github.com/daltyboy11/MultiSigWallet). To play around with it yourself do the following
```
git clone git@github.com:daltyboy11/MultiSigWallet.git
git checkout multi-call
npm install
npx hardhat clean
npx hardhat test
```