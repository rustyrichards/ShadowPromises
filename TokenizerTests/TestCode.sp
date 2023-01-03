+'Line 1 of multi-line string
Line 2 of multi-line string' # The string is complete
:test(:equals(8.8, y))
:if {
	7.7 | y
} :else {
	8.8 | y
}

<
  double[] in
>
{
	:return 
		:listReduce
		(
			in
			0.0
			:add
		)
} | sum

-1e-2 | x		# -0.01
01.01.23 | x	# not a valid number
--123 | x		# not a valid number