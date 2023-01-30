# Shadow Promises
## Tenets
* No math operators. Functions support all the math operations.
* 2 kinds of functions:
* logical - these must always execute inline on the current thread.  They also shortcut - stopping evaluation on the first definitive result.  (False for `:and` and `:nand`, True for `:or`) <br> 
    * :and
    * :or
    * :nand<br>
    NOTE: promises called inside of `:and`, `:no`, `:nand` will always be called inline, use the promises result even in the case of failure.
* promises - these can run on any thread, or cooperatively (message queue based) on the same thread. The promise has the execution status, and success or failure.
* No exceptions! See the promise result.  (Does not work well with lots of individual tasks/promises.)
* Functions take an ordered typed list.  

## Shadow
Assign to a "vairable".<br>

    5.0 | var1

    s:Random(
        1.0
        100.0
    ) | myVar

Using | again will shadow/replace the old variable.  Everything already called has the old value. 

<br>

## Ordered Lists

Functions take an ordered list of parametes.

    :And 
Operates on ordered lists and shoutcuts out on a false value

    :Or 
Operates on ordered lists and shoutcuts out on a true value

The ordered list starts with 

    (
and ends with 

    )

There can be whitespace between parameters/ordered list items, but the only separator is a line break.

### Suggested format

    myFunct (
        p1
        p2
    ) | result

## Blocks
Blocks join any number of statements together.  

function boddies, `:if` - `:else`, and `:loop` all take blocks, not single statements.

Blocks do not return values.

## Function Calls

### Call types
Call and return inline

    /FunctionName\(/ParameterList\) [| /PromiseResult\]

Event based asyncronous on the same thread 

    :startAsync  /FunctionName\(/ParameterList\) [| /PromiseResult\]

Run on a given thread or threadpool 

    [ThreadStruct]:startAsync /FunctionName\(/ParameterList\) [| /PromiseResult\]


## Promise
All functions return a result with the following:

Failed
*  bool =\ True if the function failed

Retryable
* bool =\ True if the the error can be resonably corrected.  To make it easier on the user do not set retryable on success.  (e.g. missing removable disk, out of diskspace.  Not other file write errors.)

ErrorCode
* String =\ [module UUID] [error Id]

Turn an error code into a localized error message

    :call
        s:ErrorCode
    :receive | errorString
* A function that returns a String =\ The message if the current g:locale or g:language can be looked up followed by \n[resource name]  e.g. a file open to "temp/foo.txt" fails g:local is "DE:de", g:language is "de fr es" but the there are only english resources in the module so error message would be<br>"\ntemp/foo.txt"<br>This `ErrorMessage()` should cache string for repeated calls.

Result
* Attempting to use result does a wait.
* If the function errored. Functions can specify their `errored` return.

## Defining Functions
```
<
    /protptype varaibles\
>
{
    /function body\
}
```

## Function examples
Make the function /double\
```
<
    float in
>
{
    :return
        multiply(
            in 
            in
        )
} | double
```

Make and call a sum function using tail recursion
```
<
    float[] in
>
{
    :if (:isEmpty(in))
    :then {
        :return 0.0
    }
    :if (:has1(in))
    :then {
         :return in.first
    }
    :else
    {
        :return
            :add(
                in.first
                :self(in.rest)
            )
    }
} | sum

sum( 
    1.0
    2.0
    3.0
    78.9
)
```

### Handling a retryable file open error iteratively:

```
:function OpenFileWithRetryPrompt(
    s:String filePath
){
    :loop {
        :scratch s:flag shouldRetry false
        
        s:openFile(
            filePath
        ) | openFileHandle

        :test (openFileHandle.Failed)
        :if  {
           ui:ShowErrorMessage :uiThread (
                openFileHandle.Error
            ) | shouldRetry
        } :else {
            :return openFileHandle
        }

        :test (:not(shouldRetry)) :loopExit 
    }
}
```

### Handling a retryable file open error functionally:
```
:function OpenFileWithRetryPrompt (
    s:String filePath
    :funcSpec handleFile(s:FileHandle)
) {
    s:openFile :async (
        filePath
    ) :continueWith (openFileHandle) {
        :test (openFileHandle.Failed)
        :if  {
            ui:ShowErrorMessage :uiThread (
                openFileHandle.Error
            ) :continueIf {
                OpenFileWithRetryPrompt (
                    filePath
                    handleFile
                )
            }
        } :else {
            handleFile(openFileHandle)
        }
    }
}
```

<br>

## Namespaces
Namespace prefixes are always used.

The language core namespaces
1. **:** core language  namespace.
2. **s:** core string library

Import modules have a UUID and the namespace they will use.  The suggested namespace will be in the library's comment block.

    :import "eca53738-a2a6-4b80-898c-119a35a18f46" "render"


## Functional
Functions are first class objects. Functions can be passed into function calls.  Functions can return fucntions.

Functions will tail call when possible.

