# Create a list.
elements = []

# Append empty lists in first two indexes.
elements.append([])
elements.append([])

# Add elements to empty lists.
elements[0].append("one")
elements[0].append("two")

elements[1].append("three")
elements[1].append("four")

# Display top-left element.
print(elements[0][0])

# Display entire list.
print(elements)

# Loop over rows.
for row in elements:
    # Loop over columns.
    for column in row:
        print(column),
    print()
