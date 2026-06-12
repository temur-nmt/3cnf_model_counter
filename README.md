# 3-CNF Model Counter

## Structure

```
root/
├─ Probleminstanzen_A/
│  ├─ uf20-01.cnf
│  ├─ ...
│  └─ uf20-020.cnf
├─ src
│  ├─ main.c
│  └─ ...
├─ test_instance_generator
│  ├─ test_generator.py
│  └─ requirements.txt
├─ test_instances
│  ├─ dummy_3vars_3.cnf
│  ├─ dummy_15vars_1.cnf
│  ├─ ...
│  └─ dummy_15vars_20.cnf
└─ README.md
```

## Generate Test Data

### Setup
* (`pyenv local 3.13.7`) (relevant for me)
* `python -m venv .venv`
* `source .venv/bin/activate`
* `pip install -r requirements.txt`

### Generating Test Data with Solutions
* `cd test_instance_generator`
* (`create pyenv`)
* `source .venv/bin/activate`
* `python test_generator.py`
* (`deactivate`)

How it works:

* The Generator creates per default 20 test cases with 15 variables. The variable/test amount can be simply changed in the function call.
* Every test case has an additional two lines telling the amount of possible solutions:
```
solution
52
```
* Don't you dare ask me questions about the generator, it's as much black magic to me as to you. For further information resort to [the official documentation](https://pysathq.github.io/).
* The python library is a wrapper around a reliable solver (Glucose3), so just trust me bro.
* The point of the generator is validation, because I really don't know how they expect us to build a Model Counter without test data.
* Regularely generating test cases can be useful, for now I will keep the generated ones in the repository.

## Useful

### Benchmarking

**NOTE:** Unless you have OpenMP locally installed there is no point in finicking around with the files. Upload the contents of `src/benchmark` as described and test on the OMICS Cluster.

1. For ease of use create a new folder in the OMICS Cluster
2. Upload `Makefile`, `benchmarker.c`, `sequential.c`, `sequential.h`, `parallel.c`, `parallel.h`, `parser_temur.c` and `parser_temur.h` 
3. Inside the folder create a `data` folder and upload all instances
4. run `make bench_cnf`
5. run `./bench_cnf data`

#### Recommended structure
```
home/<name>/project
├─ data/
│  ├─ uf20-01.cnf
│  ├─ ...
│  └─ uf20-020.cnf
├─ Makefile
├─ sequential.c
├─ sequential.h
├─ parallel.c
├─ parallel.h
└─ benchmarker.c
```

### Stop Visual Studio Code from bitching because of folder structure

1. Deactivate clangd in workspace
2. Create `.vscode` in root
3. Create `c_cpp_properties.json`
4. Copy following:
```
{
    "configurations": [
        {
            "name": "Linux/Mac",
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/src"
            ],
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "c11",
            "intelliSenseMode": "linux-gcc-x64"
        }
    ],
    "version": 4
}
```