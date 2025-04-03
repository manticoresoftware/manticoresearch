i=1
while i<600000:
    if i & 1:
        a= (1 << 64) - i
        print('%d,a' % a)
    else:
        print('%d,b' % i)
    i=i+1