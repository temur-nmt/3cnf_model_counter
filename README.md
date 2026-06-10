# 3-CNF Solver

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