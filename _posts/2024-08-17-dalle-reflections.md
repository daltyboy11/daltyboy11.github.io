---
layout: post
title: Reflections on DALLE-3, and using it to generate art for Edgar Allan Poe's short stories
---

WIP

I'm a longtime chatgpt user, but a very recent user of its image generation capabilities ([DALLE-3](https://openai.com/index/dall-e-3-is-now-available-in-chatgpt-plus-and-enterprise/)).

I generated over 100 images in the last two weeks for my upcoming NFT collection (more on that in a future post). It was a frustrating experience to say the least... if you want an image with even a basic level of specificity and detail, you're going to end up disappointed. This post is a reflection on my experience, my understanding of current limitations, and some suggestions for your own image prompts.

First, I want to be clear that current models are extremely impressive for what they are! Just look at [where we were two years ago compared to where we are now](https://medium.com/@junehao/comparing-ai-generated-images-two-years-apart-2022-vs-2024-6c3c4670b905). I have immense respect and admiration for all the researchers and engineers working innovating in this space. This post is more about expectation setting. If you're trying to depict a detailed scene, with all the characters in the right place, doing the right action, and so on, you're going to have a bad time, because these models just can't do it. The alternate explanation, to maintain some humility, is that I'm a terrible prompter. Not at all unlikely.

I divided my evaluation of its limitations into four categories
1. Quantity: how many of something you want in the image
2. Orientation: how is the thing in your image positioned
3. Action: what is the thing in your image doing
4. Artistic style
5. Content policy violations, and working within its boundaries

Let's look at each one in turn.

### Quantity
It struggles to fulfill a prompt that asks for a specific quantity of things. Example:

_"Two sailboats in the ocean sailing side by side. Aerial view"._ 

![sailboats-extraneous-object]({{site.baseurl}}/images/dalle-reflections/quantity/1.jpg)

There are two prominent sailboats in the result, but you can see an artifact on the left. It's smaller and not quite a sailboat, yet exhibits some attributes of a boat, like the wake it leaves in its path. DALLE consistently inserts these dud objects in the result, which are essentially botched and smaller versions of the main object.

Telling it you don't want extraneous objects _sometimes_ does the trick. In this case I actually was able to get the desired result:

_Sorry, I wasn't clear. I want the two sailboats to be the only objects in the water. No extraneous objects._

![sailboats-no-extraneous-object]({{site.baseurl}}/images/dalle-reflections/quantity/2.jpg)

This trick works when the prompt is stupidly simple, not really more complex than _"Give me two of $X"_ where X is your object. As we shall see in the next section, the trick doesn't work beyond this basic prompt category.

### Orientation
DALLE is terrible at orienting objects they way you ask. It simply can't do it. To extend the sailboat example in the previous section:

_Two sailboats in the ocean, sailing in opposite directions. Aerial view. They are the only boats in the water._

![orientation-1]({{site.baseurl}}/images/dalle-reflections/orientation/1.jpg)

I basically asked it for the same prompt, explicitly asking for just the two boats, but sailing in _opposite_ directions. It cannot re-orient the boats in opposite directions no matter how much you iterate. And we also see that it can't remove the extraneous objects, now that the promp is a little more complex.

<em>
But those sailboats are moving in the same direction. Try again, and I'll be more specific: 

<br>

Image of two sailboats viewed from above, moving in opposite directions, one west and one east, along the line. They are the only boats in the water. No extraneous details or objects please.
</em>


![orientation-2]({{site.baseurl}}/images/dalle-reflections/orientation/2.jpg)

### Actions
Getting a character to perform an action is a difficult task. Your mileage will vary. It really depends on the sophistication of the action. Here's an example from my art for "The System of Dr. Tarr and Professer Fether", trying to re-create the asylum patient who fancied himself as a champagne bottle, and liked to make a bottle popping sound by putting his thumb inside his cheek and swiftly pulling it out:

![actions-1]({{site.baseurl}}/images/dalle-reflections/orientation/1.jpg)

It looks like a crazy guy in the 19th century with a thing for champagne, that's for sure. But I had no hope in DALLE3 recreating the man making the popping sound.

Now, this post isn't supposed to be all negative... Here's an action that DALLE nailed. Another patient in the asylum believed he was a donkey, and liked to eat thistles in the yard. DALLE gave me exactly what I wanted.

![actions-2]({{site.baseurl}}/images/dalle-reflections/orientation/2.jpg)

He's about to chomp down on a thistle. He kind of looks like a donkey. He's definitely insane. I was very happy with this one.

### Artistic Styles
The artistic style requests were really hit or miss. Sometimes it would nail it, other times it would refuse to give you anything but the distinctive "AI" style of polished surfaces, over-refinement of details, a lot of lighting and depth effects, and other attributes that make you look at the image and say "ya, that's was definitely created by an AI".

Here's an example in an expressionist style that I was **extremely** happy with.

<em>

Now for the final scene, where the murderer is running around the streets mad, about to confess. Here's a passage:

At first, I made an effort to shake off this nightmare of the soul. I walked vigorously -- faster -- still faster -- at length I ran. I felt a maddening desire to shriek aloud. Every succeeding wave of thought overwhelmed me with new terror, for, alas! I well, too well understood that to think, in my situation, was to be lost. I still quickened my pace. I bounded like a madman through the crowded thoroughfares. At length, the populace took the alarm, and pursued me. I felt then the consummation of my fate. Could I have torn out my tongue, I would have done it, but a rough voice resounded in my ears -- a rougher grasp seized me by the shoulder. I turned -- I gasped for breath. For a moment I experienced all the pangs of suffocation; I became blind, and deaf, and giddy; and then some invisible fiend, I thought, struck me with his broad palm upon the back. The long imprisoned secret burst forth from my soul.

Can you show him in the crowd, surrounded by surprised and curious faces, with some men grabbing and restraining him because he is acting crazy right before the confession?

</em>

In this conversation I requested that chatgpt make a style recommendation before actually generating the image. It Dark Realism and Expressionism for this prompt, and I asked it for Expressionism:

![artistic-style-1]({{site.baseurl}}/images/dalle-reflections/artistic-style/1.jpg)

Here's an example with a lackluster result where I requested an image in the style of a renaissance painting:

_Generate an image, in the style of a renaissance painting, of two men looking into a big treasure chest. The chest is overflowing with gold, silver, jewels, and jewellery. It is night time and the treasure is emanating a glow. The men are form the 19th century, so dress them accordingly._

Now, I don't claim to be an expert in renaissnace paintings, but there's something about this that's not quite renaissance-esque. The lighting, the depth, and the detail look a lot like the iconic "AI" style.

![artistic-style-2]({{site.baseurl}}/images/dalle-reflections/artistic-style/2.jpg)

### Content Policy violations
How often you encounter this really depends on the content you want to generate. As Edgar Allan Poe's stories are gothic and macabre, I often pushed the envelope on this one. In certain scenes, it would refuse to generate what I asked for, citing a content policy violation. I could reply like "Ok, well can we de-emphasize the aspects of the scene that violate the policy, maybe make them not as graphic. Can you get as true to my requets as possible _without_ violating your policies?". Believe it or not, that sometimes worked!

In the final scene of _The Facts in the Case of M. Valdemar_ (spoiler alert), old man Valdemar, whose "dead" body has been suspended in a trance for months, finally breaks out of the mesmeric suspension and disintegrates:

_As I rapidly made the mesmeric passes, amid ejaculations of “dead! dead!” absolutely bursting from the tongue and not from the lips of the sufferer, his whole frame at once — within the space of a single minute, or even less — shrunk — crumbled — absolutely rotted away beneath my hands. Upon the bed, before that whole company, there lay a nearly liquid mass of loathsome — of detestable putrescence._

This was too graphic for DALLE:

_I was unable to generate the image due to a violation of the content policy, likely because of the extreme nature of the scene described. The content policy restricts the generation of images that involve highly graphic, violent, or disturbing content._

To which I replied:

_Can you de-emphasize the grotesqueness to the minimum extent to which the content policy isn't violated and try again? Basically a more mild version of what I requested._

And it gave me something I could work with:

![content-policy-1]({{site.baseurl}}/images/dalle-reflections/content-policy/1.jpg)

# Appendix of Edgar Allan Poe shor story art
If you're wondering why I generated > 100 images in the first place, it's because I wanted art for an upcoming NFT collection related to the works of Edgar Allan Poe, of whom I am a huge fan. Here is that art. Prompts aren't included because in most cases it was an iterative process of refinement, going back and forth with chatgpt.

Ok, you might be reading that last paragraph and ask yourself, "well, duh, why did you expect them to be able to...". My answer to that is ok, maybe I should have had more realistic expectations for what it could do. But I didn't, and I'm someone in tech who keeps up to speed with this stuff better than most. If my expectations were not aligned with reality, then I think others might have the same problem.