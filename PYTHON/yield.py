def fibonacci_generator():
    """Generates an infinite sequence of Fibonacci numbers."""
    a, b = 0, 1  # Initialize the first two numbers
    while True:
        yield a  # Yield the current value
        a, b = b, a + b  # Update the values for the next iteration

# Example usage:
for num in fibonacci_generator():
    if num > 1000000:  # Stop after generating numbers greater than 100
        break
    print(num)
