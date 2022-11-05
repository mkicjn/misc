import sys

def collatz(n):
    i = 0
    while n > 1:
        if n % 2 == 0:
            n = n / 2
        else:
            n = n * 3 + 1
        i += 1
    return i

def max_len(n):
    m = 0
    for i in range(n):
        m = max(m, collatz(i))
    return m;

if __name__ == '__main__':
    print(max_len(1000000))
