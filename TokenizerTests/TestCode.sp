'Line 1 of multi-line string
Line 2 of multi-line string' // The string is complete

5.5e2 @ a
2e-2 @ b
-.2e-1 @ b
0xABCD @ c
'test ▲\u{25B2} 𝅘𝅥\u{1D15F} \x{0d}\x{0a}' @ d

:test
    :equals(
        8.8
        y
    )
:if {
	7.7 @ y
} :else {
	8.8 @ y
}

[
  double in
]
{
	:return 
		:listReduce
		(
			in
			0.0
			:add
		)
} @ sum

-1e-2 @ x		    // -0.01

Fe3O4               // An identifier
aTestIdentifier     // An identifier (camelCase)
_a_test_identifier  // An identifier with underscores

//error tests
01.01.23            // not a valid number
--123               // not a valid number
123x                // Bad decimal number
0xqq                // Bad hex number
0x                  // Bad hex number - no digits
~ `                 // Bad punctuation
"abc def (no string end)
