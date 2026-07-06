import sys
import os
from pysat.formula import CNF
from pysat.solvers import Solver

def validate_instances(data_dir):
    print("VALIDATING groupedCNF INSTANCES WITH PySAT")
    print(f"{'File':<25}  {'PySAT Models':>15}")
    print("-" * 45)

    for i in range(1, 21):
        file_path = os.path.join(data_dir, f"groupedCNF-{i}.cnf")
        
        if not os.path.exists(file_path):
            print(f"ERROR: {file_path} not found.")
            continue
            
        valid_dimacs_lines = []
        
        with open(file_path, 'r') as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                    
                if line.startswith('c') or line.startswith('p'):
                    valid_dimacs_lines.append(line)
                else:
                    tokens = line.split()
                    if tokens and tokens[-1] != '0':
                        line = line + " 0"
                    valid_dimacs_lines.append(line)
        
        valid_cnf_string = "\n".join(valid_dimacs_lines) + "\n"
        
        try:
            cnf = CNF(from_string=valid_cnf_string)
            with Solver(name='m22', bootstrap_with=cnf) as solver:
                pysat_models = sum(1 for _ in solver.enum_models())
                    
            print(f"{os.path.basename(file_path):<25}  {pysat_models:>15}")
            
        except Exception as e:
            print(f"{os.path.basename(file_path):<25}  PARSE ERROR: {e}")

    print("-" * 45)

if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    folder = os.path.abspath(os.path.join(script_dir, "..", "Probleminstanzen_B"))
    
    validate_instances(folder)