function collatz(n)
	i = 0
	while n > 1 do
		i = i + 1
		if n % 2 == 0 then
			n = n / 2
		else
			n = 3 * n + 1
		end
	end
	return i
end

function maxlen(n)
	max = 0
	for i = 1, n do
		m = collatz(i)
		if m > max then
			max = m
		end
	end
	return max
end

print(maxlen(1000000))
