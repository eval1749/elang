#
# Pre-commit check
#

import pipes
import sys

def main():
    git_pipe = pipes.Template()
    git_pipe.prepend('git diff --cached --name-status', '.-')
    diff_output = git_pipe.open('files', 'r')
    lines = diff_output.readlines()
    exit_code = 0
    for line in lines:
        line = line.rstrip();
        if len(line) == 0:
            continue
        words = line.split()
        if words[0] == 'D':
            continue
        cpplint_pipe = pipes.Template()
        command_line = 'cpplint %(name)s 2>&1' % {'name': words[1]}
        cpplint_pipe.prepend(command_line, '.-');
        outputs = cpplint_pipe.open('files', 'r').readlines()
        if outputs[len(outputs) - 1] == 'Total errors found: 0\n':
            continue
        exit_code = 1
        for output in outputs:
            output = output.rstrip()
            print output
    diff_output.close()
    sys.exit(exit_code)

if __name__ == '__main__':
    main()
