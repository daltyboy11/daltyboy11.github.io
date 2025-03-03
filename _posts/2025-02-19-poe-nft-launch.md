---
layout: post
title: Poe NFT Launch!
---

I'm pleased to share that my PoeNFT collection has finally launched on Base! You can see it here: https://www.poenft.com/

I made [the announcement](https://warpcast.com/dgs/0x79a88c0f) on Warpcast and it was well received. The collection wasn't fully minted but there were certainly more mints than I expected.

![Poe NFT Launch Cast Screenshot](/assets/poe-nft-launch-cast.png)


## What makes Poe NFT special?

### A novel ERC721 Implementation

Many months ago, I wrote about [ERC721Reclaimable](https://daltyboy11.github.io/ERC721Reclaimable/), my implementation of title
transfers and the right of reclaim explained in the a16zcrypto article [_How NFT royalties work: Designs, challenges, and new ideas_](https://a16zcrypto.com/posts/article/how-nft-royalties-work/). To the best of my knowledge, Poe NFT is **the first NFT collection featuring title transfers and the right of reclaim**

### The Art
It's called _Poe_ NFT because the art for every piece is inspired by the short stories of Edgar Allan Poe. Every short story has one or more pieces in the collection. [Here's](https://www.goodreads.com/book/show/23919.The_Complete_Stories_and_Poems) the complete collection of stories and poems on Goodreads, which I read last year to great enjoyment.

And for the art itself, each one was laboriously prompted to DALLE by yours truly, a process that involved much learning and iteration, especially given the macabre and gothic content of much of his stories. I wrote a post, [_Reflections on DALLE-3 after creating 100+ images for an NFT collection_](https://daltyboy11.github.io/dalle-reflections/), detailing the experience.

### A Rewards Contract

But wait... there's more! To spice things up, I deployed a Rewards contract for the collection, which you can use to refund your mint fee if you can guess which story your piece belongs to.

The site hosts a ["How It Works" section](https://www.poenft.com/rewards/how-it-works) for the Rewards Contract, explaining how the contract uses ECDSA signatures to verify the correct title without that title ever being exposed in plaintext onchain. The mechanism was inspired by the [Social Dist0rtion Protocol](https://www.dist0rtion.com/2020/01/30/Planetscape-a-dystopian-escape-game-for-36C3/).