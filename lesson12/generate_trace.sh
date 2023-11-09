filename=$1
outfilename=$2
shift 2
make l12_jit
bril2json < $filename | brili-trace $@ > "/dev/null"
bril2json < $filename | ./l12_jit | bril2txt > $outfilename
bril2json < $outfilename | brili -p $@
rm trace_*.json