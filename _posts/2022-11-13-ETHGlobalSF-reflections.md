---
layout: post
title: Reflections on my first (Web3) Hackathon
---

Last weekend I attended [ETHSanFrancisco](https://sf.ethglobal.com/) at the Palace of Fine Arts. This was my
first ever hackathon and I have a lot to share. First, let me tell you about the project my team worked on.

## The Project - DrainPipe
Our hack was a near real-time NFT fraud alerting system called [DrainPipe](https://ethglobal.com/showcase/drainpipe-u7pmp).
The core idea is that _the sooner you know about a potential NFT wallet drain or hack, the sooner you can take action on it_. I admit this project
falls short of actually preventing your NFT from being stolen.. it has already left your wallet by the time you receive an alert.. **but**, if your
NFT was stolen there are still several useful actions you can take
* **Report the NFT** as stolen on OpenSea to make it harder for the hacker to cash out
* Track the movement of the NFT and the hacker's wallet to potentially **buy back the NFT**
* **Blacklist the NFT** if the NFT gives the holder governance power on a protocol

We used Dune's new API to query for suspicious on-chain activity. If we detected supsicious activity then we would send a text message and
discord server alert with a link to the suspicious transaction on etherscan/polygonscan.

## Reflections

### I overestimated how impressive our project was
I felt great about our project when we submitted on Sunday morning. In only 48 hours we hacked together a working MVP and live demo (no
demo faking bs ;)). I thought we would have a shot at becoming a finalist or winning the Dune sponsor prize and was bummed when we weren't selected..
until that afternoon when I saw the finalist demoes. They had clearly put a lot more effort into their projects. For some finalists it was clear
their project wasn't something they scrapped together in 48 hours like us, but rather a startup or project they were extremely serious about
continuing, and they demoed it in order to attract investors and/or new team members. A friend I met there informed me this is pretty common. Many
such cases...

I personally don't mind this and respect the hustle. But if you're about to do your first hackathon and plan to hack a project from start to finish in
48 hours then I recommend adjusting your expectations on winning accordingly. My expectations will be different for the next one.

### Hackathons are an excellent networking tool
I met a lot of interesting people working on interesting things that weekend! Attendees consisted of crypto veterans, complete noobs, and everyone in
between. It's great to know people who you can bounce ideas off of, chat about crypto, or who could even be potential cofounders down the line.

Even if you don't care about winning a prize for your hack, I recommend attending at least one hackathon for this reason. It's a great way to make friends :).

### It takes a toll on your body
I'm still relatively young but must admit I was exhausted after the hackathon. I was lucky enough to live in the same city the hackathon took place in
and could return to my bed each night. But plenty of hackers came from out of town and either slept at the venue or didn't sleep at all, and I feel for
them. That's really hard and props for hacking away under those conditions. I likely won't do another hackathon for a **long** time due to this.

## Thank you to the ETHGlobal team
I'll end by thanking the ETHGlobal team for organizing an incredible event. That's no small feat! Speaking for myself, it was an awesome experience I
won't forget and it ran quite smoothly.