# usage: build26.sh out.dll [-DSEASON_TEST]
set -e
clang --target=x86_64-pc-windows-msvc -O2 -c -ffreestanding -fno-stack-protector -mno-stack-arg-probe -fno-builtin -fno-math-errno $2 /home/claude/rules/season_rules_v26.c -o /tmp/claude-0/sr26_tmp.obj
lld-link /nologo /dll /entry:DllMain /nodefaultlib /tmp/claude-0/sr26_tmp.obj /home/claude/rules/k32r.lib /home/claude/rules/u32r.lib /out:$1
