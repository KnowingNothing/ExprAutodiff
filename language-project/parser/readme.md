# parser for auto

## how to use

in /language-project/parser

```
make
./auto test.f out.cc
```

the input auto-language file is test.f
output IR is out.cc

'#' in test.f means Annotated code

## TODO
some semantics about generating IR is not clear now ,need to be discussed

As a result some grammer in auto-language has not been supported such as RHS::= SRef

