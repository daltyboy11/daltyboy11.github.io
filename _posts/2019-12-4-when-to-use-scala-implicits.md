---
layout: post
title: Using Scala implicits
---

Implicits is a scala feature that often gets a bad rap. Like
any tool, it can be both used **AND** abused. I'm convinced if you know how and
when to apply them your scala code will look and feel cleaner. Here are some use
cases, mostly taken from [Programming Scala, 2nd Edition](http://shop.oreilly.com/product/0636920033073.do)


# Type Class Pattern - Adding behavior to types
This is a convenient way to add methods to any type, also known as [_ad hoc polymorphism_](https://en.wikipedia.org/wiki/Ad_hoc_polymorphism)

Suppose I am constructing an application to collect census data and I want to
write each of the following models to separate CSV files (comma separated
values).

```scala
case class Household(address: String, numOccupants: Int)
case class Person(fullName: String, age: Int, homeAddress: String)
```

How can I convert `Household` and `Person` to strings of comma separated values?
Without using implicits, I might define top level functions:

```scala
// Convert a household to its CSV representation
def toCSVRepr(household: Household): String =
  household.addres + "," + household.numOccupants.toString

// Convert a Person to their CSV representation
def toCSVRepr(person: Person): String =
  person.fullName + "," + person.age.toString + "," + person.homeAdress

val household = Household("1 Strawberry St", 5)
val person = Person("Dalton Sweeney", 22, "1 Strawberry St")

val householdCSVRepr = toCSVRepr(household)
val personCSVRepr = toCSVRepr(person)
```

This is fine but we might prefer to be able to call `toCSVRepr` directly on
instances of `Household` and `Person`. We can use implicits for this.

```scala
object Implicits {
  trait ToCSVRepr {
    def toCSVRepr: String
  }

  implicit class HouseholdToCSVRepr(household: Household) extends ToCSVRepr {
    def toCSVRepr: String = household.addres + "," + household.numOccupants.toString
  }

  implicit class PersonToCSVRepr(person: Person) extends ToCSVRepr {
    def toCSVRepr: String = person.fullName + "," + person.age.toString + "," + person.homeAdress
  }
}

import Implicits._

val householdCSVRepr = Household("1 Strawberry St", 5).toCSVRepr
val personCSVRepr = Person("Dalton Sweeney", 22, "1 Strawberry St").toCSVRepr
```

How can I call `toCSVRepr` on instances of `Household` and `Person` when it's
not defined on those classes? Here's what's happening under the hood:

1. At `val householdToCSVRepr = ...`, the compiler notices that `toCSVRepr`
   is not defined on `Household`
2. It searches for any definition of `toCSVRepr` in scope.
3. If there is a `toCSVRepr` definition in scope **and** it's wrapped in an implicit class
   that can be **instantiated** from an instance of `Household`, then we're good
   to go. `HouseholdToCSVRepr` satisfies these requirements.
4. `Household("1 Strawberry St", 5)` is **implicitly** substituted with
   `HouseholdToCSVRepr(Household("1 Strawberry St", 5))`.

Very cool! The implicit classes `HouseholdToCSVRepr` and `PersonToCSVRepr` are
known as _type classes_

It's also convenient to wrap your type classes in an `Implicits` object, that way they can
be imported only exactly where they are needed. You don't want implicits running
around your project willy-nilly!

## The object oriented alternative
Rather than using type classes, we could have had both `Household` and `Person`
extend from a trait defining `toCSVRepr`:

```scala
sealed trait Model {
  def toCSVRepr: String
}

case class Household(...) extends Model {
  override def toCSVRepr: String = ...
}

case class Person(...) extends Model {
  override def toCSVRepr: String = ...
}
```

There are several reasons you might not want this, depending on your use case.

## The benefits of type classes over inheritance
Sometimes the classes on which you want to define your common method have no obvious
shared hierarchy and it doesn't make sense to have them extend from the same
base class/trait. Creating a common superclass can seem forced and unreasonable.

Secondly, there is one great advantage to type classes over inheritance when designing
API's used by clients. Suppose a API/feature is requested by a small but important
subset of clients.

If you were to add methods for this new feature to a base
class and force all subclasses to implement them, this would burden **all**
clients with the new methods. Your new methods are exposed to a lot of clients
that don't need or care about them!

Using the type class pattern and ad hoc polymorphism, these new methods could be
defined as implicit classes in an `Implicits` object/package. Clients who need the new feature can decide to import this new object/package. The API doesn't change at all for the other clients and everyone is happy!

# Constrain the types that can be applied to a function
Now that we've successfully converted households and persons to CSV
representation using ad hoc polymorphism we need a way of converting them back
to instances of their respective classes. In this section I'll show you how
implicits can help us in going from the raw data representation (CSV) to the
model representation (`Household` and `Person`).

Without implicits it might look something like this:

```scala
def getHousehold(csvRepr: String): Household = {
  val parts = csvRepr.split(",")
  val address = parts(0)
  val numOccupants = parts(1).toInt
  Household(address, numOccupants)
}

def getPerson(csvRepr: String): Person = {
  val parts = csvRepr.split(",")
  val fullName = parts(0)
  val age = parts(1).toInt
  val homeAddress = parts(2)
  Person(fullName, age, homeAddress)
}

val householdRepr: String = ...
val personRepr: String = ...

val household = getHousehold(householdRepr)
val person = getPerson(personRepr)
```

With implicits, we can define an implicit class with a parameterized method:

```scala
implicit class ModelAsCSV(csvRepr: String) {
  def get[T](implicit val toT: String => T): T = toT(csvRepr)
}
```

Now we'll be able to call

```scala
val household = householdRepr.get[Household]
val person = personRepr.get[Person]
```

As long as we implement implicit values for `String => Household` and `String => Person`

```scala
implicit val csvReprToHousehold: String => Household = (s: String) => {
  val parts = s split ","
  val address = parts(0)
  val numOccupants = parts(1).toInt
  Household(address, numOccupants)
}

implicit val csvReprToPerson: String => Person = (s: String) => {
  val parts = s split ","
  val fullName = parts(0)
  val age = parts(1).toInt
  val homeAddress = parts(2)
  Person(fullName, age, homeAddress)
}

``` 

Voila! `get[T]` is only available for types `T` that have an implicit `String => T` in scope.

# Passing around contexts, connections, and sessions, and more
Execution contexts, database connects, authentication sessions, and more. What
do they have in common? They often act as boilerplate parameters to a lot of
methods. It would be nice if we could avoid writing them altogether for most
method calls. Implicit method parameters allow us to do this.

## Scala Futures
Execution contexts are what futures use to execute the function passed to their
`apply` method. We are often content with using the default global execution
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

It may not seem like much now, but in a highly concurrent application that uses
futures everywhere it avoids a lot of writing and makes reading code easier.

## Other objects
You could imagine similar use cases for objects like database connections,
thread pools, and authentication sessions.

# Conclusion
- If you are passing around a lot of boilerplate method parameters, implicit
  parameters are your friend.
- If you need ad hoc polymorphism and don't think type inheritance is the right
  choice, use the technique of implicit classes outlined in the article.
- If you need to constrain the allowed types on a method, use the implicit
  classes with implicit val technique outlined in the article.

There is another technique for constraining allowed types called [implicit evidence](https://stackoverflow.com/questions/3427345/what-do-and-mean-in-scala-2-8-and-where-are-they-documented) that I omitted in this article for the sake of keeping it simple. It is
less widely used, but read up if you're curious.

Thanks for reading!
