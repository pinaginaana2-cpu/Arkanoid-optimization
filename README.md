## Машинно-зависимая оптимизация (флаги сборки)

Код игры не меняется. Оптимизация достигается за счёт указания компилятору
использовать все возможности конкретного процессора.

**Для Apple Silicon (M1/M2/M3):**
gcc -O2 -mcpu=native -ffp-contract=off main.c -framework GLUT -framework OpenGL -Wno-deprecated -lm -o arkanoid

**Для процессоров Intel:**
gcc -O2 -march=native -ffp-contract=off main.c -framework GLUT -framework OpenGL -Wno-deprecated -lm -o arkanoid

