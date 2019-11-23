---
layout: post
title: Mangling Swift Source Code With SwiftSyntax
---

- [Introduction](#introduction)

- [Building a Syntax tree from a source
file](#building-a-syntax-tree-from-a-source-file)

- [Traversing the Syntax Tree](#traversing-the-syntax-tree)

- [Example 1 - Rewriting Integer Literals](#example-1---rewriting-integer-literals)

  - [Filtering for Integer Literal Nodes](#filtering-for-integer-literal-nodes)

  - [Reformatting the digits](#reformatting-the-digits)

  - [Returning A New TokenSyntax Node](#returning-a-new-tokensyntax-node)

  - [Test Input](#test-input)

  - [Test Output](#test-output)

  - [Example 2 - Converting Snake Case Declarations To Camel Case](#example-2---converting-snake-case-declarations-to-camel-case)

  - [Transforming A Snake Case String To A Camel Case String](#transforming-a-snake-case-string-to-a-camel-case-string)

  - [Visiting IdentifierExprSyntax Nodes](#visiting-identifierexprsyntax-nodes)

  - [Visiting FunctionParameterSyntax Nodes](#visiting-functionparametersyntax-nodes)
  
  - [Test Input](#test-input-1)

  - [Test Output](#test-output-1)

  - [The Meta Testcase](#the-meta-testcase)

- [Conclusion](#conclusion)

- [Source Code](#source-code)


# Introduction
At my work I recently added [SwiftLint](https://github.com/realm/SwiftLint) to
one of our iOS apps. I highly recommend it to the iOS devs out there with no
linting on their projects!

In order for SwiftLint to work its magic it has to be able to inspect and modify
Swift source code. The authors of SwiftLint opted for
[SourceKit](https://github.com/apple/swift/tree/master/tools/SourceKit), a _"framework for supporting IDE features like
indexing, syntax-coloring, code-completion, etc."_  Working with SwiftLint
piqued my curiosity about other frameworks that facilitate the handling of Swift
source code. My googling led me to
[SwiftSyntax](https://github.com/apple/swift-syntax), a set of Swift bindings
for [libSyntax](https://github.com/apple/swift/tree/master/lib/Syntax). 

I started tinkering with SwiftSyntax just for fun and built some linter-inspired
source code rewriters. Here is
a short tutorial by example on getting started with the framework. This tutorial assumes
you are familiar with Swift and have an elementary understanding of programming
language parsing.

# Building a Syntax tree from a source file
Each element in the source code's abstract syntax tree (AST) is represented as a struct inheriting from the 
[`Syntax`](https://github.com/apple/swift-syntax/blob/master/Sources/SwiftSyntax/Syntax.swift) struct and
adopting the `SyntaxProtocol` protocol. This protocol defines common attributes
of nodes in the AST such as child nodes, parent nodes, and more. You can
generate a syntax node representing the entire source file using the 
[`SyntaxParser`](https://github.com/apple/swift-syntax/blob/master/Sources/SwiftSyntax/SyntaxParser.swift).
In the release I used, this is named `SyntaxTreeParser`.

```swift
import SwiftSyntax

let url = URL(fileURLWithPath: "HelloWorld.swift")
let sourceFile = try! SyntaxTreeParser.parse(url)
```

Voila! We now have a root syntax node for the file at `pathToFile`.

# Traversing the Syntax Tree
Traversing the tree generated in the previous section is made simple by the
[`SyntaxRewriter`](https://github.com/apple/swift-syntax/blob/master/Sources/SwiftSyntax/gyb_generated/SyntaxRewriter.swift)
class. If you take a look at its implementation, there is a corresponding `visit(_:)` method for every type of node that can appear in a syntax tree.
The default implementation simply visits the node's children recursively. We can override `visit(_:)` for each type of node we're interested in.

# Example 1 - Rewriting Integer Literals

In this example we're going to clean up the integer literals in our code
base. Longer integer literals that aren't separated by underscores are hard to read. For
example, it's much easier to discern the value of `x` written this way:
```swift
let x = 1_000_000_000
```
than it is written this way:
```swift
let x = 1000000000
```

We aim to group the digits of large integer literals into
threes, making them easier to read.

A visit to this nice [AST
explorer](https://swift-ast-explorer.kishikawakatsumi.com/) tells us that
`1000000000` corresponds to an `IntegerLiteralExpr`. After some digging in the
SwiftSyntax source code I discovered that what we're looking for is a
`TokenSyntax` whose "kind" is an integer literal, similar to the
[example](https://github.com/apple/swift-syntax#example) in `SwiftSyntax`'s
README.


Let's create a skeleton of our integer literal rewriter class, overriding
`visit(_:)` for `TokenSyntax` nodes.

```swift
final class IntegerLiteralRewriter: SyntaxRewriter {
  override func visit(_ token: TokenSyntax) -> Syntax {
    return super.visit(token)
  }
}
```

The next step is to implement `visit(_:)`.

## Filtering for Integer Literal Nodes 
Let's return early if the TokenSyntax kind is not what we're looking for
```swift
guard case .integerLiteral(let digits) = token.tokenKind else {
  return super.visit(token)
}
```
A full list of kinds for TokenSyntax nodes is available in the
[`TokenKind`](https://github.com/apple/swift-syntax/blob/master/Sources/SwiftSyntax/gyb_generated/TokenKind.swift) enum.

## Reformatting the digits
Here is my implementation for reformatting the digits. Feel free to write your
own as an exercise!

There are two steps:
1. Remove any existing underscores - this is in case underscores have been used
   to format the integer literal in a way that is different from our desired
   format. For
   example, we don't want to deal with a literal like `100_0_0_0`.
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

## Returning A New TokenSyntax Node
All Syntax Nodes are structs whose members cannot be modified. We need
to return a copy of the original node with the updated integer literal. We can
use a `with` API for this.

```swift
// Return the same integer literal token, but with the underscores
let newToken = token.withKind(.integerLiteral(integerTextWithUnderscores))
return super.visit(newToken)
```

And there we have it! Let's see how it behaves on this test case:

### Test Input
```swift
let x = 100000000
let y = 1198756
let z = 987654321
```
### Test Output
```swift
let x = 100_000_000
let y = 1_198_756
let z = 987_654_321
```

# Example 2 - Converting Snake Case Declarations To Camel Case
As Swift programmers we detest snake
case. It is anathema to writing beautiful, _swifty_ code ;). We're going to
write a class to convert any snake case declarations to camel case. Again, I
used the [AST explorer](https://swift-ast-explorer.kishikawakatsumi.com/) to
determine what types of nodes we need to visit.

Here are the two functions we need to override:

```swift
open func visit(_ node: IdentifierExprSyntax) -> ExprSyntax
open func visit(_ node: FunctionParameterSyntax) -> Syntax
```

The first covers expressions like
```swift
let big_snake = small_snake + medium_snake
```
while the second covers function parameters like
```swift
func eatRats(with_snake snake: Snake, some_rats: [Rat]) {}
```

## Transforming a snake case string to a camel case string
Before we deal with the syntax nodes we need a method for doing the conversion.
As an exercise feel free to write your own implementation :). Here is mine:
```swift
/// Removes all underscores from the identifier and capitalizes characters
/// following underscores. Assumes `identifier` is a valid identifier. Ignores
/// leading underscores.
private func convertToCamelCase(_ identifier: String) -> String {
  let identifier = Array(identifier)
  var newIdentifier = ""
  var i = 0
  var hasSeenNonUnderscoreCharacter = false
  while i < identifier.count {
    if identifier[i] != "_" {
      hasSeenNonUnderscoreCharacter = true
    }

    if identifier[i] == "_" && !hasSeenNonUnderscoreCharacter {
      newIdentifier.append("_")
      i += 1
    } else if identifier[i] == "_" &&
        i+1 < identifier.count &&
        ("a"..."z").contains(identifier[i+1]) {
      newIdentifier.append(identifier[i+1].uppercased())
      i += 2
    } else if identifier[i] != "_" {
      newIdentifier.append(identifier[i])
      i += 1
    } else {
      i += 1
    }
  }

  return newIdentifier
}
```

## Visiting IdentifierExprSyntax Nodes
This is the easy one. It turns out an `IdentifierExprSyntax` node's identifer is
a `TokenSyntax` object, so the implementation is very similar to our
`IntegerLiteralRewriter`.
```swift
override func visit(_ node: IdentifierExprSyntax) -> ExprSyntax {
  guard case .identifier(let identifier) = node.identifier.tokenKind else { return node }
  if isSnakeCase(identifier) {
    let newIdentifier = convertToCamelCase(identifier)
    let newToken = node.identifier.withKind(.identifier(newIdentifier))
    let newNode = node.withIdentifier(newToken)
    return super.visit(newNode)
  }
  return super.visit(node)
 }
```

## Visiting FunctionParameterSyntax Nodes
This one is a bit more complicated. We have two cases to deal with:
1. The function parameter has only a local name - this is the only option in
   most programming languages: e.g. `func eatRats(snake: Snake)`
2. The function parameter has an external and local name. If you write Swift
   code you have seen this before: e.g. `func eatRats(withSnake snake: Snake)`

`FunctionParameterSyntax` nodes have `firstName` and `secondName` properties.
They are both `TokenSyntax` objects. Interestingly, depending on the case, the
propery that holds the _local_ parameter name is different.

When the parameter only has a local name it's stored in the `firstName`
property of the node. When the parameter has both the external and local name
the external name is stored in the `firstName` property and the local name is
stored in the `secondName` property.

We'll handle each case separately.

```swift
override func visit(_ node: FunctionParameterSyntax) -> Syntax {
  // If both firstName and secondName are non nil then it's a function
  // parameter name like foo(withX x: ...) and we want to modify the
  // secondName. If only firstName is non nil then it's a function parameter
  // like foo(x: ...) and we wont to modify the first name.
  if let _ = node.firstName,
    let secondNameToken = node.secondName,
    case .identifier(let identifier) = secondNameToken.tokenKind,
    isSnakeCase(identifier) {
    let newIdentifier = convertToCamelCase(identifier)
    let newSecondName = secondNameToken.withKind(.identifier(newIdentifier))
    let newNode = node.withSecondName(newSecondName)
    return super.visit(newNode)
  } else if let firstNameToken = node.firstName,
    case .identifier(let identifier) = firstNameToken.tokenKind,
    isSnakeCase(identifier) {
    let newIdentifier = convertToCamelCase(identifier)
    let newFirstName = firstNameToken.withKind(.identifier(newIdentifier))
    let newNode = node.withFirstName(newFirstName)
    return super.visit(newNode)
  }

  // We should never get here. You can't have an unnamed parameter!
  return super.visit(node)
}
```
And there we have it! Let's try a test case:
### Test Input
```swift
import scary_snek

let scary_anaconda = 5

var scary_cobra = 10

let big_python = scary_cobra * scary_anaconda
```
### Test Output
```swift
import scary_snek

let scaryAnaconda = 5

var scaryCobra = 10

let bigPython = scaryCobra + scaryAnaconda
```

Notice how it didn't modify the import. That's as expected because we didn't
override the `visit(_:)` function for imports!

## The Meta Testcase
For a more thorough test I
[rewrote](https://github.com/daltyboy11/SwiftSyntaxDemo/blob/master/Tests/SwiftSyntaxDemoTests/input4.txt) `SnakeCaseRewriter` using snake case
declarations! After running the snake case version of `SnakeCaseRewriter`
through `SnakeCaseRewriter`, the output was the original `SnakeCaseRewriter`
source, as we would expect!


# Conclusion
SwiftSyntax provides a great API for modifying Swift source code! All of my
examples were linter inspired but if you come up with a creative use for
SwiftSyntax let me know via email! :)

# Source Code
All the source code and test cases for this post are available as a Swift
Package
[here](https://github.com/daltyboy11/SwiftSyntaxDemo/blob/master/README.md).
