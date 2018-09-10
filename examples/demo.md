---
title: Intoroduction to Supershow
theme: simple
size: "16:9"
fonts:
  title: M PLUS Rounded 1c
  body: Noto Sans JP
  code: Source Code Pro
---

# :tophat: Introduction to Supershow
A handy markdown presentation tool.

---
# Features

- Write a clean presentation slides in Markdown
- Themes

TODO:

---
# Getting Started

```
$ npm install -g supershow
$ supershow file.md
```

**or use [a Visual Studio Code plugin](http://localhost)**

---
# Image
![Hiragishi Takadai Park](wednesday.jpg)

---
# 美味しいカレーの作り方
1. 人参，じゃがいも，玉ねぎを一口大に切ります。
2. 切った具材と肉か何かを適当に炒めます。
3. 好みのスパイス (適量) を入れます。
4. 鶏がらスープ (適量) を加えていい感じに煮込みます。
5. 最後にパクチーを入れて完成です。
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
