ar2 = [1, -4, 5, -5, 23]
print(ar2)

pop_back(ar2)
i = 0
while i < len(ar2):
	print(ar2[i])
	i = i + 1
a = [1, 2, 3, 4]
a[0] = 25
if 2 in a:
	print(1)
a = 1
print(a)
a_b = 3 + 4
print(a_b)
num = int(input("Enter a number: ")) + int(input("Enter a number: "))
factorial = 2 + (2 * 2) + 2
a = 1
b = 2
c = 3 
print(a_b)
print(a)
print(b)
print(c)
print(num)
print(factorial)
arr = [a + 2, 1 - (45 * int(float(b + (c-int(3.4 + 2j))))), a, 4 * 9, c - 5]
if c in arr:
	print(1)
if b in [1, 2, 4, 7, 8]:
	print("Hello")
if (num < 0):
	print("Sorry, factorial does not exist for negative numbers")
elif num == 0:
	print(("The factorial of 0 is 1"))
else:
	factorial = 1
	for i in range(1,num):
		factorial = factorial * i
	print("The factorial of ", num, " is ", factorial )

