---
title: Testcases
theme: simple
size: "16:9"
---

# :tophat: Testcases
A handy markdown presentation tool.

---
# Image
![Hiragishi Takadai Park](wednesday.jpg)

---
# Math
This is a equation[^foo].
$$
A = \sum (y_i - f(x_i))^2
$$

[^foo]: This is a footnote.

---
# More $\LaTeX$
**Diffie-Hellman Key Agreement:**

$$
\begin{array}{ccc}
A & & B \\
x \xleftarrow{R} \mathbb{Z}_q \\
& \xrightarrow{g^x} \\
& & y \xleftarrow{R} \mathbb{Z}_q \\
& & Z_{AB} = (g^x)^y = g^{xy} \\
& \xleftarrow{g^y} \\
Z_{AB} = (g^y)^x = g^{xy} \\
\end{array}
$$

---
# Code
```c
#include <stdio.h>

int main(void) {
    printf("Hello World!\n");
    return 0;
}
```