In general Curying is avoided, but making closures is supported. Closures are often necessary to fit existing interfaces. 

## Block structured
Key words start and end blocks.  This prevents any scoping amgiguity.

Blocks start with `{` and end with `}`

Conditionals are also blocks.<br>
`:test` takes a single boolean valued function. And then one of these
* `:if`
* `:next` Like C language continue - go to the loop start
* `:loopExit` Like the C language break, but only for loops

Function and immediates use  `(` ... `)` around the parameters.

Simple to generate; simple to use as a translation target.

<br>

## Operators
No operators.

## :and & :or
These are not functions,  They shortcut like C languages. (A true value ends the :or, a false value ends the :and)

:and uses a founction call like parameter block.  (Remember it early exits on a false value!)

    :and (
        s:IsInputWaiting(),
        s:less(
            5
            myVar
        ),
        myFlag
    )
:or uses a founction call like parameter block.  (Remember it early exits on a false value!)
    :or (
        myFirstFlag,
        mySecondFlag,
        s:grater(
            myNumber
            7.0
        ),
    )


There is no valid mathematical presidence order when mixing numberspaces. (Bitflags or logical operations mixing with numeric operations.) Enforcing an order in the compiler just makes more syntax rules to remember.

Mathematical functions should be able to take multiple paramters.
```
    (:+
        1 
        5
    )

    (:+
        1
        3
        12
        9
    )

    (:*
        5
        4
        3
        2
        1
    )
```

## :test
Test blocks are built in to the language.  They are ony compiled in a test build.
```
:test [name] {
    ...
}
```
When building for test a sorted list of all the `:test` blocks are made.  They are storted by thier `[name]` and started in alphabetical order.  (If paralized that does dnot define the completion order!)

## :option
Conditional compilation is controled by
```
:option [id] {
    ...
}
```
For example:
```
:option opt:DEBUG {
    ui:ShowErrorMessage (
        "This should not happen"
    )
}
```

## :define
`:define [flagName]` sets [flagName] on for contitional compilation with the :option statement.  In most cases the conditional compilation flags should come from the build environment, but sometimes they need to be manually turned on or off.

## :undefine
`:undefine [flagName]` sets [flagName] off for contitional compilation with the :option statement.  In most cases the conditional compilation flags should come from the build environment, but sometimes they need to be manually turned on or off.

# Syntax
## Tokens
| Prefix Match | Regex | ID | Description |
|:--- |:--- |:--- |:--- |
| " | <pre>^"[\s\S]*?(?<!\\)"</pre> | string | A string (may be multiline) |
| ' | <pre>^'[\s\S]*?(?<!\\)'</pre> | string | A string (may be multiline) |
| 0[xX] | <pre>^0\[xX](?:[0-9a-fA-F]{2})+(?=[\W])</pre> | number | A hex number digits must be pairs,<br>0 is 0x00 |
| [-\d] | <pre>^-?[\d]+(?:\.[\d]*)?(?:[eE]-?[\d]+)?(?=[^\w.])</pre> | number | A decimal number<br>NOTE: cannot start with . |
| # | <pre>^#[^\r\n]*</pre> | comment | Line comment to EOL |
| : | | : | Namespace separator |
| . | | . | Member separator |
| { | | | Block start |
| } | | | Block end |
| ( | | | Parameters start |
| ) | | | Parameters end |
| [ | | | Prototype stat |
| ] | | | Prototype end |
| \| | | \| |assignment (pipe) |
| [!$%&*,/;<=>?@^`~] | | ! | Unassigned punctuation | 
| [^\s\x00-\@\[-\`\{-\x7f] | <pre>[^\s\x00-\@\[-\`\{-\x7f][^\s\x00-\\/\:-\\@\\[-\`\{-\x7f]*</pre> | id | An itentifier or keyword |
| \s | \s* | \s | White space (No tokens are made for whitespace) |

## Typed Identifiers
| Identifier Type | Description |
|:--- |:--- |
| package | An imported package or namespace<br>no package is the core language |
| functionId | the identifier of a function |

## Statements
```
function = functionPrototype  | functionId
string = stringConstant | stringId
additionalParameters = ("\n" typedVariableId [additionalParameters] )
parameters = "(" [typedVariableId [additionalParameters]] ")""
callType = "" | ":inline" | ":async" | (":thread" threadId) | (":remote" string)

test = ":test" "(" logicalResult ")"
loopExit = test ":break"

blockStatements = (*functionCall | *if | *loop) [blockStatements]
# note: loopBlockStatements must contain 1 or more loopExit!
loopBlockStatements = [blockStatements] loopExit [blockStatements] [loopBlockStatements]

*functionCall = package ":" function [callType] parameters
*if = test ":if" block [":else" block]
*block = "{" [blockStatements] "}"
*loop = ":loop" "{" loopBlockStatements "}"
*opt = ":opt" definedOrUndefinedId *block


```