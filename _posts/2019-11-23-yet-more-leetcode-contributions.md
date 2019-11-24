---
layout: post
title: Yet More Leetcode Contributions
---

If you've been following my recent posts you know that I
[submitted](https://daltyboy11.github.io/runoff-vote-and-leetcode/) a question
contribution to leetcode.

Determined to have at least one of my contributions accepted, today I submitted
three more. They are much simpler than my first and all inspired by my
[SwiftSyntax Tutorial](https://daltyboy11.github.io/fun-with-swift-syntax/). I
was completely transparent in my reason for submitting them. I'm not doing it
because I saw these questions in interviews. I'm doing it just because I can!

## Submission 1 - Rewriting Integer Literals
This one is taken directly from the SwiftSyntax tutorial.
### Question Title
Reformat Integer Literals
### Reason for Submission
I want to contribute this question for points.
### Description
Some programming languages allow you to write integers with underscores to enhance readability. For example, in Swift you can write

```swift
let x = 1_000_000
```

to declare an integer literal. To make integer literals more readable we wish to group the digits into threes, starting from the least significant digit.

Given a string of digits with no underscores in it, return a reformatted string containing the digits separated by underscores in the manner described.

Example:

- Given `literal = "123456789"`, return `"123_456_789"`.

- Given `literal = "99999999"`, return `"99_999_999"`.

### Solutions
This is a straightforward solution! The (Swift) code speaks for itself.
Here is the leetcode playground:
[https://leetcode.com/playground/5Jw8YD8x](https://leetcode.com/playground/5Jw8YD8x)
### Test Cases
Input: `literal = "123456789"`

Output: `"123_456_789"`

Input: `literal = "11111111111"`

Output: `"11_111_111_111"`

Input: `literal = "69"`

Output: `"69"`

### Tags
String

### Contributor
ElvisTheKing

## Submission 2 - Convert Snake Case to Camel Case
This one is also taken directly from my SwiftSyntax Tutorial.

### Question Title
Convert Snake Case to Camel Case

### Reason for Submission
I want to contribute this question for the points.

### Description
Snake case and Camel case are two different conventions for variable and parameter names.

Given an identifier written in snake case, convert it to camel case.

Examples

`identifier = scary_snake`, return `scarySnake`

`identifer = _even__scarier____snake_`, return `_evenScarierSnake_`

Note that leading and trailing underscores are not considered part of the "snake".

### Solutions
A straightforward Swift solution

[https://leetcode.com/playground/jQMCN93x](https://leetcode.com/playground/jQMCN93x)

### Test Cases
Input: `scary_snake`

Output: `scarySnake`

Input: `_even__scarier____snake_`

Output: `_evenScarierSnake_`

Input: `x_123_hello`

Output: `x123Hello`

Input: `alreadyCamelCase`

Output: `alreadyCamelCase`

Input: `justonelongword`

Output: `justonelongword`

Input: `a_b_c_d_e_f_g`

Output: `aBCDEFG`

### Tags
String

### Contributor
ElvisTheKing

## Submission 3 - Convert Camel Case to Snake Case
This wasn't in my SwiftSyntax tutorial but was easy enough to come up with. As
you can clearly see it's just the opposite of the previous contribution.

### Question Title
Convert Camel Case to Snake Case

### Reason for Submission
I want to contribute this question for the points.

### Description
Snake case and Camel case are two different conventions for variable and parameter names.

Given an identifier written in camel case, convert it to snake case.

### Solutions
A straightforward Swift solution

[https://leetcode.com/playground/Ro6UPf4u](https://leetcode.com/playground/Ro6UPf4u)

### Test Cases
input: `"helloWorld"`

output: `"hello_world"`

input: `"ThisIsAString"`

output: `"this_is_a_string"`

input: `"test123"`

output: `"test123"`

### Tags
String

### Contributor
ElvisTheKing
