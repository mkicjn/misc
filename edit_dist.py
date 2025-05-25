#!/usr/bin/python3
import sys

def levenshtein(s1, s2):
    s1 = ' ' + s1
    s2 = ' ' + s2
    f = [[[0] for j in range(len(s2))] for i in range(len(s1))]

    f[0][0] = 0
    for i in range(1, len(s1)):
        f[i][0] = i
    for j in range(1, len(s2)):
        f[0][j] = j

    for i in range(1, len(s1)):
        for j in range(1, len(s2)):
            f[i][j] = min(
                    f[i][j-1] + 1,
                    f[i-1][j] + 1,
                    f[i-1][j-1] + (1 if s1[i] != s2[j] else 0))
    return f

def show_mat(f, s1, s2):
    s1 = ' ' + s1
    s2 = ' ' + s2
    print(' '*4, end='')
    for j in range(len(s2)):
        print('{:4s}'.format(s2[j]), end='')
    print('')
    for i in range(len(f)):
        print(s1[i], end='')
        for j in range(len(f[i])):
            print('{:4d}'.format(f[i][j]), end='')
        print('')

if __name__ == '__main__':
    (s1, s2) = sys.argv[1:]
    f = levenshtein(s1, s2)
    show_mat(f, s1, s2)
