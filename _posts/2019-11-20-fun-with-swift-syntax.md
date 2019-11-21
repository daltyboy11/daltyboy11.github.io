---
layout: post
title: Mangling Swift source code with SwiftSyntax
---

# Introduction
At my work I recently added [SwiftLint](https://github.com/realm/SwiftLint) to
one of our iOS apps. For the iOS devs out there with no linting on their
projects, I highly recommend it!

In order for SwiftLint to work its magic it has to be able to inspect and modify
swift source code. The authors of SwiftLint opted for
[SourceKit](https://github.com/apple/swift/tree/master/tools/SourceKit), a "framework for supporting IDE features like
indexing, syntax-coloring, code-completion, etc." Working with SwiftLint at work
got me curious about other frameworks that faciliate the mangling of Swift
source code. My googling led me to
[SwiftSyntax](https://github.com/apple/swift-syntax), a set of Swift bindings
for [libSyntax](https://github.com/apple/swift/tree/master/lib/Syntax). 

I started playing around with SwiftSyntax because, well, it's cool. Here is
a short tutorial by example on getting started with it. This tutorial assumes
you are familiar with Swift and have an elementary understanding in programming
language parsing.

# Building a Syntax tree from a source file
Each element in the source code's abstract syntax tree (AST) is represented as a struct inheriting from the 
[`Syntax`](https://github.com/apple/swift-syntax/blob/master/Sources/SwiftSyntax/Syntax.swift) struct and
adopting the `SyntaxProtocol` protocol. This protocol defines the attributes
every node in the AST shares, such as children, its parent, and more. You can
generate a syntax node representing the entire source file using the 
[`SyntaxParser`](https://github.com/apple/swift-syntax/blob/master/Sources/SwiftSyntax/SyntaxParser.swift).
In the release I used, this is named `SyntaxTreeParser`.

```swift
import SwiftSyntax

let url = URL(fileURLWithPath: "HelloWorld.swift")
let sourceFile = try! SyntaxTreeParser.parse(url)
```

Voila! You now have a root syntax node for the file at `pathToFile`.

# Traversing the Syntax Tree
Traversing the tree generated in the previous section is made simple by the
[`SyntaxRewriter`](https://github.com/apple/swift-syntax/blob/master/Sources/SwiftSyntax/gyb_generated/SyntaxRewriter.swift)
class. If you take a look at its implementation, there is a corresponding `visit(_:)` method for possible type of node that appears in our syntax tree.
The default implementation simply visits the node's children recursively. We can override `visit(_:)` for each type of node we're interested in.

# Example 1: Rewriting Integer Literals

In this linter inspired example lets clean up the integer literals in our code
base. Longer integer literals not separated by underscores are hard to read. For
example, it's much easier to discern the value in
```swift
let x = 1_000_000_000
```
than it is in
```swift
let x = 1000000000
```

We aim to group the digits of large integer literals into
three's, making them easier to read. A visit to this nice [AST
explorer](https://swift-ast-explorer.kishikawakatsumi.com/) tells us that
`1000000000` corresponds to an `IntegerLiteralExpr`. After some digging in the
SwiftSyntax source code I discovered that what we're looking for is a
`TokenSyntax` whose "kind" is an integer literal, similar to the
[example](https://github.com/apple/swift-syntax#example) in `SwiftSyntax`'s
README.


Let's create a skeleton of our integer literal rewriter class, overriding
`visit(_:)` for `TokenSyntax` nodes.

```swift
final class IntegerLiteralFormatter: SyntaxRewriter {
  override func visit(_ token: TokenSyntax) -> Syntax {
    return super.visit(token)
  }
}
```

## Filtering out the right kind of TokenSyntax
Lets early return if the TokenSyntax kind is not what we're looking for
```swift
guard case .integerLiteral(let digits) = token.tokenKind else {
  return super.visit(token)
}
```
A full list of kinds for TokenSyntax is available in the
[`TokenKind`](https://github.com/apple/swift-syntax/blob/master/Sources/SwiftSyntax/gyb_generated/TokenKind.swift) enum.

## Reformatting the digits
There are two steps
1. Remove any existing underscores - this is in case underscores have been used
   to format the integer literal in a way that is different from ours.
2. Add in the underscores.

```swift
// Remove existing underscores
let integerTextWithoutUnderscores = String(text.filter {
                                             ("0"..."9").contains($0) })

// Starting from the least significant digit, we will add an underscore
// every three digits
var integerTextWithUnderscores = ""
for (i, c) in integerTextWithoutUnderscores.reversed().enumerated() {
  if i % 3 == 0 && i != 0 { // don't add an underscore to the beginning!
    integerTextWithUnderscores.append("_")
  }
  integerTextWithUnderscores.append(c)
}
integerTextWithUnderscores = String(integerTextWithUnderscores.reversed())
```
