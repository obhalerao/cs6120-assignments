count=0
for file in benchmarks/*/*.bril ; do
  output=$(bril2json < $file | ./l6_ssa | python "lesson6/is_ssa.py")
  if [ "$output" = "no" ] 
  then
    echo "$file is not in SSA form!"
    count=$count+1
  fi
done
echo "$count file(s) are not in SSA form."