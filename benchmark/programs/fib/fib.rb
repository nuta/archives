def fib(n)
    if n < 2 then
        return n
    end

    fib(n - 1) + fib(n - 2)
end

puts fib(32)
