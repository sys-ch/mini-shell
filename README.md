# mini-shell 0.1

A tiny C shell I'm building to understand how real shells work.

### Features

- Read-eval loop with `$ ` prompt
- Builtins: `exit`, `echo`, `type`
- Executes external commands via `fork` + `execvp`
- Basic PATH lookup for `type`

### Roadmap

- [ ] Command history
- [ ] Quoting (`"..."`, `'...'`)
- [ ] Redirections (`>`, `>>`, `<`)
- [ ] Pipelines (`cmd1 | cmd2`)
- [ ] Autocompletion (tab)
- [ ] `cd` and better prompt with current directory

### How to build

cc -Wall -Wextra -O2 -o mini-shell main.c  
./mini-shell

### Credits

This project is based on the “Build your own Shell” challenge from Codecrafters.
