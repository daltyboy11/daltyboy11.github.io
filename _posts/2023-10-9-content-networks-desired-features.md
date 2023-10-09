---
layout: post
title: Desired features in P2P Content Networks
---

A few weeks ago, my curiosity was piqued by [XMTP](https://xmtp.org/), a peer-to-peer (p2p) messaging protocol. For fun, I developed two self-contained web apps: [zodiacdaily.xyz](https://zodiacdaily.xyz/) and [onchainfacts.xyz](https://onchainfacts.xyz/). The concept behind these apps is straightforward: send a small amount of USDC and receive a horoscope or three fun facts in return. Feel free to try them out.

While I crafted these projects for amusement, many serious applications are being built on XMTP. [Converse](https://converse.xyz/) is a dedicated messaging app. [Coinbase Wallet](https://help.coinbase.com/en/wallet/messaging/info) has an inbox specifically for XMTP messages. The beauty of XMTP being an open protocol is its seamless integration - you can use Converse to send your friend a message and they could reply from their Coinbase wallet. Clearly many people (at least in crypto) see the value in a decentralized p2p messaging network. It's worth noting, however, that XMTP is still on its path to full decentralization.

I resonate with the importance of XMTP and view it as a subset of p2p _content_ networks. While XMTP's forte is private messaging, content networks are not restricted to private messaging in general. [Mirror](https://mirror.xyz/) serves as a p2p blogging and publishing network, with its content stored on [Arweave](https://www.arweave.org/). [Farcaster](https://github.com/farcasterxyz/protocol) is working on decentralized social media. These are but a few notable mentions in a growing ecosystem.

Why my enthusiasm for p2p content networks? They are champions of free speech. Balaji says it [eloquently](https://thenetworkstate.com/if-the-news-is-fake-imagine-history) in his book, The Network State: when content is uncensorable and stems directly from ground sources, it becomes a challenge for dominant entities to manipulate narratives.

Yet, the ecosystem is far from mature. Challenges like spam highlight its infancy. My firsthand experience with this was the deluge of phishing attempts I encountered when I started testing my Zodiac and Fun Facts apps. As soon as my address became active on XMTP I saw a neverending stream of spam messages.

![Coinbase Wallet Spam]({{site.baseurl}}/images/cb-wallet-spam.PNG)

![Converse Spam]({{site.baseurl}}/images/converse-spam.PNG)

I suspect XMTP isn't alone in this struggle. Reflecting on these challenges, here's a wishlist for p2p content networks:

1. **Anti-spam Filters**: As we just saw, users will quickly give up if they are bombarded with a dozen spam messages for every real one.
2. **Content Authenticity Verification**: With advancements in generative AI, discerning real from fake/generated content is increasingly difficult. In a network championing free speech and resistant to censorship, mechanisms to verify content authenticity are critical.
3. **Dynamic Resource Allocation**: In p2p networks, nodes collaboratively distribute content. However, not all content is equally in demand. Dynamic resource allocation entails the network's adaptability in channeling more resources towards trending or pivotal content, increasing its accessibility.

If you're involved in addressing these challenges, or if you've identified other gaps in p2p content networks, I'd love to hear more. Connect with me at dgssgd.cb.id via XMTP!