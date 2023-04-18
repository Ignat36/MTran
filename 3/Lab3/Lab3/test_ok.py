a_b = 3 + 4
num = int(input("Enter a number: ")) + int(input("Enter a number: "))
factorial = 2 + (2 * 2) + 2
a = 1
b = 2
c = 3 
arr = [a, 1 - (45 ** int(float(b + (c-int(3.4 + 2j))))), a, 4 * 9, c - 5]
print(a)
print(b)
print(c)
arr[1] = 4
if c in arr:
	print(1)
if b in [1, 2, 4, 7, 8]:
	print("Hello")
if (num < 0):
	print("Sorry, factorial does not exist for negative numbers")
elif num == 0:
	print(("The factorial of 0 is 1"))
else:
	for i in range(1,num):
		factorial = factorial*i
	print("The factorial of", num, "is", factorial)