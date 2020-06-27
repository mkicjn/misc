\ Software single-cell integer division

: UM+ 0 TUCK D+ ;

: DIVMOD ( dividend divisor -- remainder quotient )
	0 SWAP			( convert dividend to double )
	63 FOR			( for each bit: )
		>R			( save divisor )
		DUP + >R DUP UM+ R> +	( left shift double )
		DUP R@ -		( try subtracting )
		DUP 0< IF		( if less than 0: )
			DROP			( ignore it )
		ELSE			( otherwise: )
			NIP			( keep it )
			>R 1 OR R>		( shift in a 1 )
		THEN
		R>			( restore divisor )
	NEXT
	DROP			( drop divisor )
	SWAP			( put remainder underneath )
;
