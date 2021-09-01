class Dog:
	species = "poopy"
	def __init__(self, name, age):
		self.name = name
		self.age = age
	def __str__(self):
		return "{} is {} years old".format(self.name, self.age)
	def speak(self, words):
		print("{} says {}".format(self.name, words))

class Pitbull(Dog):
	species = "pitbull"
	def speak(self, words="Arf Arf"):
		print("Proud {} {} says {}".format(self.species, self.name, words))
a = Dog("mutt", 7)
print(a)
a.speak("bow wow")

b = Pitbull("sparky", 3)
b.speak()
