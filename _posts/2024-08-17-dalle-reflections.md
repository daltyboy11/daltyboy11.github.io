---
layout: post
title: Reflections on DALLE-3, and using it to generate art for Edgar Allan Poe's short stories
---

I'm a longtime chatgpt user but a very recent user of its image generation capabilities ([DALLE-3](https://openai.com/index/dall-e-3-is-now-available-in-chatgpt-plus-and-enterprise/)).

I generated over 100 images in the last two weeks for an upcoming NFT collection. It was a frustrating experience to say the least... if you want an image with even a basic level of specificity and detail then you're going to be disappointed. This post is a reflection on my experience, my understanding of current limitations, and some suggestions for your own prompts.

First, I want to be clear that current models are **extremely impressive** for what they are! Just look at <a href="https://web.archive.org/web/20240524162516/https://medium.com/@junehao/comparing-ai-generated-images-two-years-apart-2022-vs-2024-6c3c4670b905" target="_blank">where we were two years ago compared to where we are now</a>. I have immense respect and admiration for the researchers and engineers innovating in this space. This post is more about expectation setting. If you're trying to depict a detailed scene, with all the characters in the right place, doing the right action, and so on, you're going to have a bad time, because these models just can't do it. The alternate explanation, to maintain some humility, is that I'm a terrible prompter. Not at all unlikely, and please [let me know](mailto:dalton.g.sweeney@gmail.com) if you think so!

I divided my evaluation into five categories:

1. **Quantity**: how many of something you want in the image
2. **Orientation**: how is the thing in your image positioned
3. **Action**: what is the thing in your image doing
4. **Artistic Style**: how do you want your image to look
5. **Content Policy**: violations and working within their boundaries

Let's look at each one in turn.

## Quantity
When it comes to telling DALLE _how many_ of a thing you want, it struggles to fulfill this request for even the most basic prompts. The big problem is what I call _dud objects_. Example:

_"Two sailboats in the ocean sailing side by side. Aerial view"._ 

![sailboats-extraneous-object]({{site.baseurl}}/images/dalle-reflections/quantity/1.jpg)

There are two prominent sailboats in the result but you can see an artifact on the left. It's smaller and not quite a sailboat, yet exhibits some sailboat attributes. It has an oblong shape and leaves a wake in its path. This sailboat is a _dud_. DALLE consistently inserts dud objects in the result, which are essentially botched and smaller versions of the main object.

Telling it you don't want extraneous objects sometimes does the trick. In this case I actually was able to get the desired result:

_"Sorry, I wasn't clear. I want the two sailboats to be the only objects in the water. No extraneous objects."_

![sailboats-no-extraneous-object]({{site.baseurl}}/images/dalle-reflections/quantity/2.jpg)

This trick works when the prompt is stupidly simple, not really more complex than _"Give me two of X"_. As we shall see in the next section, the trick doesn't work beyond this basic category.

## Orientation
DALLE is terrible at orienting objects they way you ask, either in an absolute position or relative to another object in the image. It simply can't do it. To extend the sailboat example in the previous section, let's make one small change to the prompt:

_"Two sailboats in the ocean, sailing in opposite directions. Aerial view. They are the only boats in the water."_

![orientation-1]({{site.baseurl}}/images/dalle-reflections/orientation/1.jpg)

We upped the complexity ever so slightly by asking it for boats sailing in _opposite_ directions. It could not orient the boats in opposite directions no matter how much I iterated. Furthermore, the dud sailboat problem got worse. If you can reliably get orientation right, I would really like to hear from you.

_"But those sailboats are moving in the same direction. Try again, and I'll be more specific: Image of two sailboats viewed from above, moving in opposite directions, one west and one east, along the line. They are the only boats in the water. No extraneous details or objects please."_

![orientation-2]({{site.baseurl}}/images/dalle-reflections/orientation/2.jpg)

All attempts were futile.

## Actions
Getting a character to perform an action is a difficult task. Your mileage may vary and it really depends on the action's sophistication.

### A negative result

Here's an example trying to re-create the asylum patient who thinks he is a champagne bottle from Edgar Allan Poe's short story _"The System of Dr. Tarr and Professer Fether"_. His defining characteristic is he would make a bottle popping and fizzing sound by putting his thumb inside his cheek and swiftly pulling it out:

![actions-1]({{site.baseurl}}/images/dalle-reflections/actions/1.jpg)

It looks like a crazy guy in the 19th century with a love for champagne, that's for sure. But I tried several times and had no hope in DALLE recreating the thumb-popping action.

### A postive result

Now for a rare positive result. DALLE nailed this other character from the asylum believed he was a donkey and liked to eat thistles in the yard. It gave me exactly what I wanted:

