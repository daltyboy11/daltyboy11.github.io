---
layout: post
title: Announcing EasyBridge, a USDC Bridging Tool
---

I'm excited to share https://easybridge.io. EasyBridge is an easy to use, self-serve interface for bridging USDC across EVM chains.

### Why did I build it?
As someone who frequently bridges USDC across chains, I wanted a tool that was:

- **Trustworthy**: The only point of trust for EasyBridge is [Circle](https://www.circle.com/en/cross-chain-transfer-protocol), which is the company that created USDC. Therefore, the smart contract risk and trust assumptions are limited to Circle and only Circle.
- **Intuitive**: The interface is easy to use for any crypto-native user.
- **Displays my history**: I like to see my bridging history all in one place. EasyBridge does this extremely well.

Judging on these criteria I wasn't satisfied with existing tools so I decided to make my own.

### How does it work?
EasyBridge interfaces with Circle's Cross Chain Transfer Protocol, or CCTP for short. There are three high level steps:
1. Deposit your USDC in the bridge on the source chain, and choose the recipient for the destination chain. Circle burns the USDC on the source chain.
2. Wait for Circle to generate the _attestation_. The attestation is the permission slip to withdraw the USDC on the destination chain. This takes ~20 minutes.
3. Withdraw the USDC on the destination chain using the attestation from step 2. The USDC is minted to the recipient address.

For a more detailed explanation please look at EasyBridge's [how it works page](https://easybridge.io/how-it-works), which has diagrams and a demo video!

### What is the future of EasyBridge?
There are several ways I could improve EasyBridge. Some ideas are:

- **Expanding to more chains - to the EVM and beyond!**
    
    Circle has recently added support for Celo and Solana. The more chains EasyBridge supports, the more useful it is!

- **Swaps**.

    Imagine if instead of bridging USDC ==> USDC, you could bridge any ERC20 to _any other_ ERC20? This is possible as long as the ERC20s have good ERC20 <==> USDC liquidity on the source and destination chains. Let's say I wanted to swap `$GFI` on mainnet to `$DEGEN` on Base. EasyBridge could intelligently swap `$GFI` ==> `USDC` on mainnet using Uniswap, deposit the USDC into the bridge, and then swap `USDC` ==> `$DEGEN` on Base using BaseSwap. This improvement would promote EasyBridge to a fully fledged bridge, without _any additional infrastructure_ 🤯.

### EasyBridge's hidden feature
EasyBridge isn't only great for moving USDC across chains. You can also use it to avoid the long wait times for moving your ETH from an L2 back to mainnet. On Optimism, it takes 7 days to move your ETH back to mainnet because you have to wait for the fraud proof window to close. The same wait time applies to Arbitrum and Base. With EasyBridge, **you can do this in 20 minutes**! That's a 500-fold decrease in time.

### Feedback
Have feedback, criticisms, or ideas for improvement? Please don't hesitate to [reach out](mailto:dalton.g.sweeney@gmail.com)!
