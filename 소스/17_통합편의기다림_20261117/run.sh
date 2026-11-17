#!/bin/bash
# run.sh <name> <dll> <exe> [flags...]  (env passes through)
n=$1; d=$2; x=$3; shift 3
L=root/game/SAN14Mods/SeasonRules/SeasonRules.log; rm -f $L
DUMP=$PWD/d_$n.txt ./peload55 $d $x $PWD/root "$@" > r_$n.txt 2>&1; ec=$?
cp $L log_$n.txt 2>/dev/null
sed -i -E 's/^(game\+0x14bba10:) .*/\1 (our hook address)/' d_$n.txt
echo "=== $n (exit $ec)"; grep -E "self-host start|changed when|changed places|IC CHECK|stand-in|threads:|wait calls" r_$n.txt
grep -E "v26k55|SM52|skipped|SKIPPED|held|time hook:|route search:" log_$n.txt | cut -c1-230
