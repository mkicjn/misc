#!/usr/bin/python3
import numpy as np

def levenshtein(s1, s2):
    f = np.zeros((len(s1)+1, len(s2)+1))
    for i in range(1, len(s1)):
        for j in range(1, len(s2)):
            f[i, j] = min(
                    f[i, j-1] + 1,
                    f[i-1, j] + 1,
                    f[i-1, j-1] + (1 if s1[i - 1] != s2[j - 1] else 0))
    return f

def lcs(s1, s2):
    f = np.zeros((len(s1)+1, len(s2)+1))
    for i in range(1, len(s1)):
        for j in range(1, len(s2)):
            if s1[i - 1] == s2[j - 1]:
                f[i, j] = f[i-1, j-1] + 1
            else:
                f[i, j] = max(f[i, j-1], f[i-1, j])
    return f

def custom_lcs(s1, s2):
    f = np.zeros((len(s1)+1, len(s2)+1))
    for i in range(1, len(s1)):
        for j in range(1, len(s2)):
            if s1[i - 1] == s2[j - 1] and i != j:
                f[i, j] = f[i-1, j-1] + 1
            else:
                f[i, j] = max(f[i-1, j-1], f[i, j-1], f[i-1, j]) * 3 // 4
    return f

def show_mat(f, s1, s2):
    s1 += ' '
    s2 += ' '
    print(' '*3, end='')
    for j in range(len(s2)):
        print('{:3s}'.format(s2[j - 1]), end='')
    print('')
    for i in range(len(f)):
        print(s1[i - 1], end='')
        for j in range(len(f[i])):
            print('{:3.0f}'.format(f[i, j]), end='')
        print('')

def reassociate(s1, f):
    return zip(s1, [max(row) for row in f[1:]])

def normal():
    return "\033[m"

def color(r, g, b):
    clamp = lambda x: int(255 * max(0.0, min(1.0, x)))
    return f"\033[48;2;{clamp(r)};{clamp(g)};{clamp(b)}m"

if __name__ == '__main__':
    import sys
    import re
    if len(sys.argv) > 2:
        (s1, s2) = sys.argv[1:]
    else:
        with open(sys.argv[1], 'r') as f:
            s1 = re.findall('\w+|\s+|[^\w\s]+', f.read())
            s2 = s1
    f = custom_lcs(s1, s2)
    heat = list(reassociate(s1, f))
    norm = max(heat, key=lambda x: x[1])[1]
    colored = ''.join(map(lambda x: f'{color(x[1]/norm, 0, 0)}{x[0]}', heat))
    colored = colored.replace("\n", " " + normal() + "\n")
    print(colored)
