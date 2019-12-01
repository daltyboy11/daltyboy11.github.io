---
layout: post
title: When to use Scala implicits
---

Implicits are a powerful and often misused feature of the scala programming
language. This post will present several design problems they can help solve.

# Passing around Context objects and other objects that rarely change
Think about objects like database connections, thread pools, and authentication
sessions. Once initialized they remain around for a long time and a required for
most business logic. Rather than having to pass them around as function
parameters everywhere we can define them implicitly.

The use of execution contexts with scala futures is the most prominent example
of implicitly passing around an object.

Execution contexts are what futures use to execute the function passed to their
`apply` method. We are often content with using the default, global execution
context:

```scala
import ExecutionContext.Implicits.Global

val f = Future[String] {
  "Hello from the future!"
}
```
is equivalent to
```scala
val ec: ExecutionContext = ...
val f = Future[String] {
  "Hello from the future!"
}(ec)
```
You could imagine similar use cases for objects like database connections,
thread pools, and authentication sessions.

# Constraining the types that can be used on a method

## Implicit Evidence

# Implicit Conversions

# Type Classes
