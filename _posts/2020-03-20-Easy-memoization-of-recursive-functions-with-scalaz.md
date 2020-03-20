---
layout: post
title: Easy Memoization of recursive functions using Scalaz
---

[Ninety-Nine Scala Problems](http://aperiodic.net/phil/scala/s-99/) is an
excellent problem set to work throught if you want to sharpen your scala
programming skills as well as brush up on your understanding of data structures
and algorithms.

I've been slowly chipping away at the problems, and I found
[P59](http://aperiodic.net/phil/scala/s-99/p59.scala), constructing all height
balanced binary trees for a given height, to be a nice case study on memoization. In this
post I show you my initial implementation and how it does redundant work, and
then show you how I used scalaz's
[Memo](https://github.com/scalaz/scalaz/blob/series/7.1.x/core/src/main/scala/scalaz/Memo.scala) to speed it up.

## The Problem
*In a height-balanced binary tree, the following property holds for every node: The height of its left subtree and the height of its right subtree are almost equal, which means their difference is not greater than one.*

*Write a method Tree.hbalTrees to construct height-balanced binary trees for a
given height with a supplied value for the nodes. The function should generate
all solutions.*

```
scala> Tree.hbalTrees(3, "x")
res0: List[Node[String]] = List(T(x T(x T(x . .) T(x . .)) T(x T(x . .) T(x . .))), T(x T(x T(x . .) T(x . .)) T(x T(x . .) .)), ...
```

## Approach
Given a balanced tree of height *n*, we have the following cases for the height
of its subtrees:
1. The left subtree has height *n - 1* and the right subtree has height *n - 1*
2. The left subtree has height *n - 2* and the right subtree has height *n - 1*
3. The left subtree has height *n - 1* and the right subtree has height *n - 2*

For the base cases, we return a single node for height 0 and the three possible
balanced trees for height 1:

```
  o    o         o
 /      \       / \
o   ,    o  ,  o   o

```

In the recursive case we generate the trees of height *n-1* and *n-2*. We take
all possible left/right subtree pairs from these two lists, excluding pairs
where both subtrees have height *n-2* (because these trees are already generated
in the recursive call for *n-1*). Here is  the full implementation.

```scala
def hBalTrees[T](height: Int, value: T): List[Tree[T]] = height match {
  case 0 => List(Node(value, End, End))
  case 1 => List(
              Node(value, Node(value, End, End), End),
              Node(value, End, Node(value, End, End)),
              Node(value, Node(value, End, End), Node(value, End, End)))
  case n => {
    val nLess1Trees = hBalTrees(height-1, value)
    val nLess2Trees = hBalTrees(height-2, value)
    val allTrees = nLess2Trees ++ nLess1Trees
    for {
      (t1, i1) <- allTrees.zipWithIndex
      (t2, i2) <- allTrees.zipWithIndex if (!(i1 < nLess2Trees.length && i2 < nLess2Trees.length))
    } yield {
      Node(value, t1, t2)
    }
  }
}
```

## Duplicate work
Can you see the glaring inefficiency in the implementation? We're calling the
function on *n - 1* and *n - 2*. Suppose we start at *n = 5*. We recursively
solve for *n = 4* and *n = 3*. When *n = 4* we recursively solve for *n = 3*
(**DUPLICATE**) and *n = 2*, and so on.

The recurrence relation for this function is *T(n) = T(n-1) + T(n-2) + O(n^2)*,
which is at least as slow as exponential runtime (the detailed analysis is not
important for our discussion).

To fix this we need to cache the values returned from recursive calls. Scalaz
provides a trait that makes this trivial.

## Scalaz - Memo
Let's look at the definition of the *Memo* trait in the Scalaz library
```scala
sealed trait Memo[@specialized(Int) K, @specialized(Int, Long, Double) V] {
  def apply(z: K => V): K => V
}
```
It consumes a function from *K* to *V* and produces another function from *K* to
*V*. We can use Memo to create a function from a tree height (*Int*) to a list
of balanced trees with that height (*List[Tree[T]]*).

```scala
def hBalTreesMemo[T](value: T): Int => List[Tree[T]] =
    Memo.immutableHashMapMemo[Int, List[Tree[T]]] {
      case 0 => List(Node(value, End, End))
      case 1 => List(
        Node(value, Node(value, End, End), End),
        Node(value, End, Node(value, End, End)),
        Node(value, Node(value, End, End), Node(value, End, End)))
      case n => {
        val nLess1Trees = hBalTreesMemo(value)(n-1)
        val nLess2Trees = hBalTreesMemo(value)(n-2)
        val allTrees = nLess2Trees ++ nLess1Trees
        for {
          (t1, i1) <- allTrees.zipWithIndex
          (t2, i2) <- allTrees.zipWithIndex if (!(i1 < nLess2Trees.length && i2 < nLess2Trees.length))
        } yield {
          Node(value, t1, t2)
        }
      }
    }
```

The signature of the function was slightly changed to fit the type of Memo but
everything else is exactly the same! Let's see how it performs against the
non-memoized implementation.

## Benchmarking
Average execution times were gathered for tree heights 1 to 4. Due to the
inherent slowness of the algorithm it wasn't practical to test above 4 with my
2014 MacBook :P.

```
| Height | Original (ms) | Memoized (ms) | Speedup |
|--------|---------------|---------------|---------|
| 1      | 2.184389      | 7.043043      | 0.31    |
| 2      | 1.546299      | 1.270925      | 1.21    |
| 3      | 0.519338      | 0.484961      | 1.07    |
| 4      | 27.109193     | 13.389669     | 2.02    |
```

The memoized version is a low slower for trees of height 1. This is because the
additional cost of memoizing is much higher when the function is in its base
case, which takes comparatively little time. For trees of size 4 we see a halving
in runtime; a pretty solid gain.

## Conclusion
If you can recognize where your recursive functions are doing extra work you can use
the `Memo` trait to provide speedup and almost 0 additional cost. As a further
optimization you can perform benchmarking to determine the input threshold at
which the memoized You can even swap back to the
non-memoized implementation for very small inputs.
