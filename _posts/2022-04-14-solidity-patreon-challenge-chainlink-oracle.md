---
layout: post
title: Adding a Chainlink Oracle to our Patreon contracts
---

# Intro

In this post I'll show you how I extended the basic Patreon contracts
from my [previous post](https://daltyboy11.github.io/solidity-patreon-challenge/)
with a new feature: for each subscription period we'll waive the subscription fee
for one lucky subscriber. When the Patreon owner charges the subscription for that period,
one subscriber won't be charged. Specifically, if we have `n` subscribers then each
subscriber will have their fee waived with probability `1/n`.

We'll need an [oracle](https://www.gemini.com/cryptopedia/crypto-oracle-blockchain-overview) to introduce randomness
to our contracts. I chose Chainlink's [VRF Oracle](https://docs.chain.link/docs/chainlink-vrf/) because it's the most popular
and well documented.

# Background on Chainlink VRF
VRFv2 is a Chainlink oracle for delivering random numbers to your smart contract.

## Request -> Callback model
The oracle follows the request/callback model. Your consumer calls `requestRandomWords`
on the oracle contract. After a predetermined number of block confirmations the oracle
will call your contract's `fulfillRandomWords` function with the random numbers you requested.

![Image](https://imgur.com/a/F3aL6PC)

In our case the Consumer is a Patreon contract and the Oracle is [VRFCoordinatorV2 contract](https://rinkeby.etherscan.io/address/0x6168499c0cFfCaCD319c818142124B7A15E857ab) deployed on Rinkeby.

## VRF Subscriptions
We pay for our usage of the oracle with LINK tokens. Chainlink has
a concept of a _subscription_ which holds the LINK funds and manages
your consumers. When your consumer calls the Oracle the funds are
drawn from the subscription. The easiest way to create a subscription is
to go to https://vrf.chain.link/rinkeby. Your subscription will have an associated id.
As the subscription's owner you can add and remove consumers.
Although you can use their UI to add consumers, we're going to add consumers
programmatically from our updated patreon registry contract.

# Contract updates
We'll create v2 versions of `Patreon.sol` and `PatreonRegistry.sol` to hold our upgrades.

## PatreonV2.sol
The key change for this contract is the `chargeSubscriptions` function. The charge occurs
across separate transactions now because we have to wait for the oracle to deliver the random
numbers. We have to process the charge in the following order:
1. Initiate the charge by requesting random numbers from the oracle
2. Wait for the oracle to supply the random numbers
3. Once we have the random numbers then execute the charge

We'll introduce an enum and some additional variables to keep track of the new state and the order of actions
```solidity
enum ChargeStatus {
    INITIATE_CHARGE,
    PENDING_RANDOM_WORDS,
    EXECUTE_CHARGE
}

ChargeStatus public chargeStatus = ChargeStatus.INITIATE_CHARGE;
uint256 public _requestId;
uint256[] public _randomWords;
address[] public subscribersToCharge;
```

* `_requestId` is to keep track of which VRF request we're waiting for
* `_randomWords` is where we'll store the random numbers given to us by the oracle
* `subscribersToCharge` is where we'll store the addresses of the subscribers we're going to process in the third and final `EXECUTE_CHARGE` step

Now we'll update the `chargeSubscriptions` function to perform different actions
depending on the state
```solidity
function chargeSubscription(address[] calldata subscribers)
    external
    override
    onlyOwner
{
    require(subscriberCount > 0, "You need subscribers first lolz");
    require(chargeStatus == ChargeStatus.INITIATE_CHARGE || chargeStatus == ChargeStatus.EXECUTE_CHARGE, "Invalid chargeStatus");

    if (chargeStatus == ChargeStatus.INITIATE_CHARGE) {
        initateCharge(subscribers);
    } else if (chargeStatus == ChargeStatus.EXECUTE_CHARGE) {
        executeCharge();
    }
}
```

If the state is `INITATE_CHARGE` we'll make our request to the oracle and store
the appropriate variables for later
```solidity
function initateCharge(address[] memory subscribers) private {
    assert(chargeStatus == ChargeStatus.INITIATE_CHARGE);
    require(subscribers.length <= 500, "Exceeded VRFCoordinatorV2.MAX_NUM_WORDS");
    subscribersToCharge = subscribers;
    chargeStatus = ChargeStatus.PENDING_RANDOM_WORDS;
    _requestId = COORDINATOR.requestRandomWords(
        keyHash,
        chainlinkSubscriptionId,
        3,
        callbackGasLimit,
        uint32(subscribers.length)
    );
}
```
**Note** we are requesting a random number for _each_ subscriber to be charged. This means
we can't exceed the oracle's limit, which is 500 random numbers. After initiating the
charge we must wait for the oracle to call `fulfillRandomWords`. In our implementation
we'll simply store the random words to be used for later.

```solidity
function fulfillRandomWords(
    uint256 requestId,
    uint256[] memory randomWords
)
    internal
    override
{
    require(chargeStatus == ChargeStatus.PENDING_RANDOM_WORDS, "Invalid chargeStatus");
    require(requestId == _requestId);
    _randomWords = randomWords;
    chargeStatus = ChargeStatus.EXECUTE_CHARGE;
} 
```

Now that our status is `EXECUTE_CHARGE` we can call `chargeSubscriptions` again and
complete the charge. The `executeCharge` function is similar to the original
`chargeSubscriptions` implementation except for a new condition in the for loop to waive
the subscriber's fee:

```
function executeCharge() private {
    assert(chargeStatus == ChargeStatus.EXECUTE_CHARGE);
    assert(subscribersToCharge.length == _randomWords.length);

    for (uint i = 0; i < subscribersToCharge.length; i++) {
        Subscriber storage subscriber = _subscribers[subscribersToCharge[i]];
        // We can charge a subscriber iff `isSubscribed` == true && `lastChargedAt` + `subscriptionPeriod` >= `block.timestamp`.abi
        // Simply ignore addresses that don't match this criteria
        if (!subscriber.isSubscribed || subscriber.lastChargedAt + subscriptionPeriod > block.timestamp)
            continue;
        
        // Waive the subscription fee with 1/numSubscribers probability. On average 1 subscriber
        // per period will have their subscription waived. Note the edge case with 1 subscriber
        // Based on our formula they will have a 100% chance of having the fee waived. So instead
        // we make it a 50% chance.
        if (
            subscriberCount == 1 && _randomWords[i] % 2 == 0 ||
            subscriberCount > 1 && _randomWords[i] % subscriberCount == 0
        )
        {
            // Fee is waived for this subscriber
            emit FeeWaived(subscribersToCharge[i]);
            continue;
        }

        uint subscriptionBalanceBeforeCharge = subscriber.balance;
        if (subscriber.balance < subscriptionFee) {
            // Subscriber has insufficient funds so we transfer their balance to the owner and cancel
            // the subscription
            ownerBalance += subscriptionBalanceBeforeCharge;
            subscriber.balance = 0;
            subscriber.isSubscribed = false;
            subscriber.lastChargedAt = block.timestamp;
            subscriberCount -= 1;
            emit SubscriptionCanceled(subscribersToCharge[i], subscriptionBalanceBeforeCharge, block.timestamp);
        } else {
            // Subscriber has sufficient funds so we allocate the fee amount to the owner balance and
            // decrement it from the subscription balance.
            ownerBalance += subscriptionFee;
            subscriber.balance -= subscriptionFee;
            subscriber.lastChargedAt = block.timestamp;
        }

        emit Charged(subscribersToCharge[i], subscriptionBalanceBeforeCharge, block.timestamp);
    }

    chargeStatus = ChargeStatus.INITIATE_CHARGE;
}
```

### Understanding the fee waiver formula
I wanted to go into more detail on this snippet
```solidity
// Waive the subscription fee with 1/numSubscribers probability. On average 1 subscriber
// per period will have their subscription waived. Note the edge case with 1 subscriber
// Based on our formula they will have a 100% chance of having the fee waived. So instead
// we make it a 50% chance.
if (
    subscriberCount == 1 && _randomWords[i] % 2 == 0 ||
    subscriberCount > 1 && _randomWords[i] % subscriberCount == 0
)
{
    // Fee is waived for this subscriber
    emit FeeWaived(subscribersToCharge[i]);
    continue;
}
```
Remember `subscriberCount` represents the total number of subscribers,
which could be higher than `subscribersToCharge.length`. When we compute
`x = _randomWords[i] % subscriberCount`, there's approximately `1 / subscriberCount`
probability that `x` is `0` (I say approximately because I haven't accounted
for [modulo bias](https://research.kudelskisecurity.com/2020/07/28/the-definitive-guide-to-modulo-bias-and-how-to-avoid-it/), which is an easy "gotcha" to fall for when attempting to restrict
some random number to a desired range).

You may be confused about this method for selecting whose fee to waive.
Why don't we just randomly select an index in the `subscribersToCharge`
array and call it a day? Why do we take this probability approach where
we cna't even guarantee exactly one subscriber fee is waived per period?

The main problem is when the total number of subscribers is very large.
Suppose a Patreon contract has so many subscribers that it's impossible to
process all the subscriptions in a single transaction because it exceeds the block
gas limit. The owner has to process subscriptions in chunks. Selecting
a random index from the `subscribersToProcess` array when the array is just
a chunk of the total is a guarantee that more than one subscriber will
have their fee waved.

Now that we've seen `PatreonV2` let's draw our attention to `PatreonRegistryV2`.

## PatreonRegistryV2.sol
The oracle will reject `requestRandomWords` requests from `PatreonV2` instances unless
it's registered as a consumer for our subscription. When the Patreon registry creates
a new `PatreonV2` instance we'd like to programmatically add it as a consumer. But
only the subscription owner can do that, which is currently the EOA we used to create
the subscription on the Chainlink UI. Thankfully the VRF coordinator has functions
for transferring ownership.

We'll add the following functions to our new registry, `PatreonRegistryV2`, to accept
ownership of the subscription and also return ownership when we're done.

```solidity
contract PatreonRegistryV2 is Ownable, PatreonRegistry {
    function acceptSubscriptionOwnerTransfer(uint64 _chainlinkSubscriptionId) external onlyOwner {
        // Once this contract has accepted subscription ownership it can programatically
        // make new PatreonV2 contracts consumers. Before calling this function, owner() must
        // call COORDINATOR.requestSubscriptionOwnerTransfer specifying this contract as the
        // recipient
        COORDINATOR.acceptSubscriptionOwnerTransfer(_chainlinkSubscriptionId);
    }

    function returnSubscriptionToOwner(uint64 _chainlinkSubscriptionId) external onlyOwner {
        // Optionally the owner can transfer subscription ownership back to themselves by
        // calling this function and then calling COORDINATOR.acceptSubscriptionOwnerTransfer
        COORDINATOR.requestSubscriptionOwnerTransfer(_chainlinkSubscriptionId, owner());
    }
}
```

To execute `acceptSubscriptionOwnerTransfer` on our registry we have to first request to transfer
ownership with the coordinator. I did this by visiting the coordinator [contract on etherscan](https://rinkeby.etherscan.io/address/0x6168499c0cFfCaCD319c818142124B7A15E857ab)
and executing the `requestSubscriptionOwnerTransfer` function. The `newOwner` parameter should
be the registry instance address, and you should sign the transaction with the same EOA you
used to create the VRF subscription in the Chainlink UI. So the full steps are

1. Create a new subscription at https://vrf.chain.link/rinkeby
2. Deploy an instance of `PatreonRegistryV2` and copy the address
3. Call `requestSubscriptionOwnerTransfer` on the VRF v2 Coordinator contract at https://rinkeby.etherscan.io/address/0x6168499c0cFfCaCD319c818142124B7A15E857ab
4. Call `acceptSubscriptionOwnerTransfer` on your `PatreonRegistryV2` instance, completing the transfer.

Now all that's left is to update our `createPatreon` function to add the newly created patreon instance as a consumer.
```solidity
function createPatreon(
    uint _subscriptionFee,
    uint _subscriptionPeriod,
    string memory _description
)
    external
    override
    returns (address)
{
    PatreonV2 patreon = new PatreonV2(
        address(this),
        _subscriptionFee,
        _subscriptionPeriod,
        _description,
        chainlinkSubscriptionId
    );
    address patreonAddress = address(patreon);

    COORDINATOR.addConsumer(chainlinkSubscriptionId, patreonAddress);
    isPatreonContract[patreonAddress] = true;
    address[] storage patreons = ownerToPatreons[msg.sender];
    patreons.push(patreonAddress);
    numPatreons += 1;
    patreon.transferOwnership(msg.sender);
    emit CreatePatreon(msg.sender, patreons[patreons.length-1], block.timestamp, _description);
    return patreonAddress;
}
```

And that's everything we need! I tested the contracts on Rinkeby. My Patreon instance had once subscriber
and I charged them twice. The first time their fee was waived, and the second time it was charged! That
was a fun learning experience

# Conclusion
If you're serious about web3 development then understanding oracles and how to integrate them with your
contracts is crucial. The greatest value web3 can provide is when we can bridge on chain action to real
world data.

# Resources
1. Full source code on GitHub (see `PatreonV2.sol` and `PatreonRegistryV2.sol` contracts): https://github.com/daltyboy11/solidity-patreon-challenge
2. Chainlink VRF docs: https://docs.chain.link/docs/chainlink-vrf/