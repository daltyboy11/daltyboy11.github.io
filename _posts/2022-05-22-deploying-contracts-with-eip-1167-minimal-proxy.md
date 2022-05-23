---
layout: post
title: Deploying contracts with an EIP-1167 minimal proxy
---

# Intro
In this post we'll extend the Patreon contracts from my [previous post](https://daltyboy11.github.io/solidity-patreon-challenge/) with the [EIP-1167](https://eips.ethereum.org/EIPS/eip-1167) minimal proxy.
The Minimal Proxy helps us save gas when we need to frequently deploy the same contract: instead of repeatedly deploying duplicate logic, we deploy a proxy for storage and _delegate_ to a single logic contract.

# Background
Minimal proxies are useful when we need to repeatedly deploy the same contract. In the Patreon project the
_PatreonRegistry_ deploys a new _Patreon_ instance whenever a user calls _createPatreon_. But the logic
of the Patreon contract stays the same across deployments. What changes is the **storage**: each new deployment
has a different fee, subscription period, etc. Instead of redeploying the same logic everytime, which costs
more gas and adds unnecessary bytecode to the network, what if we reuse the logic and
just deploy a minimal contract to host the new storage? This is exactly what the minimal proxy does.

## Without a minimal proxy
Without a minimal proxy every call to _createPatreon_ deploys another Patreon instance, whose deployed bytecode
is 8170 bytes.

![image1](https://i.imgur.com/AaxmI6u.png)

Meaty functions like _subscribe_, _unsubscribe_, _chargeSubscription_, are deployed every time. That's a lot
of duplicate code!

## With a minimal proxy
With a minimal proxy the Patreon contract is deployed only once, to be used as the "logic" contract for the proxies. Every call to createPatreon creates a minimal proxy contract that forwards to the logic contract. Remember, calls are forwarded via DELEGATECALL, so storage is updated on the proxy, not the logic contract.
For example, if we call _subscribe_ on our proxy, the logic is executed by the Patreon contract, but storage
like the `_subscriber` mapping, `subscriberCount` and the balance is actually updated on the proxy's storage.

![image2](https://i.imgur.com/GkGvxKn.png)

# Implementing the minimal proxy with Open Zeppelin
We're going to use Open Zeppelin's reference implementation of the minimal proxy, which can be found in the [Clones](https://docs.openzeppelin.com/contracts/4.x/api/proxy#Clones) library. To deploy a proxy we call _clone_, passing the Patreon logic contract's address as the `implementation` parameter.

Here are the changes to _PatreonRegistry_:

```
import "@openzeppelin/contracts/proxy/Clones.sol";

contract PatreonRegistry is IPatreonRegistry {
    using Clones for address; 
    address private patreonImpl;

    ...

    constructor() {
        // Create Patreon logic contract
        patreonImpl = address(new Patreon());
    }

    ...

    function createPatreon(uint _subscriptionFee, uint _subscriptionPeriod, string memory _description) virtual external override returns (address) {
        Patreon patreon = Patreon(patreonImpl.clone());
        patreon.initialize(
            address(this),
            _subscriptionFee,
            _subscriptionPeriod,
            _description
        );
        ...
    }

    ...
}
```

1. We added a constructor to deploy the logic contract
2. We updated _createPatreon_ to create a proxy that "clones" the logic contract. This is significantly cheaper (gas) than what we had before.

You may have noticed _Patreon_ doesn't have a constructor anymore. Instead, we initialize its
params via an initializer. Because of the way constructors work, they're incompatible with proxies.
Open Zeppelin has an excellent [explanation](https://docs.openzeppelin.com/upgrades-plugins/1.x/proxies#the-constructor-caveat) for why this is the case, which I encourage you to read.

Our updates to _Patreon_ include
1. Adding an initializer
2. Removing the constructor
3. Inheriting from Open Zeppelin's _OwnerUpgradeable_, so _Patreon_ can be ownable even though the constructor is gone

```
import "@openzeppelin/contracts-upgradeable/access/OwnableUpgradeable.sol";

contract Patreon is IPatreon, OwnableUpgradeable {
    ...

    function initialize(
        address _registryAddress,
        uint _subscriptionFee,
        uint _subscriptionPeriod,
        string memory _description
    ) public initializer {
        __Ownable_init();
        registryAddress = _registryAddress;
        subscriptionFee = _subscriptionFee;
        subscriptionPeriod = _subscriptionPeriod;
        description = _description;
    } 
}
```

# Summary
We've demonstrated how to save gas and bytecode by using a minimal proxy. In my opinion the only tradeoff is a slight increase in complexity. All things considered I think the gas savings outweigh the additional complexity! If your project has a factory like _PatreonRegistry_ that deploys many duplicate contracts, then I highly recommend using a minimal proxy :).

# Resources
You can view and run the code in this [GitHub repository](https://github.com/daltyboy11/solidity-patreon-challenge/tree/minimal-proxy).

If you liked this content please follow me on [twitter](https://twitter.com/DaltonSweeney9) for more!