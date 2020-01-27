Element employs Single Static Assignment for identifiers to keep the language implementation simple and make writing optimizers easy.

Allowing identifiers to be mutated significantly increases the complexity of programming in the language and requires safety features to handle writing optimizers.