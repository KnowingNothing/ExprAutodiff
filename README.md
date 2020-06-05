## Requirements
- sudo apt-get install bison flex

## Build
```sj
mkdir build
cd build
cmake ..
make -j 8
```

## Test
```sh
cd build/test
# run the gemm test
./demo-gemm
# run the conv2d test
./demo-conv2d
# run the pair gemm test
./demo-pair-gemm
# run the type failing test
./demo-type-failing
```
> note that you should run all the binaries in `build/test`, because the test files read inputs from `test/demo-inputs/*`, if you execute the binaries from other path, they can't find the input files.