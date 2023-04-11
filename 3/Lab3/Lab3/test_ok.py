num = int(input("Enter a number: ")) + int(input("Enter a number: "))
factorial = 2 + (2 * 2) + 2
arr = [1, 2, 3, 4, 5]
print(arr[0])
if (num < 0):
	print("Sorry, factorial does not exist for negative numbers")
elif num == 0:
	print(("The factorial of 0 is 1"))
else:
	for i in range(1,num):
		factorial = factorial*i
	print("The factorial of", num, "is", factorial)