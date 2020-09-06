---
layout: post
title: Diffing Spark DataFrames
---

_This blog post presupposes basic knowledge of Apache Spark_.

Suppose one has two data sets A and B with the same schema. We expect them to be
equal, i.e.

1. Every record in A is in B
2. Every record in B is in A
3. A and B have the same number of records

We would like to verify that these three conditions hold for the datasets. We
also want to write any records that violate a condition to a human readable file
for further inspection.

## When could this be useful?
Suppose you have a Spark application or similar ETL job that writes structured
data to a file. You want to refactor/optimize the job but are worried about the
correctness of the refactored version. In a perfect world there would be
unit tests to give you confidence in your refactor but alas, none exist **and**
the application is large and unwieldy and not amenable to unit testing. The only
option you have left is to somehow compare the final output of the existing job
with the final output of your refactored version.


## Example Scenario
You work for Mega Corp™ and manage a daily ETL job (Job A) that writes product
transactions to a data pool that's ingested by the marketing team for
analytics purposes. As the pool of input data grows your job is slowing down and
not scaling well. You attempt a performance/optimization refactor (Job B) and need to
verify the format and contents of the output has been preserved.

### Data Definition
The schema for our transaction data is defined by a case class. It could have
any number of columns but for simplicity's sake we'll only look the id,
product, and saleAmount.
```scala
// Each transaction is uniquely identified by id
case class Transaction(id: Long, product: String, saleAmount: BigDecimal, ...)

// Output of the original job
val jobAOutput = Seq(
    Transaction(1, "apple", 2.00),
    Transaction(2, "apple", 2.00),
    Transaction(3, "apple", 2.00)
).toDF

// Output of the refactored job
val jobBOutput = Seq(
    Transaction(1, "apple", 3.00),
    Transaction(2, "apple", 2.00),
    Transaction(4, "banana", 0.50)
).toDF
```
As you can tell the refactor didn't go too well... Job B added a non-existent
transaction (transaction 4), is missing a real transaction (transaction 3),
and mistook transaction 1's saleAmount! 

### Verifying output with a Spark App
Let's write a spark application to _diff_ the output of the current ETL job with
the refactored one. To reiterate our conditions, we'd like to find the following
transactions and write them to files:

1. Transactions in Job A's output that don't appear in Job B's output. I.e. the
refactored job is **missing** transactions.
2. Transactions in Job B's output that don't appear in Job A's output. I.e. the
refactored job is **adding** non-existent transactions!
3. Transactions in both Job A and B's output that don't satisfy column equality.
E.g. Transaction with "id = 2" appears in both outputs but with a different
"product" value.

Spark's API makes it very easy to handle cases 1 and 2. Case 3 requires a bit
more work.

