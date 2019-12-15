---
layout: post
title: So you think you know for comprehensions
---

At first glance you may think scala for comprehensions are a slightly
more sophisticated version of the _for loop_. Under the hood, they are
much more. What they really are is syntactic sugar over the higher order
functions `flatMap`, `map`, `withFilter`, and `foreach`, and they are compiled
down to a series of these higher order functions chained together.

The challenge for you, reader, is to write an equivalent expression for each of
the following for comprehensions using only the higher order functions mentioned.
Each comprehension is independent and you may assume the types for the named values
(`a`, `b`, etc.) are correct. Some answers are given at the end of the post.

1.

```scala
for (x <- a) yield x + 1
```

2.

```scala
for {
  x <- a
  y <- b
  z <- c
} yield x + y + z
```

3.

```scala
for {
  x <- a
  y <- x
  z <- y if z % 69 == 0
} yield s"z: ${z + 1}"
```

4.

```scala
for {
  (x, y) <- a
  z = x * y
  w <- b if w - z > 0 && z != 0
  v = w / z
} println(v)
```

# Answers
1.

```scala
a map (_ + 1)
```

2.

```scala
a flatMap (x => b flatMap (y => c map (z => x + y + z)))
```

3.

```scala
a flatMap { x =>
  x flatMap { y =>
    y withFilter (_ == 2) map { z =>
      z + 1
    }
  }
}
```

4.

```scala
a flatMap { case (x, y) =>
  val z = x * y
  b withFilter (_ - z > 0 && z != 0) map (_ / z)
} foreach println
```
