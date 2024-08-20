---
layout: post
title: Reflections on DALLE-3, and using it to generate art for Edgar Allan Poe's short stories
---

WIP

I'm a longtime chatgpt user, but a very recent user of its image generation capabilities ([DALLE-3](https://openai.com/index/dall-e-3-is-now-available-in-chatgpt-plus-and-enterprise/)).

I generated over 100 images in the last two weeks. It was a frustrating experience to say the least. If you want an image with even a basic level of specificity and detail, you're going to end up disappointed. This post goes into the details.

But first, I want to be clear that current state-of-the-art models are extremely impressive for what they are! Just look at [where we were two years ago compared to where we are now](https://medium.com/@junehao/comparing-ai-generated-images-two-years-apart-2022-vs-2024-6c3c4670b905). I have immense respect and admiration for all the researchers and engineers working on this tech. This post is more about expectation setting. If you're trying to depict a detailed scene, with all the characters in the right place, doing the right action, etc. you're going to have a bad time, because these models just can't do it. The alternate explanation, to maintain some humility, is that I'm a terrible prompter. Not at all unlikely.

### Quantity
It struggles to fulfill a prompt that asks for a specific quantity of things. Example:

_"Two sailboats in the ocean sailing side by side. Aerial view"._ 

![sailboats-extraneous-object]({{site.baseurl}}/images/dalle-reflections/quantity/1.jpg)

There are two prominent sailboats in the result, but you can see an artifact on the left. It's smaller and not quite a sailboat, yet exhibits some attributes of a boat, like the wake it leaves in its path. DALLE consistently inserts these dud objects in the result, which are essentially botched and smaller versions of the main object.

Telling it you don't want extraneous objects _sometimes_ does the trick. In this case I actually was able to get the desired result:

_Sorry, I wasn't clear. I want the two sailboats to be the only objects in the water. No extraneous objects._

![sailboats-no-extraneous-object]({{site.baseurl}}/images/dalle-reflections/quantity/2.jpg)

This trick works when the prompt is stupidly simple, not really more complex than _"Give me two of $X"_ where X is your object. As we shall see in the next section, the trick doesn't work beyond this basic prompt category.

### Direction and orientation
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
Artistic style is another axis by which to judge DALLE.

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
