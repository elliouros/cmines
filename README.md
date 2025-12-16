# cmines

Minesweeper in C.

To install (Requires `make` and probably a C compiler):
```
git clone https://github.com/elliouros/cmines.git && \
cd cmines && \
make
```

May or may not work in Windows; I haven't tried and I don't plan to.

## is it any good?

No. Escapes are printed directly instead of using a library like ncurses.
This is bad for (likely) both optimization and portability.

But, it works on my machine (Linux, Foot terminal). So. There's that :p