### Handling cases 1 and 2 with a left anti join
A [left anti join](https://sparkbyexamples.com/spark/spark-sql-dataframe-join/)
is our best friend for finding records in one dataset that don't appear in another.

```scala
val transactionsInANotInB = jobAOutput.join(
    right = jobBOutput,
    joinExprs = jobAOutput("id") === jobBOutput("id"),
    joinType = "leftanti"
)

val transactionsInBNotInA = jobBOutput.join(
    right = jobAOutput,
    joinExprs = jobAOutput("id") === jobBOutput("id"),
    joinType = "leftanti"
)

transactionsInANotInB.show
transactionsInBNotInA.show
```

And the output is:

```
// transactions in A not in B
+---+-------+--------------------+
| id|product|          saleAmount|
+---+-------+--------------------+
|  3|  apple|2.000000000000000000|
+---+-------+--------------------+

// transactions in B not in A
+---+-------+--------------------+
| id|product|          saleAmount|
+---+-------+--------------------+
|  4| banana|0.500000000000000000|
+---+-------+--------------------+
```

We can write these dataframes to a csv file for
further inspection.

### Case 3 - Columnwise comparison
Now that we've taken care of missing/added transactions we need to verify
that the remaining transactions have the correct values. Let's perform an
inner join to get the transactions present in both job's output's. Since
both sets of columns will be present after the join we need to prefix the column
names so the join result will have unique column names. We can do this by
folding over the orignal column names: 

```scala
val jobAOutputPrefixed = jobAOutput.columns.foldLeft(jobAOutput) { case (df, colName) =>
    df.withColumnRenamed(colName, s"JobA_$colName")
}

val jobBOutputPrefixed = jobBOutput.columns.foldLeft(jobBOutput) { case (df, colName) =>
    df.withColumnRenamed(colName, s"JobB_$colName")
}

val transactionPairs = jobAOutputPrefixed.join(
    right = jobBOutputPrefixed,
    joinExprs = $"JobA_id" === $"JobB_id",
    joinType = "inner"
)

transactionPairs.show
```

The result of the join is:
```
+-------+------------+--------------------+-------+------------+--------------------+
|JobA_id|JobA_product|     JobA_saleAmount|JobB_id|JobB_product|     JobB_saleAmount|
+-------+------------+--------------------+-------+------------+--------------------+
|      1|       apple|2.000000000000000000|      1|       apple|3.000000000000000000|
|      2|       apple|2.000000000000000000|      2|       apple|2.000000000000000000|
+-------+------------+--------------------+-------+------------+--------------------+
```

Now let's perform a column-wise comparison and store the comparison results as
additional columns to the right of the existing ones. A naive approach would
be to simply check all the columns explicity:

```scala
val transactionPairComparisonResult = transactionPairs
    .withColumn("productCheck", $"JobA_product" === $"JobB_product")
    .withColumn("saleAmountCheck", $"JobA_saleAmount" === $"JobB_saleAmount")
```

This is acceptable for a dataframe with a limited number of columns but will
violate the [DRY](https://en.wikipedia.org/wiki/Don%27t_repeat_yourself) principle
for larger schemas. One can do it more concisely by folding over the original
columns:


```scala
val transactionPairComparisonResult = jobAOutput.columns.foldLeft(transactionPairs) {
    case (df, colName) =>
        df.withColumn(s"${colName}Check", col(s"JobA_$colName") === col(s"JobB_$colName"))
}

transactionPairComparisonResult.show
```

And we get correct result:

```
+-------+------------+--------------------+-------+------------+--------------------+-------+------------+---------------+
|JobA_id|JobA_product|     JobA_saleAmount|JobB_id|JobB_product|     JobB_saleAmount|idCheck|productCheck|saleAmountCheck|
+-------+------------+--------------------+-------+------------+--------------------+-------+------------+---------------+
|      1|       apple|2.000000000000000000|      1|       apple|3.000000000000000000|   true|        true|          false|
|      2|       apple|2.000000000000000000|      2|       apple|2.000000000000000000|   true|        true|           true|
+-------+------------+--------------------+-------+------------+--------------------+-------+------------+---------------+
```

The final step is to filter the column comparison result and only keep the
records where at least one "check" column is `false`. Spark's API is very
flexible and I'm sure there are many ways to do this but here is the approach I
opted for:

```
// Generates a "NOT productCheck OR NOT saleAmountCheck OR ..." sql WHERE clause
val filterExpr = jobAOutput.columns
    .map(colName => s"NOT ${colName}Check")
    .reduce(_ + " OR " + _)

val mismatchedTransactions = transactionPairComparisonResult.filter(expr(filterExpr))
mismatchedTransactions.show
```

This gives us the result we want:

```
+-------+------------+--------------------+-------+------------+--------------------+-------+------------+---------------+
|JobA_id|JobA_product|     JobA_saleAmount|JobB_id|JobB_product|     JobB_saleAmount|idCheck|productCheck|saleAmountCheck|
+-------+------------+--------------------+-------+------------+--------------------+-------+------------+---------------+
|      1|       apple|2.000000000000000000|      1|       apple|3.000000000000000000|   true|        true|          false|
+-------+------------+--------------------+-------+------------+--------------------+-------+------------+---------------+
```

We can write this dataframe to a human readable file for further debugging. Now
we have three output files to show us any incorrect output of Job B.

## Conclusion
The sample code demonstrates how one can use spark to compare two data sets that
you expect to be identical and also help you debug them if they're not.

Spark has a vast and powerful API. If Spark provides a handy function for this
use case or you see an obvious way the code can be improved then please let me
know!

You can find the runnable sample code in this [github
repository](https://github.com/daltyboy11/SparkDataFrameDiffDemo/blob/master/src/main/scala/Main.scala).
Thanks for reading!
