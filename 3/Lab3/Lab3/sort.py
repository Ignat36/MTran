a = [[1, 2, 3], -4, 5, -5, 23]
print(a[0, 2])
for i in range(5):
	print(a[i])
a = [20, -4, 5, -5, 23]
print(a)
for i in range(len(a)):
	for j in range(i+1, len(a)-1):
		if (a[i] > a[j]):
			tmp = a[i]
			a[i] = a[j]
			a[j] = tmp

print(a)