---
layout: post
title: Poe NFT Launch!
description: "PoeNFT is the first NFT collection to feature onchain title transfers and reclaim rights. Inspired by the works of Edgar Allan Poe, it brings literary lore and novel mechanics to Base, complete with a rewards game for guessing each piece's story."
---

I'm thrilled to share that my **PoeNFT** collection has officially launched on Base!  
Explore the collection: [https://www.poenft.com/](https://www.poenft.com/)

I [announced the launch](https://warpcast.com/dgs/0x79a88c0f) on Warpcast, and it was received better than I expected—while the collection didn’t mint out entirely, it saw far more mints than I had anticipated.

![Poe NFT Launch Cast Screenshot](/assets/poe-nft-launch-cast.png)

## What makes Poe NFT special?

### 🏛️ A Novel ERC721 Implementation

Many months back, I wrote about [**ERC721Reclaimable**](https://daltyboy11.github.io/ERC721Reclaimable/), my take on implementing title transfers and the right of reclaim, as explored in the a16zcrypto article [_How NFT royalties work: Designs, challenges, and new ideas_](https://a16zcrypto.com/posts/article/how-nft-royalties-work/).

**PoeNFT** is the first collection to bring these mechanics onchain—allowing holders to transfer *title* separately from *ownership* and reclaim tokens under certain conditions.

### 🎨 The Art

Why _Poe_ NFT?  
Because every piece draws inspiration from the short stories of **Edgar Allan Poe**. Each story in his body of work is represented by one or more pieces in the collection. If you're curious, [here’s the complete collection](https://www.goodreads.com/book/show/23919.The_Complete_Stories_and_Poems) I read (and thoroughly enjoyed) last year.

As for the art itself—every image was laboriously crafted with **DALL·E**, a process that involved quite a bit of trial, error, and iteration, especially given Poe’s distinctly macabre and gothic aesthetic. I documented the experience in [_Reflections on DALL·E-3 after creating 100+ images for an NFT collection_](https://daltyboy11.github.io/dalle-reflections/).

### 🎁 A Rewards Contract

And as a final touch: **there’s a game.**

I built a **Rewards Contract** that lets you reclaim your mint fee—if you can correctly guess which story your piece represents.

The ["How It Works" section](https://www.poenft.com/rewards/how-it-works) on the site explains the mechanism. In short, it uses **ECDSA signatures** to verify correct guesses without ever revealing the title onchain—a kind of primitive zero knowledge proof. The design takes inspiration from the [**Social Dist0rtion Protocol**](https://www.dist0rtion.com/2020/01/30/Planetscape-a-dystopian-escape-game-for-36C3/), ensuring the answer stays hidden while still allowing trustless verification.

---

Thanks to all who've supported the project so far. It’s been a labor of love, and I'm excited to see where others take the title ownership concept!
