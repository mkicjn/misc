#!/usr/bin/python3
import sys
import numpy as np

def levenshtein(s1, s2):
    s1 = ' ' + s1
    s2 = ' ' + s2
    f = np.zeros((len(s1), len(s2)))

    for i in range(1, len(s1)):
        for j in range(1, len(s2)):
            f[i, j] = min(
                    f[i, j-1] + 1,
                    f[i-1, j] + 1,
                    f[i-1, j-1] + (1 if s1[i] != s2[j] else 0))
    return f

def show_mat(f, s1, s2):
    s1 = ' ' + s1
    s2 = ' ' + s2
    print(' '*3, end='')
    for j in range(len(s2)):
        print('{:3s}'.format(s2[j]), end='')
    print('')
    for i in range(len(f)):
        print(s1[i], end='')
        for j in range(len(f[i])):
            print('{:3d}'.format(f[i, j]), end='')
        print('')

def lcs(s1, s2):
    s1 = ' ' + s1
    s2 = ' ' + s2
    f = np.zeros((len(s1), len(s2)))

    for i in range(1, len(s1)):
        for j in range(1, len(s2)):
            if s1[i] == s2[j]:
                f[i, j] = f[i-1, j-1] + 1
            else:
                f[i, j] = max(f[i, j-1], f[i-1, j])
    return f

def custom_lcs(s1, s2):
    s1 = ' ' + s1
    s2 = ' ' + s2
    f = np.zeros((len(s1), len(s2)))

    for i in range(1, len(s1)):
        for j in range(1, len(s2)):
            if s1[i] == s2[j] and i != j:
                f[i, j] = f[i-1, j-1] + 1
            else:
                f[i, j] = max(f[i-1, j-1], f[i, j-1], f[i-1, j], 0) // 2
    return f

def reassociate(s1, f):
    return zip(s1, [max(row) for row in f[1:]])

def normal():
    return "\033[m"

def color(r, g, b):
    clamp = lambda x: int(255 * max(0.0, min(1.0, x)))
    return "\033[48;2;{};{};{}m".format(clamp(r), clamp(g), clamp(b))

if __name__ == '__main__':
    if len(sys.argv) > 2:
        (s1, s2) = sys.argv[1:]
    else:
        with open(sys.argv[1], 'r') as f:
            s1 = f.read()
            s2 = s1
    f = custom_lcs(s1, s2)
    heat = list(reassociate(s1, f))
    norm = max(heat, key=lambda x: x[1])[1]
    colored = ''.join(map(lambda x: f'{color(x[1]/norm, 0, 0)}{x[0]}', heat))
    colored = colored.replace("\n", " " + normal() + "\n")
    print(colored)
