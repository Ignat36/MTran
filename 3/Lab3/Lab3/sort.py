a = [1, -4, 5, -5, 23]
print(a)
for i in range(len(a)):
	for j in range(i+1, len(a)-1):
		if (a[i] > a[j]):
			tmp = a[i]
			a[i] = a[j]
			a[j] = tmp

print(a)