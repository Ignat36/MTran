num = int(input("Enter the number of terms: "))
print("Fibonaci sequence:")
a = 0
b = 1
for i in range(1, num + 1):
	print(" ", a)
	c = a + b
	a = b
	b = c