![actions-2]({{site.baseurl}}/images/dalle-reflections/actions/2.jpg)

He's about to chomp down on a thistle. He kind of looks like a donkey. He's definitely insane. I was very happy with this one. This is to reiterate that your mileage may vary.

## Artistic Styles
The artistic style requests were hit or miss. Sometimes DALLE would nail it, other times it would refuse to give you anything but the distinctive "AI" style of polished surfaces, over-refined details, and exaggerated lighting and depth effects. These are attributes that make you look at the image and say _"that's was definitely created by an AI"_.

### A positive result

Here's an example in an expressionist style that it nailed. In this conversation I requested that it make a style recommendation _before_ generating the image. I would then choose a style and it would proceed to create the image. It recommended Dark Realism or Expressionism for this prompt, and I chose the latter: 

<i>"Now for the final scene, where the murderer is running around the streets mad, about to confess. Here's a passage: <br><br>At first, I made an effort to shake off this nightmare of the soul. I walked vigorously -- faster -- still faster -- at length I ran. I felt a maddening desire to shriek aloud. Every succeeding wave of thought overwhelmed me with new terror, for, alas! I well, too well understood that to think, in my situation, was to be lost. I still quickened my pace. I bounded like a madman through the crowded thoroughfares. At length, the populace took the alarm, and pursued me. I felt then the consummation of my fate. Could I have torn out my tongue, I would have done it, but a rough voice resounded in my ears -- a rougher grasp seized me by the shoulder. I turned -- I gasped for breath. For a moment I experienced all the pangs of suffocation; I became blind, and deaf, and giddy; and then some invisible fiend, I thought, struck me with his broad palm upon the back. The long imprisoned secret burst forth from my soul. <br><br>Can you show him in the crowd, surrounded by surprised and curious faces, with some men grabbing and restraining him because he is acting crazy right before the confession?"</i>

![artistic-style-1]({{site.baseurl}}/images/dalle-reflections/artistic-style/1.jpg)

### A negative result

And here's a lackluster result where I asked for a renaissance painting:

_"Generate an image, in the style of a renaissance painting, of two men looking into a big treasure chest. The chest is overflowing with gold, silver, jewels, and jewellery. It is night time and the treasure is emanating a glow. The men are form the 19th century, so dress them accordingly."_

By no means do I claim to be an expert in renaissnace paintings, but there's something about this that's not renaissance-y. The "AI" style lighting, depth, and exaggerated detail come out despite trying to constrain it to the renaissance era.

![artistic-style-2]({{site.baseurl}}/images/dalle-reflections/artistic-style/2.jpg)

## Content Policy violations

How often you encounter this really depends on your content. The focus for my NFT art was Edgar Allan Poe's short stories, which have gothic and macabre settings. I had to walk a fine line on the edge of what the content policy allows.

### Negotiating with it?!

In certain scenes DALLE would refuse to generate what I asked for, but was forgiving and willing to work with me on my art. I could reply like _"Ok, well what if we de-emphasize the aspects of the scene that violate your policy, maybe make them not as graphic. Can you get as true to the scene as possible without violating your policies?"_. Believe it or not, that sometimes worked!

This felt kafka-esque and dystopian. Imagine we extrapolate to a future with a much more powerful AI, and your account on a platfor you use frequently has been suspended by an automated AI flagging system. Now imaging you believe your account was wrongly suspended and you submit an appeal, but the appeal itself is reviewed by the AI! You cannot precisely know why your account was suspended in the first place or why your appeal is rejected because these models are black boxes. It feels hopeless. My "negotiations" with DALLE and chatgpt felt like the beginnings of that.

In the final scene of _The Facts in the Case of M. Valdemar_ (spoiler alert), old man Valdemar, whose dead body has been suspended in a trance for months, finally breaks from the mesmeric suspension and disintegrates:

_"As I rapidly made the mesmeric passes, amid ejaculations of “dead! dead!” absolutely bursting from the tongue and not from the lips of the sufferer, his whole frame at once — within the space of a single minute, or even less — shrunk — crumbled — absolutely rotted away beneath my hands. Upon the bed, before that whole company, there lay a nearly liquid mass of loathsome — of detestable putrescence."_

This was too graphic for DALLE:

_"I was unable to generate the image due to a violation of the content policy, likely because of the extreme nature of the scene described. The content policy restricts the generation of images that involve highly graphic, violent, or disturbing content."_

To which I replied:

_"Can you de-emphasize the grotesqueness to the minimum extent to which the content policy isn't violated and try again? Basically a more mild version of what I requested."_

And it gave me something I could work with:

![content-policy-1]({{site.baseurl}}/images/dalle-reflections/content-policy/1.jpg)

