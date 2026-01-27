import sys
import os
from graphviz import Digraph

def generate_frame(cycle, pc, instr_hex, disasm):
    # directed graph - L-to-R layout
    dot = Digraph(name=f'Cycle_{cycle}', comment='Instruction Flow')
    dot.attr(rankdir='LR', size='8,5', fontsize='12')
    
    # Stages
    dot.node('MEM', 'Instruction Memory\\n(memory.cpp)', shape='box', style='filled', color='lightblue')
    dot.node('DEC', f'Decoder Stage\\n(decoder.cpp)\\nWord: {instr_hex}', shape='diamond', style='filled', color='lightyellow')
    dot.node('EXE', f'Execution Unit\\n(cpu.cpp)\\n{disasm}', shape='ellipse', style='filled', color='lightgreen')
    dot.node('REG', 'Register File\\n(registers)', shape='box', style='filled', color='orange')
    
    # "Active" Path for this cycle
    dot.edge('MEM', 'DEC', label=f'Fetch PC: {pc}')
    dot.edge('DEC', 'EXE', label='Control Signals')
    dot.edge('EXE', 'REG', label='Writeback Result')
    
    output_path = f'docs/frames/frame_{cycle}'
    dot.render(output_path, format='png', cleanup=True)
    print(f"Generated {output_path}.png")

if __name__ == "__main__":
    # Create output directory if it doesn't exist
    os.makedirs('docs/frames', exist_ok=True)
    
    for line in sys.stdin:
        if "[FLOW]" in line:
            parts = line.split('|')
            cycle = parts[0].split()[-1]
            pc = parts[1].strip()
            hex_val = parts[2].strip()
            disasm = parts[3].strip()
            generate_frame(cycle, pc, hex_val, disasm)