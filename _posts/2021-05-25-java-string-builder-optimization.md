---
layout: post
title: Why is string concatenation in a loop not optimized to StringBuilder.append?
---

I did a quick experiment to test out the performance of string concatenation vs
StringBuilder with OpenJDK 14.

### Without StringBuilder
```java
class StringConcatNoBuilder {
  public static void main(String[] args) {
    final int numIterations = Integer.parseInt(args[0]);
    String s = "";
    for (int i = 0; i < numIterations; ++i) {
      s += "abcdefg";
    }
    System.out.println("Finished");
  }
}
```

### With StringBuilder
```java
class StringConcatWithBuilder {
  public static void main(String[] args) {
    final int numIterations = Integer.parseInt(args[0]);
    final StringBuilder builder = new StringBuiler();
    for (int i = 0; i < numIterations; ++i) {
      builder.append("abcdefg");
    }
    final String s = builder.toString();
    System.out.println("Finished");
  }
}
```

### Timing Analysis
I ran each program for `numIterations = 1000000`. The StringBuilder program
terminated almost immediately and the non StringBuilder program failed to
terminate even after several minutes.

Needless to say, no optimization is happening at the bytecode level.

### Open question
It seems like a fairly low hanging optimization fruit for the compiler to turn
this:

```java
String s = "";
for (int i = 0; i < numIterations; ++i) {
  s += "abcdefg";
}
```

into this:

```java
StringBuilder builder = new StringBuilder();
for (int i = 0; i < numIterations; ++i) {
  builder.append("abcdefg");
}
String s = builder.toString();
```

Yet it doesn't. Why?
