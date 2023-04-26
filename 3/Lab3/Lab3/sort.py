b = [4, -2, 5, 23, -4]
a = [20, -4, 5, -5, 23]
print(1+2*2+1, input("enter number"))
print(b)
for i in b:
	append(a, i)
print(a)
for i in range(len(a)):
	for j in range(i+1, len(a) - 1):
		if (a[i] > a[j]):
			tmp = a[i]
			a[i] = a[j]
			a[j] = tmp

print(a)

b = [a[0]]
for i in range(1, len(a)-1):
	f = 0
	for j in range(len(b)):
		if (a[i] == b[j]):
			f = 1
	if (f == 0):
		append(b, a[i])

print(b)

