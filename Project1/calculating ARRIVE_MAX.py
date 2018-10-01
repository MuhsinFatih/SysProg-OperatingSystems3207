def P(n):
    j = 0
    p = 0.0
    while(j < n):
        j += 1
        p += pow(0.8,j-1)*(0.2)

    return p

print(P(3))